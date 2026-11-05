test_that("convex stacking weights match independent analytic and QP solutions", {
  set.seed(37)
  x <- matrix(runif(240), 80, 3)
  y <- rbinom(80, 1, .5)
  w <- runif(80, .5, 2)
  fit <- survlearning:::.stack_weights(x[, 1:2], y, w)
  difference <- x[, 1] - x[, 2]
  first <- max(0, min(1, sum(w * difference * (y - x[, 2])) / sum(w * difference^2)))
  expect_equal(fit$weights, c(first, 1 - first), tolerance = 1e-6)
  fit <- survlearning:::.stack_weights(x, y, w)
  independent <- quadprog::solve.QP(2 * crossprod(x, x * w) / nrow(x),
    2 * drop(crossprod(x, y * w)) / nrow(x), cbind(rep(1, 3), diag(3)), c(1, 0, 0, 0), meq = 1)
  expect_equal(fit$weights, independent$solution, tolerance = 1e-6)
  expect_lt(fit$gap_bound, 1e-7)
  expect_equal(fit$loss, mean(w * (y - drop(x %*% fit$weights))^2))
  duplicate <- survlearning:::.stack_weights(cbind(x[, 1], x[, 1]), y, w)
  expect_equal(sum(duplicate$weights), 1)
  expect_equal(duplicate$loss, mean(w * (y - x[, 1])^2))
})

test_that("stack uses disjoint OOF fits, all rows and common native curves", {
  d <- example_data(180)
  train <- d[1:120, ]
  test <- d[121:180, ]
  f <- Surv(time, event) ~ x + z
  set.seed(20)
  state <- .Random.seed
  stack <- surv_stack(f, train, c("cox", "aft"), times = c(2, 4), folds = 3, seed = 15)
  expect_identical(.Random.seed, state)
  expect_equal(stack$n, nrow(train))
  expect_equal(stack$events, sum(train$event))
  expect_equal(dim(stack$oof$survival), c(120, 2, 2))
  expect_true(all(vapply(stack$models, function(x) x$n == 120, logical(1))))
  for (k in seq_along(stack$resamples$splits)) {
    split <- stack$resamples$splits[[k]]
    expect_length(intersect(split$analysis, split$assessment), 0)
    independent <- surv_fit(f, train[split$analysis, ], "cox", seed = 15)
    expect_equal(unname(stack$oof$survival[split$assessment, , 1]),
      unname(surv_predict(independent, train[split$assessment, ], "survival", times = c(2, 4))))
  }
  times <- c(4, 0, 2, 4)
  expected <- Reduce(`+`, Map(function(model, weight) weight *
    surv_predict(model, test, "survival", times = times), stack$models, stack$weights))
  expect_equal(surv_predict(stack, test, "survival", times = times), expected)
  expect_equal(surv_predict(stack, test[, c("x", "z")]), surv_predict(stack, test))
  expect_true(is.finite(surv_score(stack, test)))
  expect_s3_class(surv_evaluate(stack, test, "brier", times = 4), "survlearning_evaluation")
  expect_s3_class(surv_reliability(stack, test[1:2, ], 4), "survlearning_reliability")
  expect_equal(summary(stack)$predictions, c("risk", "survival"))
  expect_output(print(stack), "Survival stack")
  expect_equal(surv_predict(unserialize(serialize(stack, NULL)), test), surv_predict(stack, test))
  expect_error(surv_predict(stack, test, "time"), "median")
  expect_warning(beyond <- surv_predict(stack, test, "survival", times = max(train$time) + 1), "follow-up")
  expect_true(all(is.na(beyond)))
  expect_error(surv_stack(f, train, c("cox", "svm"), times = 4), "risk-only")
  expect_error(surv_stack(f, train, times = c(4, 2)), "increasing")
  expect_error(surv_stack(f, train, times = 1e6), "follow-up")
})

test_that("stack nests supplied tuning and censoring inside each OOF fold", {
  d <- example_data(150)
  d$x[c(2, 12)] <- NA_real_
  f <- Surv(time, event) ~ x + z
  grid <- list(cox_ridge = list(lambda = c(.1, 1)))
  stack <- surv_stack(f, d, c("aft", "cox_ridge"), times = c(2, 4), grids = grid,
    folds = 3, inner_folds = 2, seed = 11, impute = "median",
    censoring = list(formula = f, method = "cox", impute = "median"))
  for (k in seq_along(stack$resamples$splits)) {
    split <- stack$resamples$splits[[k]]
    expect_equal(stack$tuning[[k]]$inner_resamples$data.n, length(split$analysis))
    expect_true(all(is.finite(stack$oof$ipcw[split$assessment, ])))
    independent <- surv_tune(f, d[split$analysis, ], "cox_ridge", grid = grid$cox_ridge,
      resamples = stack$tuning[[k]]$inner_resamples, seed = 11, impute = "median")
    expect_equal(unname(stack$oof$survival[split$assessment, , 2]),
      unname(surv_predict(independent, d[split$assessment, ], "survival", times = c(2, 4))))
  }
  repeated <- surv_resamples(f, d, folds = 3, repeats = 2, seed = 11)
  expect_error(surv_stack(f, d, times = 4, resamples = repeated), "single|one complete")
  expect_error(surv_stack(f, d, times = 4, censoring = surv_censor_fit(f, d, "km")), "formula")
})
