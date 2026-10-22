test_that("restored fits load their optional engine before S3 prediction", {
  skip_if_not_installed("glmnet", minimum_version = "5.0")
  skip_if_not_installed("callr")
  data <- example_data()
  fit <- surv_fit(Surv(time, event) ~ x + z, data, "cox_lasso")
  new <- data[1:3, c("x", "z")]
  expected <- surv_predict(fit, new)
  path <- tempfile(fileext = ".rds")
  on.exit(unlink(path), add = TRUE)
  saveRDS(fit, path)
  root <- normalizePath(testthat::test_path("..", ".."), winslash = "/")
  result <- callr::r(function(path, new, root) {
    if (file.exists(file.path(root, "R", "fit.R")) &&
        requireNamespace("pkgload", quietly = TRUE)) pkgload::load_all(root, quiet = TRUE)
    else library(survlearning)
    before <- "glmnet" %in% loadedNamespaces()
    restored <- readRDS(path)
    after_read <- "glmnet" %in% loadedNamespaces()
    prediction <- survlearning::surv_predict(restored, new)
    list(before = before, after_read = after_read, prediction = prediction,
      loaded = "glmnet" %in% loadedNamespaces())
  }, args = list(path = path, new = new, root = root), libpath = .libPaths(),
    user_profile = FALSE, system_profile = FALSE)
  expect_false(result$before)
  expect_false(result$after_read)
  expect_equal(result$prediction, expected)
  expect_true(result$loaded)
})
