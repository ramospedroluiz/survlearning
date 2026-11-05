test_that("DESCRIPTION contains linked methodological references", {
  description <- utils::packageDescription("survlearning")$Description
  expect_match(description, "Simon et al. (2011) <doi:10.18637/jss.v039.i05>",
    fixed = TRUE)
  expect_match(description, "Ishwaran et al. (2008) <doi:10.1214/08-AOAS169>",
    fixed = TRUE)
})

test_that("RSF fitting and prediction are silent by default", {
  d <- example_data(80L)
  expect_silent(fit <- surv_fit(Surv(time, event) ~ x + z + batch, d, "rsf",
    list(trees = 30L, min_node = 5L, threads = 1L), seed = 31))
  expect_silent(risk <- surv_predict(fit, d))
  expect_silent(probability <- surv_predict(fit, d, "survival", c(1, 2, 4)))
  expect_true(all(is.finite(risk)))
  expect_true(all(is.finite(probability)))
  expect_true(all(probability >= 0 & probability <= 1))
})

test_that("RSF version errors contain their diagnostic without console output", {
  d <- example_data(60L)
  withr::local_options(list(rf.cores = 1L))
  core <- survlearning:::.rsf_core
  fit <- core$fit(Surv(time, event) ~ x + z, d, ntree = 20L, seed = 31)
  for (version in list(NULL, "2.2.0")) {
    invalid <- fit
    invalid$forest$version <- version
    expect_silent(error <- tryCatch(core$predict(invalid, newdata = d),
      error = identity))
    expect_s3_class(error, "error")
    expect_match(conditionMessage(error), "RF-SRC engine version 2.3.0", fixed = TRUE)
    expect_match(conditionMessage(error), if (is.null(version)) "unknown" else version,
      fixed = TRUE)
  }
})

test_that("RSF native errors keep their class, message and call without printing", {
  d <- example_data(60L)
  withr::local_options(list(rf.cores = 1L))
  core <- survlearning:::.rsf_core
  fit <- core$fit(Surv(time, event) ~ x + z, d, ntree = 20L, seed = 31)
  # A child environment isolates the failing native call from the package core.
  local_core <- new.env(parent = environment(core$fit))
  original <- structure(list(message = "Native RSF test failure", call = quote(native_fit())),
    class = c("rsf_test_error", "error", "condition"))
  local_core$.Call <- function(...) stop(original)
  local_core$generic.predict.rfsrc <- get("generic.predict.rfsrc",
    envir = environment(core$fit))
  environment(local_core$generic.predict.rfsrc) <- local_core
  grow <- core$fit
  predict <- core$predict
  partial <- get("partial.rfsrc", envir = environment(core$fit))
  environment(grow) <- environment(predict) <- environment(partial) <- local_core

  expect_silent(grow_error <- tryCatch(
    grow(Surv(time, event) ~ x + z, d, ntree = 20L, seed = 31), error = identity))
  expect_identical(grow_error, original)
  expect_silent(predict_error <- tryCatch(predict(fit, newdata = d), error = identity))
  expect_identical(predict_error, original)
  expect_silent(partial_error <- tryCatch(partial(fit, partial.type = "mort",
    partial.xvar = "x", partial.values = c(-1, 1)), error = identity))
  expect_identical(partial_error, original)
})

test_that("optional RSF diagnostics can be suppressed without changing results", {
  ginverse <- get("ginverse", envir = environment(survlearning:::.rsf_core$fit))
  x <- matrix(c(1, 2, 3, 4, 6, 9), nrow = 3)
  expect_silent(reference <- ginverse(x))
  expect_output(expect_message(diagnostic <- ginverse(x, test = TRUE),
    "Test three properties"), NA)
  expect_silent(quiet <- suppressMessages(ginverse(x, test = TRUE)))
  expect_identical(diagnostic, reference)
  expect_identical(quiet, reference)
})
