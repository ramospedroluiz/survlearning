# Optional engines are dispatched by membership in names(.reference_specs()).
.reference_specs <- function() {
  list(
    km = list(description = "Kaplan-Meier reference (no covariates)",
      engine = "survival", predictions = c("risk", "survival"), defaults = list()),
    cox_spline = list(description = "Cox PH with natural splines for continuous design columns",
      engine = "survival", predictions = c("risk", "survival"),
      defaults = list(df = 3L)),
    flexsurv = list(description = "Royston-Parmar flexible parametric survival model",
      engine = "flexsurv", predictions = c("risk", "survival"),
      defaults = list(knots = 1L, scale = "hazard")),
    coxboost = list(description = "Componentwise likelihood-based Cox boosting",
      engine = "CoxBoost", predictions = c("risk", "survival"),
      defaults = list(steps = 100L, penalty = 100)),
    ranger = list(description = "Ranger survival forest (log-rank splits)",
      engine = "ranger", predictions = c("risk", "survival"),
      defaults = list(trees = 200L, mtry = NULL, min_node = 15L, threads = 1L)),
    aorsf = list(description = "Accelerated oblique random survival forest",
      engine = "aorsf", predictions = c("risk", "survival"),
      defaults = list(trees = 200L, mtry = NULL, min_node = 5L,
        splits = 5L, threads = 1L)),
    neural_cox = list(description = "Native R neural Cox partial likelihood (Efron/Breslow ties)",
      engine = "stats", predictions = c("risk", "survival"),
      defaults = list(hidden = 16L, epochs = 100L, learning_rate = 0.001,
        decay = 0.0001, threads = 1L, ties = "efron",
        max_weights = 100000L)),
    neural_discrete = list(description = "PLANN neural Bernoulli likelihood (completed-interval censor coarsening)",
      engine = "nnet", predictions = c("risk", "survival"),
      defaults = list(hidden = 8L, intervals = 10L, decay = 0.01,
        max_iter = 1000L, max_weights = 10000L))
  )
}

.fit_reference <- function(method, prepared, params, seed) {
  if (!method %in% names(.reference_specs())) {
    stop("Unknown reference method: ", method, ".", call. = FALSE)
  }
  .reference_require(method)
  x <- prepared$x
  y <- prepared$y
  if (method != "km" && !ncol(x)) {
    stop("Method '", method, "' requires at least one varying predictor.", call. = FALSE)
  }
  if (method %in% c("neural_cox", "neural_discrete")) {
    return(.fit_neural(method, prepared, params, seed))
  }
  data <- .engine_data(x, y)
  formula <- .reference_formula(colnames(x))
  .with_seed(seed, switch(method,
    km = survival::survfit(.reference_formula(character()), data = data),
    cox_spline = {
      .integer_scalar(params$df, "df", 1L)
      continuous <- vapply(seq_len(ncol(x)), function(j) length(unique(x[, j])) > 2L,
        logical(1))
      # Factor contrasts (including ordered factors) must stay linear.
      if (!is.null(prepared$design$columns) && length(prepared$design$xlevels)) {
        categorical <- .reference_factor_columns(prepared$design)
        continuous[prepared$design$columns %in% categorical] <- FALSE
      }
      terms <- colnames(x)
      terms[continuous] <- paste0("splines::ns(", terms[continuous], ", df = ", params$df, ")")
      if (any(vapply(which(continuous), function(j) length(unique(x[, j])) <= params$df,
          logical(1)))) {
        stop("Spline columns require more distinct training values than 'df'.", call. = FALSE)
      }
      fit <- survival::coxph(.reference_formula(terms), data = data,
        ties = "efron", x = TRUE, y = TRUE, model = TRUE)
      fit$.sl_baseline <- survival::basehaz(fit, centered = TRUE)
      fit
    },
    flexsurv = {
      .integer_scalar(params$knots, "knots", 0L)
      if (!is.character(params$scale) || length(params$scale) != 1L ||
          is.na(params$scale) || !params$scale %in% c("hazard", "odds", "normal")) {
        stop("'scale' must be hazard, odds, or normal for flexsurv.", call. = FALSE)
      }
      if (length(unique(y[y[, 2L] == 1, 1L])) < params$knots + 2L) {
        stop("Royston-Parmar requires at least knots + 2 distinct event times.", call. = FALSE)
      }
      flexsurv::flexsurvspline(formula, data = data, k = params$knots,
        scale = params$scale)
    },
    coxboost = {
      .integer_scalar(params$steps, "steps", 1L)
      .positive_scalar(params$penalty, "penalty")
      CoxBoost::CoxBoost(time = y[, 1L], status = y[, 2L], x = x,
        stepno = params$steps, penalty = params$penalty, standardize = TRUE,
        trace = FALSE)
    },
    ranger = {
      params <- .reference_forest_params(params, ncol(x))
      ranger::ranger(formula, data = data, num.trees = params$trees,
        mtry = params$mtry, min.node.size = params$min_node, splitrule = "logrank",
        num.threads = params$threads, seed = .reference_forest_seed(seed),
        write.forest = TRUE)
    },
    aorsf = {
      params <- .reference_forest_params(params, ncol(x))
      .integer_scalar(params$splits, "splits", 1L)
      aorsf::orsf(data = data, formula = formula, n_tree = params$trees,
        mtry = params$mtry, leaf_min_obs = params$min_node, n_split = params$splits,
        n_thread = params$threads, importance = "none", na_action = "fail",
        tree_seeds = sample.int(.Machine$integer.max, params$trees, replace = TRUE))
    }
  ))
}

.predict_reference <- function(object, x, type, times = NULL) {
  method <- object$method
  .reference_require(method)
  if (method %in% c("neural_cox", "neural_discrete")) {
    return(.predict_neural(object, x, type, times))
  }
  fit <- object$engine
  n <- nrow(x)
  new <- .engine_data(x)
  if (type == "risk") {
    return(as.numeric(switch(method,
      km = rep(0, n),
      cox_spline = stats::predict(fit, newdata = new, type = "lp", reference = "zero"),
      flexsurv = {
        beta <- stats::coef(fit)[colnames(x)]
        drop(x %*% beta)
      },
      coxboost = stats::predict(fit, newdata = x, at.step = object$params$steps, type = "lp"),
      ranger = rowSums(.reference_ranger_prediction(object, new)$chf),
      aorsf = stats::predict(fit, new_data = new, pred_type = "mort",
        n_thread = object$params$threads, na_action = "fail")
    )))
  }
  if (type != "survival") stop("Reference engine does not provide '", type, "'.", call. = FALSE)
  .reference_times(times)
  # Never send out-of-support horizons to engines that extrapolate by default.
  supported <- if (method == "flexsurv") rep(TRUE, length(times)) else times <= object$max_time
  out <- matrix(NA_real_, n, length(times))
  requested <- sort(unique(times[supported]))
  if (!length(requested)) return(out)
  native <- switch(method,
    km = matrix(rep(.reference_step(fit$time, fit$surv, requested), each = n), nrow = n),
    cox_spline = {
      lp <- stats::predict(fit, newdata = new, type = "lp", reference = "sample")
      h <- .reference_step(fit$.sl_baseline$time, fit$.sl_baseline$hazard, requested, initial = 0)
      .ph_survival(lp, h)
    },
    flexsurv = {
      pred <- summary(fit, newdata = new, type = "survival", t = requested,
        ci = FALSE, se = FALSE, tidy = FALSE)
      t(vapply(pred, function(row) row$est[match(requested, row$time)], numeric(length(requested))))
    },
    coxboost = {
      positive <- requested > 0
      curves <- matrix(1, n, length(requested))
      if (any(positive)) curves[, positive] <- matrix(stats::predict(fit,
        newdata = x, at.step = object$params$steps, type = "risk",
        times = requested[positive]), nrow = n)
      curves
    },
    ranger = {
      pred <- .reference_ranger_prediction(object, new)
      cbind(1, pred$survival)[, findInterval(requested, pred$unique.death.times) + 1L, drop = FALSE]
    },
    aorsf = {
      positive <- requested > 0
      curves <- matrix(1, n, length(requested))
      if (any(positive)) curves[, positive] <- matrix(stats::predict(fit,
        new_data = new, pred_type = "surv", pred_horizon = requested[positive],
        n_thread = object$params$threads, boundary_checks = FALSE,
        na_action = "fail"), nrow = n)
      curves
    }
  )
  out[, supported] <- native[, match(times[supported], requested), drop = FALSE]
  out[, times == 0] <- 1
  out
}

.reference_require <- function(method) {
  spec <- .reference_specs()[[method]]
  if (is.null(spec)) stop("Unknown reference method: ", method, ".", call. = FALSE)
  if (!requireNamespace(spec$engine, quietly = TRUE)) {
    stop("Method '", method, "' needs package '", spec$engine, "'.", call. = FALSE)
  }
  minimum <- NULL
  if (!is.null(minimum) && utils::packageVersion(spec$engine) < minimum) {
    stop("Method '", method, "' requires ", spec$engine, " >= ", minimum, ".", call. = FALSE)
  }
}

.reference_parameters <- function(method, defaults, p) {
  if (!method %in% names(.reference_specs())) return(defaults)
  for (key in intersect(names(defaults), c("df", "steps", "hidden", "epochs",
      "max_iter", "max_weights", "trees", "min_node", "threads", "splits"))) {
    .integer_scalar(defaults[[key]], key, 1L)
  }
  if ("intervals" %in% names(defaults)) .integer_scalar(defaults$intervals, "intervals", 2L)
  if ("knots" %in% names(defaults)) .integer_scalar(defaults$knots, "knots", 0L)
  if ("decay" %in% names(defaults)) .positive_scalar(defaults$decay, "decay", zero = TRUE)
  if ("penalty" %in% names(defaults)) .positive_scalar(defaults$penalty, "penalty")
  if ("learning_rate" %in% names(defaults)) .positive_scalar(defaults$learning_rate, "learning_rate")
  if (method == "neural_cox") {
    if (!is.character(defaults$ties) || length(defaults$ties) != 1L ||
        is.na(defaults$ties) || !defaults$ties %in% c("efron", "breslow")) {
      stop("'ties' must be efron or breslow.", call. = FALSE)
    }
  }
  if (method %in% c("ranger", "aorsf")) defaults <- .reference_forest_params(defaults, p)
  if (method == "flexsurv" && (!is.character(defaults$scale) || length(defaults$scale) != 1L ||
      is.na(defaults$scale) || !defaults$scale %in% c("hazard", "odds", "normal"))) {
    stop("'scale' must be hazard, odds, or normal for flexsurv.", call. = FALSE)
  }
  defaults
}

.reference_formula <- function(terms) {
  stats::reformulate(if (length(terms)) terms else "1",
    response = "survival::Surv(.sl_time, .sl_event)", env = environment(.reference_specs))
}

.reference_factor_columns <- function(design) {
  categorical <- unlist(lapply(names(design$xlevels), function(name) {
    levels <- design$xlevels[[name]]
    frame <- data.frame(.sl_factor = factor(levels, levels = levels))
    mm <- stats::model.matrix(~ .sl_factor, frame,
      contrasts.arg = list(.sl_factor = design$contrasts[[name]]))
    paste0(name, substring(colnames(mm)[-1L], nchar(".sl_factor") + 1L))
  }), use.names = FALSE)
  design$columns[vapply(strsplit(design$columns, ":", fixed = TRUE),
    function(parts) all(parts %in% categorical), logical(1))]
}

.reference_forest_params <- function(params, p) {
  for (key in c("trees", "min_node", "threads")) .integer_scalar(params[[key]], key, 1L)
  if (is.null(params$mtry)) params$mtry <- max(1L, floor(sqrt(p)))
  .integer_scalar(params$mtry, "mtry", 1L)
  if (params$mtry > p) stop("'mtry' exceeds the encoded predictor count.", call. = FALSE)
  params
}

.reference_forest_seed <- function(seed) {
  # ranger interprets seed=0 as an unseeded run.
  if (seed == 0L) 1L else seed
}

.reference_ranger_prediction <- function(object, new) {
  pred <- stats::predict(object$engine, data = new, num.threads = object$params$threads,
    seed = .reference_forest_seed(object$seed))
  pred$chf <- matrix(pred$chf, nrow = nrow(new))
  pred$survival <- matrix(pred$survival, nrow = nrow(new))
  pred
}

.reference_step <- function(time, value, times, initial = 1) {
  c(initial, as.numeric(value))[findInterval(times, time) + 1L]
}

.reference_times <- function(times) {
  if (!is.numeric(times) || !length(times) || any(!is.finite(times)) || any(times < 0)) {
    stop("Provide finite, non-negative 'times' for survival prediction.", call. = FALSE)
  }
}
