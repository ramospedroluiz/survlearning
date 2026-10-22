# Native neural Cox needs no Torch, Python, or additional neural engine.
library(survlearning)
d <- survival::veteran
set.seed(2026)
id <- sample(seq_len(nrow(d)), floor(.8 * nrow(d)))
train <- d[id, ]
test <- d[-id, ]
f <- Surv(time, status) ~ age + karno + trt

elapsed <- system.time(fit <- surv_fit(f, train, "neural_cox",
  list(hidden = 16, epochs = 100, ties = "efron"), seed = 123))
elapsed
summary(fit)
surv_score(fit, test)
surv_predict(fit, test[1, ])
surv_predict(fit, test[1, ], "survival", times = c(30, 60, 90))
surv_diagnostics(fit)
# Alternative: ties = "breslow". Times are never jittered.
# Epochs specify the optimization budget, not a convergence certificate.
