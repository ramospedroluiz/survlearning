test_that("native kernels preserve dimensions, arithmetic and training-only ranges", {
  x <- cbind(c(-2, 0, 3), c(1, 2, 5))
  newx <- rbind(c(-5, 7), c(0, 2), c(4, -2))
  expected <- matrix(0, nrow(x), nrow(newx))
  for (i in seq_len(nrow(x))) for (j in seq_len(nrow(newx))) {
    expected[i, j] <- sum((c(5, 4) - abs(x[i, ] - newx[j, ])) / c(5, 4))
  }
  expect_equal(survlearning:::.svm_kernel(x, "linear", newx), tcrossprod(x, newx))
  expect_equal(survlearning:::.svm_kernel(x, "additive", newx), expected)
  expect_equal(survlearning:::.svm_kernel(x, "additive", newx[1, , drop = FALSE]),
    expected[, 1, drop = FALSE])
  one <- x[, 1, drop = FALSE]
  expect_equal(survlearning:::.svm_kernel(one, "additive"),
    (5 - abs(outer(one[, 1], one[, 1], `-`))) / 5)
  expect_error(survlearning:::.svm_kernel(cbind(x, 1), "additive"), "constant")
  expect_error(survlearning:::.svm_kernel(x, "other"), "Unknown")
})

test_that("native kernels agree with the independent survivalsvm reference", {
  skip_if_not_installed("survivalsvm")
  for (seed in seq_len(6)) {
    set.seed(seed)
    x <- cbind(rnorm(15), sample(0:2, 15, replace = TRUE), rnorm(15))
    newx <- rbind(x[1:3, , drop = FALSE], c(-10, -2, 9), c(10, 5, -9))
    for (kernel in c("linear", "additive")) {
      reference <- survivalsvm::kernelMatrix(x,
        kernel_type = if (kernel == "linear") "lin_kernel" else "add_kernel",
        kernel_pars = NA, Xt = newx)$Mat
      expect_equal(survlearning:::.svm_kernel(x, kernel, newx), reference,
        tolerance = 1e-14)
      reference <- survivalsvm::kernelMatrix(x,
        kernel_type = if (kernel == "linear") "lin_kernel" else "add_kernel",
        kernel_pars = NA)$Mat
      expect_equal(survlearning:::.svm_kernel(x, kernel), reference, tolerance = 1e-14)
    }
  }
})

test_that("linear censored regression fits, tunes and resumes through the native engine", {
  d <- example_data(50)
  f <- Surv(time, event) ~ x + z
  expect_identical(survlearning:::.method_specs()$svm$engine, "stats")
  expect_identical(survlearning:::.method_specs()$rsf$engine, "survlearning")
  expect_identical(survlearning:::.method_specs()$ranger$engine, "ranger")
  fit <- surv_fit(f, d, "svm", params = list(solver = "clarabel"), seed = 17)
  expect_identical(fit$n, nrow(d))
  expect_true(all(is.finite(surv_predict(fit, d))))
  path <- tempfile("svm-native-checkpoint-")
  on.exit(unlink(path, recursive = TRUE), add = TRUE)
  args <- list(formula = f, data = d, method = "svm",
    grid = list(cost = c(.01, .1)), params = list(solver = "clarabel"),
    folds = 2, seed = 17, checkpoint = path)
  first <- do.call(surv_tune, args)
  second <- do.call(surv_tune, args)
  expect_equal(surv_predict(first, d), surv_predict(second, d))
  expect_equal(second$execution$resumed_jobs, 4)
  metadata <- readRDS(file.path(path, "metadata.rds"))
  expect_false(any(c("survivalsvm", "pracma") %in% names(metadata$versions)))
})

test_that("saved censored-primal objects preserve predictions outside the training range", {
  d <- example_data(35)
  f <- Surv(time, event) ~ x + z
  for (kernel in c("linear", "additive")) {
    fit <- surv_fit(f, d, "svm", list(kernel = kernel), seed = 9)
    new <- d[1:3, ]
    new$x <- new$x + c(-4, 0, 4)
    expected <- surv_predict(fit, new)
    fit <- unserialize(serialize(fit, NULL))
    expect_equal(surv_predict(fit, new), expected, tolerance = 1e-10)
  }
})
