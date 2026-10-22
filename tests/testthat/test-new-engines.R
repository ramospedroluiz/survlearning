.new_engine_skip <- function(method) {
  package <- survlearning:::.reference_specs()[[method]]$engine
  expect_true(requireNamespace(package, quietly = TRUE), info = package)
}

.new_engine_params <- function(method) {
  switch(method,
    ranger = list(trees = 20L, min_node = 5L),
    aorsf = list(trees = 20L, min_node = 5L),
    coxboost = list(steps = 20L),
    neural_cox = list(hidden = 6L, epochs = 40L, learning_rate = 0.01),
    neural_discrete = list(hidden = 4L, intervals = 5L, max_iter = 1000L),
    list())
}

.new_engine_fit <- function(method, data = example_data(100L), seed = 8L) {
  surv_fit(survival::Surv(time, event) ~ x + z + batch, data,
    method, .new_engine_params(method), seed = seed)
}

test_that("reference specs expose exactly the modular engine contract", {
  specs <- survlearning:::.reference_specs()
  expect_setequal(names(specs), c("km", "cox_spline", "flexsurv", "coxboost",
    "ranger", "aorsf", "neural_cox", "neural_discrete"))
  expect_identical(specs$neural_discrete$engine, "nnet")
  expect_identical(specs$neural_discrete$defaults$max_iter, 1000L)
  for (spec in specs) {
    expect_named(spec, c("description", "engine", "predictions", "defaults"))
    expect_true(is.character(spec$description) && nzchar(spec$description))
    expect_identical(spec$predictions, c("risk", "survival"))
    expect_type(spec$defaults, "list")
  }
})

for (id in c("km", "cox_spline", "flexsurv", "coxboost", "ranger", "aorsf",
    "neural_cox", "neural_discrete")) {
  local({
    method <- id
    test_that(paste("new engine has stable risk, curves, batches and RDS:", method), {
      .new_engine_skip(method)
      data <- example_data(100L)
      set.seed(710L)
      rng <- .Random.seed
      fit <- .new_engine_fit(method, data)
      expect_identical(.Random.seed, rng)
      new <- data[1:3, c("x", "z", "batch")]
      risk <- surv_predict(fit, new)
      expect_length(risk, 3L)
      expect_true(all(is.finite(risk)))
      times <- c(5, 0, 2, 5, fit$max_time)
      curves <- surv_predict(fit, new, "survival", times)
      expect_equal(dim(curves), c(3L, 5L))
      expect_equal(unname(curves[, 2L]), rep(1, 3))
      expect_equal(curves[, 1L], curves[, 4L])
      expect_true(all(is.finite(curves) & curves >= 0 & curves <= 1))
      expect_true(all(apply(curves[, c(2L, 3L, 1L, 5L)], 1L, diff) <= 1e-12))
      expect_equal(unname(surv_predict(fit, new[1, , drop = FALSE])), unname(risk[1]))
      expect_equal(unname(surv_predict(fit, new[1, , drop = FALSE], "survival", 5)),
        matrix(unname(curves[1, 1]), 1L, 1L))
      extreme <- new
      extreme$x[2L] <- 1e4
      extreme$z[3L] <- -1e4
      expect_equal(surv_predict(fit, extreme)[1L], risk[1L])
      expect_identical(.Random.seed, rng)
      expect_error(surv_predict(fit, new, "time"), "native")
      if (method != "flexsurv") {
        expect_warning(tail <- surv_predict(fit, new, "survival", c(0, fit$max_time + 1)),
          "beyond training")
        expect_true(all(is.na(tail[, 2L])))
      } else {
        expect_true(all(is.finite(surv_predict(fit, new, "survival", fit$max_time + 1))))
      }
      path <- tempfile(fileext = ".rds")
      on.exit(unlink(path), add = TRUE)
      saveRDS(fit, path)
      restored <- readRDS(path)
      expect_equal(surv_predict(restored, new), risk)
      expect_equal(surv_predict(restored, new, "survival", times), curves)
      again <- .new_engine_fit(method, data)
      expect_equal(surv_predict(again, new), risk)
    })
  })
}

test_that("KM ignores all predictors and agrees with native Kaplan-Meier", {
  data <- example_data()
  fit <- .new_engine_fit("km", data)
  times <- c(0, sort(unique(data$time))[c(1, 15, 30)])
  native <- survival::survfit(survival::Surv(time, event) ~ 1, data)
  expected <- c(1, native$surv)[findInterval(times, native$time) + 1L]
  new <- data.frame(ignored = c(NA, Inf, -Inf))
  expect_equal(unname(surv_predict(fit, new, "survival", times)),
    matrix(rep(expected, each = 3L), nrow = 3L))
  expect_identical(surv_predict(fit, new), rep(0, 3L))
  expect_equal(length(fit$design$columns), 0L)
})

test_that("Cox splines preserve training knots and native predictions", {
  data <- example_data(150L)
  data$batch <- ordered(rep(c("A", "B", "C", "D"), length.out = nrow(data)))
  fit <- .new_engine_fit("cox_spline", data)
  calls <- attr(fit$engine$terms, "predvars")
  expect_match(paste(deparse(calls), collapse = " "), "Boundary.knots")
  labels <- attr(fit$engine$terms, "term.labels")
  expect_equal(sum(grepl("splines::ns", labels, fixed = TRUE)), 2L)
  new <- data[1:5, c("x", "z", "batch")]
  x <- survlearning:::.new_design(fit$design, new)
  expected <- predict(fit$engine, newdata = as.data.frame(x), type = "lp", reference = "zero")
  expect_equal(surv_predict(fit, new), as.numeric(expected))
  native <- survival::survfit(fit$engine, newdata = as.data.frame(x))
  times <- c(0, 2, 5)
  expect_equal(unname(surv_predict(fit, new, "survival", times)),
    unname(t(summary(native, times = times, extend = TRUE)$surv)), tolerance = 1e-8)
  new$x <- c(-100, -50, 0, 50, 100)
  risk <- surv_predict(fit, new)
  expect_equal(risk[1L], surv_predict(fit, new[1L, , drop = FALSE]))
  expect_identical(attr(fit$engine$terms, "predvars"), calls)
})

for (id in c("coxboost", "ranger", "aorsf")) {
  local({
    method <- id
    test_that(paste("new engine agrees with its native API:", method), {
      .new_engine_skip(method)
      data <- example_data(100L)
      fit <- .new_engine_fit(method, data)
      new <- data[1:4, c("x", "z", "batch")]
      x <- survlearning:::.new_design(fit$design, new)
      times <- c(2, 5)
      expected <- switch(method,
        coxboost = predict(fit$engine, newdata = x, type = "risk", times = times,
          at.step = fit$params$steps),
        ranger = {
          native <- predict(fit$engine, data = as.data.frame(x), num.threads = 1L)
          cbind(1, native$survival)[, findInterval(times, native$unique.death.times) + 1L,
            drop = FALSE]
        },
        aorsf = predict(fit$engine, new_data = as.data.frame(x), pred_type = "surv",
          pred_horizon = times, n_thread = 1L)
      )
      expect_equal(unname(surv_predict(fit, new, "survival", times)), unname(expected))
      risk <- surv_predict(fit, new)
      expect_true(all(diff(surv_predict(fit, new[order(risk), ], "survival", 5)[, 1L]) <= 0) ||
        method %in% c("ranger", "aorsf"))
    })
  })
}

test_that("neural discrete uses native nnet entropy loss and training-only scaling", {
  .new_engine_skip("neural_discrete")
  data <- example_data(100L)
  rownames(data) <- paste0("subject-", seq_len(nrow(data)))
  fit <- .new_engine_fit("neural_discrete", data)
  engine <- fit$engine
  expect_s3_class(engine$fit, "nnet")
  expect_identical(engine$convergence, engine$fit$convergence)
  expect_true(isTRUE(engine$fit$entropy))
  expect_true(any(engine$expansion$response == 0))
  expect_equal(sum(engine$expansion$response), sum(data$event))
  expect_identical(engine$time, seq(0, max(data$time), length.out = fit$params$intervals + 1L))
  expect_equal(sum(engine$time == max(data$time)), 1L)
  expect_match(engine$coarsening, "discretization bias", fixed = TRUE)
  new <- data[1:3, c("x", "z", "batch")]
  x <- survlearning:::.new_design(fit$design, new)
  train <- survlearning:::.prepare_design(fit$formula, data)$x
  expect_equal(engine$center, colMeans(train))
  expect_equal(engine$scale, apply(train, 2L, stats::sd))
  scaled <- sweep(sweep(x, 2L, engine$center, "-"), 2L, engine$scale, "/")
  k <- fit$params$intervals
  expanded_new <- cbind(scaled[rep(seq_len(nrow(x)), each = k), , drop = FALSE],
    .sl_interval = rep(seq_len(k) / k, times = nrow(x)))
  hazard <- matrix(predict(engine$fit, newdata = expanded_new, type = "raw"),
    nrow = nrow(x), byrow = TRUE)
  native_curves <- cbind(1, t(apply(1 - hazard, 1L, cumprod)))
  expect_equal(unname(surv_predict(fit, new, "survival", engine$time)), unname(native_curves))
  expected_risk <- -drop(native_curves[, -ncol(native_curves), drop = FALSE] %*% diff(engine$time))
  expect_equal(surv_predict(fit, new), expected_risk)
  scaled_training <- sweep(sweep(train, 2L, engine$center, "-"), 2L, engine$scale, "/")
  expanded_training <- cbind(scaled_training[engine$expansion$observation, , drop = FALSE],
    .sl_interval = engine$expansion$interval / k)
  native_fit <- survlearning:::.with_seed(fit$seed, nnet::nnet(x = expanded_training,
    y = matrix(engine$expansion$response, ncol = 1L), size = fit$params$hidden,
    entropy = TRUE, linout = FALSE, decay = fit$params$decay,
    maxit = fit$params$max_iter, MaxNWts = fit$params$max_weights, trace = FALSE))
  expect_equal(engine$fit$wts, native_fit$wts)
  expect_equal(engine$fit$value, native_fit$value)
})

test_that("PLANN expansion never labels incomplete censoring intervals as survived", {
  grid <- seq(0, 4, length.out = 5L)
  y <- survival::Surv(c(0.1, 2.1, 2, 2.1, 2, 4), c(0, 0, 0, 1, 1, 1))
  periods <- survlearning:::.neural_discrete_periods(y, grid)
  expected <- data.frame(observation = c(2L, 2L, 3L, 3L, 4L, 4L, 4L, 5L, 5L,
    6L, 6L, 6L, 6L), interval = c(1L, 2L, 1L, 2L, 1L, 2L, 3L, 1L, 2L, 1L, 2L, 3L, 4L),
    response = c(0L, 0L, 0L, 0L, 0L, 0L, 1L, 0L, 1L, 0L, 0L, 0L, 1L))
  expect_identical(periods$expansion, expected)
  expect_equal(periods$exposure$likelihood_rows, c(0, 2, 2, 3, 2, 4))
  expect_equal(periods$exposure$completed_intervals, c(0, 2, 2, 2, 1, 3))
  expect_equal(periods$exposure$coarsened_time, c(0, 2, 2, 3, 2, 4))
  expect_identical(periods$exposure$partial_censor_interval_dropped,
    c(TRUE, TRUE, FALSE, FALSE, FALSE, FALSE))
  expect_identical(periods$diagnostics$zero_exposure_censored, 1L)
  expect_equal(periods$diagnostics$n_zero_exposure_censored, 1L)
  expect_equal(periods$diagnostics$n_partial_censor_intervals_dropped, 2L)
  expect_false(any(periods$expansion$observation == 1L))
  for (j in which(y[, 2L] == 0)) {
    rows <- periods$expansion[periods$expansion$observation == j, , drop = FALSE]
    expect_true(all(grid[rows$interval + 1L] <= y[j, 1L]))
  }
})

test_that("PLANN default iteration budget converges on the longer optimization fixture", {
  .new_engine_skip("neural_discrete")
  fit <- surv_fit(survival::Surv(time, event) ~ x + z + batch,
    example_data(150L), "neural_discrete", seed = 8L)
  expect_identical(fit$params$max_iter, 1000L)
  expect_equal(fit$engine$convergence, 0L)
})

test_that("PLANN hand likelihood equals nnet's unpenalized Bernoulli criterion", {
  .new_engine_skip("neural_discrete")
  grid <- seq(0, 4, length.out = 5L)
  y <- survival::Surv(c(0.1, 2.1, 2, 2.1, 2, 4), c(0, 0, 0, 1, 1, 1))
  periods <- survlearning:::.neural_discrete_periods(y, grid)
  x <- seq(-1, 1, length.out = nrow(y))
  expanded <- cbind(x = x[periods$expansion$observation],
    .sl_interval = periods$expansion$interval / 4)
  weights <- c(0.2, 0.4, -0.6, 0.1, 0.3)
  native <- nnet::nnet(expanded, matrix(periods$expansion$response, ncol = 1L),
    size = 1L, Wts = weights, entropy = TRUE, linout = FALSE,
    decay = 0, maxit = 0L, trace = FALSE)
  hidden <- plogis(0.2 + 0.4 * expanded[, 1L] - 0.6 * expanded[, 2L])
  hazard <- plogis(0.1 + 0.3 * hidden)
  target <- periods$expansion$response
  likelihood <- -sum(target * log(hazard) + (1 - target) * log1p(-hazard))
  expect_equal(as.numeric(predict(native, expanded, type = "raw")), as.numeric(hazard))
  expect_equal(native$value, likelihood)
})

test_that("PLANN grid, tolerant boundaries and step RMST have exact interval counts", {
  .new_engine_skip("neural_discrete")
  horizon <- 0.7
  k <- 7L
  grid <- seq(0, horizon, length.out = k + 1L)
  y <- survival::Surv(c(grid[-1L], grid[-1L]), rep(c(0L, 1L), each = k))
  periods <- survlearning:::.neural_discrete_periods(y, grid)
  expect_identical(periods$exposure$likelihood_rows, rep(seq_len(k), 2L))
  expect_equal(sum(grid == horizon), 1L)
  expect_equal(length(grid), k + 1L)
  expect_identical(as.integer(survlearning:::.neural_discrete_position(grid, grid)), 0:k)
  near <- c(0.3 - 1e-8, 0.3, 0.3 + 1e-8)
  expect_equal(floor(survlearning:::.neural_discrete_position(near, grid)), c(2, 3, 3))
  small <- grid * 1e-200
  expect_equal(survlearning:::.neural_discrete_position(small, small), 0:k)
  data <- data.frame(time = c(0.1, 2.1, 2, 2.1, 2, 4), event = c(0, 0, 0, 1, 1, 1),
    x = seq(-1, 1, length.out = 6L))
  fit <- surv_fit(survival::Surv(time, event) ~ x, data, "neural_discrete",
    list(intervals = 4L, hidden = 2L, max_iter = 1000L), seed = 8L)
  expect_equal(fit$engine$diagnostics$n_zero_exposure_censored, 1L)
  expect_equal(fit$engine$diagnostics$n_partial_censor_intervals_dropped, 2L)
  times <- c(2.1, 0, 2, 0.1, 4, 2.1)
  curves <- surv_predict(fit, data[1L, "x", drop = FALSE], "survival", times)
  endpoint <- surv_predict(fit, data[1L, "x", drop = FALSE], "survival", 0:4)
  expect_equal(unname(curves), unname(endpoint[, c(3L, 1L, 3L, 1L, 5L, 3L), drop = FALSE]))
  expect_equal(surv_predict(fit, data[1L, "x", drop = FALSE]), -sum(endpoint[1L, 1:4]))
})

test_that("new parameter constraints reject malformed or unsupported settings", {
  specifications <- survlearning:::.reference_specs()
  invalid <- list(cox_spline = list(df = 0L), flexsurv = list(knots = -1L),
    coxboost = list(steps = 0L), ranger = list(mtry = 10L),
    aorsf = list(threads = 0L), neural_cox = list(hidden = c(4L, 2L)),
    neural_discrete = list(intervals = 1L))
  for (method in names(invalid)) {
    params <- utils::modifyList(specifications[[method]]$defaults, invalid[[method]])
    expect_error(survlearning:::.reference_parameters(method, params, 2L))
  }
  params <- specifications$flexsurv$defaults
  params$scale <- "invalid"
  expect_error(survlearning:::.reference_parameters("flexsurv", params, 2L), "scale")
  expect_equal(survlearning:::.reference_parameters("ranger",
    specifications$ranger$defaults, 9L)$mtry, 3L)
  .new_engine_skip("neural_discrete")
  expect_error(surv_fit(survival::Surv(time, event) ~ x + z, example_data(),
    "neural_discrete", list(hidden = 1000L, max_weights = 10L)), "max_weights")
})

for (id in c("km", "cox_spline", "flexsurv", "coxboost", "ranger", "aorsf",
    "neural_cox", "neural_discrete")) {
  local({
    method <- id
    test_that(paste("new engine survives a genuinely fresh R session:", method), {
      .new_engine_skip(method)
      skip_if_not(requireNamespace("callr", quietly = TRUE), "callr unavailable")
      root <- normalizePath(testthat::test_path("..", ".."), winslash = "/", mustWork = FALSE)
      if (!file.exists(file.path(root, "DESCRIPTION")) ||
          !file.exists(file.path(root, "R", "reference-engines.R"))) root <- NULL
      if (!is.null(root)) {
        skip_if_not(requireNamespace("pkgload", quietly = TRUE), "pkgload unavailable")
      }
    data <- example_data(100L)
    fit <- .new_engine_fit(method, data)
    new <- data[1:2, c("x", "z", "batch")]
    times <- c(5, 0, 2, 5)
    expected <- list(risk = surv_predict(fit, new),
      survival = surv_predict(fit, new, "survival", times))
    path <- tempfile(fileext = ".rds")
    saveRDS(list(fit = fit, new = new, times = times), path)
    restored <- tryCatch(callr::r(function(root, path) {
      if (is.null(root)) library(survlearning) else pkgload::load_all(root, quiet = TRUE)
      bundle <- readRDS(path)
      list(risk = survlearning::surv_predict(bundle$fit, bundle$new),
        survival = survlearning::surv_predict(bundle$fit, bundle$new,
          "survival", bundle$times))
    }, args = list(root = root, path = path), libpath = .libPaths(),
      user_profile = FALSE, system_profile = FALSE), finally = unlink(path))
      expect_equal(restored, expected, info = method)
    })
  })
}

test_that("new covariate engines retain single-predictor and single-row dimensions", {
  data <- example_data(100L)
  for (method in setdiff(names(survlearning:::.reference_specs()), "km")) {
    package <- survlearning:::.reference_specs()[[method]]$engine
    expect_true(requireNamespace(package, quietly = TRUE), info = package)
    fit <- surv_fit(survival::Surv(time, event) ~ x, data, method,
      .new_engine_params(method), seed = 8L)
    new <- data[1L, "x", drop = FALSE]
    expect_length(surv_predict(fit, new), 1L)
    expect_equal(dim(surv_predict(fit, new, "survival", c(0, 2, 5))), c(1L, 3L))
  }
})

test_that("Royston-Parmar accepts each supported transformation scale", {
  .new_engine_skip("flexsurv")
  data <- example_data(100L)
  for (scale in c("hazard", "odds", "normal")) {
    fit <- surv_fit(survival::Surv(time, event) ~ x + z, data, "flexsurv",
      list(scale = scale), seed = 8L)
    expect_gt(surv_score(fit, data), 0.7)
    curves <- surv_predict(fit, data[1:2, ], "survival", c(0, 2, 5, 50))
    expect_equal(unname(curves[, 1L]), rep(1, 2L))
    expect_true(all(is.finite(curves) & curves >= 0 & curves <= 1))
  }
})
