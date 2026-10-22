#' Inspect Parameters and Fit Diagnostics
#'
#' @param method A method listed by [surv_methods()].
#' @param object A fitted or tuned survlearning object.
#' @return `surv_parameters()` returns the named default parameter list.
#'   `surv_diagnostics()` returns provenance, warnings, and the convergence
#'   status actually reported by the engine, not a universal convergence test.
#' @export
#' @examples
#' surv_parameters("aft")
surv_parameters <- function(method) {
  .method_specs()[[.method(method)]]$defaults
}

#' @rdname surv_parameters
#' @export
surv_diagnostics <- function(object) {
  if (inherits(object, "survlearning_tune")) object <- object$best_model
  if (!inherits(object, "survlearning_fit")) {
    stop("'object' must come from surv_fit() or surv_tune().", call. = FALSE)
  }
  list(method = object$method, engine = object$engine_package,
    n = object$n, events = object$events, seed = object$seed,
    fit_seconds = object$fit_seconds, params = object$params,
    diagnostics = object$diagnostics)
}

#' Compare Models on the Same Held-Out Observations
#'
#' @param models A named list of fitted or tuned models. Models must use the
#'   same response expression and must not have been selected using `newdata`.
#' @param newdata A common held-out data frame.
#' @param ... Evaluation arguments passed to [surv_evaluate()].
#' @details Metrics are evaluated on the identical rows and requested horizons.
#'   This function does not fit models, rank incomparable raw risk scales, or
#'   perform a statistical test of paired differences. Bootstrap intervals in
#'   individual evaluations are conditional on the already fitted models.
#' @return A data frame with a model column followed by the evaluation columns.
#' @export
#' @examples
#' d <- survival::veteran
#' models <- list(cox = surv_fit(Surv(time, status) ~ age + karno, d),
#'   aft = surv_fit(Surv(time, status) ~ age + karno, d, "aft"))
#' surv_compare(models, d)
surv_compare <- function(models, newdata, ...) {
  if (!is.list(models) || !length(models) || is.null(names(models)) ||
      anyNA(names(models)) || any(!nzchar(names(models))) ||
      anyDuplicated(names(models))) {
    stop("'models' must be a non-empty, uniquely named list.", call. = FALSE)
  }
  outcomes <- lapply(models, function(model) {
    if (inherits(model, "survlearning_tune")) model <- model$best_model
    if (!inherits(model, "survlearning_fit")) stop("All models must be fitted survlearning objects.", call. = FALSE)
    .outcome(model$formula, newdata)
  })
  if (!all(vapply(outcomes, function(y) identical(unname(y), unname(outcomes[[1L]])), logical(1)))) {
    stop("Models must be evaluated on the same outcome.", call. = FALSE)
  }
  evaluations <- lapply(seq_along(models), function(i) {
    result <- surv_evaluate(models[[i]], newdata, ...)
    data.frame(model = names(models)[i], as.data.frame(result), row.names = NULL)
  })
  do.call(rbind, evaluations)
}

#' Plot a Survival Evaluation Metric
#'
#' @param x A table from [surv_evaluate()].
#' @param y Unused, for S3 compatibility.
#' @param metric One metric present in the evaluation.
#' @param ... Additional graphical arguments.
#' @return Invisibly, `x`.
#' @export
plot.survlearning_evaluation <- function(x, y = NULL, metric = "brier", ...) {
  if (!metric %in% x$metric) stop("Metric is not present in the evaluation.", call. = FALSE)
  rows <- x[x$metric == metric & is.finite(x$estimate), , drop = FALSE]
  if (!nrow(rows)) stop("No finite estimates are available for this metric.", call. = FALSE)
  if (all(is.na(rows$time))) {
    graphics::barplot(rows$estimate, names.arg = rows$metric, ylab = "Estimate", ...)
  } else {
    rows <- rows[order(rows$time), , drop = FALSE]
    graphics::plot(rows$time, rows$estimate, type = "b", xlab = "Time",
      ylab = metric, ...)
  }
  invisible(x)
}
