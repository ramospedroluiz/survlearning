# Standalone additive-kernel example; no project-specific CSV is required.
# Full-cohort resource measurements are in the separate article replication.
library(survlearning)
d <- survival::veteran
f <- Surv(time, status) ~ age + karno
train <- d[1:100, ]
test <- d[101:nrow(d), ]
fit <- surv_fit(f, train, "svm", list(kernel = "additive",
  solver = "clarabel", cost = .1), seed = 20261023)
surv_diagnostics(fit)
surv_score(fit, test)
surv_predict(fit, test[1, ])
stopifnot(fit$n == nrow(train), length(fit$engine$dual) == 2L * nrow(train))
# Exact duplicates retain multiplicities; this is not unweighted subsampling.
# For an independent solver comparison use solver = "clarabel".
