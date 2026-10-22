test_that("elastic net reproduces the LASSO and ridge endpoints", {
  data <- example_data()
  formula <- Surv(time, event) ~ x + z
  for (family in c("cox", "aft")) {
    engine <- if (family == "cox") "glmnet" else "penAFT"
    skip_if_not_installed(engine)
    for (alpha in c(0, 1)) {
      endpoint <- paste0(family, if (alpha == 1) "_lasso" else "_ridge")
      method <- paste0(family, "_elasticnet")
      a <- surv_fit(formula, data, endpoint, list(lambda = 0.01))
      b <- surv_fit(formula, data, method, list(lambda = 0.01, alpha = alpha))
      expect_equal(surv_predict(a, data), surv_predict(b, data), tolerance = 1e-8)
    }
    fit <- surv_fit(formula, data, method, list(lambda = 0.01, alpha = 0.5))
    expect_true(is.finite(surv_score(fit, data)))
    expect_error(surv_fit(formula, data, method, list(alpha = 1.1)), "alpha")
    expect_error(surv_fit(formula, data, method, list(alpha = -0.1)), "alpha")
    if (family == "cox") {
      expected <- surv_fit(formula, data, "cox_lasso", list(lambda = 0.01))
      endpoint <- surv_fit(formula, data, method, list(lambda = 0.01, alpha = 1))
      expect_equal(surv_predict(endpoint, data[1:2, ], "survival", c(0, 2, 5)),
        surv_predict(expected, data[1:2, ], "survival", c(0, 2, 5)), tolerance = 1e-8)
    }
  }
})

test_that("XGBoost Cox uses censored labels and a training-only Breslow baseline", {
  skip_if_not_installed("xgboost", minimum_version = "3.0.0")
  data <- example_data()
  data$time <- pmax(0.5, round(data$time))  # Exercise tied events and censoring.
  fit <- surv_fit(Surv(time, event) ~ x + z, data, "xgboost_cox",
    list(trees = 20L))
  prepared <- survlearning:::.prepare_design(fit$formula, data)
  risk <- surv_predict(fit, data)
  expect_equal(risk, as.numeric(predict(fit$engine, prepared$x, outputmargin = TRUE)))
  expect_true(is.finite(surv_score(fit, data)))
  times <- c(0, 2, 5)
  event_times <- sort(unique(data$time[data$event == 1]))
  jumps <- vapply(event_times, function(t) {
    sum(data$time == t & data$event == 1) / sum(exp(risk[data$time >= t]))
  }, numeric(1))
  cumulative <- vapply(times, function(t) sum(jumps[event_times <= t]), numeric(1))
  expected <- exp(-outer(exp(risk[1:3]), cumulative))
  expect_equal(unname(surv_predict(fit, data[1:3, ], "survival", times)),
    expected, tolerance = 1e-7)
  expect_warning(beyond <- surv_predict(fit, data[1, , drop = FALSE],
    "survival", fit$max_time + 1), "follow-up")
  expect_true(is.na(beyond[1, 1]))
  expect_error(surv_predict(fit, data[1:2, ], "time"), "native")
})

test_that("all XGBoost AFT errors give coherent curves and conditional medians", {
  skip_if_not_installed("xgboost", minimum_version = "3.0.0")
  data <- example_data()
  formula <- Surv(time, event) ~ x + z
  for (distribution in c("normal", "logistic", "extreme")) {
    fit <- surv_fit(formula, data, "xgboost_aft",
      list(trees = 20L, distribution = distribution, scale = 0.8))
    new <- data[1, c("x", "z"), drop = FALSE]
    x <- survlearning:::.new_design(fit$design, new)
    margin <- as.numeric(predict(fit$engine, x, outputmargin = TRUE))
    expect_equal(surv_predict(fit, new), -margin)
    expect_equal(as.numeric(predict(fit$engine, x)), exp(margin), tolerance = 1e-6)
    median <- surv_predict(fit, new, "time")
    error_median <- if (distribution == "extreme") log(log(2)) else 0
    expect_equal(median, exp(margin + 0.8 * error_median))
    expect_equal(unname(surv_predict(fit, new, "survival", median)[1, 1]), 0.5,
      tolerance = 1e-8)
    times <- c(0, 2, 5, max(data$time) + 10)
    curves <- surv_predict(fit, new, "survival", times)
    expect_equal(dim(curves), c(1L, 4L))
    expect_equal(unname(curves[1, 1]), 1)
    expect_true(all(curves >= 0 & curves <= 1 & is.finite(curves)))
    expect_true(all(diff(curves[1, ]) <= 0))
  }
  expect_error(surv_fit(formula, data, "xgboost_aft", list(distribution = "weibull")),
    "distribution")
  expect_error(surv_fit(formula, data, "xgboost_aft", list(scale = 0)), "scale")
})

test_that("MTLR curves match native predictions, including a single new item", {
  skip_if_not_installed("MTLR", minimum_version = "0.2.2")
  data <- example_data()
  fit <- surv_fit(Surv(time, event) ~ x + z, data, "mtlr", list(intervals = 5L))
  new <- data[1:2, c("x", "z")]
  x <- survlearning:::.scaled_design(survlearning:::.new_design(fit$design, new), fit$design)
  times <- c(5, 0, 2, 5)
  native <- predict(fit$engine, newdata = as.data.frame(x),
    type = "prob_times", times = sort(unique(times)), add_zero = TRUE)
  expected <- t(as.matrix(native[match(times, native$time), -1L]))
  curves <- surv_predict(fit, new, "survival", times)
  expect_equal(unname(curves), unname(expected))
  single <- surv_predict(fit, new[1, , drop = FALSE], "survival", times)
  expect_equal(dim(single), c(1L, 4L))
  expect_equal(unname(single), unname(curves[1, , drop = FALSE]))
  expect_true(all(curves >= 0 & curves <= 1))
  expect_equal(unname(curves[, 2L]), c(1, 1))
  expect_equal(unname(curves[, 1L]), unname(curves[, 4L]))
  expect_true(all(diff(surv_predict(fit, new, "survival", c(0, 2, 5))[1, ]) <= 0))
  expect_equal(fit$risk_horizon, max(data$time))
  grid <- fit$risk_grid
  survival <- surv_predict(fit, new, "survival", grid)
  restricted <- vapply(seq_len(nrow(new)), function(i) {
    sum(diff(grid) * (survival[i, -1L] + survival[i, -length(grid)]) / 2)
  }, numeric(1))
  expect_equal(surv_predict(fit, new), -restricted)
  expect_equal(surv_predict(fit, new[1, , drop = FALSE]), -restricted[1L])
  expect_true(is.finite(surv_score(fit, data)))
  expect_error(surv_predict(fit, new, "time"), "native")
  expect_warning(beyond <- surv_predict(fit, new, "survival", fit$max_time + 1), "follow-up")
  expect_true(all(is.na(beyond)))
})

test_that("new priorities use common tuning, training-only transforms, and RNG control", {
  data <- example_data()
  formula <- Surv(time, event) ~ x + z
  for (method in c("cox_elasticnet", "aft_elasticnet", "xgboost_cox", "xgboost_aft", "mtlr")) {
    engine <- survlearning:::.method_specs()[[method]]$engine
    skip_if_not_installed(engine)
    params <- switch(method,
      aft_elasticnet = list(lambda = 0.01),
      xgboost_cox = , xgboost_aft = list(trees = 10L),
      mtlr = list(intervals = 4L), list())
    grid <- if (method %in% c("cox_elasticnet", "aft_elasticnet")) {
      list(alpha = c(0.25, 0.75))
    } else if (method == "mtlr") list(lambda = c(0.5, 1)) else list(depth = c(1L, 2L))
    set.seed(58)
    before <- .Random.seed
    tuned <- surv_tune(formula, data, method, grid, params, folds = 2L, seed = 8L)
    expect_identical(.Random.seed, before)
    expect_true(all(tuned$results$failed_folds == 0))
    expect_true(all(is.finite(tuned$results$mean_cindex)))
    heldout <- tuned$fold_id == 1L
    setting <- params
    name <- names(grid)[1L]
    setting[name] <- list(grid[[1L]][1L])
    manual <- surv_fit(formula, data[!heldout, ], method, setting, seed = 8L)
    expect_equal(manual$design$center, colMeans(data[!heldout, c("x", "z")]))
    expect_equal(tuned$fold_results$cindex[1L], surv_score(manual, data[heldout, ]))
    if (method == "mtlr") expect_equal(manual$risk_horizon, max(data$time[!heldout]))
    new <- data[1:2, c("x", "z")]
    expect_true(all(is.finite(surv_predict(tuned, new))))
  }
})
