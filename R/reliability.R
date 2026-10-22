#' Reliability Summaries Conditional on Survival to an Attained Time
#'
#' @param object A fitted or tuned model providing native survival curves.
#' @param newdata Original predictors for the units of interest.
#' @param horizons Distinct positive future durations, in the model's time unit.
#' @param age One non-negative attained time on the model's time scale.
#' @param probs Distinct probabilities in (0,1) for remaining-time quantiles.
#' @param grid_points Number of equally spaced integration/quantile grid points.
#'   Requested horizons and zero are also included.
#' @param extrapolate Allow parametric tail extrapolation explicitly. Default
#'   FALSE requires age + max(horizons) within training follow-up.
#' @details
#' With fixed input covariates, conditional survival is S(age+u|x)/S(age|x).
#' This conditions on survival, not on newly observed longitudinal predictors.
#' The time origin remains the model's origin, not necessarily installation.
#' Failure probability is one minus conditional survival. The restricted mean
#' remaining time integrates this curve from zero to each supplied horizon
#' by the trapezoidal rule; it is not an unrestricted life expectancy.
#' Quantiles are the first grid points reaching their CDF probability within
#' the largest horizon. NA means not reached on this grid, not infinite life,
#' cure, or a prediction interval. Accuracy depends on grid resolution.
#' No probabilities are constructed for risk-only models. Extrapolation is
#' available only for aft, xgboost_aft and flexsurv, and requires model adequacy.
#' @return A `survlearning_reliability` list with `summary`, `quantiles`,
#'   `times`, `conditional_survival`, and `settings`.
#' @export
surv_reliability <- function(object, newdata, horizons, age = 0,
                             probs = c(.1, .5), grid_points = 201L,
                             extrapolate = FALSE) {
  object <- .evaluation_object(object)
  .evaluation_times(horizons)
  if (any(horizons <= 0)) stop("'horizons' must be positive future durations.", call. = FALSE)
  if (!is.numeric(age) || length(age) != 1L || !is.finite(age) || age < 0) {
    stop("'age' must be one finite non-negative attained model time.", call. = FALSE)
  }
  if (!is.numeric(probs) || !length(probs) || any(!is.finite(probs)) ||
      any(probs <= 0 | probs >= 1) || anyDuplicated(probs)) {
    stop("'probs' must be distinct probabilities in (0, 1).", call. = FALSE)
  }
  .integer_scalar(grid_points, "grid_points", 2L)
  if (!is.logical(extrapolate) || length(extrapolate) != 1L || is.na(extrapolate)) {
    stop("'extrapolate' must be TRUE or FALSE.", call. = FALSE)
  }
  beyond <- age + max(horizons) > object$max_time
  if (beyond && (!extrapolate || !object$method %in% c("aft", "xgboost_aft", "flexsurv"))) {
    stop("Requested attained time plus horizon exceeds training follow-up; only explicit parametric extrapolation is allowed.", call. = FALSE)
  }
  times <- sort(unique(c(0, horizons, seq(0, max(horizons), length.out = grid_points))))
  survival <- .evaluation_probabilities(object, newdata, age + times)
  denominator <- survival[, 1L]
  if (any(denominator <= 0)) stop("Conditional survival requires positive S(age | x) for every unit.", call. = FALSE)
  conditional <- survival / denominator
  intervals <- diff(times)
  increments <- (conditional[, -1L, drop = FALSE] +
    conditional[, -ncol(conditional), drop = FALSE]) / 2
  increments <- sweep(increments, 2L, intervals, "*")
  integral <- matrix(0, nrow(conditional), length(times))
  for (j in seq_along(intervals)) integral[, j + 1L] <- integral[, j] + increments[, j]
  rows <- lapply(seq_len(nrow(newdata)), function(i) {
    index <- match(horizons, times)
    data.frame(observation = i, age = age, horizon = horizons,
      survival = conditional[i, index], failure = 1 - conditional[i, index],
      restricted_remaining_mean = integral[i, index])
  })
  quantiles <- lapply(seq_len(nrow(newdata)), function(i) {
    remaining <- vapply(probs, function(p) {
      reached <- which(conditional[i, ] <= 1 - p)
      if (length(reached)) times[reached[1L]] else NA_real_
    }, numeric(1))
    data.frame(observation = i, age = age, probability = probs,
      remaining_time = remaining,
      status = ifelse(is.finite(remaining), "reached_on_grid", "not_reached_within_horizon"))
  })
  structure(list(summary = do.call(rbind, rows), quantiles = do.call(rbind, quantiles),
    times = times, conditional_survival = conditional,
    settings = list(age = age, horizons = horizons, probs = probs,
      grid_points = grid_points, extrapolation_used = beyond,
      maximum_training_time = object$max_time,
      integration = "trapezoidal on the supplied augmented grid",
      quantiles = "first grid crossing, without tail completion",
      assumptions = "Fixed covariates; conditional on survival to attained model time; not a dynamic model or unrestricted life expectancy")),
    class = "survlearning_reliability")
}

#' Print Reliability Summaries
#' @param x Result from [surv_reliability()].
#' @param ... Arguments passed to [print.data.frame()].
#' @return Invisibly returns the unchanged `survlearning_reliability` list,
#'   after printing the attained time and the summary data frame.
#' @export
print.survlearning_reliability <- function(x, ...) {
  cat("Reliability conditional on survival to time ", x$settings$age, "\n", sep = "")
  print(x$summary, ...)
  invisible(x)
}

#' Plot Conditional Reliability Curves
#' @param x Result from [surv_reliability()].
#' @param rows Observation indices to plot.
#' @param type `"survival"` or `"failure"`.
#' @param ... Arguments passed to [graphics::matplot()].
#' @return Invisibly returns the unchanged `survlearning_reliability` list.
#'   The side effect is a curve plot on the current graphics device.
#' @export
plot.survlearning_reliability <- function(x, rows = seq_len(min(5L, nrow(x$conditional_survival))),
                                         type = "survival", ...) {
  type <- match.arg(type, c("survival", "failure"))
  if (!is.numeric(rows) || !length(rows) || any(!is.finite(rows)) ||
      any(rows != floor(rows)) || any(rows < 1 | rows > nrow(x$conditional_survival))) {
    stop("'rows' must index observations in the reliability result.", call. = FALSE)
  }
  curves <- x$conditional_survival[rows, , drop = FALSE]
  if (type == "failure") curves <- 1 - curves
  graphics::matplot(x$times, t(curves), type = "l", lty = seq_along(rows),
    xlab = "Future duration", ylab = paste("Conditional", type, "probability"), ...)
  invisible(x)
}
