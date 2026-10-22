# Elastic net, XGBoost and MTLR. Run sequentially after installing engines.
# Modeling engines are installed with survlearning.
library(survlearning)
set.seed(2026)
n <- 300
d <- data.frame(temperature = rnorm(n, 35, 5), power_hours = runif(n, 100, 20000))
failure <- rweibull(n, shape = 1.5,
  scale = exp(5.5 - .04 * (d$temperature - 35) - .00003 * d$power_hours))
censor <- runif(n, 30, 400)
d$time <- pmin(failure, censor)
d$event <- as.integer(failure <= censor)
id <- integer()
for (status in c(0, 1)) {
  rows <- which(d$event == status)
  id <- c(id, sample(rows, floor(.75 * length(rows))))
}
train <- d[id, ]
test <- d[-id, ]
f <- Surv(time, event) ~ temperature + power_hours

# 1. Elastic net: alpha = 1 is LASSO; alpha = 0 is ridge.
cox_en <- surv_tune(f, train, "cox_elasticnet",
  grid = list(alpha = c(.25, .75), lambda = c(.01, .1)), folds = 3)
# Penalized AFT uses the semiparametric Gehan loss, not a Weibull likelihood.
aft_en <- surv_fit(f, train, "aft_elasticnet", list(alpha = .5, lambda = .01))

# 2. Cox trees retain proportional hazards; AFT uses censoring bounds.
xgb_cox <- surv_tune(f, train, "xgboost_cox",
  grid = list(depth = c(1, 2), trees = c(50, 100)), folds = 3)
xgb_aft <- surv_tune(f, train, "xgboost_aft",
  grid = list(scale = c(.5, 1), trees = c(50, 100)),
  params = list(distribution = "normal", depth = 2), folds = 3)

# 3. MTLR estimates a curve on a training-defined time grid.
mtlr <- surv_tune(f, train, "mtlr",
  grid = list(intervals = c(5, 10), lambda = c(.1, 1)), folds = 3)
models <- list(cox_en = cox_en, aft_en = aft_en, xgb_cox = xgb_cox,
  xgb_aft = xgb_aft, mtlr = mtlr)
results <- data.frame(method = names(models), cindex = NA_real_)
for (i in seq_along(models)) results$cindex[i] <- surv_score(models[[i]], test)
results

new_unit <- data.frame(temperature = 40, power_hours = 15000)
surv_predict(xgb_aft, new_unit, "time")
1 - surv_predict(mtlr, new_unit, "survival", times = c(30, 90))
# C-index concerns ordering, not calibration. This small generated example is
# not a repeated performance benchmark. No full mean lifetime is invented.
