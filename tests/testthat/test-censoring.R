test_that("stratified reference G preserves failure/censor phase ordering and support", {
  ref <- data.frame(time = c(1, 1, 2, 3, 5, 1, 2, 2, 4, 5),
    event = c(1, 0, 1, 0, 1, 0, 1, 0, 1, 1), batch = rep(c("A", "B"), each = 5))
  g <- surv_censor_fit(Surv(time, event) ~ batch, ref, "stratified")
  nd <- data.frame(batch = c("A", "B"))
  expect_equal(surv_censor_predict(g, nd, c(0, 1, 2, 3)),
    rbind(c(1, .75, .75, .375), c(1, .8, 8/15, 8/15)))
  expect_equal(surv_censor_predict(g, nd, c(1, 2), left = TRUE, pairwise = TRUE), c(1, .8))
  expect_error(surv_censor_predict(g, data.frame(batch = "C"), 1), "Unknown")
  expect_error(surv_censor_predict(g, nd, 6), "follow-up")
  expect_error(surv_censor_predict(g, nd, 1, pairwise = TRUE), "align")
  expect_error(surv_censor_fit(Surv(time, event) ~ time, ref, "stratified"), "categorical")
})

test_that("conditional probability metrics and concordance match explicit weighted pairs", {
  ref <- data.frame(time = c(1, 1, 2, 3, 5, 1, 2, 2, 4, 5),
    event = c(1, 0, 1, 0, 1, 0, 1, 0, 1, 1), batch = rep(c("A", "B"), each = 5))
  g <- surv_censor_fit(Surv(time, event) ~ batch, ref, "stratified")
  fit <- surv_fit(Surv(time, event) ~ x + z, example_data())
  d <- data.frame(time = c(1, 2, 2, 3, 4), event = c(1, 0, 1, 1, 0),
    x = c(.2, -.8, .5, .1, 1), z = 0, batch = c("A", "A", "B", "B", "A"))
  risk <- surv_predict(fit, d)
  s <- surv_predict(fit, d, "survival", 2)[, 1]
  out <- surv_evaluate(fit, d, c("brier", "auc", "ipcw_c"), times = 2, tau = 3,
    censoring_model = g, min_g = .1)
  brier <- (s[1]^2 + s[3]^2/.8 + (1-s[4])^2/(8/15) + (1-s[5])^2/.75)/5
  expect_equal(out$estimate[1], unname(brier))
  numerator <- denominator <- 0
  for (i in c(1, 3)) for (j in c(4, 5)) {
    weight <- 1 / (c(1, NA, .8, NA, NA)[i] * c(NA, NA, NA, 8/15, .75)[j])
    numerator <- numerator + weight * ((risk[i] > risk[j]) + .5*(risk[i] == risk[j]))
    denominator <- denominator + weight
  }
  expect_equal(out$estimate[2], numerator/denominator)
  G <- function(group, t, left = FALSE) {
    if (group == "A") {
      if (if (left) t <= 1 else t < 1) return(1)
      if (if (left) t <= 3 else t < 3) return(.75)
      return(.375)
    }
    if (if (left) t <= 1 else t < 1) return(1)
    if (if (left) t <= 2 else t < 2) return(.8)
    8/15
  }
  numerator <- denominator <- 0
  for (i in which(d$event == 1 & d$time <= 3)) for (j in which(d$time > d$time[i])) {
    weight <- 1/(G(d$batch[i], d$time[i], TRUE) * G(d$batch[j], d$time[i]))
    numerator <- numerator + weight*((risk[i] > risk[j]) + .5*(risk[i] == risk[j]))
    denominator <- denominator + weight
  }
  expect_equal(out$estimate[3], numerator/denominator)
  expect_error(surv_evaluate(fit, d, "uno", tau = 3, censoring_model = g), "ipcw_c")
  expect_error(surv_evaluate(fit, d, "brier", times = 3, censoring_model = g, min_g = .6), "support")
  expect_error(surv_evaluate(fit, d, "brier", times = 2, censoring_model = g, censoring_data = d), "not both")
  a <- surv_evaluate(fit, d, "brier", times = 2, censoring_model = g, n_boot = 5, seed = 7)
  expect_equal(a, surv_evaluate(fit, d, "brier", times = 2, censoring_model = g, n_boot = 5, seed = 7))
  d$event <- 0L
  expect_true(is.finite(surv_evaluate(fit, d, "brier", times = 2, censoring_model = g)$estimate))
})

test_that("conditional Cox uses the correct exact phase risk sets and serializes", {
  d <- example_data(120)
  d$time <- pmax(1, round(d$time))
  original <- d
  g <- surv_censor_fit(Surv(time, event) ~ x + z, d, "cox")
  phase <- 2*match(d$time, sort(unique(d$time))) - d$event
  status <- 1-d$event
  independent <- survival::coxph(survival::Surv(phase, status) ~ x + z, d,
    ties = "efron", x = TRUE, y = TRUE, control = survival::coxph.control(timefix = FALSE))
  expect_equal(unname(g$coefficients), unname(coef(independent)), tolerance = 1e-12)
  expect_identical(d, original)
  nd <- d[1:2, ]
  p <- surv_censor_predict(g, nd, c(0, 2, 1, 2))
  expect_equal(unname(p[, 1]), c(1, 1))
  expect_equal(p[, 2], p[, 4])
  expect_true(all(p >= 0 & p <= 1))
  expect_true(all(p[, 2] <= p[, 3]))
  path <- tempfile()
  saveRDS(g, path)
  expect_identical(p, surv_censor_predict(readRDS(path), nd, c(0, 2, 1, 2)))
  unlink(path)
  d$event <- 1L
  expect_equal(surv_censor_predict(surv_censor_fit(Surv(time, event) ~ x + z, d), nd, 2), matrix(1, 2, 1))
})

test_that("explicit marginal models retain Uno and conditional fits stay inside folds", {
  d <- example_data(100)
  fit <- surv_fit(Surv(time, event) ~ x + z, d)
  g <- surv_censor_fit(Surv(time, event) ~ 1, d, "km")
  a <- surv_evaluate(fit, d, c("uno", "auc", "brier"), tau = 3, times = 3)
  b <- surv_evaluate(fit, d, c("uno", "auc", "brier"), tau = 3, times = 3, censoring_model = g)
  expect_equal(a$estimate, b$estimate)
  specification <- list(formula = Surv(time, event) ~ batch, method = "stratified")
  tuned <- surv_tune(Surv(time, event) ~ x + z, d, "cox", folds = 3,
    metric = "brier", times = 3, censoring = specification, seed = 9)
  id <- tuned$fold_id
  manual <- surv_fit(Surv(time, event) ~ x + z, d[id != 1, ])
  gm <- surv_censor_fit(Surv(time, event) ~ batch, d[id != 1, ], "stratified")
  expected <- surv_evaluate(manual, d[id == 1, ], "brier", times = 3, censoring_model = gm)
  expect_equal(tuned$fold_results$estimate[1], expected$estimate)
  expect_error(survlearning:::.resample_censoring(g, d), "global")
})
