stability_sample <- function(n, seed, censoring = .35) {
  sample <- withr::with_seed(seed, {
    x <- matrix(rnorm(n * 3L), n, 3L)
    time <- exp(2 - 1.2 * x[, 1] + .7 * x[, 2] + rnorm(n, sd = .65))
    list(x = x, time = time)
  })
  endpoint <- withr::with_seed(100901, {
    z <- matrix(rnorm(300000), 100000, 3L)
    as.numeric(quantile(exp(2 - 1.2 * z[, 1] + .7 * z[, 2] +
      rnorm(100000, sd = .65)), 1 - censoring))
  })
  data.frame(time = pmin(sample$time, endpoint), event = as.integer(sample$time <= endpoint),
    x1 = sample$x[, 1], x2 = sample$x[, 2], x3 = sample$x[, 3])
}

test_that("unidentified AFT fits never return silent NA predictions", {
  d <- example_data()
  d$redundant <- d$x + d$z
  for (distribution in c("weibull", "lognormal", "loglogistic")) {
    expect_error(surv_fit(Surv(time, event) ~ x + z + redundant, d, "aft",
      list(distribution = distribution)), "full-rank")
    fit <- surv_fit(Surv(time, event) ~ x + z, d, "aft", list(distribution = distribution))
    fit$engine$coefficients[2] <- NA_real_
    expect_error(surv_predict(fit, d), "non-finite")
    expect_error(surv_predict(fit, d, "survival", c(0, 2, 5)), "non-finite")
    expect_error(surv_predict(fit, d, "time"), "non-finite")
  }
})

test_that("Royston-Parmar verifies the full spline rather than only event times", {
  d <- stability_sample(45, 7004, .7)
  result <- tryCatch(surv_fit(Surv(time, event) ~ x1 + x2 + x3, d,
    "flexsurv", list(knots = 3L)), error = identity)
  if (inherits(result, "error")) {
    expect_match(conditionMessage(result), "nonmonotone|converg|optimization failed")
  } else {
    expect_gt(result$engine$diagnostics$minimum_baseline_slope, 0)
    curves <- surv_predict(result, d[1:3, ], "survival", seq(.1, 20, length.out = 2000))
    expect_true(all(apply(curves, 1, function(s) all(diff(s) <= 1e-8))))
  }
  knots <- c(-1, -.3, .2, 1)
  withr::local_seed(738)
  for (i in 1:20) {
    gamma <- rnorm(4)
    exact <- survlearning:::.rp_minimum_slope(gamma, knots)
    dense <- min(survlearning:::.rp_basis(seq(-2, 2, length.out = 10001), knots, TRUE) %*% gamma)
    expect_lte(exact, dense + 1e-10)
    expect_equal(exact, dense, tolerance = 1e-6)
  }
})

test_that("Royston-Parmar uses training scaling without changing its family", {
  d <- example_data(100)
  scaled <- d; scaled$x <- 1e12 + 1e8 * d$x; scaled$z <- 1e-8 * d$z
  f <- Surv(time, event) ~ x + z
  for (scale in c("hazard", "odds", "normal")) {
    a <- surv_fit(f, d, "flexsurv", list(scale = scale))
    b <- surv_fit(f, scaled, "flexsurv", list(scale = scale))
    expect_equal(surv_predict(a, d), surv_predict(b, scaled), tolerance = 1e-5)
    expect_equal(surv_predict(a, d, "survival", c(0, 2, 5)),
      surv_predict(b, scaled, "survival", c(0, 2, 5)), tolerance = 1e-5)
    bad <- a; bad$engine$coefficients[2] <- -100
    expect_error(surv_predict(bad, d, "survival", c(0, 2, 5)), "invalid nonmonotone")
  }
})

test_that("XGBoost AFT initialization respects the training time unit", {
  d <- stability_sample(160, 404)
  f <- Surv(time, event) ~ x1 + x2 + x3
  times <- c(0, median(d$time), max(d$time))
  for (distribution in c("normal", "logistic", "extreme")) {
    fit <- surv_fit(f, d, "xgboost_aft", list(distribution = distribution), seed = 44)
    for (unit in c(1 / 365, 365)) {
      scaled <- d; scaled$time <- unit * scaled$time
      other <- surv_fit(f, scaled, "xgboost_aft", list(distribution = distribution), seed = 44)
      expect_equal(unname(surv_predict(fit, d, "survival", times)),
        unname(surv_predict(other, scaled, "survival", times * unit)), tolerance = 1e-4)
    }
  }
})

test_that("constant censored SVM optima are explicit and not relabeled as failures", {
  d <- stability_sample(400, 3209, .97)
  expect_warning(fit <- surv_fit(Surv(time, event) ~ x1 + x2 + x3, d, "svm"),
    "constant training scores")
  expect_true(fit$engine$diagnostics$constant_prediction)
  expect_identical(fit$engine$diagnostics$distinct_training_scores, 1L)
  expect_identical(fit$diagnostics$convergence$status, "reported_success")
  expect_equal(surv_score(fit, d), .5)
  expect_lt(fit$engine$diagnostics$relative_gap, 1e-5)
  expect_equal(fit$n, nrow(d))
})

test_that("survival validation respects requested order and documented tail NA", {
  fit <- surv_fit(Surv(time, event) ~ x + z, example_data(), "cox")
  expect_error(survlearning:::.validate_survival_prediction(matrix(c(1, .4, .6), 1),
    1L, c(0, 1, 2), fit), "nonmonotone")
  expect_silent(survlearning:::.validate_survival_prediction(matrix(c(.4, 1, .6), 1),
    1L, c(2, 0, 1), fit))
  expect_silent(survlearning:::.validate_survival_prediction(matrix(c(1, NA), 1),
    1L, c(0, fit$max_time + 1), fit))
})

test_that("CoxBoost initializes penalties for a single predictor", {
  d <- stability_sample(100, 651)
  f <- Surv(time, event) ~ x1
  for (penalty in c(10, 100, 1000)) {
    fit <- surv_fit(f, d, "coxboost", list(steps = 30L, penalty = penalty))
    prep <- survlearning:::.prepare_design(f, d)
    native_x <- cbind(prep$x, .sl_padding = 0)
    native <- CoxBoost::CoxBoost(time = d$time, status = d$event, x = native_x,
      stepno = 30L, penalty = penalty, standardize = TRUE)
    expect_true(fit$engine$.sl_padding)
    expect_true(all(fit$engine$coefficients[, 2] == 0))
    expect_equal(surv_predict(fit, d), as.numeric(predict(native, newdata = native_x,
      at.step = 30L, type = "lp")), tolerance = 1e-12)
    expect_true(all(is.finite(surv_predict(fit, d, "survival", c(0, 1, 3)))))
  }
  weak <- surv_fit(f, d, "coxboost", list(steps = 1L, penalty = 10))
  strong <- surv_fit(f, d, "coxboost", list(steps = 1L, penalty = 1000))
  expect_lt(abs(strong$engine$coefficients[2, 1]), abs(weak$engine$coefficients[2, 1]))
})

test_that("prediction rejects explicitly failed optimizers", {
  d <- example_data()
  fit <- surv_fit(Surv(time, event) ~ x + z, d, "aft")
  fit$diagnostics$convergence <- list(status = "reported_failure", code = 1L)
  expect_error(surv_predict(fit, d), "optimizer reported failure")
  expect_error(surv_predict(fit, d, "time"), "optimizer reported failure")
  expect_error(surv_predict(fit, d, "survival", 1), "optimizer reported failure")
})
