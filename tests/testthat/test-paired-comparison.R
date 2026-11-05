test_that("paired contrasts use common bootstrap rows and preserve the RNG", {
  d <- example_data(160)
  train <- d[1:100, ]
  test <- d[101:160, ]
  f <- Surv(time, event) ~ x + z
  models <- list(cox = surv_fit(f, train), aft = surv_fit(f, train, "aft"))
  set.seed(18)
  old <- .Random.seed
  result <- surv_compare(models, test, reference = "cox", n_boot = 40, seed = 9)
  expect_identical(.Random.seed, old)
  manual <- survlearning:::.with_seed(9, replicate(40, {
    index <- sample.int(nrow(test), nrow(test), replace = TRUE)
    surv_score(models$aft, test[index, ]) - surv_score(models$cox, test[index, ])
  }))
  expect_equal(c(result$lower, result$upper), unname(quantile(manual, c(.025, .975))))
  expect_equal(result$difference, surv_score(models$aft, test) - surv_score(models$cox, test))
  identical_models <- surv_compare(list(a = models$cox, b = models$cox), test,
    reference = "a", n_boot = 20, seed = 5)
  expect_equal(unlist(identical_models[c("difference", "lower", "upper")]),
    c(difference = 0, lower = 0, upper = 0))
  reverse <- surv_compare(models, test, reference = "aft", n_boot = 40, seed = 9)
  expect_equal(reverse$difference, -result$difference)
  expect_equal(c(reverse$lower, reverse$upper), -c(result$upper, result$lower))
})

test_that("paired IPCW metrics share one supported reference", {
  d <- example_data(140)
  f <- Surv(time, event) ~ x + z
  models <- list(a = surv_fit(f, d[1:80, ], "aft"), b = surv_fit(f, d[1:80, ]))
  test <- d[81:140, ]
  g <- surv_censor_fit(f, d[1:80, ], method = "cox")
  result <- surv_compare(models, test, reference = "a", metrics = c("brier", "auc", "ibs", "ipcw_c"),
    times = c(2, 4), tau = 4, censoring_model = g, n_boot = 12, seed = 3)
  a <- surv_evaluate(models$a, test, c("brier", "auc", "ibs", "ipcw_c"),
    times = c(2, 4), tau = 4, censoring_model = g)
  b <- surv_evaluate(models$b, test, c("brier", "auc", "ibs", "ipcw_c"),
    times = c(2, 4), tau = 4, censoring_model = g)
  expect_equal(result$difference, b$estimate - a$estimate)
  models$b <- surv_fit(f, d[1:75, ])
  expect_error(surv_compare(models, test, reference = "a", metrics = "brier", times = 2), "common censoring")
  expect_s3_class(surv_compare(models, test, reference = "a", metrics = "brier",
    times = 2, censoring_data = d[1:80, ]), "data.frame")
  expect_error(surv_compare(models, test, reference = "missing"), "reference")
  expect_error(surv_compare(models, test, reference = "a", n_boot = 1), "at least two")
  test$event <- 0
  none <- surv_compare(models, test, reference = "a", n_boot = 10, seed = 1)
  expect_true(is.na(none$difference))
  expect_identical(none$bootstrap_valid, 0L)
  expect_identical(none$status, "undefined")
})
