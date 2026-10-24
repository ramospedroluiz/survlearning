# Exact finite representation of the censored absolute-loss regression primal.
# All distinct training coordinates are retained for the additive kernel.
.svm_primal_design <- function(x, kernel) {
  if (kernel == "linear") return(list(z = Matrix::Matrix(x, sparse = TRUE),
    p = Matrix::Diagonal(ncol(x)), kernel = kernel, knots = NULL))
  blocks <- precision <- knots <- vector("list", ncol(x))
  for (j in seq_len(ncol(x))) {
    knots[[j]] <- sort(unique(x[, j]))
    span <- diff(range(knots[[j]]))
    if (!is.finite(span) || span <= 0) stop("Additive training columns must vary.", call. = FALSE)
    u <- (knots[[j]] - knots[[j]][1L]) / span
    k <- length(u) - 1L
    index <- match(x[, j], knots[[j]]) - 1L
    blocks[[j]] <- Matrix::sparseMatrix(i = which(index > 0L),
      j = index[index > 0L], x = 1, dims = c(nrow(x), k))
    difference <- if (k == 1L) Matrix::Diagonal(1L) else
      Matrix::bandSparse(k, k, k = c(-1, 0),
        diagonals = list(rep(-1, k - 1L), rep(1, k)))
    precision[[j]] <- Matrix::crossprod(difference,
      Matrix::Diagonal(x = 1 / (2 * diff(u))) %*% difference)
  }
  list(z = do.call(cbind, blocks), p = methods::as(Matrix::bdiag(precision), "symmetricMatrix"),
    knots = knots, kernel = kernel)
}

.svm_primal_prepare <- function(x, y, params) {
  start <- proc.time()[["elapsed"]]
  n <- nrow(x)
  # Conservative sparse workspace planning allowance; not measured peak RSS.
  estimate <- 8 * n * (40 * ncol(x) + 120) / 1024^2
  .svm_memory_check(estimate, params)
  design <- .svm_primal_design(x, params$kernel)
  event <- which(y[, 2L] == 1)
  ne <- length(event)
  z <- cbind(design$z, rep(1, n))
  p <- ncol(z)
  scale <- c(1 / sqrt(Matrix::diag(design$p)), 1)
  zs <- z %*% Matrix::Diagonal(x = scale)
  zero <- function(nr, nc) Matrix::sparseMatrix(i = integer(), j = integer(), dims = c(nr, nc))
  a <- rbind(cbind(-zs, -Matrix::Diagonal(n), zero(n, ne)),
    cbind(zs[event, , drop = FALSE], zero(ne, n), -Matrix::Diagonal(ne)),
    cbind(zero(n + ne, p), -Matrix::Diagonal(n + ne)))
  h <- Matrix::bdiag(Matrix::Diagonal(x = scale[-p]) %*% design$p %*%
    Matrix::Diagonal(x = scale[-p]), 0, zero(n + ne, n + ne))
  center <- max(y[, 1L])
  target <- y[, 1L] - center
  list(design = design, z = z, scale = scale,
    a = methods::as(a, "CsparseMatrix"),
    h = methods::as(methods::as(h, "symmetricMatrix"), "CsparseMatrix"),
    b = c(-target, target[event], rep(0, n + ne)), center = center,
    event = event, y = y, columns = colnames(x), n = n, p = p,
    estimated_workspace_mb = estimate,
    prepare_seconds = unname(proc.time()[["elapsed"]] - start))
}

.svm_fit <- function(x, y, params, cache = NULL) {
  key <- paste0("censored_primal_", params$kernel)
  hit <- !is.null(cache) && exists(key, cache, inherits = FALSE)
  problem <- if (hit) get(key, cache, inherits = FALSE) else .svm_primal_prepare(x, y, params)
  .svm_memory_check(problem$estimated_workspace_mb, params)
  if (!hit && !is.null(cache)) assign(key, problem, cache)
  n <- problem$n; p <- problem$p; ne <- length(problem$event)
  q <- c(rep(0, p), rep(params$cost, n + ne))
  start <- proc.time()[["elapsed"]]
  if (params$solver == "clarabel") {
    result <- clarabel::clarabel(A = problem$a, b = problem$b, q = q, P = problem$h,
      cones = list(l = as.integer(nrow(problem$a))),
      control = clarabel::clarabel_control(verbose = FALSE, max_threads = 1L,
        tol_gap_abs = params$tolerance, tol_gap_rel = params$tolerance,
        tol_feas = params$tolerance, max_iter = as.integer(params$max_iter)))
    solved <- result$status == 2L
    solution <- result$x; multipliers <- result$z
    iterations <- result$iterations
    solver_status <- result$status
  } else {
    model <- osqp::osqp(P = problem$h, q = q, A = problem$a,
      l = rep(-Inf, length(problem$b)), u = problem$b,
      pars = osqp::osqpSettings(verbose = FALSE, eps_abs = params$tolerance,
        eps_rel = params$tolerance, max_iter = as.integer(params$max_iter), polishing = TRUE))
    result <- model@Solve()
    solved <- result$info$status_val == 1L
    solution <- result$x; multipliers <- result$y
    iterations <- result$info$iter
    solver_status <- result$info$status_val
  }
  if (!solved || any(!is.finite(solution))) stop("Censored SVM solver did not converge (",
    params$solver, " status ", solver_status, "). No model was accepted.", call. = FALSE)
  coefficients <- solution[seq_len(p)] * problem$scale
  coefficients[p] <- coefficients[p] + problem$center
  # The unpenalized intercept can have an interval of minimizers. Choose its
  # midpoint deterministically instead of retaining a solver-specific offset.
  residual <- y[, 1L] - as.vector(problem$design$z %*% coefficients[-p])
  order <- order(residual)
  cumulative <- cumsum(1 + y[order, 2L])
  index <- which(cumulative >= n)[1L]
  coefficients[p] <- if (cumulative[index] == n && index < n)
    mean(residual[order[c(index, index + 1L)]]) else residual[order[index]]
  fitted <- as.vector(problem$z %*% coefficients)
  cdual <- multipliers[seq_len(n)]
  cdual[problem$event] <- cdual[problem$event] - multipliers[n + seq_len(ne)]
  g <- as.vector(Matrix::crossprod(problem$design$z, cdual))
  dual_coefficients <- as.vector(Matrix::solve(problem$design$p, g))
  penalty <- sum(coefficients[-p] * as.vector(problem$design$p %*% coefficients[-p])) / 2
  loss <- sum(pmax(0, y[, 1L] - fitted)) +
    sum(pmax(0, fitted[problem$event] - y[problem$event, 1L]))
  primal <- penalty + params$cost * loss
  dual <- sum(y[, 1L] * cdual) - sum(g * dual_coefficients) / 2
  violation <- max(0, as.vector(problem$a %*% solution) - problem$b)
  dual_violation <- max(abs(sum(cdual)), -params$cost * y[, 2L] - cdual,
    cdual - params$cost, 0)
  relative_gap <- abs(primal - dual) / max(1, abs(primal), abs(dual))
  feasible <- violation <= max(1e-6, params$tolerance * 100 * max(y[, 1L])) &&
    dual_violation <= max(1e-6, params$tolerance * 100 * n) && relative_gap <= 1e-5
  if (!feasible) stop("Censored SVM failed the independent primal/dual certificate.", call. = FALSE)
  structure(list(formulation = "censored_primal", coefficients = coefficients,
    knots = problem$design$knots, kernel = params$kernel,
    sv = matrix(numeric(), 0L, ncol(x), dimnames = list(NULL, colnames(x))),
    response_center = problem$center,
    prediction_resolution = params$prediction_tolerance * max(abs(y[, 1L])),
    solver = params$solver, convergence = 0L,
    diagnostics = list(status = "reported_success", feasible = feasible,
      solver_status = solver_status, iterations = iterations, observations = n,
      events = ne, feature_dimension = p - 1L, objective = primal, dual_objective = dual,
      relative_gap = relative_gap, constraint_violation = violation,
      primal_residual = violation, dual_residual = dual_violation,
      duality_gap = abs(primal - dual),
      dual_constraint_violation = dual_violation,
      stationarity = max(abs(as.vector(problem$design$p %*% coefficients[-p]) - g)),
      workspace_estimate_mb = problem$estimated_workspace_mb, cache_hit = hit,
      prepare_seconds = if (hit) 0 else problem$prepare_seconds,
      solver_and_reconstruction_seconds = unname(proc.time()[["elapsed"]] - start),
      solver_version = as.character(utils::packageVersion(params$solver))),
    dual = cdual), class = "survlearning_svm")
}

.svm_primal_predict <- function(object, x, raw = FALSE) {
  if (object$kernel == "linear") {
    out <- as.vector(cbind(x, 1) %*% object$coefficients)
  } else {
    out <- rep(utils::tail(object$coefficients, 1L), nrow(x))
    offset <- 0L
    for (j in seq_along(object$knots)) {
      k <- length(object$knots[[j]]) - 1L
      values <- c(0, object$coefficients[offset + seq_len(k)])
      out <- out + stats::approx(object$knots[[j]], values, xout = x[, j], rule = 2)$y
      offset <- offset + k
    }
  }
  # Resolve solver-scale near ties before risk ordering, using a training-fixed
  # grid. Set prediction_tolerance = 0 to inspect unrounded regression scores.
  if (!raw && object$prediction_resolution > 0) {
    out <- object$response_center + object$prediction_resolution *
      round((out - object$response_center) / object$prediction_resolution)
  }
  out
}
