.evaluation_data <- function() {
  data.frame(time = c(1, 2, 2, 4, 5), event = c(1, 1, 0, 1, 0),
    risk = c(3, 0, 2, 1, 0), x = seq_len(5))
}

.evaluation_mock <- function(reference = .evaluation_data(),
    probabilities = rbind(c(.3, .2, .1), c(.5, .4, .2), c(.7, .6, .3),
      c(.9, .8, .4), c(.8, .5, .25)), probability_times = c(1, 2, 4),
    native = TRUE) {
  testthat::local_mocked_bindings(surv_predict = function(object, newdata,
      type = "risk", times = NULL) {
    if (type == "risk") return(newdata$risk)
    if (!native) stop("Method does not provide native survival predictions.")
    probabilities[, match(times, probability_times), drop = FALSE]
  }, .package = "survlearning", .env = parent.frame())
  structure(list(formula = survival::Surv(time, event) ~ x,
    training_outcome = survival::Surv(reference$time, reference$event)),
    class = "survlearning_fit")
}

test_that("all metrics agree with hand-calculated censoring and risk ties", {
  d <- .evaluation_data()
  fit <- .evaluation_mock()
  result <- surv_evaluate(fit, d, c("harrell", "uno", "auc", "brier", "ibs"),
    times = c(1, 2, 4), tau = 4)
  expect_s3_class(result, "survlearning_evaluation")
  expect_identical(class(result), c("survlearning_evaluation", "data.frame"))
  expect_named(result, c("metric", "time", "estimate", "lower", "upper",
    "n", "events", "status", "message"))
  expect_identical(rownames(result), as.character(seq_len(nrow(result))))
  expect_equal(result$estimate[result$metric == "harrell"], 11 / 16)
  expect_equal(result$estimate[result$metric == "uno"], 9 / 11)
  expect_equal(result$estimate[result$metric == "auc"], c(1, 5 / 8, 6 / 7))
  expect_equal(result$estimate[result$metric == "brier"], c(.096, 127 / 1000, 907 / 4000))
  expect_equal(result$estimate[result$metric == "ibs"], 1861 / 12000)
  expect_equal(result$time[result$metric == "uno"], 4)
  expect_true(all(is.na(result$time[result$metric %in% c("harrell", "ibs")])))
  expect_true(all(result$n == 5 & result$events == 3 & result$status == "ok"))
  expect_true(all(is.na(result$lower) & is.na(result$upper)))
  expect_equal(attr(result, "settings")$ibs_interval, c(1, 4))
  expect_match(paste(attr(result, "assumptions"), collapse = " "), "independent")
  expect_equal(attr(result, "censoring")$table$g, c(1, 2 / 3, 2 / 3, 0))
})

test_that("reverse KM removes events before tied censoring and uses true left limits", {
  d <- .evaluation_data()
  y <- survival::Surv(d$time, d$event)
  km <- survlearning:::.evaluation_censoring(y)
  # At time 2, 4 are at risk, 1 event leaves, then 1 of the 3 remaining
  # observations is censored: G(2) = 1 * (1 - 1/3), while G(2-) = 1.
  expect_equal(km$table$n_risk, c(5, 4, 2, 1))
  expect_equal(km$table$events, c(1, 1, 1, 0))
  expect_equal(km$table$g_risk, c(4, 3, 1, 1))
  expect_equal(km$table$censored, c(0, 1, 0, 1))
  expect_equal(km$table$g, c(1, 2 / 3, 2 / 3, 0))
  swapped <- survival::survfit(survival::Surv(time, 1 - event) ~ 1, d,
    timefix = FALSE)$surv
  expect_false(isTRUE(all.equal(km$table$g, swapped)))
  expect_equal(survlearning:::.evaluation_g(km, c(0, 1, 2, 3, 4, 5)),
    c(1, 1, 2 / 3, 2 / 3, 2 / 3, 0))
  expect_equal(survlearning:::.evaluation_g(km, c(0, 1, 2, 3, 4, 5), left = TRUE),
    c(1, 1, 1, 2 / 3, 2 / 3, 2 / 3))
  fit <- .evaluation_mock()
  result <- surv_evaluate(fit, d, "harrell")
  expect_equal(result$estimate,
    unname(survival::concordance(y ~ d$risk, reverse = TRUE)$concordance))
  # Uno excludes the event/censor pair at time 2, unlike Harrell.
  expect_equal(surv_evaluate(fit, d, "uno", tau = 2)$estimate, .75)
})

test_that("independent tied population recovers the complete-data Brier score", {
  population <- expand.grid(T = 1:3, C = c(1, 2, 4))
  d <- data.frame(time = pmin(population$T, population$C),
    event = as.integer(population$T <= population$C), risk = 0, x = seq_len(9))
  fit <- .evaluation_mock(reference = d, probabilities = matrix(1 / 3, 9, 1),
    probability_times = 2)
  result <- surv_evaluate(fit, d, "brier", times = 2)
  table <- attr(result, "censoring")$table
  # The cross-product population has independent, uniform T and C.
  # G(1)=2/3 and G(2)=1/3; event observation at T=2 instead uses G(2-)=2/3.
  expect_equal(table$time, 1:3)
  expect_equal(table$n_risk, c(9, 4, 1))
  expect_equal(table$events, c(3, 2, 1))
  expect_equal(table$censored, c(2, 1, 0))
  expect_equal(table$g_risk, c(6, 2, 0))
  expect_equal(table$g, c(2 / 3, 1 / 3, 1 / 3))
  g <- attr(result, "censoring")
  expect_equal(survlearning:::.evaluation_g(g, 1:3, left = TRUE), c(1, 2 / 3, 1 / 3))
  # Three events at 1, two at 2, and one control beyond 2 contribute:
  hand <- (3 * (1 / 3)^2 + 2 * (1 / 3)^2 / (2 / 3) +
    (2 / 3)^2 / (1 / 3)) / 9
  complete <- mean(((population$T > 2) - 1 / 3)^2)
  expect_equal(hand, 2 / 9)
  expect_equal(result$estimate, hand)
  expect_equal(result$estimate, complete)
})

test_that("reverse KM agrees with ordinary swapped KM only without cross-type ties", {
  d <- data.frame(time = 1:6, event = c(1, 0, 1, 0, 1, 1))
  y <- survival::Surv(d$time, d$event)
  km <- survlearning:::.evaluation_censoring(y)
  direct <- survival::survfit(survival::Surv(time, 1 - event) ~ 1, d,
    timefix = FALSE)
  expect_equal(km$table$g, direct$surv)
  # A final all-event risk set has zero censoring risk but no censoring jump.
  expect_equal(utils::tail(km$table$g_risk, 1), 0)
  expect_true(all(is.finite(km$table$g)))
  y <- survival::Surv(c(1, 1, 2, 2), c(1, 0, 1, 0))
  km <- survlearning:::.evaluation_censoring(y)
  expect_equal(km$table$g_risk, c(3, 1))
  expect_equal(km$table$g, c(2 / 3, 0))
  expect_equal(survlearning:::.evaluation_g(km, 2, left = TRUE), 2 / 3)
})

test_that("rank-tree Uno agrees with a strict pair oracle and survival on continuous data", {
  survlearning:::.with_seed(42L, {
    for (n in c(2L, 10L, 35L)) {
      for (trial in seq_len(10)) {
        time <- sample.int(6, n, replace = TRUE)
        risk <- sample.int(4, n, replace = TRUE)
        weights <- sample(c(0, 1, 2, 3), n, replace = TRUE)
        numerator <- denominator <- 0
        for (i in seq_len(n)) {
          for (j in seq_len(n)) {
            if (time[i] < time[j]) {
              denominator <- denominator + weights[i]
              numerator <- numerator + weights[i] *
                ((risk[i] > risk[j]) + .5 * (risk[i] == risk[j]))
            }
          }
        }
        expected <- if (denominator > 0) numerator / denominator else NA_real_
        expect_equal(survlearning:::.evaluation_uno(time, risk, weights), expected)
      }
    }
    time <- seq_len(40)
    event <- sample(c(0, 1), 40, replace = TRUE)
    risk <- stats::rnorm(40)
    y <- survival::Surv(time, event)
    km <- survlearning:::.evaluation_censoring(y)
    weights <- event * as.numeric(time <= 30) /
      survlearning:::.evaluation_g(km, time, left = TRUE)^2
    expect_equal(survlearning:::.evaluation_uno(time, risk, weights),
      unname(survival::concordance(y ~ risk, reverse = TRUE, timewt = "n/G2",
        ymax = 30, timefix = FALSE)$concordance), tolerance = 1e-12)
  })
})

test_that("AUC sorting agrees with weighted pair oracle including risk ties", {
  cases <- c(4, 2, 2, 0)
  controls <- c(1, 2, 2, 5, 0)
  weights <- c(1, 2, 3, 4)
  oracle <- sum(vapply(seq_along(cases), function(i) {
    weights[i] * sum((cases[i] > controls) + .5 * (cases[i] == controls))
  }, numeric(1))) / (sum(weights) * length(controls))
  expect_equal(survlearning:::.evaluation_auc(cases, controls, weights), oracle)
  expect_equal(survlearning:::.evaluation_auc(rep(2, 3), rep(2, 4), c(1, 2, 3)), .5)
  expect_true(is.na(survlearning:::.evaluation_auc(numeric(), controls, numeric())))
  expect_true(is.na(survlearning:::.evaluation_auc(cases, numeric(), weights)))
})

test_that("uncensored Brier is ordinary mean squared error and IBS uses grid interval", {
  d <- .evaluation_data()
  d$event <- 1
  fit <- .evaluation_mock(reference = d)
  result <- surv_evaluate(fit, d, c("brier", "ibs"), times = c(1, 2, 4))
  p <- surv_predict(fit, d, "survival", c(1, 2, 4))
  expected <- colMeans((outer(d$time, c(1, 2, 4), ">") - p)^2)
  expect_equal(result$estimate[result$metric == "brier"], unname(expected))
  integral <- (mean(expected[1:2]) + 2 * mean(expected[2:3])) / 3
  expect_equal(result$estimate[result$metric == "ibs"], unname(integral))
})

test_that("reference outcomes use training by default and explicit data need no predictors", {
  d <- .evaluation_data()
  fit <- .evaluation_mock()
  result <- surv_evaluate(fit, d, "brier", times = 2)
  uncensored <- data.frame(time = d$time, event = rep(1, 5))
  other <- surv_evaluate(fit, d, "brier", times = 2, censoring_data = uncensored)
  expect_equal(result$estimate, 127 / 1000)
  expect_equal(other$estimate, (.04 + .16 + .04 + .25) / 5)
  expect_identical(attr(result, "censoring")$source, "training_outcome")
  expect_identical(attr(other, "censoring")$source, "censoring_data")
  # Response evaluation must not evaluate RHS transformations in the reference.
  fit$formula <- survival::Surv(time, event) ~ log(x) + missing_predictor
  expect_equal(surv_evaluate(fit, d, "brier", times = 2,
    censoring_data = uncensored)$estimate, other$estimate)
  old <- fit
  old$training_outcome <- NULL
  expect_error(surv_evaluate(old, d, "brier", times = 2), "training_outcome")
  expect_equal(surv_evaluate(old, d, "brier", times = 2,
    censoring_data = uncensored)$estimate, other$estimate)
  expect_true(is.finite(surv_evaluate(old, d, "harrell")$estimate))
  tuned <- structure(list(best_model = fit), class = "survlearning_tune")
  expect_equal(surv_evaluate(tuned, d)$estimate, surv_evaluate(fit, d)$estimate)
})

test_that("AUC uses native risk even for probability-unsupported methods", {
  d <- .evaluation_data()
  fit <- .evaluation_mock(native = FALSE)
  expect_equal(surv_evaluate(fit, d, "auc", times = 2)$estimate, 5 / 8)
  expect_true(is.finite(surv_evaluate(fit, d, "uno", tau = 4)$estimate))
  expect_error(surv_evaluate(fit, d, "brier", times = 2), "native survival")
  expect_error(surv_evaluate(fit, d, "ibs", times = c(1, 2)), "native survival")
  expect_error(surv_calibrate(fit, d, times = 2), "native survival")
})

test_that("undefined discrimination does not prevent no-event Brier evaluation", {
  d <- .evaluation_data()
  fit <- .evaluation_mock()
  d$event <- 0
  result <- surv_evaluate(fit, d, c("harrell", "uno", "auc", "brier"),
    times = 2, tau = 4)
  expect_true(all(is.na(result$estimate[1:3])))
  expect_true(all(result$status[1:3] == "undefined"))
  expect_true(all(nchar(result$message[1:3]) > 0))
  expect_equal(result$estimate[4], (.04 + .25) / (2 / 3) / 5)
  expect_equal(result$events, rep(0, 4))
  # At the last observed time all subjects can have been censored.
  all_censored <- data.frame(time = rep(2, 5), event = 0, risk = d$risk, x = d$x)
  empty <- surv_evaluate(fit, all_censored, "brier", times = 2)
  expect_identical(empty$status, "undefined")
  expect_true(is.na(empty$estimate))
  d <- .evaluation_data()
  d$time <- 2
  d$event <- 1
  result <- surv_evaluate(fit, d, c("harrell", "uno", "auc", "brier"),
    times = 2, tau = 2)
  expect_true(all(is.na(result$estimate[1:3])))
  expect_true(is.finite(result$estimate[4]))
})

test_that("singleton outcomes and boundary AUC horizons return diagnostics", {
  d <- .evaluation_data()[1, , drop = FALSE]
  fit <- .evaluation_mock(probabilities = matrix(.3, 1, 1), probability_times = 1)
  result <- surv_evaluate(fit, d, c("harrell", "uno", "auc", "brier"),
    times = 1, tau = 1, n_boot = 4)
  expect_true(all(is.na(result$estimate[1:3])))
  expect_true(all(result$status[1:3] == "undefined"))
  expect_equal(result$estimate[4], .09)
  expect_equal(result$lower[4], .09)
  expect_equal(result$upper[4], .09)
  d <- .evaluation_data()
  result <- surv_evaluate(fit, d, "auc", times = c(0, 4))
  expect_identical(result$status, c("undefined", "ok"))
  expect_match(result$message[1], "cases")
})

test_that("large rank sets do not require pair matrices", {
  n <- 12000L
  expect_equal(survlearning:::.evaluation_uno(seq_len(n), -seq_len(n), rep(1, n)), 1)
  expect_equal(survlearning:::.evaluation_auc(seq_len(n), seq_len(n), rep(1, n)), .5)
})

test_that("unsupported horizons and unstable G fail without clipping", {
  d <- .evaluation_data()
  fit <- .evaluation_mock()
  expect_error(surv_evaluate(fit, d, "uno"), "explicit.*tau")
  expect_error(surv_evaluate(fit, d, "uno", tau = Inf), "finite.*tau")
  expect_error(surv_evaluate(fit, d, "uno", tau = 6), "evaluation follow-up")
  expect_error(surv_evaluate(fit, d, "uno", tau = 5), "G support")
  expect_error(surv_evaluate(fit, d, "auc", times = 5), "G support")
  expect_error(surv_evaluate(fit, d, "brier", times = 6), "evaluation follow-up")
  expect_error(surv_evaluate(fit, d, "brier", times = 2, min_g = 2 / 3 + .001), "not clipped")
  expect_equal(surv_evaluate(fit, d, "brier", times = 2, min_g = 2 / 3)$estimate,
    127 / 1000)
  reference <- data.frame(time = c(1, 2, 3), event = c(1, 0, 1))
  expect_error(surv_evaluate(fit, d, "auc", times = 4,
    censoring_data = reference), "censoring reference follow-up")
  expect_error(surv_evaluate(fit, d, "uno", tau = 4,
    censoring_data = reference), "censoring reference follow-up")
  # Left-limit event weighting is not the post-jump G at tied event time 2.
  expect_equal(surv_evaluate(fit, d, "brier", times = 2)$estimate, 127 / 1000)
})

test_that("arguments, right-censoring, and finite aligned predictions are validated", {
  d <- .evaluation_data()
  fit <- .evaluation_mock()
  expect_error(surv_evaluate(list(), d), "surv_fit")
  for (metric in list(character(), c("harrell", "harrell"), "bogus", NA_character_)) {
    expect_error(surv_evaluate(fit, d, metrics = metric), "metrics")
  }
  for (time in list(NULL, -1, Inf, NA_real_, c(1, 1), "2")) {
    expect_error(surv_evaluate(fit, d, "brier", times = time), "times")
  }
  for (grid in list(2, c(2, 1), c(1, 2, 1))) {
    expect_error(surv_evaluate(fit, d, "ibs", times = grid), "grid|times")
  }
  expect_error(surv_evaluate(fit, d, times = 2), "only used")
  expect_error(surv_evaluate(fit, d, tau = 2), "only used")
  expect_error(surv_evaluate(fit, d, n_boot = .5), "n_boot")
  expect_error(surv_evaluate(fit, d, seed = -1), "seed")
  for (value in c(0, 1.1, NA_real_)) {
    expect_error(surv_evaluate(fit, d, min_g = value), "min_g")
    expect_error(surv_evaluate(fit, d, conf_level = value), "conf_level")
  }
  bad <- d
  bad$event <- bad$event + 1
  expect_error(surv_evaluate(fit, bad), "Encode")
  bad <- d
  bad$time[1] <- NA_real_
  expect_error(surv_evaluate(fit, bad), "missing")
  bad <- d
  bad$time[1] <- 0
  expect_error(surv_evaluate(fit, bad), "positive")
  fit$formula <- survival::Surv(time, time + 1, event) ~ x
  expect_error(surv_evaluate(fit, d), "right-censored")
  fit$formula <- survival::Surv(time, event) ~ x
  fit$training_outcome <- survival::Surv(d$time, d$time + 1, d$event)
  expect_error(surv_evaluate(fit, d, "auc", times = 2), "reference.*right-censored")
  d$risk[1] <- Inf
  expect_error(surv_evaluate(fit, d), "Risk predictions")
})

test_that("invalid native probabilities fail instead of being repaired", {
  d <- .evaluation_data()
  fit <- .evaluation_mock(probabilities = matrix(.5, 4, 3))
  expect_error(surv_evaluate(fit, d, "brier", times = 2), "n-by-times")
  fit <- .evaluation_mock(probabilities = matrix(NA_real_, 5, 3))
  expect_error(surv_evaluate(fit, d, "brier", times = 2), "finite")
  fit <- .evaluation_mock(probabilities = matrix(1.1, 5, 3))
  expect_error(surv_evaluate(fit, d, "brier", times = 2), "\\[0, 1\\]")
  fit <- .evaluation_mock(probabilities = matrix(rep(c(.2, .5, .7), each = 5), 5))
  expect_error(surv_evaluate(fit, d, "ibs", times = c(1, 2, 4)), "non-increasing")
})

test_that("bootstrap is reproducible conditional test-case resampling with fixed G", {
  d <- .evaluation_data()
  fit <- .evaluation_mock()
  set.seed(701)
  before <- .Random.seed
  result <- surv_evaluate(fit, d, c("uno", "auc", "brier", "ibs"),
    times = c(1, 2, 4), tau = 4, n_boot = 80, seed = 13)
  expect_identical(.Random.seed, before)
  expect_identical(result, surv_evaluate(fit, d, c("uno", "auc", "brier", "ibs"),
    times = c(1, 2, 4), tau = 4, n_boot = 80, seed = 13))
  expect_true(all(result$lower <= result$upper, na.rm = TRUE))
  valid <- attr(result, "settings")$bootstrap_valid
  expect_true(all(valid <= 80 & valid >= 2))
  expect_true(any(result$status == "partial_bootstrap"))
  expect_true(all(grepl("finite bootstrap", result$message[result$status == "partial_bootstrap"])))
  # Brier bootstrap at 2, calculated with the original (not re-estimated) G.
  contributions <- c(.2^2, .4^2, 0, (1 - .8)^2 / (2 / 3), (1 - .5)^2 / (2 / 3))
  expected <- survlearning:::.with_seed(13L, {
    replicate(80, mean(contributions[sample.int(5, 5, replace = TRUE)]))
  })
  row <- which(result$metric == "brier" & result$time == 2)
  expect_equal(c(result$lower[row], result$upper[row]),
    unname(stats::quantile(expected, c(.025, .975))))
  one <- surv_evaluate(fit, d, "brier", times = 2, n_boot = 1)
  expect_identical(one$status, "insufficient_bootstrap")
  expect_true(is.na(one$lower) && is.na(one$upper))
})

test_that("explicit bootstrap seed preserves the caller's RNG state", {
  withr::local_seed(81L)
  previous <- .Random.seed
  fit <- .evaluation_mock()
  surv_evaluate(fit, .evaluation_data(), "brier", times = 2, n_boot = 3, seed = 4)
  expect_identical(.Random.seed, previous)
})

test_that("grouped KM calibration and intervals agree with survival", {
  d <- data.frame(time = c(1, 2, 3, 4, 5, 6), event = c(1, 0, 1, 1, 0, 1),
    risk = 6:1, x = seq_len(6))
  p <- matrix(c(.1, .2, .3, .7, .8, .9), ncol = 1)
  fit <- .evaluation_mock(reference = d, probabilities = p, probability_times = 3)
  expect_warning(result <- surv_calibrate(fit, d, times = 3, groups = 2), "Low support")
  expect_identical(class(result), c("survlearning_calibration", "data.frame"))
  expect_identical(rownames(result), c("1", "2"))
  expect_equal(result$predicted, c(.2, .8))
  expect_equal(result$observed, c(0, 1))
  expect_equal(result$n, c(3, 3))
  expect_equal(result$events, c(2, 0))
  expect_equal(result$n_risk, c(1, 3))
  for (g in seq_len(2)) {
    index <- if (g == 1) 1:3 else 4:6
    direct <- summary(survival::survfit(survival::Surv(time, event) ~ 1,
      data = d[index, ], conf.type = "log-log", conf.int = .95), times = 3)
    expect_equal(result$observed[g], unname(direct$surv))
    expect_equal(result$lower[g], unname(direct$lower))
    expect_equal(result$upper[g], unname(direct$upper))
  }
  fit <- .evaluation_mock(reference = d, probabilities = p, probability_times = 2)
  result <- suppressWarnings(surv_calibrate(fit, d, times = 2, groups = 1))
  direct <- summary(survival::survfit(survival::Surv(time, event) ~ 1,
    d, conf.type = "log-log"), times = 2)
  expect_equal(result$observed, 5 / 6)
  expect_equal(result$lower, unname(direct$lower))
  expect_equal(result$upper, unname(direct$upper))
})

test_that("calibration keeps probability ties together and does not extrapolate KM tails", {
  d <- .evaluation_data()
  fit <- .evaluation_mock(probabilities = matrix(.5, 5, 1), probability_times = 2)
  expect_warning(result <- surv_calibrate(fit, d, times = 2, groups = 5), "collapsed")
  expect_equal(nrow(result), 1)
  expect_equal(result$n, 5)
  expect_match(result$message, "tied predictions")
  fit <- .evaluation_mock(probabilities = matrix(c(.1, .1, .9, .9, .9), 5, 1),
    probability_times = 4)
  result <- suppressWarnings(surv_calibrate(fit, d, times = 4, groups = 2))
  expect_identical(result$status[1], "unsupported")
  expect_true(is.na(result$observed[1]) && is.na(result$lower[1]) && is.na(result$upper[1]))
  expect_true(is.finite(result$observed[2]))
  expect_equal(sum(result$n), nrow(d))
  expect_error(surv_calibrate(fit, d, times = 6), "evaluation follow-up")
  expect_error(surv_calibrate(fit, d, times = 4, groups = 0), "groups")
})

test_that("calibration plot supports multiple horizons and preserves its input", {
  fit <- .evaluation_mock()
  result <- suppressWarnings(surv_calibrate(fit, .evaluation_data(), c(1, 2), groups = 2))
  path <- tempfile("evaluation-calibration-", fileext = ".pdf")
  grDevices::pdf(path)
  on.exit({ grDevices::dev.off(); unlink(path) }, add = TRUE)
  expect_identical(plot(result), result)
  expect_identical(plot(result, time = 2), result)
  expect_error(plot(result, time = 3), "present")
  expect_error(plot(result, time = c(1, 2)), "present")
})

test_that("actual fitted and tuned objects integrate without changing surv_score", {
  d <- example_data()
  fit <- surv_fit(survival::Surv(time, event) ~ x + z, d, "cox")
  result <- surv_evaluate(fit, d, c("harrell", "uno", "auc", "brier", "ibs"),
    times = c(1, 2, 3), tau = 3)
  expect_equal(result$estimate[result$metric == "harrell"], surv_score(fit, d))
  expect_true(all(is.finite(result$estimate[result$metric %in% c("brier", "ibs")])))
  expect_equal(attr(result, "censoring")$n, nrow(d))
  tuned <- structure(list(best_model = fit), class = "survlearning_tune")
  expect_equal(surv_evaluate(tuned, d, "brier", times = 2),
    surv_evaluate(fit, d, "brier", times = 2))
  # Neither Harrell nor the rank-based AUC needs the native probability engine.
  legacy <- fit
  legacy$training_outcome <- NULL
  expect_equal(surv_evaluate(legacy, d)$estimate, surv_score(fit, d))
})
