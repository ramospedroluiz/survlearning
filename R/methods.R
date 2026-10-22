#' Available Survival Learning Methods
#'
#' List engines, prediction capabilities, and optional dependency availability.
#' A risk score always increases with risk; it is not a failure probability.
#' Penalized AFT methods use the semiparametric Gehan estimator, not a
#' penalized parametric likelihood.
#' The `installed` column describes the default configuration. Native SVM
#' kernels need no external SVM engine, but the default solver needs Clarabel;
#' the explicit Newton solver is available without it.
#' @return A data frame with one row per supported method.
#' @export
#' @examples
#' surv_methods()
surv_methods <- function() {
  specs <- .method_specs()
  data.frame(
    method = names(specs),
    description = vapply(specs, `[[`, "", "description"),
    engine = vapply(specs, `[[`, "", "engine"),
    predictions = vapply(specs, function(x) paste(x$predictions, collapse = ", "), ""),
    installed = vapply(names(specs), function(method) {
      x <- specs[[method]]
      length(find.package(x$engine, quiet = TRUE)) > 0L &&
        (method != "svm" || (length(find.package("clarabel", quiet = TRUE)) > 0L &&
          utils::packageVersion("clarabel") >= "0.11.3"))
    }, logical(1)),
    row.names = NULL
  )
}

.method_specs <- function() {
  c(list(
    aft = list(description = "Parametric AFT (Weibull, log-normal, log-logistic)",
      engine = "survival", predictions = c("risk", "time", "survival"),
      defaults = list(distribution = "weibull")),
    aft_lasso = list(description = "Semiparametric AFT with LASSO (Gehan loss)",
      engine = "penAFT", predictions = "risk",
      defaults = list(lambda = 0.1, max_iter = 10000L, tolerance = 0.00025)),
    aft_ridge = list(description = "Semiparametric AFT with ridge (Gehan loss)",
      engine = "penAFT", predictions = "risk",
      defaults = list(lambda = 0.1, max_iter = 10000L, tolerance = 0.00025)),
    aft_elasticnet = list(description = "Semiparametric AFT with elastic net (Gehan loss)",
      engine = "penAFT", predictions = "risk",
      defaults = list(lambda = 0.1, alpha = 0.5, max_iter = 10000L, tolerance = 0.00025)),
    cox = list(description = "Cox proportional hazards (Efron ties)",
      engine = "survival", predictions = c("risk", "survival"), defaults = list()),
    cox_lasso = list(description = "Cox with LASSO (Breslow ties)",
      engine = "glmnet", predictions = c("risk", "survival"),
      defaults = list(lambda = 0.01)),
    cox_ridge = list(description = "Cox with ridge (Breslow ties)",
      engine = "glmnet", predictions = c("risk", "survival"),
      defaults = list(lambda = 0.01)),
    cox_elasticnet = list(description = "Cox with elastic net (Breslow ties)",
      engine = "glmnet", predictions = c("risk", "survival"),
      defaults = list(lambda = 0.01, alpha = 0.5)),
    tree = list(description = "Survival tree (exponential deviance, not log-rank)",
      engine = "rpart", predictions = "risk",
      defaults = list(cp = 0.01, min_node = 10L, max_depth = 5L)),
    rsf = list(description = "Random survival forest (log-rank splits)",
      engine = "randomForestSRC", predictions = c("risk", "survival"),
      defaults = list(trees = 200L, mtry = NULL, min_node = 15L,
        splits = 5L, threads = 1L)),
    boosting = list(description = "Gradient-boosted trees with Cox loss",
      engine = "gbm", predictions = "risk",
      defaults = list(trees = 300L, depth = 2L, learning_rate = 0.03,
        min_node = 10L, bag_fraction = 0.7)),
    xgboost_cox = list(description = "XGBoost with Cox loss and Breslow baseline",
      engine = "xgboost", predictions = c("risk", "survival"),
      defaults = list(trees = 200L, depth = 2L, learning_rate = 0.05,
        min_child_weight = 1, leaf_penalty = 1, threads = 1L)),
    xgboost_aft = list(description = "XGBoost with censored AFT likelihood",
      engine = "xgboost", predictions = c("risk", "time", "survival"),
      defaults = list(trees = 200L, depth = 2L, learning_rate = 0.05,
        min_child_weight = 1, leaf_penalty = 1, threads = 1L,
        distribution = "normal", scale = 1)),
    mtlr = list(description = "Multi-task logistic regression (risk = negative restricted mean)",
      engine = "MTLR", predictions = c("risk", "survival"),
      defaults = list(intervals = 10L, lambda = 1, max_iter = 5000L, tolerance = 1e-5)),
    svm = list(description = "Censored support vector regression",
      engine = "stats", predictions = "risk",
      defaults = list(cost = 0.1, kernel = "linear", solver = "clarabel",
        tolerance = 1e-8, max_iter = 100000L, memory_limit_mb = 2048,
        collapse_duplicates = TRUE))
  ), .reference_specs())
}

.method <- function(method) {
  choices <- names(.method_specs())
  if (!is.character(method) || length(method) != 1L || is.na(method) ||
      !method %in% choices) {
    stop("Choose one method: ", paste(choices, collapse = ", "), ".", call. = FALSE)
  }
  method
}

.require_engine <- function(method) {
  engine <- .method_specs()[[method]]$engine
  if (!requireNamespace(engine, quietly = TRUE)) {
    stop("Method '", method, "' needs package '", engine,
      "'. Install it with install.packages(\"", engine, "\").", call. = FALSE)
  }
  minimum <- switch(engine, glmnet = "5.0", xgboost = "3.0.0", MTLR = "0.2.2", NULL)
  if (!is.null(minimum) && utils::packageVersion(engine) < minimum) {
    stop("Update ", engine, " to version ", minimum, " or later.", call. = FALSE)
  }
  invisible(NULL)
}

.parameters <- function(method, params, p) {
  defaults <- .method_specs()[[method]]$defaults
  if (!is.list(params) || (length(params) &&
      (is.null(names(params)) || any(!nzchar(names(params))) || anyDuplicated(names(params))))) {
    stop("'params' must be a named list with unique names.", call. = FALSE)
  }
  unknown <- setdiff(names(params), names(defaults))
  if (length(unknown)) stop("Unknown parameter(s) for ", method, ": ",
    paste(unknown, collapse = ", "), ".", call. = FALSE)
  for (key in names(params)) defaults[key] <- params[key]
  if (method %in% c("rsf", "ranger", "aorsf") && is.null(defaults$mtry)) {
    defaults$mtry <- max(1L, floor(sqrt(p)))
  }
  for (key in intersect(names(defaults), c("trees", "min_node", "max_depth", "depth",
      "mtry", "splits", "threads", "max_iter", "intervals"))) {
    .integer_scalar(defaults[[key]], key, 1L)
  }
  for (key in intersect(names(defaults), c("lambda", "cost", "tolerance", "learning_rate", "scale"))) {
    if (key == "scale" && method == "flexsurv") next
    .positive_scalar(defaults[[key]], key)
  }
  if ("cp" %in% names(defaults)) .positive_scalar(defaults$cp, "cp", zero = TRUE)
  for (key in intersect(names(defaults), c("leaf_penalty", "min_child_weight"))) {
    .positive_scalar(defaults[[key]], key, zero = TRUE)
  }
  if ("alpha" %in% names(defaults)) {
    .positive_scalar(defaults$alpha, "alpha", zero = TRUE)
    if (defaults$alpha > 1) stop("'alpha' must be between 0 and 1.", call. = FALSE)
  }
  if ("bag_fraction" %in% names(defaults)) {
    .positive_scalar(defaults$bag_fraction, "bag_fraction")
    if (defaults$bag_fraction > 1) stop("'bag_fraction' must be <= 1.", call. = FALSE)
  }
  if (method %in% c("rsf", "ranger", "aorsf") && defaults$mtry > p) {
    stop("'mtry' exceeds the encoded predictor count.", call. = FALSE)
  }
  if (method == "tree" && defaults$max_depth > 30) stop("'max_depth' must be <= 30.", call. = FALSE)
  if (method == "aft" && !defaults$distribution %in% c("weibull", "lognormal", "loglogistic")) {
    stop("'distribution' must be weibull, lognormal, or loglogistic.", call. = FALSE)
  }
  if (method == "xgboost_aft" && !defaults$distribution %in% c("normal", "logistic", "extreme")) {
    stop("'distribution' must be normal, logistic, or extreme for XGBoost AFT.", call. = FALSE)
  }
  if (method == "svm" && !defaults$kernel %in% c("linear", "additive")) {
    stop("'kernel' must be linear or additive.", call. = FALSE)
  }
  if (method == "svm") {
    if (!is.character(defaults$solver) || length(defaults$solver) != 1L ||
        is.na(defaults$solver) || !defaults$solver %in% c("quadprog", "osqp", "clarabel", "newton")) {
      stop("'solver' must be quadprog, osqp, clarabel or newton.", call. = FALSE)
    }
    .positive_scalar(defaults$memory_limit_mb, "memory_limit_mb")
    if (!is.logical(defaults$collapse_duplicates) || length(defaults$collapse_duplicates) != 1L ||
        is.na(defaults$collapse_duplicates)) stop("'collapse_duplicates' must be TRUE or FALSE.", call. = FALSE)
  }
  if (method %in% names(.reference_specs())) {
    defaults <- .reference_parameters(method, defaults, p)
  }
  defaults
}

.positive_scalar <- function(x, name, zero = FALSE) {
  if (!is.numeric(x) || length(x) != 1L || !is.finite(x) ||
      if (zero) x < 0 else x <= 0) {
    stop("'", name, "' must be a finite ", if (zero) "non-negative" else "positive",
      " number.", call. = FALSE)
  }
}

.integer_scalar <- function(x, name, minimum = 0L) {
  if (!is.numeric(x) || length(x) != 1L || !is.finite(x) || x < minimum ||
      x != floor(x) || x > .Machine$integer.max) {
    stop("'", name, "' must be an integer >= ", minimum, ".", call. = FALSE)
  }
}

.with_seed <- function(seed, code) {
  .validate_seed(seed)
  if (is.null(seed)) force(code) else withr::with_seed(seed, code)
}

.validate_seed <- function(seed) {
  if (!is.null(seed)) .integer_scalar(seed, "seed")
  invisible(seed)
}
