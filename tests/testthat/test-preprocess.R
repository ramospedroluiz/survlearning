test_that("imputation learns predictor medians and modes only on training", {
  d <- example_data()
  d$x[c(1, 3)] <- NA_real_
  d$batch[4] <- NA
  f <- Surv(time, event) ~ x + z + batch
  expect_error(surv_fit(f, d), "missing")
  fit <- surv_fit(f, d, "aft", impute = "median")
  expect_equal(fit$design$imputation$x, median(d$x, na.rm = TRUE))
  expect_equal(fit$design$imputation$batch, "A")
  new <- d[1:3, ]
  new$x <- c(NA, 10000, -10000)
  pred <- surv_predict(fit, new)
  filled <- new
  filled$x[1] <- median(d$x, na.rm = TRUE)
  expect_equal(pred, surv_predict(fit, filled))
  expect_true(is.finite(surv_score(fit, d)))
  expect_equal(unname(fit$training_outcome), unname(Surv(d$time, d$event)))
  expect_identical(fit$impute, "median")
})

test_that("imputation rejects absent training information and missing outcomes", {
  d <- example_data()
  f <- Surv(time, event) ~ x + z
  bad <- d
  bad$x <- NA_real_
  expect_error(surv_fit(f, bad, impute = "median"), "all training")
  bad <- d
  bad$time[1] <- NA_real_
  expect_error(surv_fit(f, bad, impute = "median"), "missing")
  bad <- d
  bad$event[1] <- NA_real_
  expect_error(surv_fit(f, bad, impute = "median"), "missing")
  bad <- d
  bad$x[1] <- Inf
  expect_error(surv_fit(f, bad, impute = "median"), "finite")
})

test_that("outcomes are extracted without evaluating missing predictors", {
  d <- example_data()
  d$x <- NA_real_
  y <- survlearning:::.outcome(Surv(time, event) ~ log(x), d)
  expect_equal(unname(y), unname(Surv(d$time, d$event)))
  d$event <- 0
  expect_equal(sum(survlearning:::.outcome(Surv(time, event) ~ x, d)[, 2]), 0)
  expect_error(survlearning:::.outcome(Surv(time, event) ~ x, d,
    require_event = TRUE), "observed event")
})

test_that("fit provenance does not pretend to assess all engine convergence", {
  d <- example_data()
  fit <- surv_fit(Surv(time, event) ~ x + z, d, "aft")
  expect_type(fit$diagnostics$warnings, "character")
  expect_true(fit$diagnostics$convergence$status %in%
    c("reported_success", "not_reported"))
  expect_identical(fit$diagnostics$engine_version,
    as.character(utils::packageVersion("survival")))
})
