.calibration_ipcw <- function(object, newdata, y, p, times, groups, censoring,
                              min_g, n_boot, conf_level, seed) {
  if (!inherits(censoring, "survlearning_censoring")) {
    stop("'censoring_model' must come from surv_censor_fit().", call. = FALSE)
  }
  .evaluation_followup(times, censoring$max_time, "censoring reference")
  rows <- .with_seed(seed, {
    result <- list()
    for (j in seq_along(times)) {
      cases <- y[, 2L] == 1 & y[, 1L] <= times[j]
      controls <- y[, 1L] > times[j]
      a <- b <- numeric(nrow(y))
      if (any(cases)) {
        g <- surv_censor_predict(censoring, newdata[cases, , drop = FALSE],
          y[cases, 1L], left = TRUE, pairwise = TRUE)
        .evaluation_g_support(g, min_g)
        b[cases] <- 1 / g
      }
      if (any(controls)) {
        g <- surv_censor_predict(censoring, newdata[controls, , drop = FALSE], times[j])[, 1L]
        .evaluation_g_support(g, min_g)
        a[controls] <- b[controls] <- 1 / g
      }
      membership <- floor((rank(p[, j], ties.method = "average") - .5) *
        min(groups, nrow(y)) / nrow(y)) + 1L
      occupied <- sort(unique(membership))
      for (k in seq_along(occupied)) {
        index <- which(membership == occupied[k])
        total <- sum(b[index])
        estimate <- if (total > 0) sum(a[index]) / total else NA_real_
        effective <- if (total > 0) total^2 / sum(b[index]^2) else 0
        lower <- upper <- NA_real_
        valid <- 0L
        if (n_boot > 0L && is.finite(estimate)) {
          samples <- rep(NA_real_, n_boot)
          for (replicate in seq_len(n_boot)) {
            draw <- index[sample.int(length(index), length(index), replace = TRUE)]
            denominator <- sum(b[draw])
            if (denominator > 0) samples[replicate] <- sum(a[draw]) / denominator
          }
          valid <- sum(is.finite(samples))
          if (valid >= 2L) {
            interval <- stats::quantile(samples, c((1 - conf_level) / 2,
              (1 + conf_level) / 2), na.rm = TRUE, names = FALSE)
            lower <- interval[1L]
            upper <- interval[2L]
          }
        }
        events <- sum(cases[index])
        at_risk <- sum(y[index, 1L] >= times[j])
        reasons <- c(if (!is.finite(estimate)) "No supported weighted case/control mass.",
          if (length(index) < 10L) "Fewer than 10 subjects.",
          if (events < 3L) "Fewer than 3 events by horizon.",
          if (at_risk < 5L) "Fewer than 5 at risk.",
          if (effective < 10) "IPCW effective sample size below 10.",
          if (length(occupied) < groups) "Fewer groups than requested; tied predictions are not split.",
          if (n_boot > 0L && valid < n_boot) "Undefined bootstrap repetitions; inspect bootstrap_valid.")
        result[[length(result) + 1L]] <- data.frame(time = times[j], group = k,
          predicted = mean(p[index, j]), observed = estimate, lower = lower, upper = upper,
          n = length(index), events = events, n_risk = at_risk, effective_n = effective,
          bootstrap_valid = valid,
          status = if (!is.finite(estimate)) "unsupported" else if (length(reasons)) "low_support" else "ok",
          message = paste(reasons, collapse = " "), stringsAsFactors = FALSE)
      }
    }
    result
  })
  out <- do.call(rbind, rows)
  row.names(out) <- NULL
  class(out) <- c("survlearning_calibration", "data.frame")
  attr(out, "settings") <- list(times = times, groups = groups, estimator = "normalized IPCW",
    censoring_method = censoring$method, min_g = min_g, n_boot = n_boot,
    conf_level = conf_level, conf_type = if (n_boot) "fixed-fit case bootstrap percentile" else "none",
    seed = seed, grouping = "per-horizon midrank bins; original grouping fixed in bootstrap")
  attr(out, "assumptions") <- c("Suitable reference G and positive support; conditional independent censoring given its predictors.",
    "Normalized weighted control mass divided by weighted case plus control mass; not KM.",
    "Bootstrap conditions on the fitted event/G models and grouping; no full-pipeline uncertainty.")
  if (any(out$status != "ok")) warning("Low support or collapsed groups in IPCW calibration; inspect status and message.", call. = FALSE)
  out
}
