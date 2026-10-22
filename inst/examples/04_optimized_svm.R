# Small executable example. The article's replication retains every eligible
# Backblaze training row for full-cohort analysis and process profiling.
library(survlearning)
d <- survival::veteran
f <- Surv(time, status) ~ age + karno
train <- d[1:100, ]
test <- d[101:nrow(d), ]
fit <- surv_tune(f, train, "svm", grid = list(cost = c(.01, .1)),
  folds = 3, seed = 20261023)
summary(fit)
surv_diagnostics(fit)
surv_score(fit, test)
surv_predict(fit, test[1, ])
# Risk scores are not calibrated days of life or failure probabilities.
