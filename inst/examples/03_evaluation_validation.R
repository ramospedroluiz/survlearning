# Evaluation and validation in one sequential English example.
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
km <- surv_fit(f, train, "km")
aft <- surv_fit(f, train, "aft")
surv_score(aft, test)
evaluation <- surv_evaluate(aft, test,
  metrics = c("harrell", "uno", "auc", "brier", "ibs"),
  times = c(30, 60, 90), tau = 90)
evaluation
plot(evaluation, metric = "brier")
surv_compare(list(km = km, aft = aft), test,
  metrics = c("harrell", "brier"), times = c(30, 60, 90))

# Calibration without a supplied censoring fit uses grouped KM.
calibration <- surv_calibrate(aft, test, times = 60, groups = 3)
plot(calibration)
# An explicit G uses normalized IPCW and a fixed-model/group bootstrap.
g <- surv_censor_fit(f, train, method = "cox")
surv_calibrate(aft, test, times = 60, groups = 3,
  censoring_model = g, n_boot = 100, seed = 10)

# Shared outer partitions; all preprocessing and inner selection stay local.
splits <- surv_resamples(f, train, folds = 3, seed = 20)
validation <- surv_validate(f, train, methods = c("km", "cox", "aft"),
  grids = list(aft = list(distribution = c("weibull", "lognormal"))),
  resamples = splits, inner_folds = 3, metric = "harrell",
  metrics = c("brier", "harrell"), times = c(30, 60))
validation$summary
# Reporting order does not change the selection objective.

train_missing <- train
train_missing$temperature[1:5] <- NA_real_
imputed <- surv_tune(f, train_missing, "aft",
  grid = list(distribution = c("weibull", "lognormal")),
  folds = 3, impute = "median")
imputed$best_model$design$imputation
# The median/mode is learned separately in each analysis fold.
