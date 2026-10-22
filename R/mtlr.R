.mtlr_survival <- function(object, x, times) {
  n <- nrow(x)
  # MTLR 0.2.2 drops the observation dimension for a one-row newdata.
  if (n == 1L) x <- rbind(x, x)
  requested <- sort(unique(times))
  native <- stats::predict(object$engine, newdata = .engine_data(x),
    type = "prob_times", times = requested, add_zero = TRUE)
  rows <- match(times, native[[1L]])
  t(as.matrix(native[rows, seq_len(n) + 1L, drop = FALSE]))
}

.mtlr_risk <- function(object, x) {
  times <- object$risk_grid
  survival <- .mtlr_survival(object, x, times)
  midpoint <- (survival[, -1L, drop = FALSE] +
    survival[, -ncol(survival), drop = FALSE]) / 2
  -drop(midpoint %*% diff(times))
}
