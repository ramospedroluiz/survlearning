# The regression QP, five-decimal support coefficients, and intercept follow
# survivalsvm 0.0.6 (regFit, GPL-2 | GPL-3). No ranking loss or subsampling is used.
.svm_kernel <- function(x, kernel, newx = NULL) {
  if (is.null(newx)) newx <- x
  if (kernel == "linear") return(tcrossprod(x, newx))
  if (kernel != "additive") stop("Unknown SVM kernel.", call. = FALSE)
  span <- vapply(seq_len(ncol(x)), function(j) max(x[, j]) - min(x[, j]), 0)
  if (!length(span) || any(!is.finite(span) | span <= 0)) {
    stop("The additive kernel cannot be applied to a constant column or non-finite range.", call. = FALSE)
  }
  # The left argument defines the ranges, including support-only prediction.
  transposed <- t(newx)
  out <- matrix(0, nrow(x), nrow(newx))
  for (i in seq_len(nrow(x))) {
    out[i, ] <- colSums((span - abs(transposed - x[i, ])) / span)
  }
  out
}

.svm_prepare <- function(x, y, params) {
  start <- proc.time()[["elapsed"]]
  n <- nrow(x)
  if (params$solver != "quadprog") {
    minimum <- if (params$solver == "osqp") "1.0.0" else "0.11.3"
    if (params$solver != "newton" && (!requireNamespace(params$solver, quietly = TRUE) ||
        utils::packageVersion(params$solver) < minimum)) {
      stop("solver = '", params$solver, "' needs ", params$solver, " >= ", minimum,
        "; install.packages('", params$solver, "').", call. = FALSE)
    }
    # Budget the design first; additive spectra have adaptive workspace checks.
    r <- min(n, 2 * ncol(x))
    estimate <- 8 * (n * ncol(x) * 8 + n * r * 24 + r^2 * 4) / 1024^2
    .svm_memory_check(estimate, params)
    groups <- if (params$kernel == "additive" && params$collapse_duplicates)
      .svm_duplicate_groups(x, y) else NULL
    compact <- if (params$kernel == "linear") .svm_linear_hessian(x, y[, 2L]) else
      .svm_additive_hessian(x, y[, 2L], params, groups)
    if (params$kernel == "additive") estimate <- compact$workspace_estimate_mb
    out <- list(compact = compact, groups = groups)
  } else {
    for (package in c("Matrix", "quadprog")) {
      if (!requireNamespace(package, quietly = TRUE)) {
        stop("solver = 'quadprog' needs ", package, "; install.packages('",
          package, "').", call. = FALSE)
      }
    }
    # Lower-bound planning estimate for live dense arrays, not measured RSS.
    estimate <- 8 * (2 * n)^2 * 8 / 1024^2
    .svm_memory_check(estimate, params)
    k <- .svm_kernel(x, params$kernel)
    d <- rbind(cbind(k, -y[, 2L] * k), cbind(-y[, 2L] * k, k))
    repaired <- Matrix::nearPD(d)
    out <- list(h = as.matrix(repaired$mat), nearpd_converged = repaired$converged)
  }
  out$x <- x
  out$y <- y
  out$kernel <- params$kernel
  out$solver <- params$solver
  out$estimated_workspace_mb <- estimate
  out$prepare_seconds <- unname(proc.time()[["elapsed"]] - start)
  out
}

.svm_memory_check <- function(estimate, params) {
  if (estimate > params$memory_limit_mb) {
    stop(sprintf(paste0("SVM workspace planning estimate %.0f MiB exceeds memory_limit_mb = %.0f. ",
      "No rows were removed. Choose a compact solver ('newton', 'clarabel' or 'osqp'), ",
      "or explicitly raise the limit on a machine with sufficient RAM."),
      estimate, params$memory_limit_mb), call. = FALSE)
  }
}

.svm_linear_hessian <- function(x, delta) {
  n <- nrow(x)
  p <- ncol(x)
  s <- svd(cbind(x, delta * x))
  keep <- s$d > max(s$d) * 1e-12
  u <- s$u[, keep, drop = FALSE]
  b <- s$d[keep] * t(s$v[, keep, drop = FALSE])
  i <- seq_len(p)
  bx <- b[, i, drop = FALSE]
  bz <- b[, p + i, drop = FALSE]
  base <- tcrossprod(bx)
  cross <- (tcrossprod(bx, bz) + tcrossprod(bz, bx)) / 2
  plus <- eigen(base + cross, symmetric = TRUE)
  minus <- eigen(base - cross, symmetric = TRUE)
  top <- max(plus$values, minus$values)
  kp <- plus$values > 1e-6 * top
  km <- minus$values > 1e-6 * top
  qp <- u %*% plus$vectors[, kp, drop = FALSE] / sqrt(2)
  qm <- u %*% minus$vectors[, km, drop = FALSE] / sqrt(2)
  q <- cbind(rbind(qp, -qp), rbind(qm, qm))
  d <- c(plus$values[kp], minus$values[km])
  .svm_project_hessian(q, d, top)
}

.svm_project_hessian <- function(q, d, top) {
  n <- nrow(q)
  eps <- 1e-8 * top
  # Match nearPD's eigenvalue cutoff, positive floor and diagonal rescaling.
  # With neither corr nor keepDiag, its repeated PSD projection is unchanged.
  old_diag <- as.vector((q * q) %*% d)
  scale <- sqrt(pmax(eps, old_diag) /
    (old_diag + eps * (1 - rowSums(q * q))))
  list(diagonal = eps * scale^2,
    factor = scale * sweep(q, 2L, sqrt(d - eps), `*`),
    rank = length(d), eigen_floor = eps)
}

.svm_optimization_data <- function(problem) {
  y <- problem$y
  groups <- problem$groups
  if (is.null(groups)) {
    n <- nrow(y)
    root <- rep(1, 2L * n)
    map <- seq_len(2L * n)
  } else {
    y <- y[groups$representatives, , drop = FALSE]
    n <- nrow(y)
    root <- rep(sqrt(groups$count), 2L)
    map <- c(groups$id, n + groups$id)
  }
  list(m = 2L * n, root = root, map = map,
    q = c(-y[, 1L], y[, 2L] * y[, 1L]) * root,
    a = c(rep(-1, n), y[, 2L]) * root)
}

.svm_workspace <- function(problem, params) {
  switch(params$solver, osqp = .svm_osqp_model(problem, params),
    clarabel = .svm_clarabel_model(problem, params),
    newton = .svm_newton_model(problem))
}

.svm_osqp_model <- function(problem, params) {
  opt <- .svm_optimization_data(problem)
  m <- opt$m
  l <- problem$compact$factor
  r <- ncol(l)
  zero <- Matrix::Matrix(0, m, r, sparse = TRUE)
  a <- rbind(
    cbind(Matrix::Matrix(opt$a, nrow = 1L, sparse = TRUE),
      Matrix::Matrix(0, 1L, r, sparse = TRUE)),
    cbind(Matrix::Diagonal(m), zero),
    cbind(Matrix::Matrix(t(l), sparse = TRUE), -Matrix::Diagonal(r)))
  model <- osqp::osqp(
    P = Matrix::Diagonal(x = c(problem$compact$diagonal, rep(1, r))),
    q = c(opt$q, rep(0, r)),
    A = a, l = rep(0, 1L + m + r),
    u = c(0, params$cost * opt$root, rep(0, r)),
    pars = osqp::osqpSettings(verbose = FALSE, eps_abs = params$tolerance,
      eps_rel = params$tolerance, max_iter = as.integer(params$max_iter),
      polishing = TRUE, adaptive_rho_interval = 50L))
  list(model = model, m = m, rank = r, root = opt$root)
}

.svm_clarabel_model <- function(problem, params) {
  opt <- .svm_optimization_data(problem)
  m <- opt$m
  l <- problem$compact$factor
  r <- ncol(l)
  zero <- Matrix::Matrix(0, m, r, sparse = TRUE)
  box <- cbind(Matrix::Diagonal(m), zero)
  a <- rbind(
    cbind(Matrix::Matrix(opt$a, nrow = 1L, sparse = TRUE),
      Matrix::Matrix(0, 1L, r, sparse = TRUE)),
    cbind(Matrix::Matrix(t(l), sparse = TRUE), -Matrix::Diagonal(r)), -box, box)
  # Release large temporary sparse blocks before the native solver copies A.
  if (length(l) > 8e6) gc(verbose = FALSE)
  model <- clarabel::clarabel_solver(A = a,
    b = c(rep(0, 1L + r + m), params$cost * opt$root),
    q = c(opt$q, rep(0, r)),
    P = Matrix::sparseMatrix(i = seq_len(m + r), j = seq_len(m + r),
      x = c(problem$compact$diagonal, rep(1, r)), symmetric = TRUE),
    cones = list(z = as.integer(1L + r), l = as.integer(2L * m)),
    control = clarabel::clarabel_control(verbose = FALSE, presolve_enable = FALSE,
      tol_gap_abs = params$tolerance, tol_gap_rel = params$tolerance,
      tol_feas = params$tolerance, max_iter = as.integer(params$max_iter), max_threads = 1L))
  list(model = model, m = m, rank = r, root = opt$root,
    tolerance = params$tolerance, max_iter = params$max_iter)
}

.svm_solve <- function(problem, params, workspace = NULL) {
  start <- proc.time()[["elapsed"]]
  n <- nrow(problem$x)
  m <- 2L * n
  q <- c(-problem$y[, 1L], problem$y[, 2L] * problem$y[, 1L])
  if (params$solver == "quadprog") {
    # Preserve the reference's constraint ordering as well as its objective.
    z <- quadprog::solve.QP(Dmat = problem$h, dvec = -q,
      Amat = cbind(c(rep(-1, n), problem$y[, 2L]), diag(m), diag(m), -diag(m)),
      bvec = c(0, rep(0, 2L * m), rep(-params$cost, m)), meq = 1L)$solution
    if (length(z) != m || any(!is.finite(z))) {
      stop("SVM solver returned a non-finite or invalid solution; no model was fitted.",
        call. = FALSE)
    }
    hz <- as.vector(problem$h %*% z)
    info <- list(status = "not_reported", code = NULL)
  } else {
    if (params$solver == "newton") {
      result <- .svm_newton_solve(problem, params, workspace)
      info <- result$info
    } else if (params$solver == "osqp") {
      if (is.null(workspace)) workspace <- .svm_osqp_model(problem, params)
      workspace$model@Update(u = c(0, params$cost * workspace$root, rep(0, workspace$rank)))
      workspace$model@UpdateSettings(list(eps_abs = params$tolerance,
        eps_rel = params$tolerance, max_iter = as.integer(params$max_iter)))
      result <- workspace$model@Solve()
      if (!identical(result$info$status_val, 1L)) {
        stop("SVM OSQP did not converge: ", result$info$status,
          ". Increase max_iter or inspect the problem; no result is accepted as converged.", call. = FALSE)
      }
      info <- list(status = "reported_success", code = result$info$status_val,
        iterations = result$info$iter, primal_residual = result$info$prim_res,
        dual_residual = result$info$dual_res, duality_gap = result$info$duality_gap)
    } else {
      if (is.null(workspace) || !identical(workspace$tolerance, params$tolerance) ||
          !identical(workspace$max_iter, params$max_iter)) workspace <- .svm_clarabel_model(problem, params)
      clarabel::solver_update(workspace$model,
        b = c(rep(0, 1L + workspace$rank + workspace$m), params$cost * workspace$root))
      result <- clarabel::solver_solve(workspace$model)
      if (result$status != 2L) stop("SVM Clarabel did not converge: status ", result$status,
        ". No reduced-accuracy solution is accepted.", call. = FALSE)
      info <- list(status = "reported_success", code = result$status,
        iterations = result$iterations, primal_residual = result$r_prim,
        dual_residual = result$r_dual, duality_gap = abs(result$obj_val - result$obj_val_dual))
    }
    opt <- .svm_optimization_data(problem)
    z_small <- result$x[seq_len(opt$m)]
    z <- (z_small / opt$root)[opt$map]
    l <- problem$compact$factor
    hz_small <- problem$compact$diagonal * z_small + as.vector(l %*% crossprod(l, z_small))
    hz <- (hz_small / opt$root)[opt$map]
  }
  if (length(z) != m || length(hz) != m ||
      any(!is.finite(z)) || any(!is.finite(hz))) {
    stop("SVM solver returned a non-finite or invalid solution; no model was fitted.",
      call. = FALSE)
  }
  objective <- sum(z * hz) / 2 + sum(q * z)
  violation <- max(0, -z, z - params$cost,
    abs(sum(c(rep(-1, n), problem$y[, 2L]) * z)))
  if (!is.finite(objective) || !is.finite(violation)) {
    stop("SVM solver returned non-finite objective or feasibility diagnostics; no model was fitted.",
      call. = FALSE)
  }
  if (violation > max(1e-5, 100 * params$tolerance)) {
    stop("SVM solver returned an infeasible solution (constraint violation = ",
      signif(violation, 4), "); no model was fitted. Consider a compact solver ",
      "or inspect the numerical conditioning.", call. = FALSE)
  }
  beta <- cbind(round(z[seq_len(n)], 5), round(z[n + seq_len(n)], 5))
  support <- which(rowSums(beta) != 0)
  if (!length(support)) stop("SVM has no support vectors at five-decimal rounding.", call. = FALSE)
  sv <- problem$x[support, , drop = FALSE]
  weights <- beta[support, 1L] - problem$y[support, 2L] * beta[support, 2L]
  if (params$kernel == "linear") {
    additive_prediction <- NULL
    linear_weights <- as.vector(crossprod(sv, weights))
    fitted <- as.vector(sv %*% linear_weights)
  } else {
    linear_weights <- NULL
    additive_prediction <- .svm_additive_prediction(sv, weights)
    fitted <- .svm_additive_predict(additive_prediction, sv)
  }
  structure(list(sv = sv, weights = weights, linear_weights = linear_weights,
    additive_prediction = additive_prediction,
    intercept = mean(problem$y[support, 1L] - fitted), kernel = params$kernel,
    solver = params$solver, convergence = if (info$status == "reported_success") 0L else NULL,
    diagnostics = c(info, list(constraint_violation = violation,
      feasible = violation <= max(1e-5, 100 * params$tolerance),
      objective = objective, support_vectors = length(support),
      observations = n, workspace_estimate_mb = problem$estimated_workspace_mb,
      optimization_variables = if (params$solver == "quadprog") m else opt$m,
      hessian_rank = if (params$solver != "quadprog") problem$compact$rank else NA_integer_,
      solver_version = as.character(utils::packageVersion(if (params$solver == "newton") "stats" else params$solver)),
      hessian_details = problem$compact$diagnostics,
      solver_and_reconstruction_seconds = unname(proc.time()[["elapsed"]] - start))),
    dual = z, frequency_groups = problem$groups), class = "survlearning_svm")
}

.svm_fit <- function(x, y, params, cache = NULL) {
  key <- paste(params$solver, params$kernel, params$collapse_duplicates, sep = "_")
  hit <- !is.null(cache) && exists(key, cache, inherits = FALSE)
  if (hit) {
    problem <- get(key, cache, inherits = FALSE)
    .svm_memory_check(problem$estimated_workspace_mb, params)
  } else {
    problem <- .svm_prepare(x, y, params)
    if (!is.null(cache)) assign(key, problem, cache)
  }
  setup <- proc.time()[["elapsed"]]
  workspace <- NULL
  if (params$solver != "quadprog") {
    wk <- paste0(key, "_workspace")
    if (!is.null(cache) && exists(wk, cache, inherits = FALSE)) workspace <- get(wk, cache)
    if (!is.null(workspace) && params$solver == "clarabel" &&
        (!identical(workspace$tolerance, params$tolerance) ||
         !identical(workspace$max_iter, params$max_iter))) workspace <- NULL
    if (is.null(workspace)) {
      workspace <- .svm_workspace(problem, params)
      if (!is.null(cache)) assign(wk, workspace, cache)
    }
  }
  setup <- unname(proc.time()[["elapsed"]] - setup)
  out <- .svm_solve(problem, params, workspace)
  out$diagnostics$cache_hit <- hit
  out$diagnostics$prepare_seconds <- if (hit) 0 else problem$prepare_seconds
  out$diagnostics$workspace_setup_seconds <- setup
  out
}

#' Predict Encoded Censored SVM Responses
#' @param object A `survlearning_svm` engine obtained as the engine of a fitted
#'   SVM. Use [surv_predict()] for original, unencoded predictors.
#' @param newdata Numeric matrix or data frame of finite encoded and standardized
#'   predictors, with the training column names and at least one row.
#' @param ... Reserved for S3 compatibility; extra arguments are rejected.
#' @return A list with numeric vector `predicted`, one value per row. These
#'   censored-regression scores are not calibrated survival times. Their negative
#'   is the package's increasing-risk score. Missing/invalid columns cause errors.
#'   Saved engines explicitly marked as infeasible or failed are rejected.
#' @export
predict.survlearning_svm <- function(object, newdata, ...) {
  if (length(list(...))) stop("Unused SVM prediction argument(s).", call. = FALSE)
  if (isFALSE(object$diagnostics$feasible) ||
      identical(object$diagnostics$status, "reported_failure") ||
      (!is.null(object$convergence) && !identical(object$convergence, 0L) &&
        !identical(object$convergence, 0))) {
    stop("Cannot predict from an SVM engine marked as infeasible or failed; refit the model.",
      call. = FALSE)
  }
  columns <- colnames(object$sv)
  if (!is.data.frame(newdata) && !is.matrix(newdata)) {
    stop("Provide an encoded predictor data frame or matrix.", call. = FALSE)
  }
  if (!is.null(columns)) {
    if (!all(columns %in% colnames(newdata))) stop("Encoded SVM predictors are missing.", call. = FALSE)
    newdata <- newdata[, columns, drop = FALSE]
  }
  x <- as.matrix(newdata)
  if (!is.numeric(x) || !nrow(x) || any(!is.finite(x))) stop("Encoded SVM predictors must be finite numeric values in at least one row.", call. = FALSE)
  if (!is.null(object$linear_weights)) {
    predicted <- as.vector(x %*% object$linear_weights) + object$intercept
  } else if (!is.null(object$additive_prediction)) {
    predicted <- .svm_additive_predict(object$additive_prediction, x) + object$intercept
  } else {
    predicted <- as.vector(crossprod(object$weights,
      .svm_kernel(object$sv, object$kernel, x))) + object$intercept
  }
  list(predicted = predicted)
}
