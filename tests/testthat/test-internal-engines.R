test_that("integrated RSF agrees with upstream with tied times and factors", {
  skip_if_not_installed("randomForestSRC")
  d <- example_data(80L)
  d$time <- pmax(.1, round(d$time, 1))
  for (threads in c(1L, 2L)) {
    fit <- surv_fit(Surv(time, event) ~ x + z + batch, d, "rsf",
      list(trees = 30L, min_node = 5L, threads = threads), seed = 31)
    p <- survlearning:::.prepare_design(fit$formula, d)
    withr::local_options(list(rf.cores = threads))
    native <- withr::with_seed(31, randomForestSRC::rfsrc(
      Surv(.sl_time, .sl_event) ~ ., survlearning:::.engine_data(p$x, p$y),
      ntree = 30L, mtry = fit$params$mtry, nodesize = 5L, nsplit = 5L,
      splitrule = "logrank", importance = FALSE, seed = 31))
    original <- withr::with_seed(31, predict(native, newdata = survlearning:::.engine_data(p$x)))
    expect_equal(surv_predict(fit, d), as.numeric(original$predicted), tolerance = 1e-12)
    expect_equal(unname(surv_predict(fit, d, "survival", original$time.interest)),
      original$survival, tolerance = 1e-12)
    restored <- unserialize(serialize(fit, NULL))
    if (threads == 1L) {
      expect_identical(surv_predict(restored, d), surv_predict(fit, d))
    } else {
      # Parallel ensemble reductions can change the last floating-point bits.
      expect_equal(surv_predict(restored, d), surv_predict(fit, d), tolerance = 1e-12)
    }
    legacy <- fit
    legacy$engine <- native
    expect_equal(surv_predict(legacy, d), surv_predict(fit, d), tolerance = 1e-12)
  }
})

test_that("Royston-Parmar gradients and zero-knot distributions agree", {
  set.seed(5)
  x <- matrix(rnorm(100), 50, 2)
  time <- exp(seq(-1, 2, length.out = 50))
  event <- rep(c(1, 1, 0, 0, 1), 10)
  knots <- c(-1, .4, 2)
  par <- c(-.4, 1.1, .01, .2, -.1)
  for (scale in c("hazard", "odds", "normal")) {
    problem <- survlearning:::.rp_problem(x, time, event, knots, scale)
    finite_difference <- vapply(seq_along(par), function(j) {
      step <- rep(0, length(par)); step[j] <- 1e-6
      (problem$value(par + step) - problem$value(par - step)) / 2e-6
    }, numeric(1))
    expect_equal(unname(problem$gradient(par)), finite_difference, tolerance = 1e-6)
    fit <- list(coefficients = c(gamma0 = -.4, gamma1 = 1.1, x1 = .2, x2 = -.1),
      knots = c(-1, 2), scale = scale)
    class(fit) <- "survlearning_rp"
    colnames(x) <- c("x1", "x2")
    eta <- outer(drop(x %*% c(.2, -.1)), -.4 + 1.1 * log(time), "+")
    expected <- switch(scale, hazard = exp(-exp(eta)), odds = plogis(-eta), normal = pnorm(-eta))
    expect_equal(survlearning:::.rp_survival(fit, x, time), expected, tolerance = 1e-14)
  }
})

test_that("internal Royston-Parmar fits match the independent reference", {
  skip_if_not_installed("flexsurv")
  d <- example_data(100L)
  for (scale in c("hazard", "odds", "normal")) for (knots in 0:1) {
    fit <- surv_fit(Surv(time, event) ~ x + z, d, "flexsurv", list(knots = knots, scale = scale))
    p <- survlearning:::.prepare_design(fit$formula, d)
    old <- flexsurv::flexsurvspline(Surv(.sl_time, .sl_event) ~ .,
      survlearning:::.engine_data(p$x, p$y), k = knots, scale = scale,
      inits = fit$engine$initial, hessian = FALSE)
    expected <- t(vapply(summary(old, newdata = as.data.frame(p$x[1:3, ]),
      t = c(0, 2, 5), ci = FALSE, type = "survival"), function(row) row$est, numeric(3)))
    expect_equal(unname(surv_predict(fit, d[1:3, ], "survival", c(0, 2, 5))), unname(expected), tolerance = 1e-5)
    expect_equal(fit$engine$loglik, old$loglik, tolerance = 1e-6)
    legacy <- fit
    legacy$engine <- old
    expect_equal(surv_predict(legacy, d[1:3, ], "survival", c(0, 2, 5)),
      surv_predict(fit, d[1:3, ], "survival", c(0, 2, 5)), tolerance = 1e-5)
  }
})

test_that("native Gehan preserves all informative pairs and penalty endpoints", {
  skip_if_not_installed("penAFT")
  d <- example_data(60L)
  d$time <- pmax(.1, round(d$time, 1))
  for (alpha in c(0, .5, 1)) {
    fit <- surv_fit(Surv(time, event) ~ x + z, d, "aft_elasticnet",
      list(lambda = .01, alpha = alpha, max_iter = 5000L), seed = 20)
    p <- survlearning:::.prepare_design(fit$formula, d)
    x <- survlearning:::.scaled_design(p$x, p$design)
    old <- suppressWarnings(withr::with_seed(20, penAFT::penAFT(x, log(p$y[, 1]), p$y[, 2],
      lambda = .01, alpha = alpha, penalty = "EN", standardize = FALSE)))
    expect_equal(as.numeric(fit$engine$beta), as.numeric(old$beta), tolerance = 1e-8)
    events <- sum(d$event)
    expect_equal(fit$engine$diagnostics$pairs, events * nrow(d) - events * (events + 1) / 2)
    expect_gte(fit$engine$diagnostics$duality_gap, -1e-10)
    residual <- log(p$y[, 1]) - drop(x %*% fit$engine$beta)
    beta <- as.numeric(fit$engine$beta)
    objective <- sum(vapply(which(d$event == 1), function(i) sum(pmax(residual - residual[i], 0)), 0)) /
      nrow(d)^2 + .01 * alpha * sum(abs(beta)) + .005 * (1 - alpha) * sum(beta^2)
    expect_equal(fit$engine$diagnostics$objective, objective, tolerance = 1e-12)
    expect_identical(surv_predict(unserialize(serialize(fit, NULL)), d), surv_predict(fit, d))
    legacy <- fit
    legacy$engine <- old
    expect_equal(surv_predict(legacy, d), surv_predict(fit, d), tolerance = 1e-8)
  }
})

test_that("Gehan iteration limits and native registration are explicit", {
  d <- example_data(40L)
  expect_warning(fit <- surv_fit(Surv(time, event) ~ x + z, d, "aft_ridge",
    list(max_iter = 1L), seed = 11), "max_iter|singular values")
  expect_identical(fit$engine$opt$convergence, 1L)
  expect_false(fit$engine$diagnostics$converged)
  registered <- getDLLRegisteredRoutines("survlearning")$.Call
  expect_true(all(c("sl_rfsrcGrow", "sl_rfsrcPredict", "sl_gehan_fit") %in% names(registered)))
})

test_that("Gehan supports a single predictor without a truncated SVD", {
  d <- example_data(50L)
  fit <- surv_fit(Surv(time, event) ~ x, d, "aft_ridge", list(lambda = .1))
  expect_true(all(is.finite(surv_predict(fit, d))))
  expect_equal(nrow(fit$engine$beta), 1L)
  expect_true(fit$engine$diagnostics$converged)
})

test_that("Royston-Parmar rejects duplicate quantile knots without changing the model", {
  d <- example_data(100L)
  d$time <- c(rep(1, 80), rep(2, 10), rep(3, 5), 4:8)
  d$event <- 1L
  for (scale in c("hazard", "odds", "normal")) {
    expect_error(surv_fit(Surv(time, event) ~ x, d, "flexsurv",
      list(knots = 2L, scale = scale)), "repeated knots")
    fit <- surv_fit(Surv(time, event) ~ x, d, "flexsurv",
      list(knots = 0L, scale = scale))
    curves <- surv_predict(fit, d[1:3, ], "survival", c(0, 1, 3, 8))
    expect_true(all(is.finite(curves)))
    expect_true(all(apply(curves, 1L, function(s) all(diff(s) <= 0))))
  }
})
