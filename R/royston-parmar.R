# Right-censored Royston-Parmar likelihood. Basis and initialization conventions
# follow flexsurv 2.3.2 (Christopher Jackson et al., GPL >= 2); see COPYRIGHTS.
.rp_basis <- function(z, knots, derivative = FALSE) {
  out <- if (derivative) cbind(rep(0, length(z)), rep(1, length(z))) else cbind(1, z)
  if (length(knots) > 2L) {
    first <- knots[1L]
    last <- knots[length(knots)]
    power <- if (derivative) 2 else 3
    multiplier <- if (derivative) 3 else 1
    for (knot in knots[-c(1L, length(knots))]) {
      weight <- (last - knot) / (last - first)
      out <- cbind(out, multiplier * (pmax(z - knot, 0)^power -
        weight * pmax(z - first, 0)^power - (1 - weight) * pmax(z - last, 0)^power))
    }
  }
  out
}

.rp_initial <- function(x, time, event, knots, scale, fallback = FALSE) {
  include <- if (sum(event) >= 5L) event == 1L else rep(TRUE, length(event))
  data <- data.frame(start = 0, time = time, event = event, x)
  names(data)[-(1:3)] <- paste0("X", seq_len(ncol(x)))
  formula <- stats::reformulate(names(data)[-(1:3)],
    response = "survival::Surv(start, time, event)")
  cox <- survival::coxph(formula, data = data[include, , drop = FALSE])
  zero <- as.data.frame(matrix(0, 1L, ncol(x), dimnames = list(NULL, names(data)[-(1:3)])))
  survival <- survival::survfit(cox, newdata = zero)
  s <- survival$surv[match(time[include], survival$time)]^exp(cox$linear.predictors)
  target <- switch(scale, hazard = log(-log(s)), odds = log((1 - s) / s),
    normal = stats::qnorm(1 - s))
  z <- log(time[include])
  design <- cbind(.rp_basis(z, knots), x[include, , drop = FALSE])
  boundaries <- z
  span <- diff(range(z))
  boundaries[1L] <- z[1L] - 0.01 * span
  boundaries[length(z)] <- z[length(z)] + 0.01 * span
  derivative <- cbind(.rp_basis(boundaries, knots, TRUE), matrix(0, length(z), ncol(x)))
  finite <- is.finite(target)
  design <- design[finite, , drop = FALSE]
  derivative <- derivative[finite, , drop = FALSE]
  target <- target[finite]
  hessian <- t(design) %*% design
  positive <- all(eigen(hessian, only.values = TRUE)$values > 0)
  if (!positive && !fallback) return(.rp_initial(x, time, event, knots, scale, TRUE))
  initial <- if (positive) quadprog::solve.QP(hessian,
    dvec = t(t(target) %*% design), Amat = t(derivative),
    bvec = rep(1e-9, length(target)))$solution else rep(0, ncol(design))
  if (fallback) {
    full_cox <- survival::coxph(formula, data = data)
    initial[length(knots) + seq_len(ncol(x))] <- stats::coef(full_cox)
  }
  initial
}

.rp_problem <- function(x, time, event, knots, scale) {
  basis <- .rp_basis(log(time), knots)
  derivative <- .rp_basis(log(time), knots, TRUE)
  design <- cbind(basis, x)
  event <- as.numeric(event)
  event_rows <- which(event == 1)
  log_time <- log(time)
  evaluate <- function(par, gradient = FALSE) {
    eta <- drop(design %*% par)
    slope <- drop(derivative %*% par[seq_along(knots)])
    if (any(!is.finite(eta)) || any(!is.finite(slope[event_rows])) || any(slope[event_rows] <= 0)) {
      return(if (gradient) rep(NaN, length(par)) else Inf)
    }
    if (scale == "hazard") {
      h <- exp(eta)
      log_s <- -h
      log_f <- eta - h
      score <- event - h
    } else if (scale == "odds") {
      softplus <- pmax(eta, 0) + log1p(exp(-abs(eta)))
      log_s <- -softplus
      log_f <- eta - 2 * softplus
      score <- event - (1 + event) * stats::plogis(eta)
    } else {
      log_s <- stats::pnorm(eta, lower.tail = FALSE, log.p = TRUE)
      log_f <- stats::dnorm(eta, log = TRUE)
      score <- -event * eta - (1 - event) * exp(log_f - log_s)
    }
    contribution <- log_s
    contribution[event_rows] <- log_f[event_rows] + log(slope[event_rows]) - log_time[event_rows]
    value <- -sum(contribution)
    if (!gradient) return(value)
    if (!is.finite(value)) return(rep(NaN, length(par)))
    grad <- drop(crossprod(design, score))
    grad[seq_along(knots)] <- grad[seq_along(knots)] +
      drop(crossprod(derivative[event_rows, , drop = FALSE], 1 / slope[event_rows]))
    -grad
  }
  list(value = function(par) evaluate(par), gradient = function(par) evaluate(par, TRUE))
}

.fit_rp <- function(x, y, params) {
  if (qr(cbind(1, x))$rank < ncol(x) + 1L) {
    stop("Royston-Parmar requires an identifiable, full-rank predictor design; ",
      "no columns or observations were removed.", call. = FALSE)
  }
  time <- y[, 1L]
  event <- y[, 2L]
  deaths <- log(time[event == 1])
  interior <- if (params$knots) as.numeric(stats::quantile(deaths,
    seq(0, 1, length.out = params$knots + 2L)[-c(1L, params$knots + 2L)])) else numeric()
  knots <- c(min(deaths), interior, max(deaths))
  if (any(diff(knots) <= 0)) {
    stop("Royston-Parmar event-time quantiles produce repeated knots; ",
      "request fewer knots. No knots or observations were removed.", call. = FALSE)
  }
  problem <- .rp_problem(x, time, event, knots, params$scale)
  initialization <- "cox_qp"
  initial_errors <- character()
  attempt <- function(fallback = FALSE) tryCatch(
    .rp_initial(x, time, event, knots, params$scale, fallback), error = function(e) {
      initial_errors <<- c(initial_errors, conditionMessage(e))
      NULL
    })
  initial <- attempt()
  if (is.null(initial) || !is.finite(problem$value(initial))) {
    initialization <- "cox_qp_full"
    initial <- attempt(TRUE)
  }
  if (is.null(initial) || !is.finite(problem$value(initial))) {
    # A zero spline slope from a singular initial regression is not a density.
    # This is a feasible starting point only; all coefficients are still fitted.
    initial <- c(-stats::median(deaths) + if (params$scale == "hazard") log(log(2)) else 0,
      1, rep(0, length(knots) - 2L + ncol(x)))
    initialization <- "feasible_intercept"
  }
  fit <- tryCatch(stats::optim(initial, problem$value, problem$gradient,
    method = "BFGS", hessian = TRUE), error = function(e) {
      stop("Royston-Parmar optimization failed: ", conditionMessage(e),
        ". Check event information and the requested number of knots.", call. = FALSE)
    })
  names(fit$par) <- c(paste0("gamma", seq_along(knots) - 1L), colnames(x))
  if (!is.finite(fit$value)) stop("Royston-Parmar returned a non-finite likelihood.", call. = FALSE)
  if (fit$convergence != 0L) stop("Royston-Parmar optimizer did not report convergence; ",
    "no fitted model was returned.", call. = FALSE)
  minimum_slope <- .rp_minimum_slope(fit$par[seq_along(knots)], knots)
  if (!is.finite(minimum_slope) || minimum_slope <= 0) {
    stop("Royston-Parmar fitted an invalid nonmonotone baseline. ",
      "Request fewer knots or reconsider the model; no curve was clipped and no knots were removed.", call. = FALSE)
  }
  if (any(!is.finite(fit$hessian))) stop("Royston-Parmar observed information is non-finite; ",
    "the fitted model cannot be verified.", call. = FALSE)
  information_eigenvalues <- eigen(fit$hessian, symmetric = TRUE, only.values = TRUE)$values
  information_ok <- min(information_eigenvalues) >
    sqrt(.Machine$double.eps) * max(abs(information_eigenvalues))
  structure(list(coefficients = fit$par, knots = knots, scale = params$scale,
    loglik = -fit$value, opt = fit, initial = initial,
    gradient = problem$gradient(fit$par), diagnostics = list(initialization = initialization,
      initialization_errors = initial_errors, minimum_baseline_slope = minimum_slope,
      information_well_conditioned = information_ok,
      information_eigenvalues = information_eigenvalues)), class = "survlearning_rp")
}

.rp_coefficients <- function(fit) {
  if (inherits(fit, "survlearning_rp")) fit$coefficients else fit$res[, "est"]
}

.rp_minimum_slope <- function(gamma, knots) {
  # The derivative is quadratic on each knot interval and constant in both
  # tails. Its minima occur at endpoints or an interior quadratic vertex.
  slope <- function(z) drop(.rp_basis(z, knots, TRUE) %*% gamma)
  edge <- slope(knots)
  middle <- slope((utils::head(knots, -1L) + utils::tail(knots, -1L)) / 2)
  left <- utils::head(edge, -1L); right <- utils::tail(edge, -1L)
  a <- 2 * (left + right - 2 * middle)
  b <- right - left - a
  u <- rep(NA_real_, length(a))
  positive <- which(a > 0 & is.finite(a))
  u[positive] <- -b[positive] / (2 * a[positive])
  inside <- which(is.finite(u) & u > 0 & u < 1)
  if (!length(inside)) return(min(edge))
  min(c(edge, slope(utils::head(knots, -1L)[inside] + diff(knots)[inside] * u[inside])))
}

.rp_risk <- function(fit, x) {
  if (!is.null(fit$predictor_scaling)) x <- .scaled_design(x, fit$predictor_scaling)
  drop(x %*% .rp_coefficients(fit)[colnames(x)])
}

.rp_survival <- function(fit, x, times) {
  coefficients <- .rp_coefficients(fit)
  gamma <- coefficients[paste0("gamma", seq_along(fit$knots) - 1L)]
  minimum_slope <- .rp_minimum_slope(gamma, fit$knots)
  if (!is.finite(minimum_slope) || minimum_slope <= 0) {
    stop("Saved Royston-Parmar fit has an invalid nonmonotone baseline; refit the model.", call. = FALSE)
  }
  risk <- .rp_risk(fit, x)
  out <- matrix(1, nrow(x), length(times))
  positive <- times > 0
  if (any(positive)) {
    baseline <- drop(.rp_basis(log(times[positive]), fit$knots) %*% gamma)
    eta <- outer(risk, baseline, "+")
    out[, positive] <- switch(fit$scale, hazard = exp(-exp(eta)),
      odds = stats::plogis(-eta), normal = stats::pnorm(eta, lower.tail = FALSE))
  }
  out
}
