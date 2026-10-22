.neural_logcumsumexp <- function(x) {
  top <- max(x)
  if (top == -Inf) return(rep(-Inf, length(x)))
  mass <- cumsum(exp(x - top))
  finite <- x[is.finite(x)]
  if (all(mass > 0 | cumsum(is.finite(x)) == 0) && diff(range(finite)) < 500) {
    return(log(mass) + top)
  }
  out <- numeric(length(x))
  previous <- -Inf
  for (i in seq_along(x)) {
    high <- max(previous, x[i])
    previous <- if (high == -Inf) -Inf else high + log1p(exp(-abs(previous - x[i])))
    out[i] <- previous
  }
  out
}

.neural_cox_groups <- function(y, ties) {
  index <- order(y[, 1L], decreasing = TRUE)
  time <- y[index, 1L]
  event <- as.numeric(y[index, 2L])
  id <- cumsum(!duplicated(time))
  last <- which(!duplicated(time, fromLast = TRUE))
  deaths <- as.numeric(rowsum(event, id, reorder = FALSE))
  active <- which(deaths > 0)
  list(index = index, id = id, last = last, time = time[last], event = event,
    deaths = deaths, active = active, total = sum(event), ties = ties,
    expanded = rep(active, deaths[active]),
    fractions = (sequence(deaths[active]) - 1) / rep(deaths[active], deaths[active]))
}

# All members of a tied time, including censorings, enter its risk set.
# Efron's fractions remove event mass only; censorings are not deaths.
.neural_cox_loss <- function(lp, groups) {
  ordered <- lp[groups$index]
  if (any(!is.finite(ordered))) stop("Non-finite neural Cox log risks.", call. = FALSE)
  log_risk <- .neural_logcumsumexp(ordered)[groups$last]
  top <- max(ordered)
  mass <- exp(ordered - top)
  event_mass <- as.numeric(rowsum(mass * groups$event, groups$id, reorder = FALSE))
  ratio <- exp(log(event_mass) + top - log_risk)
  # A rare wide-range fallback avoids underflow in event mass as well.
  underflow <- groups$active[event_mass[groups$active] == 0]
  for (g in underflow) {
    v <- ordered[groups$id == g & groups$event == 1]
    high <- max(v)
    ratio[g] <- exp(high + log(sum(exp(v - high))) - log_risk[g])
  }
  a <- groups$deaths
  b <- numeric(length(a))
  correction <- 0
  if (groups$ties == "efron") {
    fraction <- groups$fractions
    denominator <- 1 - fraction * ratio[groups$expanded]
    if (any(!is.finite(denominator)) || any(denominator <= 0)) {
      stop("Invalid Efron risk denominator.", call. = FALSE)
    }
    a[groups$active] <- as.numeric(rowsum(1 / denominator, groups$expanded, reorder = FALSE))
    b[groups$active] <- as.numeric(rowsum(fraction / denominator, groups$expanded, reorder = FALSE))
    correction <- sum(log(denominator))
  }
  log_a <- log(a) - log_risk
  suffix <- rev(.neural_logcumsumexp(rev(log_a)))
  gradient <- exp(ordered + suffix[groups$id]) - groups$event -
    groups$event * exp(ordered - log_risk[groups$id]) * b[groups$id]
  value <- (sum(groups$deaths * (log_risk - top)) + correction -
    sum(groups$event * (ordered - top))) / groups$total
  restored <- numeric(length(lp))
  restored[groups$index] <- gradient / groups$total
  list(value = value, gradient = restored, log_increment = log_a)
}

.neural_cox_unpack <- function(theta, p, hidden) {
  m <- p * hidden
  list(w = matrix(theta[seq_len(m)], p, hidden),
    bias = theta[m + seq_len(hidden)],
    output = theta[m + hidden + seq_len(hidden)], intercept = theta[length(theta)])
}

.neural_cox_initial <- function(p, hidden) {
  c(stats::runif(p * hidden, -1 / sqrt(p), 1 / sqrt(p)),
    stats::runif(hidden, -1 / sqrt(p), 1 / sqrt(p)),
    stats::runif(hidden, -1 / sqrt(hidden), 1 / sqrt(hidden)),
    stats::runif(1, -1 / sqrt(hidden), 1 / sqrt(hidden)))
}

.neural_cox_forward <- function(weights, x) {
  hidden <- tanh(sweep(x %*% weights$w, 2L, weights$bias, "+"))
  list(hidden = hidden, lp = as.vector(hidden %*% weights$output + weights$intercept))
}

.neural_cox_objective <- function(theta, x, groups, hidden, decay) {
  weights <- .neural_cox_unpack(theta, ncol(x), hidden)
  forward <- .neural_cox_forward(weights, x)
  loss <- .neural_cox_loss(forward$lp, groups)
  delta <- (loss$gradient %o% weights$output) * (1 - forward$hidden^2)
  gradient <- c(as.vector(crossprod(x, delta)), colSums(delta),
    as.vector(crossprod(forward$hidden, loss$gradient)), sum(loss$gradient)) + decay * theta
  list(value = loss$value + decay * sum(theta^2) / 2,
    data_loss = loss$value, gradient = gradient)
}

.fit_neural_cox_native <- function(x, y, params) {
  if (params$threads != 1L) stop("Native neural Cox does not control BLAS threads; use threads = 1.", call. = FALSE)
  count <- (as.double(ncol(x)) + 2) * params$hidden + 1
  if (count > params$max_weights) stop("Neural network exceeds 'max_weights'.", call. = FALSE)
  moments <- .design_moments(x)
  center <- moments$center
  scale <- moments$scale
  scaled <- .scaled_design(x, moments)
  groups <- .neural_cox_groups(y, params$ties)
  theta <- .neural_cox_initial(ncol(x), params$hidden)
  initial <- theta
  m <- v <- numeric(length(theta))
  history <- numeric(params$epochs)
  for (epoch in seq_len(params$epochs)) {
    current <- .neural_cox_objective(theta, scaled, groups, params$hidden, params$decay)
    if (!is.finite(current$value) || any(!is.finite(current$gradient))) {
      stop("Native neural Cox training produced a non-finite objective/gradient.", call. = FALSE)
    }
    history[epoch] <- current$value
    m <- .9 * m + .1 * current$gradient
    v <- .999 * v + .001 * current$gradient^2
    theta <- theta - params$learning_rate * (m / (1 - .9^epoch)) /
      (sqrt(v / (1 - .999^epoch)) + 1e-8)
  }
  final <- .neural_cox_objective(theta, scaled, groups, params$hidden, params$decay)
  if (!is.finite(final$value) || any(!is.finite(final$gradient))) {
    stop("Native neural Cox final objective/gradient is non-finite.", call. = FALSE)
  }
  weights <- .neural_cox_unpack(theta, ncol(x), params$hidden)
  lp <- .neural_cox_forward(weights, scaled)$lp
  loss <- .neural_cox_loss(lp, groups)
  reference <- max(lp)
  baseline <- list(time = rev(groups$time), center = reference,
    logcumhaz = .neural_logcumsumexp(rev(loss$log_increment + reference)),
    ties = params$ties)
  list(backend = "native", weights = weights, coefficients = theta, initial = initial,
    hidden = params$hidden, center = center, scale = scale, baseline = baseline,
    loss_history = history, final_loss = final$data_loss,
    diagnostics = list(backend = "native R", optimizer = "full-batch Adam",
      objective = final$value, penalty = params$decay * sum(theta^2) / 2,
      gradient_max = max(abs(final$gradient)), epochs = params$epochs,
      convergence_basis = "fixed_epoch_budget_not_convergence_certificate",
      ties = params$ties, baseline_ties = params$ties,
      tied_event_groups = sum(groups$deaths > 1),
      event_censor_tied_groups = sum(groups$deaths > 0 &
        tabulate(groups$id) > groups$deaths),
      weights = length(theta), observations = nrow(x)))
}
