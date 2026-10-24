.svm_learning_methods <- function() c("svm_rank", "svm_hybrid", "svm_ipcw")

.svm_learning_specs <- function() {
  common <- list(lambda = 0.01, kernel = "linear", tolerance = 1e-6,
    max_iter = 2000L, memory_limit_mb = 2048, prediction_tolerance = 1e-8)
  list(svm_rank = list(description = "Survival SVM with squared-hinge ranking loss",
      engine = "survlearning", predictions = "risk", defaults = common),
    svm_hybrid = list(description = "Survival SVM with ranking and squared censored regression",
      engine = "survlearning", predictions = "risk",
      defaults = c(common, list(rank_weight = 0.5))),
    svm_ipcw = list(description = "IPCW squared epsilon-insensitive SVR at a restricted horizon",
      engine = "survlearning", predictions = "risk",
      defaults = c(common, list(horizon = NULL, censoring_method = "km",
        min_g = 0.05, epsilon = 0))))
}

.svm_learning_parameters <- function(method, params) {
  if (!is.character(params$kernel) || length(params$kernel) != 1L ||
      is.na(params$kernel) || !params$kernel %in% c("linear", "additive"))
    stop("'kernel' must be linear or additive.", call. = FALSE)
  .positive_scalar(params$memory_limit_mb, "memory_limit_mb")
  .positive_scalar(params$prediction_tolerance, "prediction_tolerance", zero = TRUE)
  if (method == "svm_hybrid") {
    .positive_scalar(params$rank_weight, "rank_weight")
    if (params$rank_weight >= 1) stop("'rank_weight' must be strictly between zero and one; use svm_rank for pure ranking.", call. = FALSE)
  }
  if (method == "svm_ipcw") {
    .positive_scalar(params$horizon, "horizon")
    .positive_scalar(params$min_g, "min_g")
    if (params$min_g > 1) stop("'min_g' must not exceed one.", call. = FALSE)
    .positive_scalar(params$epsilon, "epsilon", zero = TRUE)
    if (!is.character(params$censoring_method) || length(params$censoring_method) != 1L ||
        is.na(params$censoring_method) || !params$censoring_method %in% c("km", "cox"))
      stop("'censoring_method' must be km or cox.", call. = FALSE)
  }
  params
}

.svm_rank_loss <- function(score, y) {
  .Call("sl_svm_rank", as.numeric(score), as.numeric(y[, 1L]),
    as.numeric(y[, 2L]), PACKAGE = "survlearning")
}

.svm_learning_design <- function(x, kernel) {
  out <- .svm_primal_design(x, kernel)
  out$segments <- if (kernel == "additive") lengths(out$knots) - 1L else NULL
  out$increments <- if (kernel == "additive")
    unlist(lapply(out$knots, function(k) sqrt(2 * diff(k) / diff(range(k)))), use.names = FALSE) else NULL
  out
}

# Whiten the exact additive RKHS penalty through all successive knot increments.
# Neither the design nor its transpose needs a dense cumulative-basis matrix.
.svm_learning_map <- function(a, design, transpose = FALSE) {
  if (design$kernel == "linear") return(a)
  answer <- numeric(length(a))
  start <- 0L
  for (size in design$segments) {
    index <- start + seq_len(size)
    answer[index] <- if (transpose) design$increments[index] * rev(cumsum(rev(a[index]))) else
      cumsum(design$increments[index] * a[index])
    start <- start + size
  }
  answer
}

.svm_ipcw_response <- function(x, y, params) {
  horizon <- params$horizon
  if (horizon > max(y[, 1L])) stop("'horizon' exceeds training follow-up.", call. = FALSE)
  data <- .engine_data(x, y)
  censor <- surv_censor_fit(.engine_formula(), data, method = params$censoring_method)
  # T restricted to horizon is observed also for units followed to that horizon.
  observed <- y[, 1L] >= horizon | y[, 2L] == 1
  target <- pmin(y[, 1L], horizon) / horizon
  support <- surv_censor_predict(censor, data, rep(horizon, nrow(x)),
    left = TRUE, pairwise = TRUE)
  g <- surv_censor_predict(censor, data[observed, , drop = FALSE],
    pmin(y[observed, 1L], horizon), left = TRUE, pairwise = TRUE)
  if (any(!is.finite(c(support, g))) || any(c(support, g) < params$min_g))
    stop("Insufficient censoring support for IPCW at the requested horizon. No weights were clipped or observations removed.", call. = FALSE)
  weights <- numeric(nrow(x))
  weights[observed] <- 1 / g
  list(target = target, weights = weights, censoring = censor,
    diagnostics = list(horizon = horizon, censoring_method = params$censoring_method,
      observed_targets = sum(observed), minimum_g = min(c(support, g)),
      maximum_weight = max(weights), effective_sample_size = sum(weights)^2 / sum(weights^2),
      assumptions = if (params$censoring_method == "km")
        "Marginal censoring weights require G(t|X)=G(t), positive support, and conditionally independent censoring." else
        "Conditional Cox censoring requires correct functional form/PH, positive support, and conditionally independent censoring; it does not model a deterministic administrative endpoint exactly."))
}

.svm_learning_objective <- function(method, design, y, params, response = NULL) {
  p <- ncol(design$z)
  n <- nrow(y)
  with_intercept <- method != "svm_rank"
  target <- if (method == "svm_ipcw") response$target else y[, 1L] / max(y[, 1L])
  last <- answer <- NULL
  function(a) {
    if (identical(a, last)) return(answer)
    coefficients <- .svm_learning_map(a[seq_len(p)], design)
    score <- as.numeric(design$z %*% coefficients)
    if (with_intercept) score <- score + a[p + 1L]
    loss <- 0
    gradient <- numeric(n)
    pairs <- NA_real_
    if (method != "svm_ipcw") {
      ranking <- .svm_rank_loss(score, y)
      weight <- if (method == "svm_rank") 1 else params$rank_weight
      loss <- weight * ranking$loss
      gradient <- weight * ranking$gradient
      pairs <- ranking$pairs
    }
    if (method == "svm_hybrid") {
      residual <- score + target
      residual[y[, 2L] == 0] <- pmax(0, residual[y[, 2L] == 0])
      loss <- loss + (1 - params$rank_weight) * mean(residual^2)
      gradient <- gradient + (1 - params$rank_weight) * 2 * residual / n
    }
    if (method == "svm_ipcw") {
      residual <- score + target
      active <- pmax(abs(residual) - params$epsilon, 0)
      loss <- mean(response$weights * active^2)
      gradient <- 2 * response$weights * active * sign(residual) / n
    }
    g <- .svm_learning_map(as.numeric(Matrix::crossprod(design$z, gradient)),
      design, transpose = TRUE) + params$lambda * a[seq_len(p)]
    if (with_intercept) g <- c(g, sum(gradient))
    answer <<- list(value = loss + params$lambda * sum(a[seq_len(p)]^2) / 2,
      gradient = g, pairs = pairs, loss = loss)
    last <<- a
    answer
  }
}

.svm_learning_acceptance <- function(result, evaluated, method, params) {
  stationarity <- sqrt(sum(evaluated$gradient^2))
  gap_bound <- if (method == "svm_rank") stationarity^2 / (2 * params$lambda) else NA_real_
  # Roundoff can stop the line search after the requested accuracy is attained.
  line_search <- identical(result$convergence, 52L) &&
    is.character(result$message) && length(result$message) == 1L &&
    !is.na(result$message) && grepl("ABNORMAL_TERMINATION_IN_LNSRCH", result$message, fixed = TRUE)
  accepted <- (result$convergence == 0L || line_search) &&
    all(is.finite(result$par)) && is.finite(evaluated$value) &&
    is.finite(stationarity) &&
    stationarity <= params$tolerance * max(1, sqrt(sum(result$par^2)))
  if (method == "svm_rank") accepted <- accepted &&
    gap_bound <= params$tolerance * max(1, abs(evaluated$value))
  list(accepted = isTRUE(accepted), gradient_norm = stationarity,
    gap_bound = gap_bound, status = if (!isTRUE(accepted)) "reported_failure" else
      if (line_search) "stationarity_verified" else "reported_success")
}

.svm_learning_fit <- function(method, x, y, params, cache = NULL) {
  estimate <- 8 * nrow(x) * (40 * ncol(x) + 120) / 1024^2
  .svm_memory_check(estimate, params)
  key <- paste0("learning_design_", params$kernel)
  hit <- !is.null(cache) && exists(key, cache, inherits = FALSE)
  design <- if (hit) get(key, cache, inherits = FALSE) else .svm_learning_design(x, params$kernel)
  if (!hit && !is.null(cache)) assign(key, design, cache)
  response <- if (method == "svm_ipcw") .svm_ipcw_response(x, y, params) else NULL
  objective <- .svm_learning_objective(method, design, y, params, response)
  p <- ncol(design$z)
  initial <- rep(0, p)
  if (method != "svm_rank") initial <- c(initial, -1)
  result <- stats::optim(initial, function(a) objective(a)$value,
    function(a) objective(a)$gradient, method = "L-BFGS-B",
    control = list(maxit = params$max_iter, factr = 10,
      pgtol = params$tolerance * min(1, params$lambda) / max(10, sqrt(length(initial))), lmm = 20L))
  evaluated <- objective(result$par)
  checked <- .svm_learning_acceptance(result, evaluated, method, params)
  stationarity <- checked$gradient_norm
  gap_bound <- checked$gap_bound
  if (!checked$accepted) stop("Survival SVM did not satisfy the convergence/stationarity checks (",
    method, ", optimizer status ", result$convergence, ", gradient norm ",
    signif(stationarity, 4), "). No model was accepted.", call. = FALSE)
  coefficients <- .svm_learning_map(result$par[seq_len(p)], design)
  coefficients <- c(coefficients, if (method == "svm_rank") 0 else result$par[p + 1L])
  structure(list(formulation = method, kernel = params$kernel, coefficients = coefficients,
    knots = design$knots, convergence = 0L, response_center = 0,
    prediction_resolution = params$prediction_tolerance,
    censoring = if (is.null(response)) NULL else response$censoring,
    diagnostics = list(status = checked$status, observations = nrow(x),
      events = sum(y[, 2L]), comparable_pairs = evaluated$pairs,
      feature_dimension = p, objective = evaluated$value, loss = evaluated$loss,
      gradient_norm = stationarity, strong_convexity_gap_bound = gap_bound,
      optimizer_status = result$convergence, optimizer_message = result$message,
      function_evaluations = unname(result$counts[1L]),
      workspace_estimate_mb = estimate, cache_hit = hit,
      time_normalization = if (method == "svm_hybrid") max(y[, 1L]) else if (method == "svm_ipcw") params$horizon else NA_real_,
      ipcw = if (is.null(response)) NULL else response$diagnostics)),
    class = "survlearning_svm_learning")
}

.svm_learning_predict <- function(object, x) {
  if (!identical(object$convergence, 0L)) stop("Cannot predict from a failed survival SVM fit.", call. = FALSE)
  result <- .svm_primal_predict(object, x, raw = TRUE)
  if (object$prediction_resolution > 0)
    result <- object$prediction_resolution * round(result / object$prediction_resolution)
  if (any(!is.finite(result))) stop("Non-finite survival SVM predictions.", call. = FALSE)
  result
}
