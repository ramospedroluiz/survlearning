test_that("PSOCK and serial tuning preserve scores, winner, ordering and RNG", {
  d <- example_data(90)
  f <- Surv(time, event) ~ x + z
  set.seed(19)
  previous <- .Random.seed
  serial <- surv_tune(f, d, "aft", list(distribution = c("weibull", "lognormal")), folds = 3, seed = 8)
  parallel <- surv_tune(f, d, "aft", list(distribution = c("weibull", "lognormal")), folds = 3, seed = 8, workers = 2)
  expect_identical(.Random.seed, previous)
  expect_equal(serial$fold_results$cindex, parallel$fold_results$cindex, tolerance = 1e-12)
  expect_identical(serial$fold_results$resample, parallel$fold_results$resample)
  expect_identical(serial$best_params, parallel$best_params)
  expect_equal(surv_predict(serial, d), surv_predict(parallel, d), tolerance = 1e-12)
  expect_equal(parallel$execution$workers_used, 2)
  expect_error(surv_tune(f, d, "rsf", params = list(threads = 2), workers = 2), "nested")
})

test_that("completed, partial and failed searches resume without mixing inputs", {
  d <- example_data(90)
  f <- Surv(time, event) ~ x + z
  path <- tempfile("checkpoint-")
  first <- surv_tune(f, d, "aft", list(distribution = c("weibull", "lognormal")),
    folds = 3, seed = 8, checkpoint = path, workers = 2)
  expect_false(dir.exists(file.path(path, ".survlearning-lock")))
  second <- surv_tune(f, d, "aft", list(distribution = c("weibull", "lognormal")),
    folds = 3, seed = 8, checkpoint = path, workers = 1)
  expect_equal(second$execution$resumed_jobs, 6)
  expect_equal(second$execution$computed_jobs, 0)
  expect_true(second$execution$refit_reused)
  expect_identical(first$fold_results, second$fold_results)
  unlink(file.path(path, "candidate-00002-fold-00003.rds"))
  unlink(file.path(path, "winner.rds"))
  partial <- surv_tune(f, d, "aft", list(distribution = c("weibull", "lognormal")),
    folds = 3, seed = 8, checkpoint = path, workers = 2)
  expect_equal(partial$execution$resumed_jobs, 5)
  expect_equal(partial$execution$computed_jobs, 1)
  expect_equal(partial$fold_results$estimate, first$fold_results$estimate)
  changed <- d
  changed$x[1] <- changed$x[1] + 1
  expect_error(surv_tune(f, changed, "aft", list(distribution = c("weibull", "lognormal")),
    folds = 3, seed = 8, checkpoint = path), "mismatch")
  expect_error(surv_tune(f, d, "aft", list(distribution = "weibull"),
    folds = 3, seed = 8, checkpoint = path), "mismatch")
  expect_error(surv_tune(f, d, "aft", list(distribution = c("weibull", "lognormal")),
    folds = 3, seed = 8, checkpoint = path, resume = FALSE), "already exists")
  expect_false(dir.exists(file.path(path, ".survlearning-lock")))
  broken <- tempfile("failed-checkpoint-")
  failed <- surv_tune(f, d, "aft", list(distribution = c("invalid", "weibull")), folds = 3, checkpoint = broken, seed = 8)
  retried <- surv_tune(f, d, "aft", list(distribution = c("invalid", "weibull")), folds = 3,
    checkpoint = broken, retry_failed = TRUE, seed = 8)
  expect_equal(retried$execution$computed_jobs, 3)
  expect_equal(retried$execution$resumed_jobs, 3)
  expect_equal(retried$results$failed_folds, failed$results$failed_folds)
  expect_identical(retried$best_candidate, 2L)
})

test_that("portable formulas and checkpoint corruption fail clearly", {
  d <- example_data(60)
  captured <- 3
  expect_error(surv_tune(Surv(time, event) ~ I(x*captured) + z, d,
    checkpoint = tempfile("captured-"), seed = 8), "external")
  custom <- function(x) x
  expect_error(surv_tune(Surv(time, event) ~ custom(x) + z, d, workers = 2), "Custom")
  path <- tempfile("corrupt-")
  surv_tune(Surv(time, event) ~ x + z, d, folds = 2, checkpoint = path, seed = 8)
  writeLines("not an RDS", file.path(path, "candidate-00001-fold-00001.rds"))
  expect_error(surv_tune(Surv(time, event) ~ x + z, d, folds = 2, checkpoint = path, seed = 8), "Unreadable")
  expect_false(dir.exists(file.path(path, ".survlearning-lock")))
})

test_that("parallel conditional evaluation learns G only from analysis folds", {
  d <- example_data(90)
  f <- Surv(time, event) ~ x + z
  spec <- list(formula = Surv(time, event) ~ batch, method = "stratified")
  serial <- surv_tune(f, d, "cox", folds = 3, metric = "brier", times = 3, censoring = spec, seed = 8)
  parallel <- surv_tune(f, d, "cox", folds = 3, metric = "brier", times = 3, censoring = spec, workers = 2, seed = 8)
  expect_equal(serial$fold_results$estimate, parallel$fold_results$estimate)
  expect_equal(parallel$fold_results$censoring_n, parallel$fold_results$n_analysis)
})
