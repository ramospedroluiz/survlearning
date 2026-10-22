test_that("inspection and comparison preserve explicit evaluation semantics", {
  d <- example_data()
  f <- Surv(time, event) ~ x + z
  models <- list(aft = surv_fit(f, d, "aft"), cox = surv_fit(f, d))
  expect_identical(surv_parameters("aft")$distribution, "weibull")
  expect_identical(surv_diagnostics(models$aft)$method, "aft")
  result <- surv_compare(models, d)
  expect_equal(result$model, names(models))
  expect_equal(result$estimate, vapply(models, surv_score, numeric(1), newdata = d),
    ignore_attr = TRUE)
  expect_error(surv_compare(unname(models), d), "named")
  expect_error(surv_diagnostics(1), "must come")
})

test_that("evaluation plots render without mutating the results", {
  d <- example_data()
  fit <- surv_fit(Surv(time, event) ~ x + z, d, "aft")
  result <- surv_evaluate(fit, d, metrics = "brier", times = c(1, 2, 3))
  file <- tempfile(fileext = ".pdf")
  grDevices::pdf(file)
  on.exit({ grDevices::dev.off(); unlink(file) }, add = TRUE)
  expect_identical(plot(result, metric = "brier"), result)
  expect_error(plot(result, metric = "unknown"), "not present")
})
