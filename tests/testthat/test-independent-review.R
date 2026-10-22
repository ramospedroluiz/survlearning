test_that("dot censoring uses the original response and training-only imputation", {
  d <- example_data(140)[c("time", "event", "x", "z")]
  d$time <- pmax(1, round(d$time))
  for (events in list(d$event, rep(1L, nrow(d)), rep(0L, nrow(d)))) {
    d$event <- events
    d$x[c(2, 8)] <- NA_real_
    dot <- surv_censor_fit(Surv(time, event) ~ ., d, impute = "median")
    explicit <- surv_censor_fit(Surv(time, event) ~ x + z, d, impute = "median")
    expect_setequal(all.vars(dot$design$terms), c("x", "z"))
    expect_equal(dot$design$imputation$x, median(d$x, na.rm = TRUE))
    expect_equal(dot$coefficients, explicit$coefficients)
    new <- d[1:7, c("x", "z")]
    for (left in c(FALSE, TRUE)) {
      expect_equal(surv_censor_predict(dot, new, c(0, 1, 2, 1), left),
        surv_censor_predict(explicit, new, c(0, 1, 2, 1), left), tolerance = 1e-12)
      expect_equal(surv_censor_predict(dot, new, rep(2, 7), left, pairwise = TRUE),
        surv_censor_predict(explicit, new, rep(2, 7), left, pairwise = TRUE))
    }
  }
  d$event[1] <- NA
  expect_error(surv_censor_fit(Surv(time, event) ~ ., d, impute = "median"), "missing")
})

test_that("dot censoring is fitted only to each tuning analysis fold", {
  d <- example_data(140)[c("time", "event", "x", "z")]
  a <- surv_tune(Surv(time, event) ~ ., d, "cox", folds = 3,
    metric = "brier", times = 2, censoring = Surv(time, event) ~ ., seed = 17)
  b <- surv_tune(Surv(time, event) ~ x + z, d, "cox", folds = 3,
    metric = "brier", times = 2, censoring = Surv(time, event) ~ x + z, seed = 17)
  expect_equal(a$fold_results$estimate, b$fold_results$estimate, tolerance = 1e-12)
  id <- a$fold_id
  fit <- surv_fit(Surv(time, event) ~ ., d[id != 1, ])
  g <- surv_censor_fit(Surv(time, event) ~ ., d[id != 1, ])
  expected <- surv_evaluate(fit, d[id == 1, ], "brier", times = 2, censoring_model = g)
  expect_equal(a$fold_results$estimate[1], expected$estimate, tolerance = 1e-12)
})

test_that("interpretation resolves training dot terms without assessment leakage", {
  d <- example_data(120)[c("time", "event", "x", "z")]
  dot <- surv_fit(Surv(time, event) ~ ., d)
  explicit <- surv_fit(Surv(time, event) ~ x + z, d)
  assessment <- d
  assessment$unseen_column <- seq_len(nrow(d))
  a <- surv_importance(dot, assessment, n_repeats = 3, seed = 8)
  b <- surv_importance(explicit, assessment, n_repeats = 3, seed = 8)
  expect_equal(a, b, tolerance = 1e-12)
  expect_equal(attr(a, "permutations"), attr(b, "permutations"), tolerance = 1e-12)
  expect_equal(surv_ale(dot, d[c("x", "z")], "x", bins = 5),
    surv_ale(explicit, d[c("x", "z")], "x", bins = 5), tolerance = 1e-12)
  for (name in c("time", "event", "unseen_column")) {
    expect_error(surv_importance(dot, assessment, name), "predictor")
    expect_error(surv_ale(dot, assessment, name), "predictor")
  }
  transformed <- surv_fit(Surv(time, event) ~ log(abs(x) + 1) * z, d)
  expect_setequal(survlearning:::.interpret_predictors(transformed), c("x", "z"))
  expect_true(all(is.finite(surv_ale(transformed, d, "x")$ale)))
  path <- tempfile(fileext = ".rds")
  on.exit(unlink(path))
  saveRDS(dot, path)
  expect_equal(surv_importance(readRDS(path), d, seed = 8, n_repeats = 3), a)
  explicit$design$terms <- NULL
  expect_setequal(survlearning:::.interpret_predictors(explicit), c("x", "z"))
  dot$design$terms <- NULL
  expect_error(survlearning:::.interpret_predictors(dot), "resolved training terms")
})

test_that("XGBoost Breslow survival is stable for extreme margins and zero-event times", {
  # Substitute only margin acquisition; exercise the installed baseline/prediction bodies.
  env <- new.env(parent = asNamespace("survlearning"))
  env$.xgboost_margin <- function(engine, x) engine
  baseline <- survlearning:::.xgboost_baseline
  predict <- survlearning:::.xgboost_survival
  environment(baseline) <- environment(predict) <- env
  y <- cbind(time = c(1, 2), event = c(1, 1))
  for (shift in c(-1000, 0, 1000)) {
    margin <- c(1000, -1000) + shift
    object <- list(method = "xgboost_cox", engine = margin,
      baseline = baseline(margin, NULL, y))
    p <- predict(object, NULL, c(2, 0, 1, 2))
    expect_equal(p[2, 1], exp(-1), tolerance = 1e-12)
    expect_equal(p[, 2], c(1, 1))
    expect_equal(p[, 1], p[, 4])
    expect_true(all(is.finite(p) & p >= 0 & p <= 1))
    expect_true(all(p[, 1] <= p[, 3]))
  }
  y[, 2] <- c(1, 0)
  object$baseline <- baseline(object$engine, NULL, y)
  expect_equal(predict(object, NULL, c(1, 2))[2, ], c(1, 1))
  y[, 2] <- 0
  object$baseline <- baseline(object$engine, NULL, y)
  expect_equal(predict(object, NULL, c(0, 1, 2)), matrix(1, 2, 3))
  expect_error(baseline(c(Inf, 1), NULL, y), "finite margin")
  object$engine <- c(NaN, 1)
  expect_error(predict(object, NULL, 1), "finite")

  # Independent explicit risk-set sum, with tied failures and censorings.
  lp <- c(-2, 0, 1, .5, -1, 2)
  y <- cbind(time = c(1, 1, 2, 3, 3, 4), event = c(1, 0, 1, 1, 0, 0))
  times <- sort(unique(y[, 1]))
  increments <- vapply(times, function(t) {
    sum(y[, 1] == t & y[, 2] == 1) / sum(exp(lp[y[, 1] >= t]))
  }, numeric(1))
  object <- list(method = "xgboost_cox", engine = lp, baseline = baseline(lp, NULL, y))
  expected <- exp(-outer(exp(lp), cumsum(increments)))
  expect_equal(predict(object, NULL, times), expected, tolerance = 1e-12)
  object$baseline <- list(time = times, cumhaz = cumsum(increments), center = 0)
  expect_equal(predict(object, NULL, times), expected, tolerance = 1e-12)
  object$baseline$cumhaz[4] <- Inf
  expect_error(predict(object, NULL, times), "rebuild it")
})

test_that("XGBoost log baselines preserve small risk-set counts under common offsets", {
  env <- new.env(parent = asNamespace("survlearning"))
  env$.xgboost_margin <- function(engine, x) engine
  baseline <- survlearning:::.xgboost_baseline
  predict <- survlearning:::.xgboost_survival
  environment(baseline) <- environment(predict) <- env
  y <- cbind(time = c(1, 2), event = c(1, 0))
  for (offset in c(0, -1000, 1000, -1e12, 1e12, -1e16, 1e16, -1e300, 1e300)) {
    lp <- rep(offset, 2)
    object <- list(method = "xgboost_cox", engine = lp,
                   baseline = baseline(lp, NULL, y))
    expect_true(is.finite(object$baseline$center))
    expect_equal(predict(object, NULL, c(2, 0, 1, 2)),
      matrix(rep(c(exp(-.5), 1, exp(-.5), exp(-.5)), each = 2), 2),
      tolerance = 1e-14)
  }
  # A tied event/censor set and a late censor-only set have a separate direct oracle.
  y <- cbind(time = c(1, 1, 2, 3), event = c(1, 1, 1, 0))
  lp <- c(-2, 0, 1, 2)
  increments <- c(2 / sum(exp(lp)), 1 / sum(exp(lp[3:4])), 0)
  expected <- exp(-outer(exp(lp), cumsum(increments)))
  for (offset in c(-1e12, 0, 1e12)) {
    object <- list(method = "xgboost_cox", engine = lp + offset,
                   baseline = baseline(lp + offset, NULL, y))
    expect_equal(predict(object, NULL, c(1, 2, 3)), expected, tolerance = 1e-12)
  }
})

test_that("non-default calibration confidence agrees with an independent KM oracle", {
  d <- data.frame(time = seq_len(30), event = rep(c(1L, 0L, 1L), 10))
  fit <- surv_fit(Surv(time, event) ~ 1, d, "km")
  oracle <- survival::survfit(survival::Surv(time, event) ~ 1, d,
    conf.int = .8, conf.type = "log-log", timefix = FALSE)
  at <- findInterval(15, oracle$time)
  result <- surv_calibrate(fit, d, times = 15, groups = 1, conf_level = .8)
  expect_equal(result$observed, oracle$surv[at])
  expect_equal(result$lower, oracle$lower[at])
  expect_equal(result$upper, oracle$upper[at])
  wider <- surv_calibrate(fit, d, times = 15, groups = 1, conf_level = .95)
  expect_true(result$lower > wider$lower && result$upper < wider$upper)
})

test_that("the real XGBoost Cox engine retains native probabilities and serialization", {
  skip_if_not_installed("xgboost", minimum_version = "3.0.0")
  d <- example_data(100)
  fit <- surv_fit(Surv(time, event) ~ x + z, d, "xgboost_cox",
    list(trees = 10L, depth = 1L), seed = 9)
  expect_true(!is.null(fit$baseline$logcumhaz))
  p <- surv_predict(fit, d, "survival", c(0, 1, 2))
  expect_true(all(is.finite(p) & p >= 0 & p <= 1))
  expect_true(all(p[, 3] <= p[, 2]))
  path <- tempfile(fileext = ".rds")
  on.exit(unlink(path))
  saveRDS(fit, path)
  expect_equal(surv_predict(readRDS(path), d, "survival", c(0, 1, 2)), p)
})
