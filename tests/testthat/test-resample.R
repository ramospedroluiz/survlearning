test_that("explicit stratified plans preserve allocation, indices, and RNG", {
  d <- example_data()
  f <- survival::Surv(time, event) ~ x + z
  set.seed(901)
  before <- .Random.seed
  p <- surv_resamples(f, d, folds = 3, repeats = 2, seed = 7)
  expect_identical(.Random.seed, before)
  expect_identical(p, surv_resamples(f, d, 3, 2, seed = 7))
  expect_length(p$splits, 6)
  expect_equal(p$data.n, nrow(d))
  expect_identical(p$formula, f)
  expect_null(p$fold_id)
  for (r in 1:2) {
    s <- p$splits[vapply(p$splits, function(x) x$repeat_id == r, logical(1))]
    expect_equal(sort(unlist(lapply(s, `[[`, "assessment"), use.names = FALSE)), seq_len(nrow(d)))
  }
  for (s in p$splits) {
    expect_length(intersect(s$analysis, s$assessment), 0)
    expect_equal(sort(c(s$analysis, s$assessment)), seq_len(nrow(d)))
    expect_gt(sum(d$event[s$assessment]), 0)
  }
  first <- surv_resamples(f, d, 3, seed = 7)
  legacy <- survlearning:::.with_seed(7, {
    ids <- integer(nrow(d))
    for (status in c(0, 1)) {
      index <- which(d$event == status)
      ids[index] <- rep(1:3, length.out = length(index))[sample.int(length(index))]
    }
    ids
  })
  expect_identical(first$fold_id, legacy)
  expect_equal(first$diagnostics$events_assessment,
    vapply(first$splits, function(s) sum(d$event[s$assessment]), numeric(1)), ignore_attr = TRUE)
})

test_that("grouped plans never divide groups and respect input names/vectors", {
  d <- example_data()
  d$subject <- rep(1:20, each = 4)
  f <- survival::Surv(time, event) ~ x + z
  p <- surv_resamples(f, d, 4, 2, "grouped", group = "subject", seed = 13)
  expect_identical(p$splits,
    surv_resamples(f, d, 4, 2, "grouped", group = d$subject, seed = 13)$splits)
  for (s in p$splits) {
    expect_length(intersect(d$subject[s$analysis], d$subject[s$assessment]), 0)
    expect_equal(sort(c(s$analysis, s$assessment)), seq_len(nrow(d)))
  }
  expect_error(surv_resamples(f, d, 21, type = "grouped", group = "subject"), "distinct groups")
  expect_error(surv_resamples(f, d, type = "grouped"), "group")
})

test_that("temporal plans are forward chaining and preserve complete time ties", {
  d <- example_data()
  d$entry <- rep(20:1, each = 4)
  f <- survival::Surv(time, event) ~ x + z
  p <- surv_resamples(f, d, 3, type = "temporal", time = "entry")
  expect_null(p$fold_id)
  for (s in p$splits) {
    expect_lt(max(d$entry[s$analysis]), min(d$entry[s$assessment]))
    expect_length(intersect(d$entry[s$analysis], d$entry[s$assessment]), 0)
    for (t in unique(d$entry[s$assessment])) {
      expect_true(all(which(d$entry == t) %in% s$assessment))
    }
  }
  expect_true(all(p$splits[[1]]$analysis %in% p$splits[[2]]$analysis))
  expect_true(all(p$splits[[1]]$assessment %in% p$splits[[2]]$analysis))
  dates <- as.Date("2020-01-01") + d$entry
  expect_identical(p$splits, surv_resamples(f, d, 3, type = "temporal", time = dates)$splits)
  expect_error(surv_resamples(f, d, 3, 2, "temporal", time = "entry"), "duplicate deterministic")
  expect_error(surv_resamples(f, d, 20, type = "temporal", time = "entry"), "distinct times")
  expect_error(surv_resamples(f, d, type = "temporal", time = letters[rep(1:20, each = 4)]), "finite numeric")
})

test_that("plan consumers reject mismatched signatures and corrupt indices", {
  d <- example_data()
  f <- survival::Surv(time, event) ~ x + z
  p <- surv_resamples(f, d, 3)
  use <- function(q, data = d) surv_tune(f, data, resamples = q)
  bad <- p
  bad$data.n <- nrow(d) - 1L
  expect_error(use(bad), "data.n")
  expect_error(use(p, d[nrow(d):1, ]), "signature|row order")
  changed <- d
  changed$time[1] <- changed$time[1] + 1
  expect_error(use(p, changed), "signature")
  expect_error(surv_tune(survival::Surv(time, event) ~ x, d, resamples = p), "signature")
  bad <- p
  bad$splits[[1]]$analysis[1] <- bad$splits[[1]]$assessment[1]
  expect_error(use(bad), "disjoint")
  for (indices in list(c(1L, 1L), NA_integer_, 0L, 81L, 1.5, integer())) {
    bad <- p
    bad$splits[[1]]$analysis <- indices
    expect_error(use(bad), "invalid analysis")
  }
  bad <- p
  bad$splits[[2]]$id <- bad$splits[[1]]$id
  expect_error(use(bad), "unique")
  d$g <- rep(1:20, each = 4)
  grouped <- surv_resamples(f, d, 3, type = "grouped", group = "g")
  changed <- d
  changed$g[1] <- 100
  expect_error(use(grouped, changed), "group values")
  s <- grouped$splits[[1]]
  donor <- s$analysis[1]
  grouped$splits[[1]]$analysis <- setdiff(s$analysis, which(d$g == d$g[donor]))
  grouped$splits[[1]]$assessment <- c(s$assessment, donor)
  grouped$splits[[1]]$analysis <- c(grouped$splits[[1]]$analysis,
    setdiff(which(d$g == d$g[donor]), donor))
  expect_error(use(grouped), "groups must be disjoint")
  d$entry <- seq_len(nrow(d))
  temporal <- surv_resamples(f, d, 3, type = "temporal", time = "entry")
  temporal$splits[[1]]$analysis <- 80L
  expect_error(use(temporal, d), "strictly precede")
})

test_that("sparse and no-event plans stay explicit and outcomes never impute", {
  d <- example_data(12)
  f <- survival::Surv(time, event) ~ x + z
  d$x[1:2] <- NA_real_
  d$event <- 0L
  p <- surv_resamples(f, d, folds = 5)
  expect_true(all(p$diagnostics$events_assessment == 0))
  expect_true(all(nzchar(p$diagnostics$diagnostic)))
  v <- surv_validate(f, d, methods = "cox", resamples = p, tune = FALSE)
  expect_true(all(is.na(v$results$estimate)))
  expect_true(all(grepl("No observed events in analysis", v$results$error)))
  expect_true(is.na(v$summary$mean))
  expect_equal(v$summary$n_valid, 0)
  d$time[1] <- NA_real_
  expect_error(surv_resamples(f, d), "missing|NA")
  d <- example_data(4)
  d$event <- c(0L, 0L, 1L, 1L)
  p <- surv_resamples(f, d, folds = 4)
  expect_equal(p$diagnostics$n_assessment, rep(1L, 4))
})

test_that("repeated explicit tuning retains historical Harrell fields", {
  d <- example_data()
  f <- survival::Surv(time, event) ~ x + z
  p <- surv_resamples(f, d, 3, 2)
  a <- surv_tune(f, d, "aft", list(distribution = c("invalid", "weibull")), resamples = p)
  expect_null(a$fold_id)
  expect_equal(a$results$failed_folds, c(6L, 0L))
  expect_equal(a$fold_results$cindex, a$fold_results$estimate)
  expect_equal(a$results$mean_cindex, a$results$mean_estimate)
  expect_equal(a$best_candidate, 2)
  expect_equal(a$best_model$n, nrow(d))
  expect_equal(nrow(a$failures), 6)
  expect_output(print(a), "Folds: 6")
  failed <- tryCatch(surv_tune(f, d, "aft", list(distribution = "invalid"), resamples = p), error = identity)
  expect_s3_class(failed, "survlearning_tune_error")
  expect_equal(nrow(failed$fold_results), 6)
})

test_that("nested validation supports the documented KM/Cox/AFT shared plan", {
  d <- example_data()
  f <- survival::Surv(time, event) ~ x + z
  p <- surv_resamples(f, d, 3, 2, seed = 11)
  v <- surv_validate(f, d, methods = c("km", "cox", "aft"),
    grids = list(aft = list(distribution = c("weibull", "lognormal"))),
    resamples = p, inner_folds = 3)
  expect_s3_class(v, "survlearning_validate")
  expect_equal(nrow(v$results), 18)
  expect_true(all(is.finite(v$results$estimate)))
  expect_equal(v$summary$n_valid, rep(6L, 3))
  expect_equal(v$summary$n_paired, rep(6L, 3))
  expect_equal(v$summary$mean, v$summary$mean_common)
  expect_equal(nrow(v$failures), 0)
  expect_false(any(c("se", "stderr", "lower", "upper") %in% names(v$summary)))
  for (i in seq_along(p$splits)) {
    s <- p$splits[[i]]
    inside <- v$inner_resamples[[i]]
    expect_equal(inside$data.n, length(s$analysis))
    for (inner in inside$splits) {
      expect_length(intersect(s$assessment, s$analysis[inner$analysis]), 0)
      expect_length(intersect(s$assessment, s$analysis[inner$assessment]), 0)
    }
    for (method in c("km", "cox", "aft")) {
      expect_equal(unique(v$tuning[[i]][[method]]$fold_results$resample), names(inside$splits))
    }
    manual <- surv_fit(f, d[s$analysis, ], "cox")
    expect_equal(v$results$estimate[v$results$method == "cox" & v$results$resample == s$id],
      surv_score(manual, d[s$assessment, ]))
  }
})

test_that("structured outer plans never silently fall back to ordinary inner CV", {
  d <- example_data()
  d$g <- rep(1:20, each = 4)
  d$entry <- seq_len(nrow(d))
  f <- survival::Surv(time, event) ~ x + z
  for (type in c("grouped", "temporal")) {
    p <- surv_resamples(f, d, 3, type = type, group = "g", time = "entry")
    expect_error(surv_validate(f, d, resamples = p), "unsupported.*tune = FALSE")
    v <- surv_validate(f, d, methods = c("km", "cox"), resamples = p, tune = FALSE)
    expect_equal(nrow(v$results), 6)
    expect_equal(v$resamples$splits, p$splits)
    expect_true(all(is.finite(v$results$estimate)))
    expect_null(v$inner_resamples[[1]])
  }
})

test_that("metric dispatch sends only relevant horizons and preserves diagnostics", {
  calls <- list()
  testthat::local_mocked_bindings(surv_evaluate = function(object, newdata, metrics,
      times = NULL, tau = NULL) {
    calls[[metrics]] <<- list(times = times, tau = tau)
    if (metrics == "uno") expect_null(times) else expect_null(tau)
    data.frame(metric = metrics, time = if (metrics == "uno") tau else NA_real_,
      estimate = if (metrics == "auc") NA_real_ else 0.2,
      status = if (metrics == "auc") "undefined" else "ok",
      message = if (metrics == "auc") "no cases at requested horizon" else "")
  }, .package = "survlearning")
  d <- example_data()
  f <- survival::Surv(time, event) ~ x + z
  v <- surv_validate(f, d, methods = "cox", folds = 2, tune = FALSE,
    metrics = c("harrell", "uno", "auc", "brier", "ibs"), times = c(1, 2), tau = 2)
  expect_equal(calls$uno$tau, 2)
  expect_equal(calls$ibs$times, c(1, 2))
  auc <- v$results[v$results$metric == "auc", ]
  expect_true(all(auc$status == "undefined"))
  expect_true(all(grepl("no cases", auc$diagnostic)))
  expect_true(all(is.finite(v$results$estimate[v$results$metric == "harrell"])))
})

test_that("Brier and IBS minimize and multi-horizon tuning rejects ambiguity", {
  testthat::local_mocked_bindings(surv_evaluate = function(object, newdata, metrics,
      times = NULL, tau = NULL) {
    expect_null(tau)
    horizon <- if (metrics == "ibs") NA_real_ else times
    data.frame(metric = metrics, time = horizon,
      estimate = if (object$params$distribution == "weibull") 0.3 else 0.1)
  }, .package = "survlearning")
  d <- example_data()
  f <- survival::Surv(time, event) ~ x + z
  for (metric in c("brier", "ibs")) {
    a <- surv_tune(f, d, "aft", list(distribution = c("weibull", "lognormal")),
      folds = 2, metric = metric, times = if (metric == "ibs") c(1, 2) else 1, tau = 3)
    expect_equal(a$best_candidate, 2)
    expect_true(all(is.na(a$results$mean_cindex)))
    expect_true(all(is.na(a$fold_results$cindex)))
  }
  expect_error(surv_tune(f, d, metric = "brier", times = c(1, 2), folds = 2), "scalar metric")
})

test_that("available summaries and pair-specific intersections survive other method failures", {
  rows <- data.frame(method = rep(c("cox", "aft", "km"), each = 3),
    resample = rep(c("a", "b", "c"), 3), metric = "harrell", time = NA_real_,
    estimate = c(0.7, 0.8, 0.9, 0.6, 0.7, NA, NA, NA, NA))
  out <- survlearning:::.resample_summaries(rows, c("cox", "aft", "km"))
  expect_equal(out$summary$n_valid, c(3L, 2L, 0L))
  expect_equal(out$summary$mean[1:2], c(0.8, 0.65))
  expect_true(all(is.na(out$summary$mean_common)))
  pair <- out$paired[out$paired$method1 == "cox" & out$paired$method2 == "aft", ]
  expect_equal(pair$n_paired, 2)
  expect_equal(pair$mean_difference, 0.1)
})

test_that("real all-metric validation isolates risk-only probability failures", {
  skip_if_not_installed("rpart")
  d <- example_data()
  f <- survival::Surv(time, event) ~ x + z
  p <- surv_resamples(f, d, folds = 2)
  v <- surv_validate(f, d, methods = c("km", "cox", "tree"), resamples = p,
    tune = FALSE, metrics = c("harrell", "uno", "auc", "brier", "ibs"),
    times = c(1, 2), tau = 2)
  expect_equal(nrow(v$results), 42)
  brier <- v$results[v$results$metric == "brier", ]
  expect_true(all(is.finite(brier$estimate[brier$method != "tree"])))
  expect_true(all(is.na(brier$estimate[brier$method == "tree"])))
  expect_true(all(nzchar(brier$diagnostic[brier$method == "tree"])))
  pairs <- v$paired_summary[v$paired_summary$metric == "brier" &
    v$paired_summary$method1 == "km" & v$paired_summary$method2 == "cox", ]
  expect_equal(pairs$n_paired, c(2L, 2L))
  expect_true(all(is.na(v$summary$mean_common[v$summary$metric == "brier"])))
  s <- p$splits[[1]]
  fit <- surv_fit(f, d[s$analysis, ], "cox")
  expected <- surv_evaluate(fit, d[s$assessment, ], metrics = "brier", times = c(1, 2))
  expect_equal(brier$estimate[brier$method == "cox" & brier$resample == s$id], expected$estimate)
})

test_that("fixed configurations and invalid validation controls are explicit", {
  d <- example_data()
  f <- survival::Surv(time, event) ~ x + z
  v <- surv_validate(f, d, methods = "aft", folds = 2, tune = FALSE,
    params = list(aft = list(distribution = "lognormal")))
  expect_true(all(vapply(v$results$params, function(p) p$distribution == "lognormal", logical(1))))
  expect_error(surv_validate(f, d, methods = c("cox", "cox")), "distinct")
  expect_error(surv_validate(f, d, tune = NA), "TRUE or FALSE")
  expect_error(surv_validate(f, d, grids = list(list(distribution = "weibull"))), "named list")
  expect_error(surv_validate(f, d, params = list(unknown = list())), "named list")
  expect_error(surv_validate(f, d, tune = FALSE, grids = list(aft = list())), "requires tune")
  expect_error(surv_resamples(f, d, folds = 1), "integer")
  expect_error(surv_resamples(f, d, repeats = 0), "integer")
  expect_error(surv_resamples(f, d, type = "random"), "arg")
  expect_error(surv_tune(f, d, metric = c("harrell", "uno")), "one scalar")
})

test_that("no-event assessment permits Brier while discrimination remains diagnosed", {
  d <- example_data(40)
  d$time <- seq_len(nrow(d))
  d$event <- c(rep(1L, 20), rep(0L, 20))
  d$entry <- seq_len(nrow(d))
  f <- survival::Surv(time, event) ~ x + z
  p <- surv_resamples(f, d, 3, type = "temporal", time = "entry")
  v <- surv_validate(f, d, methods = "km", resamples = p, tune = FALSE,
    metrics = c("harrell", "brier"), times = 5)
  no_events <- v$results$events_assessment == 0L
  expect_true(any(no_events))
  expect_true(all(is.finite(v$results$estimate[no_events & v$results$metric == "brier"])))
  expect_true(all(is.na(v$results$estimate[no_events & v$results$metric == "harrell"])))
  expect_true(all(nzchar(v$results$diagnostic[no_events & v$results$metric == "harrell"])))
})

test_that("imputation is learned inside every inner/outer training fit", {
  d <- example_data()
  d$x[seq(1, 80, by = 5)] <- NA_real_
  f <- survival::Surv(time, event) ~ x + z
  calls <- list()
  original <- surv_fit
  testthat::local_mocked_bindings(surv_fit = function(formula, data, method, params, seed, ...) {
    fitted <- original(formula, data, method, params, seed, ...)
    calls[[length(calls) + 1L]] <<- list(rows = row.names(data), imputed = fitted$design$imputation$x,
      expected = stats::median(data$x, na.rm = TRUE))
    fitted
  }, .package = "survlearning")
  p <- surv_resamples(f, d, 2)
  v <- surv_validate(f, d, methods = "cox", resamples = p, inner_folds = 2, impute = "median")
  expect_true(all(is.finite(v$results$estimate)))
  expect_length(calls, 6)
  for (call in calls) {
    expect_equal(call$imputed, call$expected)
    expect_lt(length(call$rows), nrow(d))
    expect_true(any(vapply(p$splits,
      function(s) all(call$rows %in% row.names(d)[s$analysis]), logical(1))))
  }
})

test_that("warnings stay visible and explicit optimizer failures are ineligible", {
  original <- surv_fit
  testthat::local_mocked_bindings(surv_fit = function(formula, data, method, params, seed, ...) {
    fitted <- original(formula, data, method, params, seed, ...)
    warning("engine convergence warning", call. = FALSE)
    fitted$diagnostics$convergence <- list(status = if (
      identical(params$distribution, "weibull")) "reported_failure" else "not_reported", code = 1L)
    fitted
  }, .package = "survlearning")
  d <- example_data()
  f <- survival::Surv(time, event) ~ x + z
  seen <- character()
  a <- withCallingHandlers(surv_tune(f, d, "aft",
    list(distribution = c("weibull", "lognormal")), folds = 2), warning = function(w) {
      seen <<- c(seen, conditionMessage(w))
      invokeRestart("muffleWarning")
    })
  expect_length(seen, 5)
  expect_equal(a$best_candidate, 2)
  expect_true(all(grepl("optimizer failure", a$failures$error)))
  expect_equal(nrow(a$warnings), 4)
  expect_equal(a$refit_warnings, "engine convergence warning")
  v <- suppressWarnings(surv_validate(f, d, methods = "aft", folds = 2, tune = FALSE,
    params = list(aft = list(distribution = "weibull"))))
  expect_equal(nrow(v$failures), 2)
  expect_equal(nrow(v$warnings), 2)
})
