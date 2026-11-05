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
#' @param reference Optional name in `models`. If supplied, return each other
#'   model minus this reference instead of separate model evaluations.
#' @param n_boot Number of paired test-case bootstrap repetitions. Use zero
#'   for point estimates or at least two; several hundred are usually needed.
#' @param conf_level Confidence level for percentile intervals.
#' @param seed Optional local bootstrap seed, restoring the caller's RNG.
#' @param ... Evaluation arguments passed to [surv_evaluate()].
#' @details Metrics are evaluated on the identical rows and requested horizons.
#'   Paired intervals resample independent assessment units with replacement,
#'   using the same row indices for every model and metric. Predictions and the
#'   censoring fit remain fixed. Intervals exclude training, tuning and G-fitting
#'   uncertainty and are pointwise, not simultaneous. No fold-level t test or
#'   p-value is returned. Do not pass pooled/repeated cross-validation rows or
#'   repeated observations of the same unit as independent assessment units.
#'   Positive differences favor the model for concordance/AUC; negative
#'   differences favor it for Brier/IBS. Undefined replicates are counted.
#'   IPCW comparisons require identical training outcomes or one explicit common
#'   censoring_data/censoring_model. Risk scales themselves are never compared.
#' @return Without `reference`, a data frame of model evaluations. With it,
#'   a data frame containing `model`, `reference`, `metric`, `time`, `estimate`,
#'   `reference_estimate`, `difference`, `lower`, `upper`, `bootstrap_valid`,
#'   `n`, `events`, `status`, and `message`. Limits concern the difference.
#' @export
#' @examples
#' d <- survival::veteran
#' models <- list(cox = surv_fit(Surv(time, status) ~ age + karno, d),
#'   aft = surv_fit(Surv(time, status) ~ age + karno, d, "aft"))
#' surv_compare(models, d)
surv_compare <- function(models, newdata, ..., reference = NULL, n_boot = 0L,
                         conf_level = .95, seed = NULL) {
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
  .integer_scalar(n_boot, "n_boot")
  .evaluation_fraction(conf_level, "conf_level")
  .validate_seed(seed)
  if (!is.null(reference)) {
    if (!is.character(reference) || length(reference) != 1L || is.na(reference) ||
        !reference %in% names(models) || length(models) < 2L) {
      stop("'reference' must name one of at least two models.", call. = FALSE)
    }
    if (n_boot == 1L) stop("Use n_boot = 0 or at least two repetitions.", call. = FALSE)
    return(.compare_paired(models, newdata, reference, n_boot, conf_level, seed, ...))
  }
  evaluations <- lapply(seq_along(models), function(i) {
    result <- surv_evaluate(models[[i]], newdata, ..., n_boot = n_boot,
      conf_level = conf_level, seed = seed)
    data.frame(model = names(models)[i], as.data.frame(result), row.names = NULL)
  })
  do.call(rbind, evaluations)
}

.compare_paired <- function(models, newdata, reference, n_boot, conf_level, seed,
    metrics = "harrell", times = NULL, tau = NULL, censoring_data = NULL,
    min_g = .05, censoring_model = NULL) {
  models <- lapply(models, .evaluation_object)
  if (any(metrics != "harrell") && is.null(censoring_data) && is.null(censoring_model)) {
    original <- unname(models[[reference]]$training_outcome)
    if (!all(vapply(models, function(x) identical(unname(x$training_outcome), original), logical(1)))) {
      stop("Paired IPCW comparisons need a common censoring reference; supply censoring_data or censoring_model.", call. = FALSE)
    }
  }
  prepared <- lapply(models, .evaluation_setup, newdata = newdata, metrics = metrics,
    times = times, tau = tau, censoring_data = censoring_data, min_g = min_g,
    censoring_model = censoring_model)
  n <- nrow(newdata)
  other <- setdiff(names(models), reference)
  point <- lapply(prepared, function(x) x$calculate(seq_len(n)))
  contrast <- function(index) {
    values <- lapply(prepared, function(x) x$calculate(index)$estimate)
    unlist(lapply(other, function(name) values[[name]] - values[[reference]]), use.names = FALSE)
  }
  out <- do.call(rbind, lapply(other, function(name) {
    a <- point[[name]]
    b <- point[[reference]]
    data.frame(model = name, reference = reference, metric = a$metric, time = a$time,
      estimate = a$estimate, reference_estimate = b$estimate,
      difference = a$estimate - b$estimate, lower = NA_real_, upper = NA_real_,
      bootstrap_valid = 0L, n = n, events = sum(prepared[[name]]$y[, 2L]),
      status = ifelse(is.finite(a$estimate - b$estimate), "ok", "undefined"),
      message = trimws(paste(a$message, b$message)))
  }))
  if (n_boot > 0L) {
    boot <- .with_seed(seed, {
      values <- matrix(NA_real_, n_boot, nrow(out))
      for (b in seq_len(n_boot)) values[b, ] <- contrast(sample.int(n, n, replace = TRUE))
      values
    })
    out$bootstrap_valid <- as.integer(colSums(is.finite(boot)))
    for (j in seq_len(nrow(out))) {
      if (!is.finite(out$difference[j])) next
      if (out$bootstrap_valid[j] >= 2L) {
        ci <- stats::quantile(boot[, j], c((1 - conf_level) / 2, (1 + conf_level) / 2),
          na.rm = TRUE, names = FALSE)
        out$lower[j] <- ci[1L]
        out$upper[j] <- ci[2L]
      }
      if (out$bootstrap_valid[j] < n_boot) {
        out$status[j] <- if (out$bootstrap_valid[j] < 2L) "insufficient_bootstrap" else "partial_bootstrap"
        out$message[j] <- paste0(out$bootstrap_valid[j], "/", n_boot, " finite paired replicates.")
      }
    }
  }
  row.names(out) <- NULL
  attr(out, "settings") <- list(reference = reference, n_boot = n_boot,
    conf_level = conf_level, seed = seed, difference = "model minus reference",
    interval = "pointwise paired test-case percentile; fixed event/G fits")
  attr(out, "assumptions") <- c(.evaluation_assumptions(prepared[[1L]]$conditional),
    "Independent assessment units; no fitting/selection uncertainty or simultaneous coverage.")
  out
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
