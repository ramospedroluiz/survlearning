# A sequential example using only survlearning and its mandatory dependencies.
# Install the supplied source archive first, then run this file from top to bottom.
library(survlearning)
library(survival)

# 1. Generate a small example with conditionally independent censoring.
set.seed(20261026)
d <- data.frame(load = runif(600, -1, 1), design = rbinom(600, 1, .5))
failure_time <- rexp(600, rate = .02 * exp(.8 * d$load - .3 * d$design))
censor_time <- pmin(80, rexp(600, rate = .02 * exp(.6 * d$load)))
d$time <- pmin(failure_time, censor_time)
d$event <- as.integer(failure_time <= censor_time)
train <- d[1:400, ]
test <- d[401:600, ]
f <- Surv(time, event) ~ load + design

# 2. Fit a model using the engines installed with survlearning.
fit <- surv_fit(f, train, method = "cox")
surv_score(fit, test)

# 3. Explain predictions on a held-out assessment sample, without refitting.
importance <- surv_importance(fit, test, n_repeats = 5, seed = 10)
importance
plot(importance)
effects <- surv_ale(fit, test, variable = "load", bins = 8,
  type = "failure", time = 10)
effects
plot(effects)
# Importance SD is permutation variation, not a confidence interval.
# Neither importance nor ALE estimates a causal effect.

# 4. Predict for two new profiles, conditional on survival to model time 5.
profiles <- data.frame(load = c(-.5, .5), design = c(0, 1))
remaining <- surv_reliability(fit, profiles, horizons = c(10, 20),
  age = 5, probs = c(.1, .5), grid_points = 501)
remaining$summary
remaining$quantiles
plot(remaining, type = "failure")
# Restricted remaining means stop at the requested horizon.
# An unreached quantile is NA, not infinite life or a cured component.

# 5. Learn the censoring distribution from training only.
g <- surv_censor_fit(f, train, method = "cox")
surv_evaluate(fit, test, metrics = c("harrell", "ipcw_c", "auc", "brier"),
  times = 10, tau = 15, censoring_model = g)
# Conditional weights require a suitable censoring model and positive support.
# Historical metric = "uno" remains marginal; use "ipcw_c" here.

# 6. Two local workers, one owned checkpoint, and the same fold allocation.
# Windows workers need this version of survlearning installed in .libPaths().
checkpoint <- tempfile("survlearning-example-")
tuned <- surv_tune(f, train, method = "aft",
  grid = list(distribution = c("weibull", "lognormal")), folds = 3,
  seed = 20, workers = 2, checkpoint = checkpoint, progress = TRUE,
  metric = "brier", times = 10, censoring = f)
tuned$results
tuned$execution
surv_evaluate(tuned, test, metrics = "brier", times = 10, censoring_model = g)

# 7. An identical call reuses completed folds and the winning refit.
resumed <- surv_tune(f, train, method = "aft",
  grid = list(distribution = c("weibull", "lognormal")), folds = 3,
  seed = 20, workers = 2, checkpoint = checkpoint,
  metric = "brier", times = 10, censoring = f)
resumed$execution
stopifnot(resumed$execution$computed_jobs == 0,
  resumed$execution$refit_reused)
# Changed data/settings require a new checkpoint directory.
# More workers consume more memory; speedup is not guaranteed.
