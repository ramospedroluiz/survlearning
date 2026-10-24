rank_enumerated <- function(score, y) {
  pairs <- which(outer(y[, 1], y[, 1], "<") & y[, 2] == 1, arr.ind = TRUE)
  residual <- pmax(0, 1 - score[pairs[, 1]] + score[pairs[, 2]])
  gradient <- numeric(length(score))
  for (k in seq_len(nrow(pairs))) {
    gradient[pairs[k, 1]] <- gradient[pairs[k, 1]] - 2 * residual[k] / nrow(pairs)
    gradient[pairs[k, 2]] <- gradient[pairs[k, 2]] + 2 * residual[k] / nrow(pairs)
  }
  list(loss = mean(residual^2), gradient = gradient, pairs = as.numeric(nrow(pairs)))
}

svm_learning_data <- function(n = 70L) {
  set.seed(622)
  x <- rnorm(n)
  z <- rnorm(n)
  t <- exp(1 - x + 0.3 * z + rnorm(n, sd = 0.4))
  c <- runif(n, 1, 6)
  data.frame(time = pmin(t, c), event = as.integer(t <= c), x = x, z = z)
}

test_that("Fenwick ranking matches every comparable pair including ties", {
  for (seed in 1:15) {
    set.seed(seed)
    n <- 15 + seed
    y <- cbind(sample(1:8, n, TRUE), sample(c(0, 1), n, TRUE))
    score <- if (seed %% 2) rnorm(n) else sample(-2:2, n, TRUE)
    expected <- rank_enumerated(score, y)
    expect_equal(.svm_rank_loss(score, y), expected, tolerance = 1e-12)
    expect_equal(.svm_rank_loss(score + 40, y), expected, tolerance = 1e-11)
    order <- sample.int(n)
    reordered <- .svm_rank_loss(score[order], y[order, ])
    expect_equal(reordered$loss, expected$loss, tolerance = 1e-12)
    expect_equal(reordered$gradient[order(order)], expected$gradient, tolerance = 1e-12)
    expect_equal(sum(expected$gradient), 0, tolerance = 1e-12)
  }
  expect_error(.svm_rank_loss(c(0, 0), cbind(c(1, 1), c(1, 0))), "No strictly")
  expect_error(.svm_rank_loss(c(NA, 0), cbind(c(1, 2), c(1, 0))), "finite")
})

test_that("exact additive whitening and all learning gradients are correct", {
  d <- svm_learning_data(18)
  x <- as.matrix(d[c("x", "z")])
  y <- as.matrix(d[c("time", "event")])
  for (kernel in c("linear", "additive")) {
    design <- .svm_learning_design(x, kernel)
    p <- ncol(design$z)
    a <- seq_len(p) / p
    beta <- .svm_learning_map(a, design)
    expect_equal(sum(beta * as.numeric(design$p %*% beta)), sum(a^2), tolerance = 1e-10)
    for (method in .svm_learning_methods()) {
      setting <- list(kernel = kernel, lambda = 0.1)
      if (method == "svm_ipcw") setting$horizon <- 1.2
      params <- .parameters(method, setting, ncol(x))
      response <- if (method == "svm_ipcw") .svm_ipcw_response(x, y, params) else NULL
      objective <- .svm_learning_objective(method, design, y, params, response)
      par <- if (method == "svm_rank") a else c(a, -1)
      epsilon <- 1e-6
      numeric_gradient <- vapply(seq_along(par), function(j) {
        step <- numeric(length(par)); step[j] <- epsilon
        (objective(par + step)$value - objective(par - step)$value) / (2 * epsilon)
      }, 0)
      expect_equal(objective(par)$gradient, numeric_gradient, tolerance = 1e-6)
    }
  }
})

test_that("ranking loss and gradient are invariant to large common offsets", {
  y <- cbind(c(1, 2, 3, 4), c(1, 0, 1, 0))
  score <- c(0, .5, -.25, 1.5)
  expected <- rank_enumerated(score, y)
  for (offset in c(-1e12, -1e10, 0, 1e8, 1e10, 1e12)) {
    shifted <- score + offset
    original <- shifted
    expect_equal(.svm_rank_loss(shifted, y), expected, tolerance = 1e-12)
    expect_identical(shifted, original)
  }
  expect_error(.svm_rank_loss(c(-1e308, 1e308), cbind(1:2, c(1, 0))), "range")
})

test_that("hybrid and positive-epsilon IPCW agree with independent quadratic programs", {
  d <- svm_learning_data(18)
  formula <- survival::Surv(time, event) ~ x + z
  prepared <- .prepare_design(formula, d)
  x <- .scaled_design(prepared$x, prepared$design)
  y <- prepared$y
  n <- nrow(d)
  pairs <- which(outer(y[, 1], y[, 1], "<") & y[, 2] == 1, arr.ind = TRUE)
  m <- nrow(pairs)
  for (kernel in c("linear", "additive")) {
    design <- .svm_primal_design(x, kernel)
    z <- cbind(design$z, 1)
    p <- ncol(z)
    penalty <- Matrix::bdiag(.1 * design$p, 0)
    for (method in c("svm_hybrid", "svm_ipcw")) {
      params <- list(kernel = kernel, lambda = .1, prediction_tolerance = 0)
      if (method == "svm_hybrid") {
        eta <- .75
        params$rank_weight <- eta
        target <- y[, 1] / max(y[, 1])
        event <- which(y[, 2] == 1)
        censor <- which(y[, 2] == 0)
        h <- Matrix::bdiag(penalty, Matrix::Diagonal(m, 2 * eta / m),
          Matrix::Diagonal(n, 2 * (1 - eta) / n))
        a <- rbind(cbind(z[event, , drop = FALSE], Matrix::Matrix(0, length(event), m),
                        -Matrix::Diagonal(n)[event, , drop = FALSE]),
          cbind(-(z[pairs[, 1], , drop = FALSE] - z[pairs[, 2], , drop = FALSE]),
                -Matrix::Diagonal(m), Matrix::Matrix(0, m, n)),
          cbind(z[censor, , drop = FALSE], Matrix::Matrix(0, length(censor), m),
                -Matrix::Diagonal(n)[censor, , drop = FALSE]))
        b <- c(-target[event], rep(-1, m), -target[censor])
        cones <- list(z = length(event), l = m + length(censor))
      } else {
        params$horizon <- 1.5
        params$epsilon <- .02
        response <- .svm_ipcw_response(x, y, .parameters(method, params, ncol(x)))
        h <- Matrix::bdiag(penalty, Matrix::Diagonal(x = 2 * response$weights / n))
        a <- rbind(cbind(z, -Matrix::Diagonal(n)), cbind(-z, -Matrix::Diagonal(n)))
        b <- c(params$epsilon - response$target, params$epsilon + response$target)
        cones <- list(l = 2L * n)
      }
      reference <- clarabel::clarabel(P = methods::as(h, "symmetricMatrix"),
        q = rep(0, ncol(h)), A = a, b = b, cones = cones,
        control = clarabel::clarabel_control(verbose = FALSE, max_threads = 1L,
          tol_gap_abs = 1e-11, tol_gap_rel = 1e-11, tol_feas = 1e-11))
      fit <- surv_fit(formula, d, method, params)
      expect_equal(reference$status, 2L)
      expect_equal(fit$engine$diagnostics$objective, reference$obj_val, tolerance = 1e-8)
      expect_equal(surv_predict(fit, d), as.numeric(z %*% reference$x[seq_len(p)]), tolerance = 1e-5)
    }
  }
})

test_that("strict ranking pairs differ explicitly from Harrell event-censor ties", {
  y <- survival::Surv(c(1, 1, 2), c(1, 0, 1))
  risk <- c(1, 3, 0)
  expect_equal(.svm_rank_loss(risk, y)$pairs, 1)
  expect_equal(.cindex(y, risk), .5)
  expect_equal(rank_enumerated(risk, y)$loss, 0)
})

test_that("ranking agrees with independent explicit-pair QP", {
  d <- svm_learning_data(16)
  formula <- survival::Surv(time, event) ~ x + z
  prepared <- .prepare_design(formula, d)
  x <- .scaled_design(prepared$x, prepared$design)
  y <- prepared$y
  pairs <- which(outer(y[, 1], y[, 1], "<") & y[, 2] == 1, arr.ind = TRUE)
  m <- nrow(pairs)
  for (kernel in c("linear", "additive")) {
    design <- .svm_primal_design(x, kernel)
    p <- ncol(design$z)
    difference <- design$z[pairs[, 1], , drop = FALSE] - design$z[pairs[, 2], , drop = FALSE]
    h <- Matrix::bdiag(0.1 * design$p, Matrix::Diagonal(m, 2 / m))
    h <- methods::as(h, "symmetricMatrix")
    result <- clarabel::clarabel(P = h, q = rep(0, p + m),
      A = cbind(-difference, -Matrix::Diagonal(m)), b = rep(-1, m), cones = list(l = m),
      control = clarabel::clarabel_control(verbose = FALSE, max_threads = 1L,
        tol_gap_abs = 1e-11, tol_gap_rel = 1e-11, tol_feas = 1e-11))
    expect_equal(result$status, 2L)
    model <- surv_fit(formula, d, "svm_rank", list(kernel = kernel, lambda = 0.1,
      prediction_tolerance = 0, tolerance = 1e-7))
    expected <- as.numeric(design$z %*% result$x[seq_len(p)])
    expect_equal(surv_predict(model, d), expected, tolerance = 1e-5)
    expect_equal(model$engine$diagnostics$objective, result$obj_val, tolerance = 1e-8)
    expect_lte(model$engine$diagnostics$strong_convexity_gap_bound, 1e-7)
  }
})

test_that("IPCW includes observed restricted outcomes and rejects unsupported horizons", {
  x <- matrix(1:6, ncol = 1, dimnames = list(NULL, "x1"))
  y <- cbind(c(1, 2, 2, 3, 4, 5), c(1, 1, 0, 0, 1, 0))
  params <- .parameters("svm_ipcw", list(horizon = 3, min_g = 0.01), 1)
  response <- .svm_ipcw_response(x, y, params)
  expect_equal(response$weights, c(1, 1, 0, 4/3, 4/3, 4/3), tolerance = 1e-12)
  expect_equal(response$target, c(1/3, 2/3, 2/3, 1, 1, 1))
  expect_equal(response$diagnostics$observed_targets, 5)
  params$horizon <- 6
  expect_error(.svm_ipcw_response(x, y, params), "exceeds")
  params$horizon <- 4
  params$min_g <- 0.9
  expect_error(.svm_ipcw_response(x, y, params), "Insufficient censoring support")
  params$horizon <- 2
  params$min_g <- 0.01
  params$censoring_method <- "cox"
  d <- svm_learning_data()
  fit <- .svm_ipcw_response(as.matrix(d[c("x", "z")]), as.matrix(d[c("time", "event")]), params)
  expect_true(all(is.finite(fit$weights)))
})

test_that("new variants preserve risk direction, serialization, and native capabilities", {
  d <- svm_learning_data()
  formula <- survival::Surv(time, event) ~ x + z
  for (method in .svm_learning_methods()) for (kernel in c("linear", "additive")) {
    params <- list(kernel = kernel)
    if (method == "svm_ipcw") params$horizon <- 1.5
    model <- surv_fit(formula, d, method, params)
    score <- surv_predict(model, d)
    expect_gt(stats::cor(score, d$x), 0.5)
    expect_equal(score, surv_predict(unserialize(serialize(model, NULL)), d))
    expect_equal(surv_predict(model, d[1:2, ]), score[1:2])
    expect_error(surv_predict(model, d, type = "survival", times = 1), "does not provide")
    expect_error(surv_predict(model, d, type = "time"), "does not provide")
    expect_equal(model$n, nrow(d))
    expect_true(model$diagnostics$convergence$status %in%
      c("reported_success", "stationarity_verified"))
  }
  expect_error(surv_fit(formula, d, "svm_ipcw"), "horizon")
  expect_error(surv_fit(formula, d, "svm_hybrid", list(rank_weight = 1)), "strictly")
  expect_error(surv_fit(formula, d, "svm_rank", list(max_iter = 1L)), "No model")
})

test_that("line-search exits require the same independent accuracy certificate", {
  result <- list(par = c(.2, -.1), convergence = 52L,
    message = "ERROR: ABNORMAL_TERMINATION_IN_LNSRCH")
  evaluated <- list(value = .4, gradient = c(1e-8, -1e-8))
  params <- list(lambda = .01, tolerance = 1e-6)
  for (method in .svm_learning_methods()) {
    checked <- .svm_learning_acceptance(result, evaluated, method, params)
    expect_true(checked$accepted)
    expect_identical(checked$status, "stationarity_verified")
    bad <- evaluated
    bad$gradient <- c(1e-3, 0)
    expect_false(.svm_learning_acceptance(result, bad, method, params)$accepted)
    for (code in c(1L, 51L)) {
      failed <- result
      failed$convergence <- code
      expect_false(.svm_learning_acceptance(failed, evaluated, method, params)$accepted)
    }
    failed <- result
    failed$message <- "ERROR: other failure"
    expect_false(.svm_learning_acceptance(failed, evaluated, method, params)$accepted)
    bad$gradient <- c(NA_real_, 0)
    expect_false(.svm_learning_acceptance(result, bad, method, params)$accepted)
  }
  params$lambda <- 1e-12
  expect_false(.svm_learning_acceptance(result, evaluated, "svm_rank", params)$accepted)
  engine <- structure(list(diagnostics = list(status = "stationarity_verified",
    optimizer_status = 52L)), class = "survlearning_svm_learning")
  expect_identical(.engine_convergence(engine),
    list(status = "stationarity_verified", code = 52L))
})

test_that("new SVM tuning caches only within folds and supports parallel resume", {
  d <- svm_learning_data()
  formula <- survival::Surv(time, event) ~ x + z
  for (method in .svm_learning_methods()) {
    params <- if (method == "svm_ipcw") list(horizon = 1.2) else list()
    grid <- list(lambda = c(0.01, 0.1))
    cached <- surv_tune(formula, d, method, grid, params, folds = 2, seed = 7)
    cold <- surv_tune(formula, d, method, grid, params, folds = 2, seed = 7, cache = FALSE)
    expect_equal(cached$results$mean_cindex, cold$results$mean_cindex)
    expect_equal(surv_predict(cached, d), surv_predict(cold, d))
    expect_true(any(cached$fold_results$cache_hit))
  }
  directory <- tempfile()
  on.exit(unlink(directory, recursive = TRUE))
  a <- surv_tune(formula, d, "svm_rank", list(lambda = c(0.01, 0.1)),
    folds = 2, seed = 8, workers = 2, checkpoint = directory)
  b <- surv_tune(formula, d, "svm_rank", list(lambda = c(0.01, 0.1)),
    folds = 2, seed = 8, checkpoint = directory)
  expect_equal(surv_predict(a, d), surv_predict(b, d))
  expect_equal(b$execution$computed_jobs, 0L)
})

test_that("IPCW zero-epsilon solution matches independent weighted normal equations", {
  d <- svm_learning_data(22)
  formula <- survival::Surv(time, event) ~ x + z
  prepared <- .prepare_design(formula, d)
  x <- .scaled_design(prepared$x, prepared$design)
  for (kernel in c("linear", "additive")) {
    params <- .parameters("svm_ipcw", list(kernel = kernel, lambda = 0.1, horizon = 1.5), 2)
    response <- .svm_ipcw_response(x, prepared$y, params)
    design <- .svm_primal_design(x, kernel)
    z <- cbind(design$z, 1)
    w <- Matrix::Diagonal(x = response$weights)
    h <- Matrix::crossprod(z, w %*% z) / nrow(d) +
      params$lambda / 2 * Matrix::bdiag(design$p, 0)
    rhs <- -Matrix::crossprod(z, response$weights * response$target) / nrow(d)
    beta <- as.numeric(Matrix::solve(h, rhs))
    fit <- surv_fit(formula, d, "svm_ipcw", params)
    expect_equal(surv_predict(fit, d), as.numeric(z %*% beta), tolerance = 1e-5)
    scaled <- d
    scaled$time <- scaled$time * 100
    params$horizon <- params$horizon * 100
    rescaled <- surv_fit(formula, scaled, "svm_ipcw", params)
    expect_equal(surv_predict(fit, d), surv_predict(rescaled, scaled), tolerance = 1e-7)
  }
})

test_that("ranking can distinguish failure rates under 97.5 percent administrative censoring", {
  d <- data.frame(x = rep(0:1, each = 1000),
    event = c(rep(1, 10), rep(0, 990), rep(1, 40), rep(0, 960)))
  d$time <- ifelse(d$event == 1, 1, 10)
  formula <- survival::Surv(time, event) ~ x
  regression <- surv_fit(formula, d, "svm", list(cost = 0.1))
  ranking <- surv_fit(formula, d, "svm_rank", list(lambda = 0.01))
  expect_equal(length(unique(surv_predict(regression, d))), 1L)
  expect_gt(surv_predict(ranking, data.frame(x = 1)), surv_predict(ranking, data.frame(x = 0)))
  expect_gt(surv_score(ranking, d), 0.6)
  expect_equal(ranking$n, 2000L)
})
