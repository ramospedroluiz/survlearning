library(survlearning)

data <- survival::veteran
formula <- Surv(time, status) ~ age + karno
set.seed(2026)
train_rows <- sample(seq_len(nrow(data)), 100)
train <- data[train_rows, ]
test <- data[-train_rows, ]

# Linear and nonlinear additive ranking, selected within training only.
ranking <- surv_tune(formula, train, method = "svm_rank",
  grid = list(kernel = c("linear", "additive"), lambda = c(0.001, 0.01, 0.1)),
  folds = 3, seed = 2026)
ranking$results
ranking$best_params
surv_score(ranking, test)
surv_predict(ranking, test[1:3, ], type = "risk")

# Explicitly different objectives, not replacements for the original svm.
hybrid <- surv_fit(formula, train, method = "svm_hybrid",
  params = list(lambda = 0.01, rank_weight = 0.5))
ipcw <- surv_fit(formula, train, method = "svm_ipcw",
  params = list(lambda = 0.01, horizon = 60, censoring_method = "km"))
surv_score(hybrid, test)
surv_score(ipcw, test)
ipcw$engine$diagnostics$ipcw

# These methods return risk scores, not a survival curve or a lifetime median.
surv_parameters("svm_rank")
surv_diagnostics(ranking$best_model)
