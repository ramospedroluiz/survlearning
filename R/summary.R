#' Summarize a Survival Learning Fit or Search
#'
#' @param object A fitted or tuned survlearning object.
#' @param conf_level Confidence level for classical fixed-model Wald intervals.
#' @param ... Reserved; unused arguments are rejected.
#' @details Coefficients and covariance are exposed for ordinary Cox and
#' parametric AFT only, with original design-column names. For Cox, the summary
#' additionally reports hazard ratios. AFT coefficients are log-time effects,
#' not log-hazards. Intervals condition on the fitted model and do not include
#' hyperparameter selection or training-sample uncertainty from a full pipeline.
#' No coefficient vector, p-value, or interval is invented for other learners.
#' @return `summary()` returns a `summary.survlearning_fit` list with training information, prediction capabilities,
#' parameters, warnings, numerical diagnostics and, where appropriate, a classical
#' coefficient table. Search summaries also retain the complete candidate table.
#' `print()` invisibly returns this summary unchanged after console output.
#' @export
summary.survlearning_fit <- function(object, conf_level = .95, ...) {
  if (length(list(...))) stop("Unused summary argument(s).", call. = FALSE)
  .evaluation_fraction(conf_level, "conf_level")
  coefficients <- NULL
  if (object$method %in% c("cox", "aft")) {
    beta <- stats::coef(object)
    se <- sqrt(diag(stats::vcov(object)))
    z <- beta / se
    critical <- stats::qnorm((1 + conf_level) / 2)
    coefficients <- data.frame(estimate = beta, std_error = se,
      lower = beta - critical * se, upper = beta + critical * se,
      p_value = 2 * stats::pnorm(-abs(z)))
    if (object$method == "cox") {
      coefficients$hazard_ratio <- exp(beta)
      coefficients$hr_lower <- exp(coefficients$lower)
      coefficients$hr_upper <- exp(coefficients$upper)
    }
  }
  structure(list(method = object$method, engine = object$engine_package,
    n = object$n, events = object$events,
    censoring_percent = 100 * (1 - object$events / object$n),
    predictions = .method_specs()[[object$method]]$predictions,
    params = object$params, seconds = object$fit_seconds,
    warnings = object$diagnostics$warnings,
    convergence = object$diagnostics$convergence,
    coefficients = coefficients, conf_level = conf_level,
    inference = "Classical fixed-model Wald intervals only; no selection-adjusted inference"),
    class = "summary.survlearning_fit")
}

#' @rdname summary.survlearning_fit
#' @export
summary.survlearning_tune <- function(object, conf_level = .95, ...) {
  out <- summary(object$best_model, conf_level = conf_level, ...)
  out$search <- list(candidates = nrow(object$grid),
    objective = if (is.null(object$metric)) "harrell" else object$metric,
    best_candidate = object$best_candidate, results = object$results,
    failed_folds = sum(object$results$failed_folds), execution = object$execution,
    refit_warnings = object$refit_warnings)
  out
}

#' @rdname summary.survlearning_fit
#' @param x A summary returned by summary().
#' @export
print.summary.survlearning_fit <- function(x, ...) {
  cat("Survival learning summary\nMethod: ", x$method, " (", x$engine, ")\n",
    "Training: ", x$n, " rows; ", x$events, " events; ",
    round(x$censoring_percent, 1), "% censored\n",
    "Predictions: ", paste(x$predictions, collapse = ", "), "\n",
    "Fit seconds: ", format(x$seconds, digits = 4), "\n",
    "Numerical convergence: ", x$convergence$status, "\n", sep = "")
  if (length(x$params)) { cat("Parameters:\n"); print(x$params) }
  if (!is.null(x$search)) {
    cat("Selection objective: ", x$search$objective, "; candidate ",
      x$search$best_candidate, " of ", x$search$candidates, "\n", sep = "")
    if (x$search$candidates == 1L) cat("One fixed configuration; no hyperparameter search.\n")
    cat("Candidate results (fold SD is descriptive):\n")
    print(x$search$results, row.names = FALSE)
  }
  if (!is.null(x$coefficients)) {
    cat(if (x$method == "cox") "Cox log-hazard coefficients and hazard ratios:\n" else
      "AFT log-time coefficients:\n")
    print(x$coefficients, ...)
    cat(x$inference, "\n")
  }
  warnings <- unique(c(x$warnings, x$search$refit_warnings))
  if (length(warnings)) cat("Recorded warnings:\n", paste(warnings, collapse = "\n"), "\n", sep = "")
  invisible(x)
}

.classical_fit <- function(object) {
  if (inherits(object, "survlearning_tune")) object <- object$best_model
  if (!object$method %in% c("cox", "aft")) {
    stop("Classical coefficients/covariance are exposed for ordinary cox and aft only; use model-specific diagnostics for other learners.", call. = FALSE)
  }
  object
}

.coefficient_names <- function(names, design) {
  index <- match(names, design$names)
  matched <- !is.na(index)
  names[matched] <- design$columns[index[matched]]
  names
}

#' Classical Coefficients and Covariance
#' @param object An ordinary Cox or parametric AFT fitted/tuned object.
#' @param ... Reserved; unused arguments are rejected.
#' @return Named coefficients or their covariance matrix, on the original design
#' scale. The AFT covariance excludes the ancillary log-scale parameter to match
#' the returned regression coefficient vector.
#' @export
coef.survlearning_fit <- function(object, ...) {
  if (length(list(...))) stop("Unused coefficient argument(s).", call. = FALSE)
  object <- .classical_fit(object)
  beta <- stats::coef(object$engine)
  names(beta) <- .coefficient_names(names(beta), object$design)
  beta
}

#' @rdname coef.survlearning_fit
#' @export
coef.survlearning_tune <- function(object, ...) coef.survlearning_fit(object$best_model, ...)

#' @rdname coef.survlearning_fit
#' @export
vcov.survlearning_fit <- function(object, ...) {
  if (length(list(...))) stop("Unused covariance argument(s).", call. = FALSE)
  object <- .classical_fit(object)
  beta <- stats::coef(object$engine)
  covariance <- stats::vcov(object$engine)[names(beta), names(beta), drop = FALSE]
  labels <- .coefficient_names(names(beta), object$design)
  dimnames(covariance) <- list(labels, labels)
  covariance
}

#' @rdname coef.survlearning_fit
#' @export
vcov.survlearning_tune <- function(object, ...) vcov.survlearning_fit(object$best_model, ...)
