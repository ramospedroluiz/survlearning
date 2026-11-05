.calibration_metrics <- function(object, newdata, times, censoring_model,
                                 min_g, n_boot, conf_level, seed) {
  prepared <- .evaluation_setup(object, newdata, "brier", times = times,
    min_g = min_g, censoring_model = censoring_model)
  y <- prepared$y
  n <- nrow(y)
  weights <- .horizon_weights(y, times, prepared$g_event, prepared$g_time)
  calculate <- function(index) {
    do.call(rbind, lapply(seq_along(times), function(j) {
      fit <- .calibration_logistic(as.integer(y[index, 2L] == 1 & y[index, 1L] <= times[j]),
        1 - prepared$probabilities[index, j], weights[index, j])
      fit$time <- times[j]
      fit
    }))
  }
  out <- calculate(seq_len(n))
  out$lower <- out$upper <- NA_real_
  out$bootstrap_valid <- 0L
  if (n_boot > 0L) {
    boot <- .with_seed(seed, {
      values <- matrix(NA_real_, n_boot, nrow(out))
      for (b in seq_len(n_boot)) values[b, ] <- calculate(sample.int(n, n, replace = TRUE))$estimate
      values
    })
    out$bootstrap_valid <- as.integer(colSums(is.finite(boot)))
    for (j in seq_len(nrow(out))) {
      if (!is.finite(out$estimate[j])) next
      if (out$bootstrap_valid[j] >= 2L) {
        ci <- stats::quantile(boot[, j], c((1 - conf_level) / 2, (1 + conf_level) / 2),
          na.rm = TRUE, names = FALSE)
        out$lower[j] <- ci[1L]
        out$upper[j] <- ci[2L]
      }
      if (out$bootstrap_valid[j] < n_boot) {
        out$status[j] <- if (out$bootstrap_valid[j] < 2L) "insufficient_bootstrap" else "partial_bootstrap"
        out$message[j] <- paste(out$message[j], paste0(out$bootstrap_valid[j], "/", n_boot,
          " finite bootstrap replicates."))
      }
    }
  }
  out$n <- n
  out <- out[c("metric", "time", "estimate", "ideal", "lower", "upper", "n",
    "known", "events", "effective_n", "bootstrap_valid", "status", "message")]
  row.names(out) <- NULL
  class(out) <- c("survlearning_calibration_metrics", "data.frame")
  attr(out, "censoring") <- prepared$censoring
  attr(out, "settings") <- list(type = "metrics", times = times, min_g = min_g,
    n_boot = n_boot, conf_level = conf_level, seed = seed,
    interval = "pointwise test-case percentile; fixed event/G fits",
    intercept = "offset fit with slope one", slope = "joint intercept/slope fit")
  attr(out, "assumptions") <- .evaluation_assumptions(prepared$conditional)
  if (any(out$status != "ok")) warning("Some calibration measures are undefined or have limited support; inspect status and message.", call. = FALSE)
  out
}

.horizon_weights <- function(y, times, g_event, g_time) {
  weights <- matrix(0, nrow(y), length(times))
  for (j in seq_along(times)) {
    cases <- y[, 2L] == 1 & y[, 1L] <= times[j]
    controls <- y[, 1L] > times[j]
    weights[cases, j] <- 1 / g_event[cases]
    weights[controls, j] <- 1 / if (is.matrix(g_time)) g_time[controls, j] else g_time[j]
  }
  weights
}

.calibration_logistic <- function(event, probability, weight) {
  known <- weight > 0
  event <- event[known]
  probability <- probability[known]
  weight <- weight[known]
  effective <- if (length(weight)) sum(weight)^2 / sum(weight^2) else 0
  out <- data.frame(metric = c("intercept", "slope"), estimate = NA_real_,
    ideal = c(0, 1), known = length(weight), events = sum(event), effective_n = effective,
    status = "undefined", message = "")
  if (length(unique(event)) < 2L) {
    out$message <- "Both weighted cases and controls are required."
    return(out)
  }
  if (any(probability <= 0 | probability >= 1)) {
    out$message <- "Calibration logits require probabilities strictly between zero and one; no clipping is applied."
    return(out)
  }
  eta <- stats::qlogis(probability)
  # An offset intercept remains identifiable even when all predictions agree.
  for (j in 1:2) {
    if (j == 2L && (diff(range(eta)) <= 1e-10 ||
        max(eta[event == 0]) <= min(eta[event == 1]) ||
        max(eta[event == 1]) <= min(eta[event == 0]))) {
      out$message[j] <- "Slope is not identifiable: constant scores or complete/quasi separation."
      next
    }
    warnings <- character()
    fit <- tryCatch(withCallingHandlers(stats::glm.fit(
      x = if (j == 1L) matrix(1, length(eta), 1L) else cbind(1, eta),
      y = event, weights = weight, offset = if (j == 1L) eta else NULL,
      family = stats::quasibinomial(), control = stats::glm.control(maxit = 100L)),
      warning = function(w) { warnings <<- c(warnings, conditionMessage(w)); invokeRestart("muffleWarning") }),
      error = function(e) e)
    if (inherits(fit, "error") || !isTRUE(fit$converged) ||
        length(warnings) || any(!is.finite(fit$coefficients))) {
      out$message[j] <- if (inherits(fit, "error")) conditionMessage(fit) else
        paste(c("Calibration fit did not give a reliable finite solution.", warnings), collapse = " ")
      next
    }
    out$estimate[j] <- unname(fit$coefficients[j])
    out$status[j] <- if (effective < 10 || min(sum(event), sum(1 - event)) < 3) "low_support" else "ok"
    if (out$status[j] == "low_support") out$message[j] <- "Few cases/controls or IPCW effective sample size below ten."
  }
  out
}
