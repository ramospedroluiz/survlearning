test_that("catalogue lists capabilities without loading engines", {
  methods <- surv_methods()
  expect_gte(nrow(methods), 15L)
  expect_true(all(c("aft_lasso", "rsf", "svm") %in% methods$method))
  expect_identical(methods$predictions[methods$method == "svm"], "risk")
})

test_that("core fit, score, and native AFT predictions agree", {
  data <- example_data()
  fit <- surv_fit(survival::Surv(time, event) ~ x + z, data, "aft")
  new <- data[1:4, c("x", "z")]
  risk <- surv_predict(fit, new)
  x <- survlearning:::.new_design(fit$design, new)
  native <- -stats::predict(fit$engine, as.data.frame(x), type = "lp")
  expect_equal(risk, unname(native))
  expect_equal(predict(fit, new), risk)
  expect_true(surv_score(fit, data) > 0.75)
  medians <- surv_predict(fit, new, "time")
  expect_true(all(is.finite(medians) & medians > 0))
  curves <- surv_predict(fit, new, "survival", c(0, 2, 10, 20))
  expect_equal(dim(curves), c(4L, 4L))
  expect_equal(unname(curves[, 1L]), rep(1, 4))
  expect_true(all(curves >= 0 & curves <= 1))
  expect_true(all(apply(curves, 1, diff) <= 0))
})

test_that("Cox native survival probabilities match survfit", {
  data <- example_data()
  fit <- surv_fit(survival::Surv(time, event) ~ x + z, data)
  new <- data[1:3, c("x", "z")]
  times <- c(0, 2, 5)
  curves <- surv_predict(fit, new, "survival", times)
  x <- survlearning:::.new_design(fit$design, new)
  direct <- survival::survfit(fit$engine, newdata = as.data.frame(x))
  expected <- t(summary(direct, times = times, extend = TRUE)$surv)
  expect_equal(unname(curves), unname(expected), tolerance = 1e-8)
  expect_warning(beyond <- surv_predict(fit, new, "survival", max(data$time) + 1), "follow-up")
  expect_true(all(is.na(beyond)))
})

test_that("factor encoding is fixed and new outcomes are unnecessary", {
  data <- example_data()
  fit <- surv_fit(survival::Surv(time, event) ~ x * batch + z, data, "aft")
  new <- data[1:3, c("x", "z", "batch")]
  expect_length(surv_predict(fit, new), 3L)
  new$batch <- as.character(new$batch)
  expect_length(surv_predict(fit, new), 3L)
  new$batch[1L] <- "unknown"
  expect_error(surv_predict(fit, new), "new level")
})

test_that("invalid inputs fail clearly without silently changing the model", {
  data <- example_data()
  formula <- survival::Surv(time, event) ~ x + z
  expect_error(surv_fit(formula, data, "unknown"), "Choose one")
  expect_error(surv_fit(formula, data, "cox", list(lambda = 1)), "Unknown")
  expect_error(surv_fit(formula, data, "cox_lasso", list(lambda = -1)), "positive")
  expect_error(surv_fit(formula, data, "svm", list(kernel = "unknown")), "kernel")
  bad <- data
  bad$x[1L] <- NA_real_
  expect_error(surv_fit(formula, bad), "missing")
  bad <- data
  bad$time[1L] <- 0
  expect_error(surv_fit(formula, bad), "positive")
  bad <- data
  bad$event <- 0L
  expect_error(surv_fit(formula, bad), "event")
  bad <- data
  bad$event <- bad$event + 1L
  expect_error(surv_fit(formula, bad), "Encode")
  fit <- surv_fit(formula, data)
  expect_error(surv_predict(fit, data[1:2, ], "time"), "native")
  expect_error(surv_predict(fit, data[1:2, ], "survival", -1), "non-negative")
  expect_error(surv_predict(fit, data[1:2, ], times = 1), "only used")
  expect_error(predict(fit, data[1:2, ], arbitrary = 2), "Unused")
  expect_error(surv_fit(survival::Surv(time, event) ~ x + offset(z), data), "offset")
  expect_error(surv_fit(survival::Surv(time, event) ~ x + survival::strata(batch), data), "Strata")
  expect_error(surv_fit(survival::Surv(time, event) ~ 0 + x, data), "intercept")
})

test_that("local seeds do not alter the caller's RNG", {
  data <- example_data()
  set.seed(42)
  before <- .Random.seed
  surv_fit(survival::Surv(time, event) ~ x + z, data, "aft", seed = 5)
  expect_identical(.Random.seed, before)
})

test_that("named event arguments and formulas with dots work", {
  data <- example_data()
  fit <- surv_fit(survival::Surv(event = event, time = time) ~ ., data, "aft")
  expect_length(surv_predict(fit, data[1:2, c("x", "z", "batch")]), 2L)
})

test_that("formula transformations retain their training design", {
  data <- example_data()
  fit <- surv_fit(Surv(time, event) ~ log(abs(x) + 1) + I(z^2), data, "aft")
  expect_true(all(is.finite(surv_predict(fit, data[1:3, c("x", "z")]))))
  poly_fit <- surv_fit(Surv(time, event) ~ stats::poly(x, 2) + z, data, "aft")
  new <- data[1:3, c("x", "z")]
  x <- survlearning:::.new_design(poly_fit$design, new)
  native <- -stats::predict(poly_fit$engine, newdata = as.data.frame(x), type = "lp")
  expect_equal(surv_predict(poly_fit, new), unname(native))
})
