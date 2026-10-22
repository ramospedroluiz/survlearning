test_that("Cox survival is defined at zero for extreme finite log-risk", {
  d <- withr::with_seed(42L, data.frame(x = seq(-2, 2, length.out = 80),
    time = exp(-seq(-2, 2, length.out = 80) + stats::rnorm(80, 0, .4)), event = 1L))
  f <- survival::Surv(time, event) ~ x
  fit <- surv_fit(f, d, "cox")
  p <- surv_predict(fit, data.frame(x = c(1e4, -1e4)), "survival", c(0, .1, 1))
  expect_identical(unname(p[, 1]), c(1, 1))
  expect_true(all(is.finite(p)))
  expect_identical(unname(p[, 3]), c(0, 1))
  ordinary <- surv_predict(fit, d, "survival", c(0, .2, 1))
  reference <- summary(survival::survfit(fit$engine,
    newdata = survlearning:::.engine_data(survlearning:::.new_design(fit$design, d))),
    times = c(0, .2, 1), extend = TRUE)$surv
  expect_equal(unname(ordinary), unname(t(reference)), tolerance = 1e-12)
})

test_that("penalized Cox also handles extreme finite log-risk", {
  d <- example_data(100L)
  fit <- surv_fit(survival::Surv(time, event) ~ x + z, d, "cox_lasso")
  p <- surv_predict(fit, data.frame(x = c(1e8, -1e8), z = 0),
    "survival", c(0, min(d$time)))
  expect_identical(unname(p[, 1]), c(1, 1))
  expect_true(all(is.finite(p)))
})

test_that("all-censored Kaplan-Meier fits and probability validation are valid", {
  d <- data.frame(time = 1:12, event = 0L)
  f <- survival::Surv(time, event) ~ 1
  fit <- surv_fit(f, d, "km")
  expect_equal(fit$events, 0)
  expect_equal(unname(surv_predict(fit, d[1:2, ], "survival", c(0, 2, 12))),
    matrix(1, 2, 3))
  expect_identical(surv_predict(fit, d), rep(0, 12))
  expect_equal(surv_evaluate(fit, d, "brier", times = .5)$estimate, 0)
  validation <- surv_validate(f, d, methods = "km", metrics = "brier",
    times = .5, folds = 2, seed = 8, tune = FALSE)
  expect_true(all(validation$results$status == "ok"))
  expect_equal(validation$results$estimate, c(0, 0))
  tuned <- surv_tune(f, d, "km", folds = 2, metric = "brier", times = .5, seed = 8)
  expect_equal(tuned$best_model$events, 0)
  expect_error(surv_fit(survival::Surv(time, event) ~ time, d, "cox"), "observed event")
})

test_that("standardization does not rely on extended-precision accumulation", {
  x <- cbind(constant = rep(-1e307, 10), varying = seq(-1e307, 1e307, length.out = 10))
  moments <- survlearning:::.design_moments(x)
  expect_equal(moments$center[1], c(constant = -1e307))
  expect_equal(moments$scale[1], c(constant = 0))
  expect_equal(unname(moments$scale[2]) / 1e307, stats::sd(seq(-1, 1, length.out = 10)))
  scaled <- survlearning:::.scaled_design(x[, 2, drop = FALSE],
    list(center = moments$center[2], scale = moments$scale[2]))
  expect_true(all(is.finite(scaled)))
  expect_equal(stats::sd(scaled[, 1]), 1)
  tiny <- matrix(seq(-1, 1, length.out = 10) * 1e-307, ncol = 1)
  tiny_moments <- survlearning:::.design_moments(tiny)
  expect_equal(unname(tiny_moments$scale) / 1e-307,
    stats::sd(seq(-1, 1, length.out = 10)))
  expect_equal(stats::sd(survlearning:::.scaled_design(tiny, tiny_moments)[, 1]), 1)
  expect_error(survlearning:::.design_moments(matrix(c(rep(0, 99), 5e-324), ncol = 1)),
    "below the representable")
  expect_error(survlearning:::.design_moments(matrix(c(-1.7e308, 1.7e308), ncol = 1)),
    "not representable")
  d <- example_data(40L)
  d$huge_constant <- -1e307
  fit <- surv_fit(survival::Surv(time, event) ~ x + huge_constant, d)
  expect_identical(fit$design$columns, "x")
})

test_that("AFT upper tails are computed directly rather than by subtraction", {
  d <- example_data(80L)
  for (distribution in c("weibull", "lognormal", "loglogistic")) {
    fit <- surv_fit(survival::Surv(time, event) ~ x + z, d, "aft",
      list(distribution = distribution))
    new <- d[1:2, ]
    location <- as.numeric(stats::predict(fit$engine,
      survlearning:::.engine_data(survlearning:::.new_design(fit$design, new)), type = "lp"))
    horizon <- exp(location[1] + 9 * fit$engine$scale)
    p <- surv_predict(fit, new, "survival", c(0, horizon))
    expect_identical(unname(p[, 1]), c(1, 1))
    expected <- switch(distribution, weibull = exp(-exp(9)),
      lognormal = stats::pnorm(9, lower.tail = FALSE),
      loglogistic = stats::plogis(9, lower.tail = FALSE))
    if (expected == 0) expect_identical(unname(p[1, 2]), 0)
    else expect_equal(unname(p[1, 2]) / expected, 1, tolerance = 1e-12)
    if (distribution != "weibull") expect_gt(p[1, 2], 0)
  }
})

test_that("NULL seed uses normal RNG and explicit seeds restore it on error", {
  withr::local_seed(91)
  before <- .Random.seed
  d <- example_data(40L)
  surv_fit(survival::Surv(time, event) ~ x + z, d, "cox")
  expect_identical(.Random.seed, before)
  a <- surv_fit(survival::Surv(time, event) ~ x + z, d, "neural_cox", list(epochs = 2))
  expect_false(identical(.Random.seed, before))
  b <- surv_fit(survival::Surv(time, event) ~ x + z, d, "neural_cox", list(epochs = 2))
  expect_false(identical(a$engine$weights, b$engine$weights))
  previous <- .Random.seed
  expect_error(survlearning:::.with_seed(3, { stats::runif(2); stop("deliberate") }), "deliberate")
  expect_identical(.Random.seed, previous)
  expect_error(surv_tune(survival::Surv(time, event) ~ x, d,
    checkpoint = tempfile()), "explicit 'seed'")
  for (fun in list(surv_fit, surv_tune, surv_resamples, surv_validate,
    surv_evaluate, surv_calibrate, surv_importance)) expect_null(formals(fun)$seed)
})
