test_that("censored primal agrees with an independently assembled dense dual", {
  for (seed in 1:12) for (kernel in c("linear", "additive")) {
    set.seed(seed)
    n <- 24L
    x <- cbind(rnorm(n), rep(0:3, 6), rep(0:1, 12))
    if (seed == 3) x[, 1] <- x[, 2]
    y <- cbind(runif(n, 1, 8), rbinom(n, 1, if (seed %% 2) .15 else .8))
    if (seed == 12) y[, 2] <- 1
    if (!any(y[, 2] == 1)) y[1, 2] <- 1
    p <- survlearning:::.parameters("svm",
      list(kernel = kernel, cost = .7, prediction_tolerance = 0), ncol(x))
    fit <- survlearning:::.svm_fit(x, y, p)
    k <- if (kernel == "linear") tcrossprod(x) else {
      out <- matrix(0, n, n)
      for (j in seq_len(ncol(x))) out <- out + 1 -
        abs(outer(x[, j], x[, j], `-`)) / diff(range(x[, j]))
      out
    }
    a <- rbind(Matrix::Matrix(1, 1, n, sparse = TRUE),
      -Matrix::Diagonal(n), Matrix::Diagonal(n))
    ref <- clarabel::clarabel(A = as(a, "CsparseMatrix"),
      b = c(0, p$cost * y[, 2], rep(p$cost, n)), q = -y[, 1],
      P = as(as(Matrix::Matrix(k, sparse = TRUE), "symmetricMatrix"), "CsparseMatrix"),
      cones = list(z = 1L, l = as.integer(2 * n)),
      control = clarabel::clarabel_control(verbose = FALSE, max_threads = 1L,
        tol_gap_abs = 1e-10, tol_gap_rel = 1e-10, tol_feas = 1e-10))
    expect_equal(ref$status, 2L)
    expect_equal(fit$diagnostics$objective, -ref$obj_val, tolerance = 1e-7)
    expect_lt(fit$diagnostics$relative_gap, 1e-7)
    expect_lt(fit$diagnostics$dual_constraint_violation, 1e-7)
    predicted <- survlearning:::.svm_primal_predict(fit, x, raw = TRUE)
    representer <- as.vector(k %*% fit$dual)
    expect_equal(predicted - mean(predicted),
      representer - mean(representer), tolerance = 1e-6)
    expect_equal(length(fit$dual), n)
    expect_equal(fit$diagnostics$observations, n)
    expect_equal(fit$formulation, "censored_primal")
  }
})

test_that("two sparse solvers target the same censored loss", {
  d <- example_data(36)
  f <- Surv(time, event) ~ x + z
  for (kernel in c("linear", "additive")) {
    a <- surv_fit(f, d, "svm", list(kernel = kernel, solver = "osqp", tolerance = 1e-9))
    b <- surv_fit(f, d, "svm", list(kernel = kernel))
    expect_equal(a$engine$diagnostics$objective, b$engine$diagnostics$objective, tolerance = 1e-6)
    pa <- surv_predict(a, d); pb <- surv_predict(b, d)
    expect_equal(pa - mean(pa), pb - mean(pb), tolerance = 1e-5)
    expect_equal(surv_score(a, d), surv_score(b, d), tolerance = 1e-5)
    expect_lt(a$engine$diagnostics$relative_gap, 1e-5)
    expect_error(surv_fit(f, d, "svm", list(kernel = kernel, max_iter = 1)), "did not converge")
  }
  for (solver in c("newton", "quadprog", "other")) {
    expect_error(surv_fit(f, d, "svm", list(solver = solver)), "solver")
  }
  expect_error(surv_fit(f, d, "svm", list(memory_limit_mb = .001)), "No rows were removed")
  expect_error(surv_fit(f, d, "svm", list(prediction_tolerance = -1)), "prediction_tolerance")
})

test_that("certificates, failure states and saved models are checked", {
  d <- example_data(30)
  fit <- surv_fit(Surv(time, event) ~ x + z, d, "svm")
  expect_equal(surv_predict(unserialize(serialize(fit, NULL)), d), surv_predict(fit, d))
  fit$engine$diagnostics$feasible <- FALSE
  expect_error(surv_predict(fit, d), "infeasible or failed")
  fit$engine$diagnostics$feasible <- TRUE
  fit$engine$diagnostics$status <- "reported_failure"
  expect_error(surv_predict(fit, d), "infeasible or failed")
  fake <- structure(list(diagnostics = list(convergence =
    list(status = "reported_failure", code = 1L))), class = "survlearning_fit")
  expect_error(survlearning:::.resample_check_convergence(fake), "optimizer failure")
})

test_that("caching is fold-local and gives cold-fit results for each kernel", {
  d <- example_data(50)
  d$x[c(1, 3, 8)] <- NA
  f <- Surv(time, event) ~ x + z
  plan <- surv_resamples(f, d, folds = 2, seed = 16)
  for (kernel in c("linear", "additive")) {
    args <- list(formula = f, data = d, method = "svm",
      grid = list(cost = c(.01, .1, 1)), params = list(kernel = kernel),
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
  }
})
