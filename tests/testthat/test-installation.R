test_that("all supported engines and solvers are required dependencies", {
  metadata <- utils::installed.packages()
  imports <- tools::package_dependencies("survlearning", db = metadata,
    which = "Imports")[["survlearning"]]
  engines <- unique(vapply(survlearning:::.method_specs(), `[[`, "", "engine"))
  solvers <- c("Matrix", "quadprog", "osqp", "clarabel")
  required <- setdiff(unique(c(engines, solvers, "splines", "withr", "Rcpp", "irlba")), "survlearning")
  expect_true(all(required %in% imports))
  expect_true(all(surv_methods()$installed))
  expect_true(all(vapply(required, requireNamespace, logical(1), quietly = TRUE)))
  expect_false(any(c("survivalsvm", "testthat", "knitr", "rmarkdown",
    "callr", "pkgload", "torch", "reticulate", "randomForestSRC", "flexsurv", "penAFT") %in% imports))
})
