#' Fit a Survival Learning Model
#'
#' All methods use the same formula and training-only predictor encoding.
#' Missing values cause an error by default. Optional median/mode imputation
#' is learned on training predictors only; outcomes are never imputed.
#' `svm` retains censored absolute-loss regression. Separate `svm_rank`,
#' `svm_hybrid`, and `svm_ipcw` methods have explicitly different objectives.
#' None uses automatic subsampling.
#'
#' @param formula A formula such as `Surv(time, event) ~ temperature + age`.
#'   Events must be coded 1 and right-censoring 0; times must be positive.
#' @param data A data frame of training observations.
#' @param method One method listed by [surv_methods()].
#' @param params A named list of method-specific parameters. See Details.
#' @param seed NULL (default) uses the caller's RNG normally. An integer >= 0
#'   supplies a local reproducible seed through withr, restoring the caller's
#'   RNG state even on error. Forest and XGBoost fits with NULL draw and store an engine
#'   seed so that later prediction does not reseed or advance the caller's RNG.
#' @param impute `"none"` (default) or `"median"`. The latter replaces missing
#'   numeric predictors by training medians and categorical predictors by
#'   training modes, including when predicting or fitting validation folds.
#' @details
#' Parameters are: `aft`: `distribution` (default `"weibull"`, also
#' `"lognormal"` or `"loglogistic"`); `aft_lasso`, `aft_ridge`: `lambda`
#' (0.1), `max_iter` (10000), `tolerance` (0.00025); `cox`: none;
#' `cox_lasso`, `cox_ridge`: `lambda` (0.01); `tree`: `cp` (0.01),
#' `min_node` (10), `max_depth` (5); `rsf`: `trees` (200), `mtry`
#' (square root of the encoded predictor count), `min_node` (15), `splits`
#' (5), `threads` (1); `boosting`: `trees` (300), `depth` (2),
#' `learning_rate` (0.03), `min_node` (10), `bag_fraction` (0.7);
#' `svm`: `cost` (0.1), `kernel` (`"linear"` or `"additive"`),
#' `solver` (`"clarabel"`, default, or `"osqp"`), `tolerance` (1e-12),
#' `max_iter` (100000), `memory_limit_mb` (2048 MiB workspace planning limit,
#' not measured peak memory), and `prediction_tolerance` (1e-8).
#' The exact censored absolute-loss primal uses all training rows. The additive
#' representation retains every distinct training coordinate and fixes ranges
#' in training. No nearPD projection, spectral truncation or subsampling is used.
#' Solver success, primal/dual feasibility and an independent objective gap
#' are checked before returning a model. Legacy Newton/quadprog fits targeted
#' a different conditioned quadratic program and must be refitted.
#' Predictions are placed on a training-fixed numerical grid of width
#' prediction_tolerance times the maximum training follow-up, preventing
#' solver-scale near ties from producing artificial risk ordering. Set this
#' parameter to zero to inspect raw scores; it does not change fitting.
#' `collapse_duplicates` is accepted for compatibility and has no effect:
#' all observations have individual loss constraints. Cost must be tuned
#' separately for each kernel, without using test outcomes. SVM scores are
#' not calibrated times or probabilities. See the SVM optimization vignette.
#'
#' `svm_rank`, `svm_hybrid`, and `svm_ipcw` use native R/C++ with no additional
#' dependencies: `lambda` (0.01), `kernel` (`"linear"` or `"additive"`),
#' `tolerance` (1e-6), `max_iter` (2000), `memory_limit_mb` (2048), and
#' `prediction_tolerance` (1e-8 in dimensionless score units; zero disables it).
#' These lambdas multiply half the squared RKHS norm; losses are averaged,
#' so their scale is not the original svm cost. Ranking uses every pair with
#' an observed earlier failure and strictly larger later follow-up, excluding
#' tied times. A native Fenwick calculation evaluates the exact squared-hinge
#' objective/gradient without storing all pairs. The pair evaluator
#' centers scores internally to avoid cancellation of a common
#' offset; this does not change the fitted objective or prediction orientation.
#' Harrell evaluation also admits event/censoring ties in recorded time, unlike
#' the strict ranking training pairs. The additive representation
#' retains every distinct training coordinate. L-BFGS-B fits are checked for
#' stationarity; ranking also reports a strong-convexity objective-gap bound.
#' A line-search termination is accepted only when these same accuracy checks
#' pass. It is labeled `stationarity_verified`, with the original optimizer
#' code and message retained; iteration-limit and other failures are rejected.
#' Stationarity means the Euclidean gradient norm is at most tolerance times
#' max(1, parameter norm). Ranking also requires its gap bound to be at most
#' tolerance times max(1, absolute objective). Optimizer success is required.
#' `svm_hybrid` adds `rank_weight` (0.5, strictly between zero and one), mixing
#' mean pair loss with mean squared one-sided censored-regression loss.
#' Regression times are divided by maximum training follow-up; risk is the
#' negative normalized regression response. This is not the absolute-loss svm.
#'
#' `svm_ipcw` requires an explicit positive `horizon` in the outcome time unit.
#' It fits the restricted outcome min(T,horizon)/horizon with squared
#' epsilon-insensitive loss: `epsilon` (0), `censoring_method` (`"km"`, default,
#' or `"cox"`), and `min_g` (0.05). At epsilon zero it is weighted kernel ridge
#' regression, not sparse epsilon-insensitive absolute-loss SVR. The censoring
#' model is refitted inside each training split using the encoded predictors
#' for Cox. Failures before the horizon and all rows followed to the horizon
#' have observed restricted outcomes; earlier censorings inform G. Weights use
#' G(min(Y,horizon)-). Insufficient fitted support raises an error, never clips
#' weights or drops rows. KM needs marginal censoring assumptions; Cox needs a
#' valid conditional PH model. Both require conditional independent censoring
#' and positivity. A smooth Cox G does not repair deterministic administrative
#' support failure. Empirical support is checked at the horizon for every
#' training row as well as at each observed restricted response; finite fitted
#' probabilities alone do not verify population positivity. At epsilon zero,
#' the unpenalized unrestricted population target is the negative normalized
#' restricted mean, but a penalized fitted score is not an unbiased mean estimate.
#' All three methods return risk only, not calibrated survival
#' probabilities or unrestricted lifetime estimates. See the survival SVM
#' learning vignette for objectives, assumptions and examples.
#'
#' `aft_elasticnet` and `cox_elasticnet` add `alpha` (0.5) to their
#' respective penalized AFT/Cox parameters: 1 is LASSO, 0 is ridge.
#' `xgboost_cox` and `xgboost_aft` use `trees` (200), `depth` (2),
#' `learning_rate` (0.05), `min_child_weight` (1), `leaf_penalty` (1),
#' and `threads` (1). XGBoost AFT also accepts `distribution` (`"normal"`,
#' `"logistic"`, or `"extreme"`) and `scale` (1), referring to the error
#' distribution of log-time. `mtlr` accepts `intervals` (10), `lambda`
#' (1, mapped to C1), `max_iter` (5000), and `tolerance` (1e-5).
#'
#' Factors are encoded consistently with training. Numeric design columns
#' are standardized using training means and standard deviations for SVM
#' penalized AFT, and MTLR. Other engines receive the unscaled design; glmnet
#' performs its own training-only standardization.
#' Penalized Cox follows a descending regularization path ending at the exact
#' requested lambda, rather than fitting a weak-penalty cold start in isolation.
#'
#' Penalized AFT uses the Gehan loss and an integrated ADMM solver adapted from
#' penAFT. It returns a relative risk ordering,
#' not absolute survival times. Cox boosting means gradient-boosted trees with
#' Cox loss (gbm), not the componentwise algorithm in the CoxBoost package.
#' Dependencies are loaded only for the selected engine.
#'
#' Reference parameters: `km` has no parameters and ignores the formula's
#' predictors; `cox_spline` uses `df` (3) for natural splines of continuous
#' design columns; `flexsurv` uses `knots` (1) and `scale` (`"hazard"`,
#' `"odds"` or `"normal"`); `coxboost` uses `steps` (100) and `penalty` (100).
#' `ranger` uses `trees` (200), `mtry` (square root of predictor count),
#' `min_node` (15) and `threads` (1). `aorsf` uses the same forest arguments
#' with `min_node` (5) and `splits` (5), specifying oblique rather than axis
#' aligned splits. See [surv_parameters()] for inspectable defaults.
#'
#' `neural_cox` uses `hidden` (16), `epochs` (100), `learning_rate` (0.001),
#' `decay` (0.0001), `ties` (`"efron"`, also `"breslow"`), and `max_weights`
#' (100000). It uses native R matrix operations and analytic backpropagation,
#' with full-batch Adam and L2 penalty decay/2 times the squared weight norm.
#' No Torch, Python, external runtime or separate neural engine is needed.
#' All observations at an event time enter the risk set before removing deaths
#' or censorings; only event mass enters the Efron correction. The baseline
#' uses the same ties method as training. `threads` must remain 1; this setting
#' does not control the R installation's BLAS threads. Fixed epochs are not a
#' convergence certificate; final objective/gradient diagnostics are reported.
#' `neural_discrete` uses PLANN through nnet with `hidden` (8), `intervals` (10), `decay`
#' (0.01), `max_iter` (1000), and `max_weights` (10000). Its risk is negative
#' restricted mean on the training interval grid. No test-based early stopping
#' is performed and no external runtime is installed automatically.
#' Censoring contributes only completed intervals; events contribute through
#' their failure interval. This explicit discretization loses partial-interval
#' information, rather than inventing survival until the next grid endpoint.
#' XGBoost Cox uses signed time labels for censoring and an explicitly
#' estimated training-only Breslow baseline. XGBoost AFT uses lower/upper
#' censoring bounds, not ordinary regression on censored times. MTLR knots
#' and scaling are learned within training, including each tuning fold.
#' @return A `survlearning_fit` object with engine, parameters, design, and
#'   training metadata. Access the engine via `$engine`. Neural adapters store
#'   serializable R weights alongside fitted components. Training outcomes are
#'   retained for IPCW evaluation; [surv_diagnostics()] exposes fit provenance.
#'   Components `n`, `events`, `max_time`, `seed`, and `fit_seconds` describe
#'   the training rows, observed failures, largest follow-up (input time unit),
#'   seed used, and elapsed fitting seconds. `diagnostics` records warnings and
#'   the engine's reported convergence, not a universal convergence certificate.
#'   Kaplan-Meier accepts fully censored training outcomes and returns survival
#'   one within observed support; regression engines require an observed failure.
#'   Invalid inputs or unavailable required dependencies cause informative errors.
#'   No files are written and no dependencies are installed automatically.
#' @export
#' @examples
#' fit <- surv_fit(survival::Surv(time, status) ~ age + karno,
#'   survival::veteran, method = "aft")
#' surv_predict(fit, survival::veteran[1:3, ], type = "time")
surv_fit <- function(formula, data, method = "cox", params = list(), seed = NULL,
                     impute = "none") {
  method <- .method(method)
  impute <- match.arg(impute, c("none", "median"))
  input_formula <- formula
  if (method == "km") formula[[3L]] <- 1
  prepared <- .prepare_design(formula, data, impute, allow_empty = method == "km",
    require_event = method != "km")
  .surv_fit_prepared(formula, input_formula, prepared, method, params, seed, impute)
}

.surv_fit_prepared <- function(formula, input_formula, prepared, method, params,
                               seed, impute, svm_cache = NULL) {
  params <- .parameters(method, params, ncol(prepared$x))
  spec <- .method_specs()[[method]]
  .require_engine(method)
  .validate_seed(seed)
  if (is.null(seed) && method %in% c("rsf", "ranger", "aorsf", "xgboost_cox", "xgboost_aft")) {
    seed <- sample.int(.Machine$integer.max, 1L)
  }
  start <- proc.time()[["elapsed"]]
  warnings <- character()
  engine <- withCallingHandlers(
    .with_seed(seed, .fit_engine(method, prepared, params, seed, svm_cache)),
    warning = function(w) warnings <<- c(warnings, conditionMessage(w)))
  out <- structure(list(method = method, params = params, engine = engine,
    engine_package = spec$engine, formula = formula, input_formula = input_formula,
    design = prepared$design, impute = impute, training_outcome = prepared$y,
    n = nrow(prepared$x), events = sum(prepared$y[, 2L]),
    max_time = max(prepared$y[, 1L]), seed = seed,
    fit_seconds = unname(proc.time()[["elapsed"]] - start)), class = "survlearning_fit")
  if (method %in% c("cox_lasso", "cox_ridge", "cox_elasticnet")) {
    gx <- .glmnet_design(prepared$x)
    zero <- matrix(0, nrow = 1L, ncol = ncol(gx),
      dimnames = list(NULL, colnames(gx)))
    out$baseline <- survival::survfit(engine, x = gx, y = prepared$y,
      newx = zero, s = params$lambda)
  }
  if (method == "xgboost_cox") {
    out$baseline <- .xgboost_baseline(engine, prepared$x, prepared$y)
  }
  if (method == "mtlr") {
    out$risk_horizon <- out$max_time
    out$risk_grid <- sort(unique(c(0, engine$time_points,
      seq(0, out$risk_horizon, length.out = 101L))))
  }
  out$fit_seconds <- unname(proc.time()[["elapsed"]] - start)
  out$diagnostics <- list(warnings = unique(warnings),
    convergence = .engine_convergence(engine),
    engine_version = as.character(utils::packageVersion(spec$engine)),
    package_version = as.character(utils::packageVersion("survlearning")),
    r_version = as.character(getRversion()),
    imputation = prepared$design$imputation)
  if (method %in% c("neural_discrete", "neural_cox")) {
    out$diagnostics$engine_details <- engine$diagnostics
    if (method == "neural_discrete") out$diagnostics$coarsening <- engine$coarsening
  }
  if (method %in% c("svm", .svm_learning_methods())) out$diagnostics$engine_details <- engine$diagnostics
  if (method %in% c("aft_lasso", "aft_ridge", "aft_elasticnet", "flexsurv")) {
    out$diagnostics$engine_details <- engine$diagnostics
  }
  out
}

.fit_engine <- function(method, prepared, params, seed, svm_cache = NULL) {
  if (method %in% names(.reference_specs())) {
    return(.fit_reference(method, prepared, params, seed))
  }
  x <- prepared$x
  y <- prepared$y
  if (method %in% c("aft_lasso", "aft_ridge", "aft_elasticnet", "svm", "mtlr", .svm_learning_methods())) {
    x <- .scaled_design(x, prepared$design)
  }
  data <- .engine_data(x, y)
  formula <- .engine_formula()
  if (method %in% .svm_learning_methods()) return(.svm_learning_fit(method, x, y, params, svm_cache))
  switch(method,
    aft = survival::survreg(formula, data = data, dist = params$distribution),
    aft_lasso = , aft_ridge = , aft_elasticnet = {
      .fit_gehan(x, y, params,
        if (method == "aft_elasticnet") params$alpha else as.integer(method == "aft_lasso"))
    },
    cox = survival::coxph(formula, data = data, ties = "efron", x = TRUE, y = TRUE),
    cox_lasso = , cox_ridge = , cox_elasticnet = {
      .fit_penalized_cox(.glmnet_design(x), y, params$lambda,
        if (method == "cox_elasticnet") params$alpha else as.integer(method == "cox_lasso"))
    },
    tree = rpart::rpart(formula, data = data, method = "exp",
      control = rpart::rpart.control(cp = params$cp, minbucket = params$min_node,
        maxdepth = params$max_depth, xval = 0L)),
    rsf = {
      old <- options(rf.cores = params$threads)
      on.exit(options(old), add = TRUE)
      .fit_rsf(formula, data, params, seed)
    },
    boosting = gbm::gbm(formula, data = data, distribution = "coxph",
      n.trees = params$trees, interaction.depth = params$depth,
      shrinkage = params$learning_rate, n.minobsinnode = params$min_node,
      bag.fraction = params$bag_fraction, train.fraction = 1, verbose = FALSE),
    xgboost_cox = , xgboost_aft = .fit_xgboost(method, x, y, params, seed),
    mtlr = MTLR::mtlr(formula, data = data, normalize = FALSE,
      nintervals = params$intervals, C1 = params$lambda,
      maxit = params$max_iter, threshold = params$tolerance),
    svm = .svm_fit(x, y, params, svm_cache)
  )
}

.engine_convergence <- function(engine) {
  if (inherits(engine, "survlearning_svm_learning")) {
    return(list(status = engine$diagnostics$status,
      code = engine$diagnostics$optimizer_status))
  }
  if (is.list(engine) && is.list(engine$opt) &&
      is.numeric(engine$opt$convergence) && length(engine$opt$convergence) == 1L) {
    return(list(status = if (engine$opt$convergence == 0) "reported_success" else "reported_failure",
      code = engine$opt$convergence))
  }
  if (is.list(engine) && !is.null(engine$jerr)) {
    return(list(status = if (engine$jerr == 0) "reported_success" else "reported_failure",
      code = engine$jerr))
  }
  if (is.list(engine) && !is.null(engine$convergence) &&
      is.numeric(engine$convergence) && length(engine$convergence) == 1L) {
    return(list(status = if (engine$convergence == 0) "reported_success" else "reported_failure",
      code = engine$convergence))
  }
  if (is.list(engine) && !is.null(engine$fail)) {
    return(list(status = "reported_failure", code = engine$fail))
  }
  list(status = "not_reported", code = NULL)
}

#' Print a Survival Learning Fit
#' @param x A `survlearning_fit` object from [surv_fit()].
#' @param ... Reserved for S3 compatibility.
#' @return Invisibly returns `x` unchanged, after printing the method, engine,
#'   training counts, censoring percentage and prediction capabilities.
#' @export
print.survlearning_fit <- function(x, ...) {
  cat("SurvLearning fit\n", " Method: ", x$method, "\n", " Engine: ",
    x$engine_package, "\n", " Training: ", x$n, " rows, ", x$events,
    " events (", round(100 * (1 - x$events / x$n), 1), "% censored)\n",
    " Predictions: ", paste(.method_specs()[[x$method]]$predictions, collapse = ", "),
    "\n", sep = "")
  invisible(x)
}
