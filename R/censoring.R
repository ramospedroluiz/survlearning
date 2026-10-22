#' Fit a Reference Censoring Distribution
#'
#' @param formula Original event-response formula Surv(time,event) ~ predictors.
#'   Event is still 1 for failure, not 1 for censoring.
#' @param data Reference training data, not evaluation outcomes.
#' @param method `"km"`, `"cox"`, or `"stratified"`. The last uses joint
#'   strata of the named original categorical predictors, without interactions
#'   or transformations in the formula.
#' @param impute `"none"` or training-only `"median"`/mode imputation.
#' @details
#' KM and stratified KM remove failures before tied censorings, consistently
#' with event = I(T <= C). Cox uses an exact ordering of two phases at each
#' distinct observed time: original failures depart in phase one, censoring
#' events occur in phase two. The phase index is an ordering device for the
#' censoring partial likelihood, not jitter or a change to event-model times.
#' Efron ties and the matching native Cox baseline are used within phase two.
#' A conditional Cox G requires its PH/functional-form assumptions; it does
#' not solve dependent censoring unexplained by the predictors or model a
#' terminal administrative point mass exactly. Stratified KM avoids PH but
#' needs support in every relevant stratum. Unknown strata are rejected.
#' @return A serializable `survlearning_censoring` object. No neural runtime or
#'   external engine beyond survival is needed.
#' @export
surv_censor_fit <- function(formula, data, method = "cox", impute = "none") {
  method <- match.arg(method, c("km", "cox", "stratified"))
  impute <- match.arg(impute, c("none", "median"))
  y <- .outcome(formula, data, require_event = FALSE)
  out <- list(method = method, formula = formula, n = nrow(y),
    events = sum(y[, 2L]), censorings = sum(1 - y[, 2L]),
    max_time = max(y[, 1L]), impute = impute,
    event_limit = "G(T-)", horizon_limit = "G(t)",
    ties = "original failures depart before tied censoring events",
    source = "explicit reference censoring fit")
  if (method == "km") {
    out$table <- .evaluation_censoring(y)$table
  } else if (method == "stratified") {
    variables <- all.vars(formula[[3L]])
    labels <- attr(stats::terms(formula), "term.labels")
    if (!length(variables) || !setequal(variables, labels) || any(!variables %in% names(data))) {
      stop("Stratified censoring requires named categorical columns joined by '+'.", call. = FALSE)
    }
    imputed <- .fit_imputation(formula, data, impute)
    out$imputation <- imputed$values
    out$variables <- variables
    out$levels <- lapply(imputed$data[variables], function(x) {
      if (!(is.factor(x) || is.character(x) || is.logical(x)) || anyNA(x)) {
        stop("Stratified censoring predictors must be complete categorical columns.", call. = FALSE)
      }
      sort(unique(as.character(x)))
    })
    membership <- .censor_strata(out, imputed$data)
    out$strata <- lapply(split(seq_len(nrow(y)), membership), function(index) {
      fit <- .evaluation_censoring(y[index, , drop = FALSE])
      fit
    })
  } else {
    # Resolve dot and imputation before replacing the original response.
    prepared <- .prepare_design(formula, data, impute, require_event = FALSE)
    out$design <- prepared$design
    out$constant <- !out$censorings
    if (!out$constant) {
      distinct <- sort(unique(y[, 1L]))
      phase <- 2L * match(y[, 1L], distinct) - y[, 2L]
      encoded <- .engine_data(prepared$x)
      encoded$.sl_time <- phase
      encoded$.sl_event <- 1 - y[, 2L]
      warnings <- character()
      fit <- withCallingHandlers(survival::coxph(.engine_formula(), data = encoded,
        ties = "efron", x = TRUE, y = TRUE, model = TRUE,
        control = survival::coxph.control(timefix = FALSE)),
        warning = function(w) warnings <<- c(warnings, conditionMessage(w)))
      if (any(!is.finite(stats::coef(fit)))) {
        stop("Conditional censoring Cox coefficients are non-finite; simplify the reference model.", call. = FALSE)
      }
      baseline <- survival::basehaz(fit, centered = TRUE)
      baseline <- baseline[baseline$time %% 2L == 0, , drop = FALSE]
      out$baseline <- data.frame(time = distinct[baseline$time / 2L], hazard = baseline$hazard)
      out$coefficients <- stats::coef(fit)
      out$center <- sum(fit$means * out$coefficients)
      out$engine <- fit
      out$warnings <- unique(warnings)
    }
  }
  class(out) <- "survlearning_censoring"
  out
}

.censor_strata <- function(object, newdata) {
  if (!is.data.frame(newdata) || !nrow(newdata) || any(!object$variables %in% names(newdata))) {
    stop("Censoring prediction requires the original stratum columns.", call. = FALSE)
  }
  newdata <- .apply_imputation(newdata, object$imputation)
  codes <- lapply(object$variables, function(name) {
    index <- match(as.character(newdata[[name]]), object$levels[[name]])
    if (anyNA(index)) stop("Unknown or missing censoring stratum level.", call. = FALSE)
    index
  })
  key <- do.call(paste, c(codes, sep = ":"))
  if (!is.null(object$strata) && any(!key %in% names(object$strata))) {
    stop("Unknown joint censoring stratum; no pooling or extrapolation was added.", call. = FALSE)
  }
  key
}

#' Predict Reference Censoring Survival
#' @param object A fit from [surv_censor_fit()].
#' @param newdata Predictor data, with one row per unit.
#' @param times Finite non-negative horizons; order and repetitions are kept.
#' @param left Use G(t-) instead of G(t).
#' @param pairwise If TRUE, one time per row is required and a vector is
#'   returned, avoiding a subject-by-subject time matrix.
#' @return An n-by-times matrix, or an aligned vector when pairwise = TRUE.
#'   Horizons beyond reference/stratum follow-up are rejected.
#' @export
surv_censor_predict <- function(object, newdata, times, left = FALSE, pairwise = FALSE) {
  if (!inherits(object, "survlearning_censoring")) stop("Use a fit from surv_censor_fit().", call. = FALSE)
  if (!is.data.frame(newdata) || !nrow(newdata)) stop("Provide nonempty 'newdata'.", call. = FALSE)
  if (!is.numeric(times) || !length(times) || any(!is.finite(times)) || any(times < 0)) {
    stop("Censoring prediction needs finite non-negative times.", call. = FALSE)
  }
  for (value in list(left, pairwise)) if (!is.logical(value) || length(value) != 1L || is.na(value)) {
    stop("'left' and 'pairwise' must be TRUE or FALSE.", call. = FALSE)
  }
  if (pairwise && length(times) != nrow(newdata)) stop("Pairwise times must align with rows.", call. = FALSE)
  .evaluation_followup(times, object$max_time, "censoring reference")
  n <- nrow(newdata)
  if (object$method == "km") {
    g <- .evaluation_g(object, times, left)
    if (pairwise) return(g)
    return(matrix(rep(g, each = n), nrow = n))
  }
  if (object$method == "stratified") {
    key <- .censor_strata(object, newdata)
    out <- if (pairwise) numeric(n) else matrix(NA_real_, n, length(times))
    for (group in unique(key)) {
      index <- which(key == group)
      requested <- if (pairwise) times[index] else times
      fit <- object$strata[[group]]
      .evaluation_followup(requested, fit$max_time, "censoring stratum")
      g <- .evaluation_g(fit, requested, left)
      if (pairwise) out[index] <- g else out[index, ] <- rep(g, each = length(index))
    }
    return(out)
  }
  x <- .new_design(object$design, newdata)
  if (object$constant) return(if (pairwise) rep(1, n) else matrix(1, n, length(times)))
  lp <- drop(x %*% object$coefficients) - object$center
  if (any(!is.finite(lp))) stop("Conditional censoring log risks must be finite.", call. = FALSE)
  index <- findInterval(times, object$baseline$time)
  if (left) {
    tied <- index > 0L
    tied[tied] <- times[tied] == object$baseline$time[index[tied]]
    index[tied] <- index[tied] - 1L
  }
  h <- c(0, object$baseline$hazard)[index + 1L]
  if (pairwise) return(exp(-exp(log(h) + lp)))
  exp(-exp(outer(lp, log(h), "+")))
}

.resample_censoring <- function(specification, data) {
  if (is.null(specification)) return(NULL)
  if (inherits(specification, "formula")) specification <- list(formula = specification, method = "cox")
  if (!is.list(specification) || is.null(specification$formula) ||
      any(!names(specification) %in% c("formula", "method", "impute")) ||
      anyDuplicated(names(specification))) {
    stop("'censoring' must be a formula or a list(formula, method, impute); fitted global censoring models are not accepted in validation.", call. = FALSE)
  }
  do.call(surv_censor_fit, c(specification, list(data = data)))
}
