test_that("additive prefix products reproduce the native kernel with ties", {
  set.seed(73)
  x <- cbind(rnorm(45), rep(c(0, 1, 2), 15), rep(c(0, 1, 1), 15))
  columns <- survlearning:::.svm_additive_columns(x)
  kernel <- survlearning:::.svm_kernel(x, "additive")
  for (j in 1:4) {
    v <- rnorm(nrow(x))
    expect_equal(survlearning:::.svm_additive_multiply(v, columns),
      as.vector(kernel %*% v), tolerance = 1e-10)
  }
  expect_error(survlearning:::.svm_additive_columns(cbind(x, 1)), "constant")
})

test_that("fast additive prediction keeps support-specific ranges and extrapolation", {
  set.seed(81)
  sv <- cbind(runif(30, .2, .7), rep(c(0, 1, 2), 10))
  w <- rnorm(30)
  newx <- rbind(sv, c(-1, -2), c(4, 5))
  prefix <- survlearning:::.svm_additive_prediction(sv, w)
  actual <- survlearning:::.svm_additive_predict(prefix, newx)
  reference <- as.vector(crossprod(w, survlearning:::.svm_kernel(sv, "additive", newx)))
  expect_equal(actual, reference, tolerance = 1e-10)
  expect_equal(survlearning:::.svm_additive_predict(prefix, newx[1, , drop = FALSE]), actual[1])
  expect_error(survlearning:::.svm_additive_prediction(cbind(sv, 1), w), "constant")
})

test_that("additive compact Hessians retain the same nearPD spectral rule", {
  skip_if_not_installed("RSpectra")
  skip_if_not_installed("Matrix")
  for (seed in 1:4) {
    set.seed(seed)
    x <- cbind(rnorm(25), rep(c(0, 1, 2, 1, 0), 5))
    delta <- if (seed == 4) rep(1, 25) else rep(c(0, 1, 0, 1, 0), 5)
    p <- survlearning:::.parameters("svm", list(kernel = "additive", solver = "clarabel"), 2)
    compact <- survlearning:::.svm_additive_hessian(x, delta, p)
    k <- survlearning:::.svm_kernel(x, "additive")
    d <- rbind(cbind(k, -delta * k), cbind(-delta * k, k))
    h <- as.matrix(Matrix::nearPD(d)$mat)
    actual <- tcrossprod(compact$factor) + diag(compact$diagonal)
    expect_lt(max(abs(h - actual)), 1e-8)
    expect_true(all(compact$diagonal > 0))
  }
})

test_that("matrix-free additive spectrum is complete above the reference cutoff", {
  skip_if_not_installed("RSpectra")
  skip_if_not_installed("Matrix")
  # More rows than the dense fallback, but few distinct covariate values.
  set.seed(91)
  x <- cbind(sample(0:5, 300, TRUE), sample(0:3, 300, TRUE))
  delta <- rep(c(0, 1, 0), 100)
  p <- survlearning:::.parameters("svm", list(kernel = "additive", solver = "clarabel"), 2)
  compact <- survlearning:::.svm_additive_hessian(x, delta, p)
  k <- survlearning:::.svm_kernel(x, "additive")
  d <- rbind(cbind(k, -delta * k), cbind(-delta * k, k))
  h <- as.matrix(Matrix::nearPD(d)$mat)
  expect_lt(max(abs(h - tcrossprod(compact$factor) - diag(compact$diagonal))), 1e-7)
  expect_false(compact$diagnostics$kernel_matrix_allocated)
  expect_true(all(compact$diagnostics$spectral_method == "matrix_free_lanczos"))
  expect_lt(compact$diagnostics$excluded_upper, compact$diagnostics$reference_eigen_cutoff)
})

test_that("additive solver, cache, and one-item prediction use the common interface", {
  skip_if_not_installed("clarabel", minimum_version = "0.11.3")
  skip_if_not_installed("RSpectra")
  d <- example_data(45)
  f <- Surv(time, event) ~ x + z
  args <- list(formula = f, data = d, method = "svm",
    grid = list(cost = c(.01, .1)), params = list(kernel = "additive", solver = "clarabel"),
    folds = 2, seed = 23)
  a <- do.call(surv_tune, c(args, list(cache = TRUE)))
  b <- do.call(surv_tune, c(args, list(cache = FALSE)))
  expect_equal(a$results$mean_cindex, b$results$mean_cindex)
  expect_equal(a$best_candidate, b$best_candidate)
  expect_equal(surv_predict(a, d), surv_predict(b, d), tolerance = 1e-8)
  expect_equal(a$best_model$n, nrow(d))
  expect_equal(sum(a$fold_results$cache_hit), 2)
  expect_equal(surv_predict(a, d[1, , drop = FALSE]), surv_predict(a, d)[1])
  expect_equal(a$best_model$diagnostics$convergence$status, "reported_success")
  expect_error(surv_fit(f, d, "svm", list(solver = "clarabel", kernel = "additive", max_iter = 1)), "did not converge")
})

test_that("identical-row multiplicities preserve the full corrected Hessian and QP", {
  skip_if_not_installed("clarabel", minimum_version = "0.11.3")
  skip_if_not_installed("RSpectra")
  set.seed(7)
  base <- data.frame(x = rnorm(8), z = rep(c(0, 1), 4),
    time = runif(8, 1, 4), event = rep(c(0, 1), 4))
  d <- base[rep(1:8, 1:8), ]
  f <- Surv(time, event) ~ x + z
  p <- survlearning:::.parameters("svm", list(kernel = "additive", solver = "clarabel",
    tolerance = 1e-10), 2)
  prep <- survlearning:::.prepare_design(f, d)
  x <- survlearning:::.scaled_design(prep$x, prep$design)
  problem <- survlearning:::.svm_prepare(x, prep$y, p)
  groups <- problem$groups
  expect_equal(sum(groups$count), nrow(d))
  expect_length(groups$representatives, 8)
  g <- length(groups$count)
  map <- c(groups$id, g + groups$id)
  root <- rep(sqrt(groups$count), 2)
  l <- (problem$compact$factor / root)[map, , drop = FALSE]
  h <- tcrossprod(l) + diag(problem$compact$diagonal[map])
  k <- survlearning:::.svm_kernel(x, "additive")
  raw <- rbind(cbind(k, -d$event * k), cbind(-d$event * k, k))
  expect_lt(max(abs(h - as.matrix(Matrix::nearPD(raw)$mat))), 1e-8)
  a <- surv_fit(f, d, "svm", p)
  p$collapse_duplicates <- FALSE
  b <- surv_fit(f, d, "svm", p)
  expect_equal(a$engine$diagnostics$objective, b$engine$diagnostics$objective, tolerance = 1e-7)
  expect_equal(surv_predict(a, d), surv_predict(b, d), tolerance = 1e-6)
  expect_equal(length(a$engine$dual), 2 * nrow(d))
  expect_equal(a$engine$diagnostics$optimization_variables, 16)
  expect_equal(b$engine$diagnostics$optimization_variables, 2 * nrow(d))
  expect_equal(a$n, nrow(d))
  expect_lt(a$engine$diagnostics$constraint_violation, 1e-6)
  expect_equal(surv_score(a, d), surv_score(b, d))
  # Equal predictors are not enough: follow-up and event indicator must match.
  altered <- d
  altered$time[2] <- altered$time[2] + .5
  pp <- survlearning:::.prepare_design(f, altered)
  expect_length(survlearning:::.svm_duplicate_groups(pp$x, pp$y)$representatives, 9)
})

test_that("weighted matrix-free operators include every replicate", {
  skip_if_not_installed("RSpectra")
  set.seed(12)
  base <- data.frame(x = sample(0:5, 300, TRUE), z = sample(0:3, 300, TRUE),
    time = seq_len(300), event = rep(c(0, 1, 0), 100))
  d <- base[rep(seq_len(300), rep(1:3, 100)), ]
  f <- Surv(time, event) ~ x + z
  prep <- survlearning:::.prepare_design(f, d)
  groups <- survlearning:::.svm_duplicate_groups(prep$x, prep$y)
  x <- prep$x[groups$representatives, , drop = FALSE]
  delta <- prep$y[groups$representatives, 2L]
  root <- sqrt(groups$count)
  k <- survlearning:::.svm_kernel(x, "additive")
  k <- root * k * rep(root, each = nrow(x))
  args <- list(columns = survlearning:::.svm_additive_columns(x),
    root_count = root, delta = delta, sign = 1)
  v <- rnorm(nrow(x))
  expected <- (k + (delta * k + t(delta * k)) / 2) %*% v
  expect_equal(survlearning:::.svm_additive_operator(v, args), as.vector(expected), tolerance = 1e-9)
  p <- survlearning:::.parameters("svm", list(kernel = "additive", solver = "clarabel"), 2)
  compact <- survlearning:::.svm_additive_hessian(prep$x, prep$y[, 2L], p, groups)
  expect_false(compact$diagnostics$kernel_matrix_allocated)
  expect_equal(compact$diagnostics$multiplicity_total, nrow(d))
  expect_equal(compact$diagnostics$optimization_groups, nrow(base))
})

test_that("Newton solves the same additive QP and rejects an uncertified iterate", {
  skip_if_not_installed("RSpectra")
  skip_if_not_installed("clarabel", minimum_version = "0.11.3")
  d <- example_data(50)
  f <- Surv(time, event) ~ x + z
  for (kernel in c("linear", "additive")) {
    a <- surv_fit(f, d, "svm", list(kernel = kernel, solver = "newton"))
    b <- surv_fit(f, d, "svm", list(kernel = kernel, solver = "clarabel"))
    expect_equal(a$engine$diagnostics$objective, b$engine$diagnostics$objective, tolerance = 1e-7)
    expect_lt(a$engine$diagnostics$relative_duality_gap, 1e-8)
    expect_lt(abs(a$engine$diagnostics$objective - a$engine$diagnostics$dual_objective), 1e-6)
    expect_lt(a$engine$diagnostics$constraint_violation, 1e-6)
    expect_equal(surv_score(a, d), surv_score(b, d))
    pa <- surv_predict(a, d)
    pb <- surv_predict(b, d)
    expect_equal(pa - mean(pa), pb - mean(pb), tolerance = 1e-4)
  }
  expect_error(surv_fit(f, d, "svm", list(kernel = "additive", solver = "newton", max_iter = 1)), "did not converge")
  args <- list(formula = f, data = d, method = "svm", grid = list(cost = c(.01, .1)),
    params = list(kernel = "additive", solver = "newton"), folds = 2, seed = 123)
  a <- do.call(surv_tune, c(args, list(cache = TRUE)))
  b <- do.call(surv_tune, c(args, list(cache = FALSE)))
  expect_equal(a$results$mean_cindex, b$results$mean_cindex)
  expect_equal(a$best_candidate, b$best_candidate)
})

test_that("Newton analytic derivatives match independent finite differences", {
  set.seed(58)
  n <- 12
  problem <- list(x = matrix(rnorm(n * 2), n, 2),
    y = survival::Surv(runif(n, .1, 1), rep(c(0, 1), 6)),
    compact = list(diagonal = runif(2 * n, .5, 1),
      factor = matrix(rnorm(2 * n * 3, sd = .2), 2 * n, 3)))
  opt <- survlearning:::.svm_optimization_data(problem)
  theta <- c(.1, .2, -.1)
  e <- survlearning:::.svm_newton_evaluate(theta, problem, opt, .6)
  numerical_gradient <- numeric(3)
  numerical_hessian <- matrix(0, 3, 3)
  for (j in 1:3) {
    step <- rep(0, 3); step[j] <- 1e-5
    up <- survlearning:::.svm_newton_evaluate(theta + step, problem, opt, .6, FALSE)
    down <- survlearning:::.svm_newton_evaluate(theta - step, problem, opt, .6, FALSE)
    numerical_gradient[j] <- (up$value - down$value) / 2e-5
    numerical_hessian[, j] <- (up$gradient - down$gradient) / 2e-5
  }
  expect_equal(e$gradient, numerical_gradient, tolerance = 1e-6)
  expect_equal(e$hessian, numerical_hessian, tolerance = 1e-6)
})

test_that("an ambiguous spectral boundary is rejected rather than rounded to a rank", {
  skip_if_not_installed("RSpectra")
  x <- cbind(seq_len(12), rep(c(0, 1), 6))
  delta <- rep(c(0, 1), 6)
  k <- survlearning:::.svm_kernel(x, "additive")
  cutoff <- max(eigen(k + (delta * k + t(delta * k)) / 2,
    symmetric = TRUE, only.values = TRUE)$values)
  p <- survlearning:::.parameters("svm", list(kernel = "additive", solver = "newton"), 2)
  expect_error(survlearning:::.svm_additive_eigen(x,
    survlearning:::.svm_additive_columns(x), delta, 1, cutoff, p, rep(1, 12)), "ambiguous")
})

test_that("compact additive fits support one predictor and one new observation", {
  skip_if_not_installed("RSpectra")
  d <- example_data(40)
  f <- Surv(time, event) ~ x
  fit <- surv_fit(f, d, "svm", list(kernel = "additive", solver = "newton"))
  expect_length(surv_predict(fit, d[1, , drop = FALSE]), 1)
  expect_true(all(is.finite(surv_predict(fit, d))))
  x <- matrix(d$x, ncol = 1)
  span <- diff(range(d$x))
  expect_equal(survlearning:::.svm_additive_dense(x),
    (span - abs(outer(d$x, d$x, `-`))) / span)
})
