test_that("Cox neural losses agree with independent Cox and finite differences", {
  d <- data.frame(time = c(1,1,1,2,2,3,4,4,5), event = c(1,1,0,1,0,1,1,0,0))
  lp <- c(.4,-.2,.7,.1,.8,-.6,.3,.9,-.1)
  y <- survival::Surv(d$time, d$event)
  for (ties in c("breslow", "efron")) {
    groups <- survlearning:::.neural_cox_groups(y, ties)
    actual <- survlearning:::.neural_cox_loss(lp, groups)
    ref <- survival::coxph(survival::Surv(time,event) ~ offset(lp), d, ties = ties)
    expect_equal(actual$value, -tail(ref$loglik,1) / sum(d$event), tolerance = 1e-10)
    numerical <- vapply(seq_along(lp), function(j) {
      up <- down <- lp
      up[j] <- up[j] + 1e-5
      down[j] <- down[j] - 1e-5
      (survlearning:::.neural_cox_loss(up, groups)$value -
        survlearning:::.neural_cox_loss(down, groups)$value) / 2e-5
    }, 0)
    expect_equal(actual$gradient, numerical, tolerance = 1e-8)
    expect_equal(sum(actual$gradient), 0, tolerance = 1e-12)
    expect_equal(survlearning:::.neural_cox_loss(lp + 10000, groups)$value,
      actual$value, tolerance = 1e-10)
    order <- c(9,3,2,7,5,1,6,8,4)
    permuted <- survlearning:::.neural_cox_loss(lp[order],
      survlearning:::.neural_cox_groups(y[order, ], ties))
    expect_equal(permuted$value, actual$value, tolerance = 1e-12)
    expect_equal(permuted$gradient, actual$gradient[order], tolerance = 1e-12)
    # Compare baseline increments independently, not by differentiating itself.
    manual <- vapply(sort(unique(d$time)), function(t) {
      at <- which(d$time == t & d$event == 1)
      if (!length(at)) return(0)
      risk <- sum(exp(lp[d$time >= t]))
      if (ties == "breslow") return(length(at) / risk)
      sum(1 / (risk - (seq_along(at)-1)/length(at) * sum(exp(lp[at]))))
    }, 0)
    expect_equal(exp(rev(actual$log_increment)), manual, tolerance = 1e-12)
  }
})

test_that("untied Cox approximations coincide and extreme risks stay finite", {
  y <- survival::Surv(1:8, c(1,0,1,1,0,1,1,0))
  lp <- c(-1000,1000,-700,700,0,-100,100,0)
  a <- survlearning:::.neural_cox_loss(lp, survlearning:::.neural_cox_groups(y,"efron"))
  b <- survlearning:::.neural_cox_loss(lp, survlearning:::.neural_cox_groups(y,"breslow"))
  expect_true(is.finite(a$value))
  expect_true(all(is.finite(a$gradient)))
  expect_equal(a$value, b$value)
  expect_equal(a$gradient, b$gradient)
  expect_equal(sum(a$gradient), 0, tolerance = 1e-10)
  expect_equal(survlearning:::.neural_logcumsumexp(c(-Inf,-Inf,0,-Inf)), c(-Inf,-Inf,0,0))
})

test_that("analytic backpropagation includes every weight and regularization", {
  set.seed(19)
  x <- matrix(rnorm(30),15,2)
  y <- survival::Surv(rep(1:5,each=3), rep(c(1,1,0),5))
  theta <- survlearning:::.neural_cox_initial(2,3)
  for (ties in c("efron","breslow")) {
    g <- survlearning:::.neural_cox_groups(y,ties)
    f <- function(z) survlearning:::.neural_cox_objective(z,x,g,3,.02)
    actual <- f(theta)
    numerical <- vapply(seq_along(theta), function(j) {
      up <- down <- theta
      up[j] <- up[j]+1e-5
      down[j] <- down[j]-1e-5
      (f(up)$value-f(down)$value)/2e-5
    },0)
    expect_equal(actual$gradient,numerical,tolerance=1e-8)
  }
})

test_that("native neural Cox fits ties, predicts, tunes and restores RNG", {
  d <- example_data(100)
  d$time <- ceiling(d$time)
  f <- survival::Surv(time,event) ~ x+z+batch
  set.seed(81)
  old <- .Random.seed
  a <- surv_fit(f,d,"neural_cox",list(epochs=60L),seed=8)
  expect_identical(.Random.seed,old)
  expect_identical(a$engine_package,"stats")
  expect_identical(a$engine$backend,"native")
  expect_identical(a$diagnostics$convergence$status,"not_reported")
  expect_lt(a$engine$final_loss,a$engine$loss_history[1])
  expect_true(all(is.finite(surv_predict(a,d))))
  times <- c(3,0,1,3,a$max_time+1)
  expect_warning(curves <- surv_predict(a,d[1:2,],"survival",times), "beyond training")
  expect_equal(dim(curves),c(2L,5L))
  expect_equal(unname(curves[,2]),c(1,1))
  expect_equal(curves[,1],curves[,4])
  expect_true(all(is.na(curves[,5])))
  expect_true(all(curves[,3] >= curves[,1]))
  path <- tempfile(fileext=".rds")
  saveRDS(a,path)
  expect_equal(surv_predict(readRDS(path),d),surv_predict(a,d))
  unlink(path)
  tuned <- surv_tune(f,d,"neural_cox",grid=list(decay=c(.001,.01)),
    params=list(epochs=10L),folds=2)
  expect_true(all(tuned$results$failed_folds==0))
  expect_error(surv_fit(f,d,"neural_cox",list(ties="random")),"ties")
  expect_error(surv_fit(f,d,"neural_cox",list(threads=2)),"BLAS")
  expect_error(surv_fit(f,d,"neural_cox",list(max_weights=2)),"max_weights")
})

test_that("saved old neural arrays predict without their former engine", {
  d <- example_data(50)
  fit <- surv_fit(survival::Surv(time,event) ~ x+z,d,"neural_cox",list(epochs=5L))
  native <- surv_predict(fit,d)
  w <- fit$engine$weights
  fit$engine$backend <- NULL
  fit$engine$state <- list("0.weight"=t(w$w), "0.bias"=w$bias,
    "2.weight"=matrix(-w$output,nrow=1), "2.bias"=-w$intercept)
  fit$engine$weights <- NULL
  expect_equal(surv_predict(fit,d),native,tolerance=1e-12)
})
