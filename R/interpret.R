#' Permutation Importance Using Harrell Concordance
#'
#' @param object A fitted or tuned model.
#' @param newdata Assessment data including outcomes and original predictors.
#' @param variables Predictor names, or a named list of groups of predictor
#'   names permuted together. Defaults to the original formula predictors.
#' @param n_repeats Number of independent permutations per variable/group.
#' @param seed NULL (default) uses the caller's RNG. An integer >= 0 supplies
#'   a local seed and restores the caller's RNG state.
#' @details
#' Importance is original Harrell C minus C after permuting the selected raw
#' columns using a common row permutation within each group. Formula-based
#' transformations are recomputed by prediction. No models are refitted and
#' no rows are sampled away. The same permutations are used across groups.
#' Negative importance is retained. Correlated predictors complicate marginal
#' permutation importance; this is not conditional importance or causality.
#' Use independent assessments or outer-fold assessments, not the final test
#' set repeatedly for feature selection. SD describes permutation variability,
#' not model-fitting uncertainty or a confidence interval.
#' @return A data.frame with class `survlearning_importance`. Attributes retain
#'   individual permutation results, groups, baseline concordance and settings.
#' @export
surv_importance <- function(object, newdata, variables = NULL,
                            n_repeats = 10L, seed = NULL) {
  object <- .evaluation_object(object)
  .integer_scalar(n_repeats, "n_repeats", 1L)
  .validate_seed(seed)
  predictors <- .interpret_predictors(object)
  if (is.null(variables)) variables <- predictors
  if (is.character(variables)) variables <- stats::setNames(as.list(variables), variables)
  if (!is.list(variables) || !length(variables) || is.null(names(variables)) ||
      anyNA(names(variables)) || any(!nzchar(names(variables))) || anyDuplicated(names(variables))) {
    stop("'variables' must name predictors or be a named list of predictor groups.", call. = FALSE)
  }
  for (group in variables) {
    if (!is.character(group) || !length(group) || anyNA(group) || anyDuplicated(group) ||
        any(!group %in% predictors) || any(!group %in% names(newdata))) {
      stop("Permutation groups must contain original predictor columns, not outcomes or encoded contrasts.", call. = FALSE)
    }
  }
  baseline <- surv_score(object, newdata)
  if (!is.finite(baseline)) stop("Importance requires defined assessment concordance.", call. = FALSE)
  permutations <- .with_seed(seed, replicate(n_repeats, sample.int(nrow(newdata)), simplify = FALSE))
  rows <- vector("list", length(variables) * n_repeats)
  k <- 0L
  for (name in names(variables)) for (b in seq_len(n_repeats)) {
    changed <- newdata
    changed[variables[[name]]] <- newdata[permutations[[b]], variables[[name]], drop = FALSE]
    estimate <- surv_score(object, changed)
    if (!is.finite(estimate)) stop("A permuted prediction produced undefined concordance.", call. = FALSE)
    k <- k + 1L
    rows[[k]] <- data.frame(variable = name, repetition = b, concordance = estimate,
      importance = baseline - estimate)
  }
  individual <- do.call(rbind, rows)
  out <- do.call(rbind, lapply(names(variables), function(name) {
    values <- individual$importance[individual$variable == name]
    data.frame(variable = name, importance = mean(values),
      sd = if (length(values) > 1L) stats::sd(values) else NA_real_, n_repeats = n_repeats)
  }))
  row.names(out) <- NULL
  class(out) <- c("survlearning_importance", "data.frame")
  attr(out, "permutations") <- individual
  attr(out, "settings") <- list(metric = "harrell", baseline = baseline,
    variables = variables, seed = seed, n = nrow(newdata),
    uncertainty = "SD over permutations, conditional on the fitted model and assessment")
  out
}

#' First-Order Accumulated Local Effects
#'
#' @param object A fitted or tuned model.
#' @param newdata Predictor data; outcomes are not needed.
#' @param variable One original numeric predictor column.
#' @param bins Requested number of empirical quantile intervals.
#' @param type `"risk"` or `"failure"`. The latter requires native survival.
#' @param time One supported horizon when type is `"failure"`.
#' @details
#' Computes mean upper-minus-lower prediction differences inside each interval,
#' accumulates them, and centers their upper-bound values by empirical interval
#' frequencies. Repeated quantiles and empty intervals are merged. The returned
#' curve is a binwise approximation; `ale` is evaluated at `upper`. No smoothing,
#' causal interpretation, or uncertainty interval is added. Risk effects are
#' on the model-specific risk scale; failure effects are on 1-S(time).
#' @return A `survlearning_ale` data.frame with interval endpoints, counts,
#'   local differences, and centered accumulated effects.
#' @export
surv_ale <- function(object, newdata, variable, bins = 10L,
                     type = "risk", time = NULL) {
  object <- .evaluation_object(object)
  .integer_scalar(bins, "bins", 2L)
  type <- match.arg(type, c("risk", "failure"))
  if (!is.character(variable) || length(variable) != 1L || is.na(variable) ||
      !variable %in% .interpret_predictors(object) || !variable %in% names(newdata)) {
    stop("Choose one original predictor column for 'variable'.", call. = FALSE)
  }
  x <- newdata[[variable]]
  if (!is.numeric(x) || is.object(x) || any(!is.finite(x)) || length(unique(x)) < 2L) {
    stop("ALE requires a finite, varying numeric predictor without missing values.", call. = FALSE)
  }
  if (type == "failure") {
    if (!is.numeric(time) || length(time) != 1L || !is.finite(time) || time < 0) {
      stop("Failure-probability ALE requires one finite non-negative 'time'.", call. = FALSE)
    }
    .evaluation_probabilities(object, newdata, time)
  } else if (!is.null(time)) stop("'time' is only used for failure-probability ALE.", call. = FALSE)
  bounds <- unique(as.numeric(stats::quantile(x, seq(0, 1, length.out = bins + 1L), names = FALSE)))
  repeat {
    membership <- findInterval(x, bounds, rightmost.closed = TRUE, all.inside = TRUE)
    counts <- tabulate(membership, nbins = length(bounds) - 1L)
    empty <- which(counts == 0L)
    if (!length(empty)) break
    bounds <- bounds[-empty]
  }
  difference <- numeric(length(counts))
  for (j in seq_along(counts)) {
    lower <- upper <- newdata[membership == j, , drop = FALSE]
    lower[[variable]] <- bounds[j]
    upper[[variable]] <- bounds[j + 1L]
    if (type == "risk") {
      delta <- surv_predict(object, upper) - surv_predict(object, lower)
    } else {
      delta <- .evaluation_probabilities(object, lower, time)[, 1L] -
        .evaluation_probabilities(object, upper, time)[, 1L]
    }
    if (any(!is.finite(delta))) stop("ALE predictions must be finite.", call. = FALSE)
    difference[j] <- mean(delta)
  }
  accumulated <- cumsum(difference)
  center <- sum(counts * accumulated) / length(x)
  out <- data.frame(lower = utils::head(bounds, -1L), upper = utils::tail(bounds, -1L),
    n = counts, local_difference = difference, ale = accumulated - center)
  class(out) <- c("survlearning_ale", "data.frame")
  attr(out, "settings") <- list(variable = variable, type = type, time = time,
    requested_bins = bins, actual_bins = nrow(out), center = center,
    target = if (type == "risk") "model-specific increasing-risk score" else "failure probability",
    assumptions = "Predictive, not causal; binwise first-order ALE, without refitting or confidence intervals")
  out
}

.interpret_predictors <- function(object) {
  # Saved training terms resolve dot and retain raw names inside transformations.
  terms <- object$design$terms
  if (!is.null(terms)) {
    predictors <- all.vars(stats::delete.response(terms))
  } else {
    formula <- if (is.null(object$input_formula)) object$formula else object$input_formula
    predictors <- all.vars(formula[[3L]])
    if ("." %in% predictors) {
      stop("Saved dot-formula model has no resolved training terms; refit before interpretation.", call. = FALSE)
    }
  }
  setdiff(predictors, all.vars(object$formula[[2L]]))
}

#' Plot Permutation Importance
#' @param x Result from [surv_importance()].
#' @param ... Arguments passed to [graphics::barplot()].
#' @return Invisibly returns the unchanged `survlearning_importance` data frame.
#'   The side effect is a bar plot on the current graphics device.
#' @export
plot.survlearning_importance <- function(x, ...) {
  graphics::barplot(x$importance, names.arg = x$variable,
    ylab = "Decrease in Harrell C-index", ...)
  graphics::abline(h = 0, col = "gray60")
  invisible(x)
}

#' Plot Accumulated Local Effects
#' @param x Result from [surv_ale()].
#' @param ... Arguments passed to [graphics::plot.default()].
#' @return Invisibly returns the unchanged `survlearning_ale` data frame.
#'   The side effect is a step plot on the current graphics device.
#' @export
plot.survlearning_ale <- function(x, ...) {
  settings <- attr(x, "settings")
  graphics::plot(x$upper, x$ale, type = "s", xlab = settings$variable,
    ylab = paste("ALE:", settings$target), ...)
  graphics::abline(h = 0, col = "gray60")
  invisible(x)
}
