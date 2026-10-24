test_that("penalized Cox reaches the requested penalty on a descending path", {
  d <- example_data(150L)
  f <- survival::Surv(time, event) ~ x + z
  for (method in c("cox_lasso", "cox_ridge", "cox_elasticnet")) {
    for (lambda in c(1e-4, .01, 1e4)) {
      fit <- surv_fit(f, d, method, list(lambda = lambda))
      expect_identical(tail(fit$engine$lambda, 1L), lambda)
      expect_true(all(diff(fit$engine$lambda) < 0))
      expect_identical(fit$engine$jerr, 0L)
      expect_identical(fit$params$lambda, lambda)
      expect_true(all(is.finite(surv_predict(fit, d))))
      curves <- surv_predict(fit, d[1:3, ], "survival", c(0, 1, 2))
      expect_true(all(is.finite(curves)))
      expect_true(all(curves >= 0 & curves <= 1))
    }
  }
})

test_that("a weak penalized Cox fit agrees with an independent dense glmnet path", {
  d <- example_data(300L)
  f <- survival::Surv(time, event) ~ x + z
  x <- model.matrix(~ x + z, d)[, -1, drop = FALSE]
  y <- survival::Surv(d$time, d$event)
  for (alpha in c(0, .5, 1)) {
    fit <- surv_fit(f, d, "cox_elasticnet", list(lambda = .001, alpha = alpha))
    reference <- glmnet::glmnet(x, y, family = "cox", alpha = alpha,
      lambda = 10^seq(2, -3, length.out = 250), cox.ties = "breslow")
    actual <- surv_predict(fit, d)
    expected <- as.numeric(predict(reference, newx = x, s = .001, type = "link"))
    expect_lt(max(abs(actual - expected)), .01)
    expect_lt(abs(surv_score(fit, d) - survival::concordance(y ~ expected,
      reverse = TRUE)$concordance), .001)
  }
})
