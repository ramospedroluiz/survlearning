for (id in c("aft_lasso", "aft_ridge", "cox_lasso", "cox_ridge", "tree", "rsf", "boosting", "svm")) {
  local({
    method <- id
    test_that(paste("required engine works:", method), {
      data <- example_data()
      fit <- surv_fit(survival::Surv(time, event) ~ x + z, data,
        method, small_params(method), seed = 8)
      new <- data[1:3, c("x", "z")]
      risk <- surv_predict(fit, new)
      expect_length(risk, 3L)
      expect_true(all(is.finite(risk)))
      expect_true(is.finite(surv_score(fit, data)))
      if (method %in% c("rsf", "cox_lasso", "cox_ridge")) {
        curves <- surv_predict(fit, new, "survival", c(0, 2, 5))
        expect_equal(dim(curves), c(3L, 3L))
        expect_equal(unname(curves[, 1L]), rep(1, 3))
        expect_true(all(curves >= 0 & curves <= 1))
        expect_true(all(apply(curves, 1, diff) <= 0))
      } else {
        expect_error(surv_predict(fit, new, "survival", 1), "native")
      }
      if (method == "svm") {
        expect_identical(fit$n, nrow(data))
        x <- survlearning:::.scaled_design(survlearning:::.new_design(fit$design, new), fit$design)
        native <- -as.numeric(predict(fit$engine, newdata = as.data.frame(x))$predicted)
        expect_equal(risk, native)
        additive <- surv_fit(survival::Surv(time, event) ~ x + z, data,
          method, list(kernel = "additive"), seed = 8)
        expect_true(all(is.finite(surv_predict(additive, new))))
      }
      if (method %in% c("cox_lasso", "cox_ridge")) {
        prepared <- survlearning:::.prepare_design(fit$formula, data)
        x <- survlearning:::.new_design(fit$design, new)
        native <- survival::survfit(fit$engine, x = prepared$x, y = prepared$y,
          newx = x, s = fit$params$lambda)
        expected <- t(summary(native, times = c(0, 2, 5), extend = TRUE)$surv)
        expect_equal(unname(curves), unname(expected), tolerance = 1e-6)
      }
    })
  })
}

test_that("penalized Cox also supports a single predictor", {
  data <- example_data()
  for (method in c("cox_lasso", "cox_ridge")) {
    fit <- surv_fit(survival::Surv(time, event) ~ x, data, method)
    expect_length(surv_predict(fit, data[1:2, "x", drop = FALSE]), 2L)
    expect_equal(dim(surv_predict(fit, data[1:2, "x", drop = FALSE],
      "survival", c(0, 3))), c(2L, 2L))
  }
})
