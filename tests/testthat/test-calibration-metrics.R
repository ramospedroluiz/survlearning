test_that("IPCW calibration measures equal independent weighted GLM fits", {
  d <- example_data(240)
  f <- Surv(time, event) ~ x + z
  train <- d[1:140, ]
  test <- d[141:240, ]
  fit <- surv_fit(f, train, "aft", params = list(distribution = "lognormal"))
  for (method in c("km", "cox")) {
    g <- surv_censor_fit(f, train, method)
    result <- surv_calibrate(fit, test, times = 5, type = "metrics", censoring_model = g)
    cases <- test$event == 1 & test$time <= 5
    controls <- test$time > 5
    w <- numeric(nrow(test))
    w[cases] <- 1 / surv_censor_predict(g, test[cases, ], test$time[cases], left = TRUE, pairwise = TRUE)
    w[controls] <- 1 / surv_censor_predict(g, test[controls, ], 5)[, 1]
    eta <- qlogis(1 - surv_predict(fit, test, "survival", times = 5)[, 1])
    input <- data.frame(event = as.integer(cases), eta, w)
    joint <- glm(event ~ eta, input, weights = w, family = quasibinomial())
    offset <- glm(event ~ offset(eta), input, weights = w, family = quasibinomial())
    expect_equal(result$estimate, unname(c(coef(offset)[1], coef(joint)[2])))
    expect_equal(result$effective_n, rep(sum(w)^2 / sum(w^2), 2))
    expect_equal(result$known, rep(sum(w > 0), 2))
  }
  set.seed(10)
  state <- .Random.seed
  boot <- surv_calibrate(fit, test, 5, type = "metrics", n_boot = 30, seed = 10)
  expect_identical(.Random.seed, state)
  expect_true(all(is.finite(boot$lower)))
  expect_equal(boot$ideal, c(0, 1))
  expect_equal(surv_calibrate(fit, test, 5, type = "metrics")$estimate,
    surv_calibrate(fit, test, 5, type = "metrics",
      censoring_model = surv_censor_fit(f, train, "km"))$estimate)
})

test_that("calibration diagnoses boundaries, separation and degenerate outcomes", {
  calculate <- survlearning:::.calibration_logistic
  constant <- calculate(rep(c(0, 1), 10), rep(.25, 20), rep(1, 20))
  expect_equal(constant$estimate[1], qlogis(.5) - qlogis(.25), tolerance = 1e-6)
  expect_true(is.na(constant$estimate[2]))
  separated <- calculate(c(0, 0, 1, 1), c(.1, .2, .8, .9), rep(1, 4))
  expect_true(is.na(separated$estimate[2]))
  expect_match(separated$message[2], "separation")
  boundary <- calculate(c(0, 1), c(0, .5), c(1, 1))
  expect_true(all(is.na(boundary$estimate)))
  expect_match(boundary$message[1], "no clipping")
  expect_true(all(is.na(calculate(rep(1, 5), rep(.5, 5), rep(1, 5))$estimate)))
  d <- example_data(120)
  fit <- surv_fit(Surv(time, event) ~ x + z, d[1:80, ], "aft")
  expect_error(surv_calibrate(fit, d[81:120, ], 1e6, type = "metrics"), "follow-up")
  expect_error(surv_calibrate(fit, d[81:120, ], 4, type = "metrics", n_boot = 1), "at least two")
  expect_error(surv_calibrate(fit, d[81:120, ], 4, type = "wrong"), "arg")
  d$event <- 0
  expect_warning(none <- surv_calibrate(fit, d[81:120, ], 4, type = "metrics"), "undefined")
  expect_true(all(is.na(none$estimate)))
})
