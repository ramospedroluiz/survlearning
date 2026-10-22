test_that("reliability summaries agree with fitted Weibull conditional survival", {
  d <- example_data(100)
  fit <- surv_fit(Surv(time, event) ~ x + z, d, "aft")
  new <- d[1:2, c("x", "z")]
  a <- surv_reliability(fit, new, horizons = c(3, 1), age = 1, grid_points = 501)
  expected <- surv_predict(fit, new, "survival", times = c(4, 2)) /
    as.vector(surv_predict(fit, new, "survival", times = 1))
  expect_equal(a$summary$survival, as.vector(t(expected)))
  lp <- as.vector(predict(fit$engine, newdata = survlearning:::.engine_data(
    survlearning:::.new_design(fit$design, new)), type = "lp"))
  shape <- 1 / fit$engine$scale
  for (i in 1:2) for (h in c(1, 3)) {
    truth <- integrate(function(u) exp(-((1 + u) / exp(lp[i]))^shape +
      (1 / exp(lp[i]))^shape), 0, h)$value
    expect_equal(a$summary$restricted_remaining_mean[a$summary$observation == i &
      a$summary$horizon == h], truth, tolerance = 1e-5)
  }
  expect_equal(a$conditional_survival[, 1], c(1, 1))
  expect_true(all(a$summary$restricted_remaining_mean <= a$summary$horizon))
  b <- surv_reliability(fit, new[1, , drop = FALSE], horizons = .01, probs = .99)
  expect_true(is.na(b$quantiles$remaining_time))
  expect_identical(b$quantiles$status, "not_reached_within_horizon")
})

test_that("risk-only models and unsupported horizons cannot invent remaining lives", {
  d <- example_data()
  fit <- surv_fit(Surv(time, event) ~ x + z, d)
  expect_error(surv_reliability(fit, d[1, ], horizons = max(d$time) + 1), "exceeds")
  expect_error(surv_reliability(fit, d[1, ], horizons = max(d$time) + 1, extrapolate = TRUE), "parametric")
  expect_error(surv_reliability(fit, d[1, ], 1, age = -1), "age")
  expect_error(surv_reliability(fit, d[1, ], 1, probs = 1), "probs")
  expect_error(surv_reliability(fit, d[1, ], 0), "positive")
  tree <- surv_fit(Surv(time, event) ~ x + z, d, "tree")
  expect_error(surv_reliability(tree, d[1, ], 1), "native")
  aft <- surv_fit(Surv(time, event) ~ x + z, d, "aft")
  a <- surv_reliability(aft, d[1, ], max(d$time) + 1, extrapolate = TRUE)
  expect_true(a$settings$extrapolation_used)
  path <- tempfile(fileext = ".pdf")
  grDevices::pdf(path)
  plot(a)
  plot(a, type = "failure")
  grDevices::dev.off()
  expect_output(print(a), "conditional")
  unlink(path)
})
