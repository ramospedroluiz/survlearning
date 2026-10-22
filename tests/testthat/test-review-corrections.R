test_that("classical summaries and generics retain original coefficient names", {
  d <- example_data(90)
  f <- Surv(time, event) ~ x + z
  for (method in c("cox", "aft")) {
    model <- surv_fit(f, d, method)
    native <- coef(model$engine)
    expect_equal(unname(coef(model)), unname(native))
    expect_equal(names(coef(model)), if (method == "cox") c("x", "z") else c("(Intercept)", "x", "z"))
    expect_equal(unname(vcov(model)), unname(vcov(model$engine)[names(native), names(native)]))
    value <- summary(model)
    expect_s3_class(value, "summary.survlearning_fit")
    expect_equal(value$n, nrow(d))
    expect_equal(value$coefficients$estimate, unname(native))
    expect_output(print(value), "fixed-model")
    if (method == "cox") expect_equal(value$coefficients$hazard_ratio, exp(unname(native)))
  }
  km <- surv_fit(f, d, "km")
  expect_null(summary(km)$coefficients)
  expect_error(coef(km), "ordinary cox and aft")
  expect_error(vcov(km), "ordinary cox and aft")
  tuned <- surv_tune(f, d, "cox", folds = 2)
  expect_equal(coef(tuned), coef(tuned$best_model))
  expect_equal(vcov(tuned), vcov(tuned$best_model))
  expect_equal(summary(tuned)$search$candidates, 1L)
  expect_output(print(tuned), "no hyperparameter search")
  expect_false(tuned$search$hyperparameter_search)
})

test_that("selection is independent of report ordering and has an explicit horizon", {
  d <- example_data(100)
  f <- Surv(time, event) ~ x + z
  grid <- list(aft = list(distribution = c("weibull", "lognormal")))
  a <- surv_validate(f, d, "aft", grids = grid, folds = 2, inner_folds = 2,
    metrics = c("harrell", "brier"), times = 2, seed = 123)
  b <- surv_validate(f, d, "aft", grids = grid, folds = 2, inner_folds = 2,
    metrics = c("brier", "harrell"), times = 2, seed = 123)
  expect_identical(a$metric, "harrell")
  for (i in seq_along(a$tuning)) {
    expect_equal(a$tuning[[i]]$aft$best_params, b$tuning[[i]]$aft$best_params)
    expect_equal(a$tuning[[i]]$aft$results$mean_estimate,
      b$tuning[[i]]$aft$results$mean_estimate)
  }
  selected <- surv_validate(f, d, "aft", grids = grid, folds = 2, inner_folds = 2,
    metrics = c("harrell", "brier"), times = c(1, 2), metric = "brier", selection_time = 2)
  expect_true(all(is.finite(selected$results$estimate)))
  expect_identical(selected$metric, "brier")
  expect_error(surv_tune(f, d, metric = "brier"), "times")
  expect_error(surv_tune(f, d, metric = "unknown"), "Unknown metric")
  expect_error(surv_validate(f, d, metric = "brier", times = c(1, 2)), "scalar metric")
  expect_error(surv_validate(f, d, selection_time = 2), "only used")
})

test_that("IPCW calibration agrees with explicit normalized case-control weights", {
  d <- example_data(120)
  d$batch <- factor(rep(c("A", "B"), 60))
  f <- Surv(time, event) ~ x + z
  model <- surv_fit(f, d)
  g <- surv_censor_fit(Surv(time, event) ~ batch, d, "stratified")
  horizon <- 2
  observed <- suppressWarnings(surv_calibrate(model, d, horizon, groups = 1, censoring_model = g))
  cases <- d$event == 1 & d$time <= horizon
  controls <- d$time > horizon
  event_mass <- sum(1 / surv_censor_predict(g, d[cases, ], d$time[cases], left = TRUE, pairwise = TRUE))
  control_mass <- sum(1 / surv_censor_predict(g, d[controls, ], horizon))
  expect_equal(observed$observed, control_mass / (control_mass + event_mass))
  expect_true(is.na(observed$lower))
  expect_identical(attr(observed, "settings")$estimator, "normalized IPCW")
  set.seed(98)
  rng <- .Random.seed
  a <- suppressWarnings(surv_calibrate(model, d, horizon, groups = 2,
    censoring_model = g, n_boot = 30, seed = 9))
  expect_identical(.Random.seed, rng)
  b <- suppressWarnings(surv_calibrate(model, d, horizon, groups = 2,
    censoring_model = g, n_boot = 30, seed = 9))
  expect_equal(a, b)
  expect_true(all(a$observed >= 0 & a$observed <= 1))
  expect_true(all(a$lower >= 0 & a$upper <= 1))
  expect_true(all(a$bootstrap_valid == 30))
  expect_error(surv_calibrate(model, d, 2, censoring_model = list()), "surv_censor_fit")
  expect_error(surv_calibrate(model, d, 2, n_boot = 2), "explicit censoring_model")
  expect_error(surv_calibrate(model, d, 2, censoring_model = g, n_boot = 1), "at least two")
  file <- tempfile(fileext = ".pdf")
  grDevices::pdf(file)
  expect_identical(plot(a), a)
  grDevices::dev.off()
  unlink(file)
})

test_that("the public default SVM is compact without observation removal", {
  expect_identical(surv_parameters("svm")$solver, "clarabel")
  d <- example_data(3000)
  model <- surv_fit(Surv(time, event) ~ x + z, d, "svm")
  expect_equal(model$n, 3000L)
  expect_identical(model$params$solver, "clarabel")
  expect_true(all(is.finite(surv_predict(model, d[1:2, ]))))
})
