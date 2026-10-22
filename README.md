# survlearning

An R interface for fitting, tuning, predicting, and evaluating statistical
learning models for right-censored survival and reliability data.

The same formula and workflow cover reference models, penalized regression,
survival forests, boosting, censored support vector regression, and neural
survival models. Predictions distinguish risk scores from survival probabilities
and median times. Preprocessing is learned within each training sample.

## Installation

```r
install.packages("remotes")
remotes::install_github("ramospedroluiz/survlearning", dependencies = NA)
```

R 4.3.0 or later is required. This command installs all modeling engines and
numerical solvers as required dependencies. After successful installation, all
supported methods are available without additional package-installation steps.
The initial download includes these dependencies and their own requirements;
source installations may also require a compiler toolchain. Fitting functions
do not install software or download data. Neural Cox fitting needs neither
Torch nor Python. This version is available from GitHub, not CRAN.

For a downloaded source archive, use an installer that resolves dependencies:

```r
remotes::install_local("survlearning_0.2.6.tar.gz", dependencies = NA)
```

Base R's `install.packages(..., repos = NULL)` installs a local archive but does
not download its missing dependencies. Development tools and the `survivalsvm`
reference implementation are suggested packages, not required for analysis.

## Fit and predict

```r
library(survlearning)

d <- survival::veteran
set.seed(2026)
index <- sample(seq_len(nrow(d)), 100)
train <- d[index, ]
test <- d[-index, ]
f <- Surv(time, status) ~ age + karno

fit <- surv_fit(f, train, method = "aft")
summary(fit)
surv_score(fit, test)  # Harrell's concordance index

new_items <- data.frame(age = c(55, 65), karno = c(80, 60))
surv_predict(fit, new_items, type = "risk")
surv_predict(fit, new_items, type = "time")  # conditional median
surv_predict(fit, new_items, type = "survival", times = c(30, 60, 90))
```

Larger risk scores always indicate earlier failure, but their scales remain
method-specific. A risk score is not a failure probability. Use `surv_methods()`
to inspect available engines and prediction types, and
`surv_parameters("aft")` to inspect a method's named parameters.

## Tune and validate

```r
splits <- surv_resamples(f, train, folds = 3, seed = 10)
tuned <- surv_tune(f, train, method = "aft",
  grid = list(distribution = c("weibull", "lognormal")), resamples = splits)
tuned$results
surv_evaluate(tuned, test)

validation <- surv_validate(f, train, methods = c("km", "cox", "aft"),
  grids = list(aft = list(distribution = c("weibull", "lognormal"))),
  resamples = splits, inner_folds = 3, metric = "harrell")
validation$summary
```

The external test sample is not used for parameter selection. A `NULL` grid
evaluates one fixed configuration. Stratified, grouped, and forward-temporal
resampling plans are supported; nested tuning currently requires stratified
outer plans. For grouped or temporal validation use prespecified settings with
`tune = FALSE`. Temporal partitions still require an appropriate definition of
when predictors and outcomes become available.

For parallel tuning, supply `workers = 2`. Add `checkpoint = "my-search"` and
an explicit `seed` to resume an identical search. A checkpoint belongs to one
live search; changed data, settings, or software versions invalidate it.
Workers duplicate data and may increase memory requirements.

## Evaluate predictions

```r
evaluation <- surv_evaluate(fit, test,
  metrics = c("harrell", "uno", "auc", "brier", "ibs"),
  times = c(30, 60, 90), tau = 90)
evaluation
plot(evaluation, metric = "brier")
surv_calibrate(fit, test, times = 60, groups = 4)

censoring <- surv_censor_fit(f, train, method = "cox")
surv_evaluate(fit, test, metrics = c("ipcw_c", "auc", "brier"),
  times = 60, tau = 60, censoring_model = censoring)
```

Marginal censoring weights use the training outcomes by default. Conditional
Cox or stratified-KM censoring models are also available. These evaluations
require suitable independent-censoring assumptions, transportability of the
censoring distribution, and positive support. Unsupported horizons and unstable
weights are diagnosed. `uno` uses marginal weights; `ipcw_c` supports conditional
ordered-pair concordance. Probability metrics require a method with survival
predictions. Bootstrap intervals condition on the fitted models and do not
include refitting or tuning uncertainty.

## Interpretation and reliability

```r
surv_importance(fit, test, n_repeats = 5)
surv_ale(fit, test, variable = "age", bins = 8, type = "failure", time = 60)

remaining <- surv_reliability(fit, new_items, age = 10,
  horizons = c(30, 60), probs = c(.1, .5))
remaining$summary
remaining$quantiles
```

Importance and accumulated local effects describe predictive associations,
not causal effects. Reliability summaries condition on survival to the supplied
model time, holding covariates fixed. Restricted means stop at the requested
horizon; quantiles not reached on the prediction grid are returned as `NA`.

## Methods

| Method | Engine | Predictions |
| --- | --- | --- |
| `km` | survival | constant risk, survival |
| `aft` | survival | risk, median, survival |
| `cox`, `cox_spline` | survival | risk, survival |
| `aft_lasso`, `aft_ridge`, `aft_elasticnet` | penAFT | risk |
| `cox_lasso`, `cox_ridge`, `cox_elasticnet` | glmnet | risk, survival |
| `tree` | rpart | risk |
| `rsf` | randomForestSRC | risk, survival |
| `ranger` | ranger | risk, survival |
| `aorsf` | aorsf | risk, survival |
| `boosting` | gbm | risk |
| `coxboost` | CoxBoost | risk, survival |
| `xgboost_cox` | xgboost | risk, survival |
| `xgboost_aft` | xgboost | risk, median, survival |
| `mtlr` | MTLR | risk, survival |
| `svm` | native R; Clarabel, OSQP, quadprog, or Newton | risk |
| `flexsurv` | flexsurv | risk, survival |
| `neural_cox` | native R | risk, survival |
| `neural_discrete` | nnet | risk, survival |

The 23 identifiers include alternative penalties and engines, not 23 unrelated
statistical families. Penalized AFT uses Gehan loss. The `rpart` tree uses
exponential deviance. Neural Cox retains proportional hazards and supports
Efron and Breslow ties. Censored SVM uses linear or additive kernels and keeps
the complete training sample. Its default compact solver is `clarabel`;
linear `solver = "newton"` uses base R optimization, while compact additive
fitting uses `RSpectra`. All supported solver packages are installed together
with `survlearning`.

## Documentation

- [Sequential examples](inst/examples) illustrate fitting, evaluation, SVM,
  interpretation, and reliability predictions.
- [Vignettes](vignettes) explain the statistical conventions and workflows.
- [Article and replication material](paper) provide theory and a reliability
  application to hard-drive failure data.
- [Contribution guide](CONTRIBUTING.md) describes the adapter interface and tests.

The response is a single right-censored outcome with positive observed times.
Competing risks, left truncation, interval censoring, recurrent events,
time-varying covariates, and cure models are outside the current interface.
Nonparametric and semiparametric survival predictions are unavailable beyond
training follow-up; parametric extrapolation requires checking model assumptions.
Use `surv_diagnostics()` to inspect warnings and reported convergence status.

Maintainer: Pedro Luiz Ramos, Pontificia Universidad Catolica de Chile.
Original package code is licensed under GPL (>= 3). External engines retain
their authorship and licenses. Use `citation("survlearning")` for citation details.
