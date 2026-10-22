test_that("tuning is reproducible, stratified, and refits the winner", {
  data <- example_data()
  formula <- survival::Surv(time, event) ~ x + z
  grid <- list(distribution = c("weibull", "lognormal"))
  a <- surv_tune(formula, data, "aft", grid, folds = 3, seed = 7)
  b <- surv_tune(formula, data, "aft", grid, folds = 3, seed = 7)
  expect_identical(a$fold_id, b$fold_id)
  expect_equal(a$results$mean_cindex, b$results$mean_cindex)
  expect_true(all(table(a$fold_id, data$event)[, "1"] > 0))
  expect_equal(a$best_candidate, which.max(a$results$mean_cindex))
  expect_equal(a$best_model$n, nrow(data))
  expect_equal(a$best_model$design$center, colMeans(data[c("x", "z")]))
  expect_equal(surv_predict(a, data[1:2, ]), predict(a$best_model, data[1:2, ]))
  expect_equal(predict(a, data[1:2, ]), surv_predict(a, data[1:2, ]))
  expect_error(surv_tune(formula, data, "aft", grid, folds = 1), "integer")
  expect_error(surv_tune(formula, data, "aft", grid, folds = 80), "fewer folds")
  expect_error(surv_tune(formula, data, "aft", list(unknown = 1)), "Unknown")
})

test_that("failed candidates cannot win on a partial set of folds", {
  data <- example_data()
  tuned <- surv_tune(survival::Surv(time, event) ~ x + z, data, "aft",
    list(distribution = c("invalid", "weibull")), folds = 3)
  expect_equal(tuned$results$failed_folds, c(3L, 0L))
  expect_true(is.na(tuned$results$mean_cindex[1]))
  expect_identical(tuned$best_candidate, 2L)
  expect_true(all(nzchar(tuned$fold_results$error[1:3])))
})

test_that("a model without tuning parameters uses the same validation interface", {
  data <- example_data()
  tuned <- surv_tune(Surv(time, event) ~ x + z, data, "cox", folds = 3)
  expect_equal(nrow(tuned$results), 1L)
  expect_true(is.finite(tuned$results$mean_cindex))
  expect_equal(tuned$best_model$method, "cox")
})

test_that("fold-only standardization is used and test data is not needed", {
  skip_if_not_installed("clarabel", minimum_version = "0.11.3")
  data <- example_data(36L)
  tuned <- surv_tune(survival::Surv(time, event) ~ x + z, data, "svm",
    list(cost = c(0.1, 0.2)), params = list(solver = "clarabel"), folds = 2, seed = 12)
  heldout <- tuned$fold_id == 1
  manual <- surv_fit(survival::Surv(time, event) ~ x + z, data[!heldout, ],
    "svm", list(cost = 0.1, solver = "clarabel"), seed = 12)
  expect_equal(manual$design$center, colMeans(data[!heldout, c("x", "z")]))
  expect_equal(tuned$fold_results$cindex[1], surv_score(manual, data[heldout, ]))
})
