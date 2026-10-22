.svm_newton_model <- function(problem) {
  state <- new.env(parent = emptyenv())
  state$theta <- rep(0, ncol(problem$compact$factor))
  list(state = state)
}

.svm_newton_evaluate <- function(theta, problem, opt, cost, hessian = TRUE) {
  l <- problem$compact$factor
  d <- problem$compact$diagonal
  upper <- cost * opt$root
  b <- as.vector(l %*% theta) + opt$q
  nonzero <- which(opt$a != 0)
  aa <- opt$a[nonzero]
  bb <- b[nonzero]
  dd <- d[nonzero]
  uu <- upper[nonzero]
  endpoints <- c(-bb / aa, (-bb - dd * uu) / aa)
  equation <- function(mu) sum(aa * pmin(uu, pmax(0, -(bb + mu * aa) / dd)))
  bracket <- range(endpoints)
  if (bracket[1L] == bracket[2L]) mu <- bracket[1L] else {
    bracket <- bracket + c(-1, 1) * max(1, abs(bracket)) * 1e-12
    mu <- stats::uniroot(equation, bracket,
      tol = max(1e-14, .Machine$double.eps * max(1, abs(bracket))),
      maxiter = 1000L, check.conv = TRUE)$root
  }
  z <- pmin(upper, pmax(0, -(b + mu * opt$a) / d))
  gradient <- theta - as.vector(crossprod(l, z))
  value <- sum(theta^2) / 2 - sum(z * (d * z / 2 + b + mu * opt$a))
  out <- list(value = value, gradient = gradient, z = z, mu = mu,
    equality = sum(opt$a * z))
  if (hessian) {
    free <- which(z > 0 & z < upper)
    h <- diag(length(theta))
    if (length(free)) {
      la <- l[free, , drop = FALSE] / sqrt(d[free])
      af <- opt$a[free] / sqrt(d[free])
      h <- h + crossprod(la)
      den <- sum(af * af)
      if (den > 0) {
        v <- as.vector(crossprod(la, af)) / sqrt(den)
        h <- h - tcrossprod(v)
      }
    }
    out$hessian <- h
  }
  out
}

.svm_newton_solve <- function(problem, params, workspace = NULL) {
  if (is.null(workspace)) workspace <- .svm_newton_model(problem)
  opt <- .svm_optimization_data(problem)
  evaluations <- 0L
  objective <- function(theta) {
    e <- .svm_newton_evaluate(theta, problem, opt, params$cost)
    value <- e$value
    attr(value, "gradient") <- e$gradient
    attr(value, "hessian") <- e$hessian
    evaluations <<- evaluations + 1L
    value
  }
  result <- stats::nlm(objective, workspace$state$theta,
    gradtol = max(1e-12, params$tolerance / 10), steptol = 1e-12,
    iterlim = as.integer(params$max_iter), check.analyticals = FALSE)
  final <- .svm_newton_evaluate(result$estimate, problem, opt, params$cost, hessian = FALSE)
  hz <- problem$compact$diagonal * final$z +
    as.vector(problem$compact$factor %*% crossprod(problem$compact$factor, final$z))
  primal <- sum(final$z * hz) / 2 + sum(opt$q * final$z)
  # Box stationarity holds by construction. Equality and the Fenchel gap
  # provide an independent certificate, not an inference from an nlm code.
  stationarity <- result$estimate -
    as.vector(crossprod(problem$compact$factor, final$z))
  gap_bound <- max(sum(stationarity^2) / 2 + abs(final$mu * final$equality),
    abs(primal + final$value))
  relative_gap <- gap_bound / max(1, abs(primal))
  if (result$code %in% c(4L, 5L) || !is.finite(relative_gap) ||
      relative_gap > params$tolerance ||
      abs(final$equality) > max(1e-7, 10 * params$tolerance)) {
    stop("SVM Newton did not converge: nlm code ", result$code,
      ", certified relative gap ", signif(relative_gap, 4),
      ", equality residual ", signif(abs(final$equality), 4), ".", call. = FALSE)
  }
  workspace$state$theta <- result$estimate
  list(x = final$z, info = list(status = "reported_success", code = 0L,
    optimizer_code = result$code, iterations = result$iterations,
    free_dimension = length(result$estimate), box_coordinates = opt$m,
    evaluations = evaluations, convergence_basis = "primal_dual_certificate",
    primal_residual = abs(final$equality), dual_residual = sqrt(sum(stationarity^2)),
    duality_gap = gap_bound, relative_duality_gap = relative_gap,
    dual_objective = -final$value))
}
