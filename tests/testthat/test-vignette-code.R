test_that("installed workflow vignette code runs in a fresh session", {
  skip_if_not_installed("callr")
  path <- system.file("doc", "workflow-extensions.R", package = "survlearning")
  skip_if(!nzchar(path), "Generated vignette code is available after R CMD build")
  result <- callr::r(function(path) {
    environment <- new.env(parent = globalenv())
    grDevices::pdf(tempfile(fileext = ".pdf"))
    on.exit(grDevices::dev.off(), add = TRUE)
    sys.source(path, envir = environment)
    c(n_training = environment$fit$n,
      resumed_jobs = environment$resumed$execution$computed_jobs)
  }, args = list(path = path), libpath = .libPaths(),
    user_profile = FALSE, system_profile = FALSE)
  expect_equal(unname(result), c(200, 0))
})
