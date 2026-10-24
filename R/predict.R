#' Predict From a Survival Learning Model
#'
#' @param object A fitted object from [surv_fit()] or [surv_tune()].
#' @param newdata A data frame of new predictors. Outcomes are not required.
#' @param type `"risk"` (default), `"survival"`, or `"time"`.
#' @param times Non-negative finite time horizons, required for survival curves.
#' @param ... Reserved for S3 compatibility; extra arguments are rejected.
#' @details
#' Risk scores always increase with risk. AFT scores are negative log-time
#' predictors; the original SVM scores are negative predicted responses.
#' svm_rank returns its learned risk margin; svm_hybrid and svm_ipcw return
#' negative normalized regression responses. These are risk-only capabilities.
#' RSF mortality
#' scores are not probabilities. Absolute scales must not be compared
#' between methods.
#' XGBoost Cox returns its log-risk margin, whereas XGBoost AFT returns the
#' negative log-time margin. MTLR risk is the negative restricted mean survival
#' up to the largest training follow-up, computed by trapezoidal integration
#' on 101 equally spaced horizons plus the learned knots. This convention
#' does not assume proportional hazards or extrapolate a full mean/median.
#'
#' `type = "time"` returns the conditional median for parametric AFT and
#' XGBoost AFT, including the extreme-value error median correction.
#' `type = "survival"` returns native model survival probabilities for AFT,
#' Cox, penalized Cox, RSF, and MTLR. XGBoost Cox uses a training-only Breslow
#' baseline; XGBoost AFT uses its fitted error distribution and scale.
#' Other methods return an informative error;
#' no distribution or calibration is silently added. Nonparametric and
#' semiparametric curves return `NA` beyond the largest training follow-up.
#' Parametric AFT and XGBoost AFT curves can extrapolate and require model
#' adequacy checks.
#'
#' Additional native curves are available for Kaplan-Meier, spline Cox,
#' Royston-Parmar, componentwise CoxBoost, ranger, aorsf, neural Cox and PLANN.
#' Royston-Parmar is parametric and can extrapolate; other new engines return
#' NA beyond training follow-up. Kaplan-Meier returns identical curves for
#' all rows and constant zero risk. PLANN survival is a step function on its
#' training interval endpoints; its risk is negative restricted mean on that
#' same grid. Neural Cox returns log hazard with a training-only baseline.
#' @return Risk and time predictions are numeric vectors. Survival predictions
#'   are matrices with one row per observation and one column per requested
#'   horizon, in the requested order.
#'   Matrix columns are named `t=<horizon>`. Predictions use the outcome time
#'   unit supplied for training. Unsupported targets and non-finite log-risks
#'   cause errors. Cox survival at zero cumulative hazard is exactly one,
#'   including for extreme finite log-risk. Plotting and file output are not
#'   performed. Models do not imply a cure probability.
#' @export
#' @examples
#' fit <- surv_fit(survival::Surv(time, status) ~ age + karno,
#'   survival::veteran, method = "aft")
#' surv_predict(fit, survival::veteran[1:3, ], type = "survival", times = c(30, 90))
surv_predict <- function(object, newdata, type = "risk", times = NULL) {
  if (inherits(object, "survlearning_tune")) object <- object$best_model
  if (!inherits(object, "survlearning_fit")) {
    stop("'object' must come from surv_fit() or surv_tune().", call. = FALSE)
  }
  .require_engine(object$method)
  type <- match.arg(type, c("risk", "survival", "time"))
  if (!type %in% .method_specs()[[object$method]]$predictions) {
    stop("Method '", object$method, "' does not provide native '", type,
      "' predictions. Use type = 'risk'. No calibration is added.", call. = FALSE)
  }
  if (type == "survival" && (!is.numeric(times) || !length(times) ||
      any(!is.finite(times)) || any(times < 0))) {
    stop("Provide finite, non-negative 'times' for survival prediction.", call. = FALSE)
  }
  if (type != "survival" && !is.null(times)) {
    stop("'times' is only used with type = 'survival'.", call. = FALSE)
  }
  x <- .new_design(object$design, newdata)
  if (object$method %in% c("aft_lasso", "aft_ridge", "aft_elasticnet", "svm", "mtlr", .svm_learning_methods())) {
    x <- .scaled_design(x, object$design)
  }
  if (type == "survival") {
    result <- if (object$method %in% names(.reference_specs())) {
      .predict_reference(object, x, type, times)
    } else .survival_prediction(object, x, times)
    if (!object$method %in% c("aft", "xgboost_aft", "flexsurv") && any(times > object$max_time)) {
      result[, times > object$max_time] <- NA_real_
      warning("Horizons beyond training follow-up return NA; no tail extrapolation was added.", call. = FALSE)
    }
    colnames(result) <- paste0("t=", format(times, trim = TRUE))
    return(result)
  }
  if (object$method %in% names(.reference_specs())) {
    return(as.numeric(.predict_reference(object, x, type, times)))
  }
  if (type == "time") {
    if (object$method == "xgboost_aft") return(.xgboost_median(object, x))
    return(as.numeric(stats::predict(object$engine, newdata = .engine_data(x),
      type = "quantile", p = 0.5)))
  }
  if (object$method %in% .svm_learning_methods()) return(.svm_learning_predict(object$engine, x))
  result <- switch(object$method,
    aft = -stats::predict(object$engine, newdata = .engine_data(x), type = "lp"),
    aft_lasso = , aft_ridge = , aft_elasticnet = {
      beta <- .gehan_coefficients(object$engine, object$params$lambda)
      -drop(x %*% beta)
    },
    cox = stats::predict(object$engine, newdata = .engine_data(x), type = "lp", reference = "zero"),
    cox_lasso = , cox_ridge = , cox_elasticnet = stats::predict(object$engine, newx = .glmnet_design(x),
      s = object$params$lambda, type = "link"),
    tree = stats::predict(object$engine, newdata = .engine_data(x)),
    rsf = .rsf_prediction(object, x)$predicted,
    boosting = stats::predict(object$engine, newdata = .engine_data(x),
      n.trees = object$params$trees, type = "link"),
    xgboost_cox = .xgboost_margin(object$engine, x),
    xgboost_aft = -.xgboost_margin(object$engine, x),
    mtlr = .mtlr_risk(object, x),
    svm = -stats::predict(object$engine, newdata = .engine_data(x))$predicted
  )
  as.numeric(result)
}

#' @rdname surv_predict
#' @export
predict.survlearning_fit <- function(object, newdata, type = "risk", times = NULL, ...) {
  if (length(list(...))) stop("Unused prediction argument(s).", call. = FALSE)
  surv_predict(object, newdata, type, times)
}

#' @rdname surv_predict
#' @export
predict.survlearning_tune <- function(object, newdata, type = "risk", times = NULL, ...) {
  if (length(list(...))) stop("Unused prediction argument(s).", call. = FALSE)
  surv_predict(object, newdata, type, times)
}

.rsf_prediction <- function(object, x) {
  old <- options(rf.cores = object$params$threads)
  on.exit(options(old), add = TRUE)
  .with_seed(object$seed, .predict_rsf(object$engine, newdata = .engine_data(x)))
}

.survival_prediction <- function(object, x, times) {
  n <- nrow(x)
  if (object$method == "aft") {
    location <- as.numeric(stats::predict(object$engine, newdata = .engine_data(x), type = "lp"))
    out <- vapply(times, function(t) {
      z <- (log(t) - location) / object$engine$scale
      switch(object$params$distribution,
        weibull = exp(-exp(z)),
        lognormal = stats::pnorm(z, lower.tail = FALSE),
        loglogistic = stats::plogis(z, lower.tail = FALSE))
    }, numeric(n))
    return(matrix(out, nrow = n, ncol = length(times)))
  }
  if (object$method == "cox") {
    baseline <- survival::basehaz(object$engine, centered = TRUE)
    lp <- stats::predict(object$engine, newdata = .engine_data(x),
      type = "lp", reference = "sample")
    cumulative <- c(0, baseline$hazard)[findInterval(times, baseline$time) + 1L]
    return(.ph_survival(lp, cumulative))
  }
  if (object$method %in% c("cox_lasso", "cox_ridge", "cox_elasticnet")) {
    baseline <- object$baseline
    lp <- as.numeric(stats::predict(object$engine, newx = .glmnet_design(x),
      s = object$params$lambda, type = "link"))
    cumulative <- c(0, as.numeric(baseline$cumhaz))[
      findInterval(times, baseline$time) + 1L]
    return(.ph_survival(lp, cumulative))
  }
  if (object$method %in% c("xgboost_cox", "xgboost_aft")) {
    return(.xgboost_survival(object, x, times))
  }
  if (object$method == "mtlr") return(.mtlr_survival(object, x, times))
  pred <- .rsf_prediction(object, x)
  index <- findInterval(times, pred$time.interest)
  # Include S(0)=1 explicitly, including horizons before the first event.
  cbind(1, pred$survival)[, index + 1L, drop = FALSE]
}

.ph_survival <- function(lp, cumulative) {
  if (any(!is.finite(lp)) || anyNA(cumulative) || any(cumulative < 0)) {
    stop("Non-finite log-risk or invalid cumulative baseline hazard.", call. = FALSE)
  }
  out <- matrix(1, nrow = length(lp), ncol = length(cumulative))
  positive <- which(cumulative > 0)
  if (length(positive)) {
    out[, positive] <- exp(-exp(outer(as.numeric(lp), log(cumulative[positive]), "+")))
  }
  out
}
