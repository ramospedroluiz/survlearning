test_that("compact linear Hessian matches the reference nearPD, not another loss", {
  skip_if_not_installed("Matrix")
  for (seed in 1:5) {
    set.seed(seed)
    x <- scale(matrix(rnorm(72), 24, 3))
    if (seed == 2) x[, 3] <- x[, 1]
    if (seed == 3) x[1, ] <- 0
    delta <- if (seed == 4) rep(1, 24) else rbinom(24, 1, .5)
    k <- tcrossprod(x)
    d <- rbind(cbind(k, -delta * k), cbind(-delta * k, k))
    reference <- as.matrix(Matrix::nearPD(d)$mat)
    compact <- survlearning:::.svm_linear_hessian(x, delta)
    actual <- tcrossprod(compact$factor) + diag(compact$diagonal)
    expect_lt(max(abs(reference - actual)), 1e-10)
    expect_true(all(compact$diagonal > 0))
  }
})

test_that("dense reference predictions agree only when the solution is feasible", {
  skip_if_not_installed("survivalsvm")
  skip_if_not_installed("quadprog")
  d <- example_data(28L)
  f <- survival::Surv(time, event) ~ x + z
  for (kernel in c("linear", "additive")) {
    prep <- survlearning:::.prepare_design(f, d)
    x <- survlearning:::.scaled_design(prep$x, prep$design)
    params <- survlearning:::.parameters("svm", list(kernel = kernel, solver = "quadprog"), ncol(x))
    problem <- survlearning:::.svm_prepare(x, prep$y, params)
    n <- nrow(x)
    m <- 2L * n
    aa <- c(rep(-1, n), prep$y[, 2L])
    q <- c(-prep$y[, 1L], prep$y[, 2L] * prep$y[, 1L])
    dual <- quadprog::solve.QP(problem$h, -q,
      cbind(aa, diag(m), diag(m), -diag(m)),
      c(0, rep(0, 2L * m), rep(-params$cost, m)), meq = 1L)$solution
    violation <- max(0, -dual, dual - params$cost, abs(sum(aa * dual)))
    if (violation > max(1e-5, 100 * params$tolerance)) {
      expect_error(surv_fit(f, d, "svm", list(kernel = kernel, solver = "quadprog")),
        "infeasible solution")
      next
    }
    a <- surv_fit(f, d, "svm", list(kernel = kernel, solver = "quadprog"))
    b <- survivalsvm::survivalsvm(survlearning:::.engine_formula(),
      data = survlearning:::.engine_data(x, prep$y), type = "regression",
      gamma.mu = .1, opt.meth = "quadprog",
      kernel = if (kernel == "linear") "lin_kernel" else "add_kernel")
    expect_equal(surv_predict(a, d),
      -as.numeric(predict(b, as.data.frame(x))$predicted), tolerance = 1e-8)
    expect_equal(a$n, nrow(d))
    expect_equal(surv_predict(a, d[1, , drop = FALSE]), surv_predict(a, d)[1])
  }
})

test_that("invalid dense solutions stop before a fitted model can be constructed", {
  skip_if_not_installed("quadprog")
  problem <- list(x = matrix(c(0, 1), 2L),
    y = cbind(time = c(1, 2), event = c(1, 1)), h = diag(4L))
  params <- list(solver = "quadprog", cost = .1, tolerance = 1e-8)
  bad <- rep(1, 4L)
  local_mocked_bindings(solve.QP = function(...) list(solution = bad), .package = "quadprog")
  expect_error(survlearning:::.svm_solve(problem, params), "infeasible solution")
  bad <- rep(Inf, 4L)
  expect_error(survlearning:::.svm_solve(problem, params), "non-finite or invalid solution")
  bad <- rep(0, 3L)
  expect_error(survlearning:::.svm_solve(problem, params), "non-finite or invalid solution")
  bad <- rep(1e200, 4L)
  params$cost <- 1e201
  expect_error(survlearning:::.svm_solve(problem, params), "non-finite objective")
})

test_that("saved infeasible engines cannot be used for prediction", {
  d <- example_data(28L)
  fit <- surv_fit(survival::Surv(time, event) ~ x + z, d, "svm", list(solver = "newton"))
  expected <- surv_predict(fit, d)
  fit$engine$diagnostics$feasible <- FALSE
  expect_error(surv_predict(fit, d), "infeasible or failed")
  expect_error(predict(fit$engine, matrix(0, 1L, 2L)), "infeasible or failed")
  fit$engine$diagnostics$feasible <- TRUE
  fit$engine$diagnostics$status <- "reported_failure"
  expect_error(surv_predict(fit, d), "infeasible or failed")
  fit$engine$diagnostics <- NULL
  fit$engine$convergence <- NULL
  expect_equal(surv_predict(fit, d), expected)
})

test_that("compact solvers solve the same QP and report valid constraints", {
  skip_if_not_installed("osqp", minimum_version = "1.0.0")
  skip_if_not_installed("clarabel", minimum_version = "0.11.3")
  set.seed(19)
  d <- data.frame(time = runif(24, 1, 4), event = rep(c(0, 1), 12),
    x = rnorm(24), z = rnorm(24))
  f <- survival::Surv(time, event) ~ x + z
  a <- surv_fit(f, d, "svm", list(solver = "osqp", tolerance = 1e-10))
  b <- surv_fit(f, d, "svm", list(solver = "clarabel", tolerance = 1e-10))
  expect_equal(a$engine$diagnostics$objective, b$engine$diagnostics$objective, tolerance = 1e-7)
  expect_lt(a$engine$diagnostics$constraint_violation, 1e-6)
  expect_lt(b$engine$diagnostics$constraint_violation, 1e-6)
  # The legacy intercept uses the rounded support set: tiny dual differences
  # can change its constant offset. Risk ordering, not absolute time, is public.
  pa <- surv_predict(a, d)
  pb <- surv_predict(b, d)
  expect_equal(pa - mean(pa), pb - mean(pb), tolerance = 1e-4)
  expect_equal(surv_score(a, d), surv_score(b, d))
  expect_equal(a$diagnostics$convergence$status, "reported_success")
  expect_equal(b$diagnostics$convergence$status, "reported_success")
  expect_lt(as.numeric(object.size(a$engine)), 20000)
  expect_error(surv_fit(f, d, "svm", list(solver = "osqp", max_iter = 1)), "did not converge")
  expect_error(surv_fit(f, d, "svm", list(solver = "clarabel", max_iter = 1)), "did not converge")
})

test_that("SVM caching is split-local, bounded, and agrees with cold tuning", {
  skip_if_not_installed("clarabel", minimum_version = "0.11.3")
  d <- example_data(50L)
  d$x[c(1, 3, 8)] <- NA
  f <- survival::Surv(time, event) ~ x + z
  plan <- surv_resamples(f, d, folds = 2, seed = 16)
  args <- list(formula = f, data = d, method = "svm",
    grid = list(cost = c(.01, .1, .2)), params = list(solver = "clarabel"),
    resamples = plan, impute = "median", seed = 123)
  a <- do.call(surv_tune, c(args, list(cache = TRUE)))
  b <- do.call(surv_tune, c(args, list(cache = FALSE)))
  expect_equal(a$results$mean_cindex, b$results$mean_cindex)
  expect_equal(a$best_candidate, b$best_candidate)
  expect_equal(surv_predict(a, d), surv_predict(b, d))
  expect_equal(sum(a$fold_results$cache_hit), 4)
  expect_false(any(b$fold_results$cache_hit))
  expect_equal(a$best_model$n, nrow(d))
  expect_equal(a$fold_results$n_analysis, b$fold_results$n_analysis)
  expect_equal(a$best_model$design$imputation, b$best_model$design$imputation)
})

test_that("dense memory guard never substitutes a kernel or reduces rows", {
  d <- example_data(20L)
  f <- survival::Surv(time, event) ~ x + z
  expect_error(surv_fit(f, d, "svm", list(solver = "newton", memory_limit_mb = .001)), "No rows were removed")
  expect_error(surv_fit(f, d, "svm", list(solver = "other")), "solver")
  expect_error(surv_tune(f, d, "svm", cache = NA), "cache")
})

test_that("numerically infeasible SVM fits cannot win model selection", {
  fake <- structure(list(diagnostics = list(convergence =
    list(status = "reported_failure", code = 1L))), class = "survlearning_fit")
  expect_error(survlearning:::.resample_check_convergence(fake), "optimizer failure")
})
