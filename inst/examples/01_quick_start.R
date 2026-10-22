# Sequential quick start. Install survlearning and its dependencies first.
# See the README for repository and local-archive installation commands.
# One row per unit. event = 1 is a failure; event = 0 is right censoring.
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

# 1. Same fitting interface; the external test is not used for tuning.
cox <- surv_fit(f, train, "cox")
aft <- surv_fit(f, train, "aft", params = list(distribution = "weibull"))
summary(cox)
coef(aft)
surv_score(cox, test)
surv_score(aft, test)

# 2. Cross-validation selects between explicitly supplied distributions.
tuned <- surv_tune(f, train, "aft",
  grid = list(distribution = c("weibull", "lognormal")), folds = 3, seed = 2026)
summary(tuned)
surv_score(tuned, test)

# 3. A new unit needs predictors only.
new_unit <- data.frame(temperature = 40, power_hours = 15000)
surv_predict(tuned, new_unit, "risk")
surv_predict(tuned, new_unit, "time") # Conditional AFT median, not an exact failure date.
1 - surv_predict(tuned, new_unit, "survival", times = c(30, 90))

# 4. Forest and SVM engines are available after package installation.
forest <- surv_fit(f, train, "rsf", list(trees = 100, min_node = 10))
surv_score(forest, test)
svm <- surv_fit(f, train, "svm") # Default compact linear Clarabel.
surv_predict(svm, new_unit)      # Risk only, not a probability.
