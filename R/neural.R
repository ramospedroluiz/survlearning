# nnet optimizes the PLANN Bernoulli likelihood after explicit time coarsening;
# Neural Cox uses a native analytic partial likelihood and gradient.
.fit_neural <- function(method, prepared, params, seed) {
  .integer_scalar(params$hidden, "hidden", 1L)
  .positive_scalar(params$decay, "decay", zero = TRUE)
  x <- prepared$x
  y <- prepared$y
  if (method == "neural_discrete") {
    .integer_scalar(params$intervals, "intervals", 2L)
    .integer_scalar(params$max_iter, "max_iter", 1L)
    .integer_scalar(params$max_weights, "max_weights", 1L)
    weights <- (as.double(ncol(x)) + 3) * params$hidden + 1
    if (weights > params$max_weights) {
      stop("Neural network exceeds 'max_weights'; increase it or reduce 'hidden'.", call. = FALSE)
    }
    moments <- .design_moments(x)
    center <- moments$center
    scale <- moments$scale
    scaled <- .scaled_design(x, moments)
    horizon <- max(y[, 1L])
    grid <- seq(0, horizon, length.out = params$intervals + 1)
    if (any(diff(grid) <= 0)) {
      stop("'intervals' cannot form a strictly increasing grid at this time scale.", call. = FALSE)
    }
    periods <- .neural_discrete_periods(y, grid)
    training <- cbind(scaled[periods$expansion$observation, , drop = FALSE],
      .sl_interval = periods$expansion$interval / params$intervals)
    fit <- .with_seed(seed, nnet::nnet(x = training,
      y = matrix(periods$expansion$response, ncol = 1L),
      size = params$hidden, entropy = TRUE, linout = FALSE,
      softmax = FALSE, skip = FALSE, decay = params$decay,
      maxit = params$max_iter, MaxNWts = params$max_weights, trace = FALSE))
    if (fit$convergence != 0L) {
      warning("PLANN optimizer reached max_iter without convergence.", call. = FALSE)
    }
    return(list(fit = fit, center = center, scale = scale,
      time = grid, horizon = horizon, convergence = fit$convergence,
      expansion = periods$expansion, exposure = periods$exposure,
      diagnostics = periods$diagnostics,
      coarsening = paste("Events use right-closed intervals; censored observations contribute",
        "only completed intervals. Incomplete censoring intervals are omitted,",
        "which may introduce discretization bias; this is not a continuous-time likelihood.")))
  }
  .fit_neural_cox_native(x, y, params)
}

.predict_neural <- function(object, x, type, times = NULL) {
  fit <- object$engine
  scaled <- .scaled_design(x, fit)
  if (object$method == "neural_discrete") {
    if (!inherits(fit$fit, "nnet")) {
      stop("Obsolete neural_discrete fit; refit using the nnet engine.", call. = FALSE)
    }
    n <- nrow(x)
    intervals <- length(fit$time) - 1L
    new <- cbind(scaled[rep(seq_len(n), each = intervals), , drop = FALSE],
      .sl_interval = rep(seq_len(intervals) / intervals, times = n))
    hazards <- matrix(stats::predict(fit$fit, newdata = new, type = "raw"),
      nrow = n, ncol = intervals, byrow = TRUE)
    curves <- matrix(1, nrow = n, ncol = intervals + 1L)
    for (j in seq_len(intervals)) curves[, j + 1L] <- curves[, j] * (1 - hazards[, j])
    if (type == "risk") {
      # Step-function restricted mean on the training interval grid.
      return(-drop(curves[, -ncol(curves), drop = FALSE] %*% diff(fit$time)))
    }
    if (type != "survival") stop("Neural engine does not provide '", type, "'.", call. = FALSE)
    .reference_times(times)
    index <- as.integer(floor(.neural_discrete_position(pmin(times, fit$horizon), fit$time))) + 1L
    out <- curves[, index, drop = FALSE]
    out[, times > fit$horizon] <- NA_real_
    return(out)
  }
  if (identical(fit$backend, "native")) {
    lp <- .neural_cox_forward(fit$weights, scaled)$lp
  } else {
    # Old adapters saved ordinary arrays, so prediction needs no old runtime.
    weights <- list(w = t(fit$state[["0.weight"]]), bias = fit$state[["0.bias"]],
      output = -as.numeric(fit$state[["2.weight"]]), intercept = -fit$state[["2.bias"]])
    if (is.null(weights$w) || !length(weights$output)) stop("Unsupported legacy neural Cox state.", call. = FALSE)
    lp <- .neural_cox_forward(weights, scaled)$lp
  }
  if (type == "risk") return(lp)
  if (type != "survival") stop("Neural engine does not provide '", type, "'.", call. = FALSE)
  .reference_times(times)
  log_h <- if (!is.null(fit$baseline$logcumhaz)) {
    .reference_step(fit$baseline$time, fit$baseline$logcumhaz, times, initial = -Inf)
  } else log(.reference_step(fit$baseline$time, fit$baseline$cumhaz, times, initial = 0))
  out <- exp(-exp(outer(lp - fit$baseline$center, log_h, "+")))
  out[, times > object$max_time] <- NA_real_
  out
}

.neural_discrete_position <- function(time, grid) {
  intervals <- length(grid) - 1L
  position <- (time / grid[length(grid)]) * intervals
  nearest <- round(position)
  # Snap only floating-point roundoff, not substantively incomplete intervals.
  tolerance <- 8 * .Machine$double.eps * pmax(1, abs(position))
  snap <- abs(position - nearest) <= tolerance
  position[snap] <- nearest[snap]
  pmin(intervals, pmax(0, position))
}

.neural_discrete_periods <- function(y, grid) {
  intervals <- length(grid) - 1L
  position <- .neural_discrete_position(y[, 1L], grid)
  event <- as.integer(y[, 2L])
  completed <- as.integer(floor(position))
  event_interval <- ifelse(event == 1L, pmax(1L, as.integer(ceiling(position))), 0L)
  counts <- ifelse(event == 1L, event_interval, completed)
  if (sum(counts) > .Machine$integer.max) {
    stop("Person-period expansion exceeds the nnet row limit; reduce 'intervals'.", call. = FALSE)
  }
  observation <- rep.int(seq_len(nrow(y)), counts)
  interval <- sequence(counts)
  response <- as.integer(event[observation] == 1L & interval == event_interval[observation])
  partial <- event == 0L & position > completed
  zero <- which(event == 0L & counts == 0L)
  list(expansion = data.frame(observation = observation, interval = interval, response = response),
    exposure = data.frame(observation = seq_len(nrow(y)), time = as.numeric(y[, 1L]),
      event = event, completed_intervals = counts - event, event_interval = event_interval,
      likelihood_rows = counts, coarsened_time = grid[counts + 1L],
      partial_censor_interval_dropped = partial),
    diagnostics = list(n_zero_exposure_censored = length(zero),
      zero_exposure_censored = zero, n_partial_censor_intervals_dropped = sum(partial),
      n_likelihood_rows = length(response), n_intervals = intervals,
      zero_exposure_action = "No Bernoulli likelihood term for censored observations with no completed interval."))
}

.neural_breslow <- function(lp, y) {
  center <- max(lp)
  index <- order(y[, 1L])
  time <- y[index, 1L]
  first <- !duplicated(time)
  denominator <- rev(cumsum(rev(exp(lp[index] - center))))[first]
  events <- as.numeric(rowsum(y[index, 2L], cumsum(first), reorder = FALSE))
  list(time = time[first], cumhaz = cumsum(events / denominator), center = center)
}

