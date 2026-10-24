test_that("additive interpolation is the complete training-kernel representer", {
  set.seed(73)
  x <- cbind(rnorm(45), rep(0:2, 15), rep(c(0, 1, 1), 15))
  y <- cbind(runif(45, 1, 6), rep(c(0, 1, 0), 15))
  p <- survlearning:::.parameters("svm",
    list(kernel = "additive", prediction_tolerance = 0), 3)
  fit <- survlearning:::.svm_fit(x, y, p)
  expect_equal(fit$knots, lapply(seq_len(3), function(j) sort(unique(x[, j]))))
  expect_equal(fit$diagnostics$feature_dimension,
    sum(vapply(fit$knots, length, 1L) - 1L))
  newx <- rbind(x, c(-10, -2, -3), c(10, 4, 3))
  k <- matrix(0, nrow(x), nrow(newx))
  for (j in seq_len(ncol(x))) k <- k + 1 -
    abs(outer(x[, j], newx[, j], `-`)) / diff(range(x[, j]))
  reference <- as.vector(crossprod(fit$dual, k))
  actual <- survlearning:::.svm_primal_predict(fit, newx)
  expect_equal(actual - mean(actual), reference - mean(reference), tolerance = 1e-6)
  expect_equal(survlearning:::.svm_primal_predict(fit, newx[1, , drop = FALSE]), actual[1])
  expect_equal(survlearning:::.svm_primal_predict(fit, newx[c(47, 1, 46), ]),
    actual[c(47, 1, 46)])
  expect_error(survlearning:::.svm_primal_design(cbind(x, 1), "additive"), "vary")
})

test_that("rare training levels and duplicate rows remain in the fitted problem", {
  d <- example_data(40)
  d$rare <- c(1, rep(0, 39))
  d <- d[rep(seq_len(40), rep(1:4, 10)), ]
  f <- Surv(time, event) ~ x + z + rare
  a <- surv_fit(f, d, "svm", list(kernel = "additive"))
  b <- surv_fit(f, d, "svm", list(kernel = "additive", collapse_duplicates = FALSE))
  expect_equal(a$n, 100L)
  expect_equal(a$engine$diagnostics$observations, 100L)
  expect_equal(length(a$engine$dual), 100L)
  expect_equal(surv_predict(a, d), surv_predict(b, d))
  expect_true(all(is.finite(surv_predict(a, d[1:2, ]))))
  expect_equal(surv_predict(a, d[1, , drop = FALSE]), surv_predict(a, d)[1])
})

test_that("training-fixed numerical resolution prevents false ordering of ties", {
  set.seed(16)
  d <- data.frame(entry = rep(0:9, 12), z = rnorm(120))
  d$time <- 100 - d$entry
  d$event <- 0
  d$event[c(3, 37, 82)] <- 1
  d$time[d$event == 1] <- 1
  f <- Surv(time, event) ~ entry + z
  a <- surv_fit(f, d, "svm", list(cost = 10))
  b <- surv_fit(f, d, "svm", list(cost = 10, tolerance = 1e-11))
  expect_equal(surv_predict(a, d), surv_predict(b, d), tolerance = 1e-7)
  expect_equal(surv_score(a, d), surv_score(b, d), tolerance = 1e-7)
  expect_equal(a$engine$prediction_resolution, 1e-6)
  for (entry in 0:9) expect_equal(length(unique(surv_predict(a, d[d$entry == entry, ]))), 1L)
  expect_equal(surv_predict(a, d[c(10, 1), ]), surv_predict(a, d)[c(10, 1)])
  raw <- a
  raw$engine$prediction_resolution <- 0
  expect_lt(max(abs(surv_predict(a, d) - surv_predict(raw, d))), 1e-6)
})

test_that("time-unit transformations preserve the estimator with transformed cost", {
  d <- example_data(45)
  for (kernel in c("linear", "additive")) {
    a <- surv_fit(Surv(time, event) ~ x + z, d, "svm",
      list(kernel = kernel, cost = .1))
    d2 <- d; d2$time <- 10 * d$time
    b <- surv_fit(Surv(time, event) ~ x + z, d2, "svm",
      list(kernel = kernel, cost = 1))
    expect_equal(surv_predict(b, d2) / 10, surv_predict(a, d), tolerance = 1e-6)
    expect_equal(surv_score(a, d), surv_score(b, d2))
  }
})
