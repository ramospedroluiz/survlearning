#' Combine Native Survival Curves Using Out-of-Fold IPCW Brier Loss
#'
#' @param formula Two-sided formula with a right-censored response.
#' @param data Training data, not the final assessment sample.
#' @param methods Distinct method identifiers providing native survival curves.
#' @param times Explicit positive increasing horizons for learning the weights.
#' @param grids Optional named tuning grids. Each supplied grid is selected
#'   within each weight-training fold by inner Harrell-C validation. No grid
#'   is invented for methods without one; their parameters stay fixed.
#' @param params Named list of fixed parameter lists, one per method.
#' @param folds Number of out-of-fold partitions for learning the weights.
#' @param inner_folds Number of inner folds for supplied tuning grids.
#' @param resamples Optional single-repeat stratified plan from [surv_resamples()].
#' @param seed Optional local seed, restoring the caller's RNG state.
#' @param impute `"none"` or training-fold median/mode imputation.
#' @param censoring Formula/list for G, refitted in every training fold.
#'   NULL uses marginal reverse KM. Global fitted G objects are not accepted.
#' @param min_g Smallest supported G; weights are never clipped.
#' @details Each training unit receives predictions from models that did not
#'   use its outcomes or predictors for fitting, preprocessing or tuning.
#'   One nonnegative weight per method, summing to one and constant over time,
#'   minimizes the mean IPCW Brier loss over the supplied horizons. Earlier
#'   censorings have zero outcome-loss weight but remain in the denominator
#'   and contribute to training and G estimation. All rows are retained.
#'   Clarabel, already required by survlearning, solves the small convex QP.
#'   Feasibility and a simplex first-order objective-gap bound are verified.
#'   Tiny solver feasibility errors are projected onto the simplex before
#'   checking the bound. Duplicate curves may have nonunique weights.
#'
#'   Models are then refitted on all training data. The OOF loss used to choose
#'   weights is a training criterion, not an unbiased performance estimate of
#'   the stack. Evaluate on an independent test set or nest the whole call in
#'   outer validation. Base grids and censoring estimation remain inside each
#'   training fold. Unsupported horizons, failed fits or risk-only methods
#'   stop the fit rather than dropping observations or learners.
#'
#'   Survival predictions use the same convex weights at all times, preserving
#'   probability bounds and monotonicity. Risk is negative restricted mean,
#'   approximated by trapezoidal integration over zero plus `times`. It is not
#'   a Cox log hazard or an unrestricted mean. No median, extrapolation, Python
#'   or Torch is added. Mixing uncalibrated risk scores is not supported.
#' @return A `survlearning_stack` inheriting from `survlearning_fit`, usable by
#'   [surv_predict()], [surv_score()], [surv_evaluate()], [surv_compare()],
#'   [surv_calibrate()] and [surv_reliability()]. Includes named `weights`,
#'   refitted `models`, OOF curves/weights, resamples, fold tuning diagnostics,
#'   training loss and numerical checks. Weights are not variable importance.
#' @export
#' @examples
#' d <- survival::veteran
#' stack <- surv_stack(Surv(time, status) ~ age + karno, d,
#'   methods = c("cox", "aft"), times = c(30, 60), folds = 3, seed = 11)
#' stack$weights
#' surv_predict(stack, d[1:2, ], "survival", times = c(30, 60))
surv_stack <- function(formula, data, methods = c("cox", "rsf"), times,
    grids = NULL, params = NULL, folds = 5L, inner_folds = 3L, resamples = NULL,
    seed = NULL, impute = "none", censoring = NULL, min_g = .05) {
  .validate_seed(seed)
  .evaluation_times(times)
  .evaluation_fraction(min_g, "min_g", inclusive = TRUE)
  .integer_scalar(inner_folds, "inner_folds", 2L)
  if (any(times <= 0) || any(diff(times) <= 0)) stop("Stacking 'times' must be positive and increasing.", call. = FALSE)
  if (!is.character(methods) || length(methods) < 2L || anyNA(methods) || anyDuplicated(methods)) {
    stop("Provide at least two distinct survival method identifiers.", call. = FALSE)
  }
  methods <- vapply(methods, .method, character(1), USE.NAMES = FALSE)
  for (method in methods) if (!"survival" %in% .method_specs()[[method]]$predictions) {
    stop("Stacking requires native survival curves; method '", method, "' is risk-only.", call. = FALSE)
  }
  impute <- match.arg(impute, c("none", "median"))
  grids <- .resample_method_lists(grids, methods, "grids")
  params <- .resample_method_lists(params, methods, "params")
  y <- .outcome(formula, data)
  if (is.null(resamples)) resamples <- surv_resamples(formula, data, folds = folds, seed = seed)
  else resamples <- .validate_resamples(resamples, formula, data)
  if (resamples$type != "stratified" || resamples$repeats != 1L) {
    stop("Stacking requires one complete stratified partition; grouped, temporal and repeated plans are unsupported.", call. = FALSE)
  }
  n <- nrow(y); m <- length(methods); nt <- length(times)
  oof <- array(NA_real_, c(n, nt, m), dimnames = list(NULL, paste0("t=", times), methods))
  weight <- matrix(NA_real_, n, nt)
  tuning <- vector("list", length(resamples$splits))
  names(tuning) <- names(resamples$splits)
  warnings <- character()
  started <- proc.time()[["elapsed"]]
  result <- withCallingHandlers(.with_seed(seed, {
    train_one <- function(train, method, plan) {
      fixed <- params[[method]]
      if (is.null(fixed)) fixed <- list()
      if (!is.null(grids[[method]])) {
        fit <- surv_tune(formula, train, method, grid = grids[[method]], params = fixed,
          resamples = plan, seed = seed, impute = impute, censoring = censoring)
        list(model = fit$best_model, tuning = list(best_params = fit$best_params,
          fold_results = fit$fold_results, results = fit$results))
      } else list(model = .resample_fit(formula, train, method, fixed, seed, impute), tuning = NULL)
    }
    for (k in seq_along(resamples$splits)) {
      split <- resamples$splits[[k]]
      train <- data[split$analysis, , drop = FALSE]
      assessment <- data[split$assessment, , drop = FALSE]
      inner <- if (length(grids)) surv_resamples(formula, train, folds = inner_folds, seed = seed) else NULL
      g <- .resample_censoring(censoring, train)
      tuning[[k]] <- list(inner_resamples = inner, methods = list())
      for (j in seq_along(methods)) {
        fitted <- train_one(train, methods[j], inner)
        prepared <- .evaluation_setup(fitted$model, assessment, "brier", times = times,
          min_g = min_g, censoring_model = g)
        oof[split$assessment, , j] <- prepared$probabilities
        if (j == 1L) weight[split$assessment, ] <- .horizon_weights(prepared$y, times,
          prepared$g_event, prepared$g_time)
        tuning[[k]]$methods[[methods[j]]] <- fitted$tuning
      }
    }
    if (any(!is.finite(oof)) || any(!is.finite(weight))) {
      stop("Incomplete out-of-fold predictions or weights; no rows were removed.", call. = FALSE)
    }
    target <- outer(y[, 1L], times, ">") * 1
    x <- matrix(oof, nrow = n * nt, ncol = m)
    solution <- .stack_weights(x, as.vector(target), as.vector(weight))
    names(solution$weights) <- methods
    models <- final_tuning <- stats::setNames(vector("list", m), methods)
    for (method in methods) {
      fitted <- train_one(data, method, resamples)
      models[[method]] <- fitted$model
      final_tuning[method] <- list(fitted$tuning)
    }
    list(weights = solution$weights, models = models, final_tuning = final_tuning,
      solution = solution)
  }), warning = function(w) warnings <<- c(warnings, conditionMessage(w)))
  structure(list(method = "stack", engine_package = "survlearning",
    formula = formula, input_formula = formula,
    design = result$models[[which(methods != "km")[1L]]]$design,
    impute = impute, training_outcome = y, n = n, events = sum(y[, 2L]),
    max_time = max(y[, 1L]), seed = seed, fit_seconds = proc.time()[["elapsed"]] - started,
    params = list(methods = methods, times = times, weights = result$weights),
    weights = result$weights, models = result$models, times = times,
    oof = list(survival = oof, ipcw = weight), resamples = resamples,
    tuning = tuning, final_tuning = result$final_tuning,
    diagnostics = list(warnings = unique(warnings), convergence = list(status = "reported_success"),
      objective = "mean OOF IPCW Brier over supplied horizons", loss = result$solution$loss,
      gap_bound = result$solution$gap_bound, simplex_error = result$solution$simplex_error,
      uncertainty = "OOF loss selects weights; it is not independent evaluation")),
    class = c("survlearning_stack", "survlearning_fit"))
}

.stack_weights <- function(x, target, weight) {
  if (!is.matrix(x) || any(!is.finite(x)) || any(!is.finite(weight)) ||
      any(weight < 0) || !any(weight > 0)) stop("Invalid stacking design or IPCW weights.", call. = FALSE)
  h <- crossprod(x, x * weight) / nrow(x)
  linear <- drop(crossprod(x, target * weight)) / nrow(x)
  m <- ncol(x)
  a <- rbind(rep(1, m), rep(-1, m), -diag(m))
  fit <- clarabel::clarabel(P = Matrix::forceSymmetric(Matrix::Matrix(2 * h, sparse = TRUE)),
    q = -2 * linear, A = Matrix::Matrix(a, sparse = TRUE), b = c(1, -1, rep(0, m)),
    cones = list(l = as.integer(m + 2L)),
    control = clarabel::clarabel_control(verbose = FALSE, max_threads = 1L,
      tol_gap_abs = 1e-10, tol_gap_rel = 1e-10, tol_feas = 1e-10, max_iter = 200L))
  if (fit$status != 2L || any(!is.finite(fit$x)) || min(fit$x) < -1e-7 ||
      abs(sum(fit$x) - 1) > 1e-7) stop("Stacking weight optimizer failed; no ensemble was returned.", call. = FALSE)
  w <- pmax(fit$x, 0)
  w <- w / sum(w)
  gradient <- 2 * (drop(h %*% w) - linear)
  gap <- sum(w * gradient) - min(gradient)
  loss <- mean(weight * (target - drop(x %*% w))^2)
  if (!is.finite(gap) || gap > 1e-7 * max(1, loss)) stop("Stacking optimality gap is too large.", call. = FALSE)
  list(weights = w, loss = loss, gap_bound = max(0, gap),
    simplex_error = max(abs(sum(w) - 1), max(0, -min(w))))
}

.stack_predict <- function(object, newdata, type, times) {
  if (identical(object$diagnostics$convergence$status, "reported_failure")) {
    stop("Cannot predict from a stack whose optimizer reported failure.", call. = FALSE)
  }
  type <- match.arg(type, c("risk", "survival", "time"))
  if (type == "time") stop("Stacking does not provide a native median; use surv_reliability() for supported quantiles.", call. = FALSE)
  if (type == "risk" && !is.null(times)) stop("'times' is only used with type = 'survival'.", call. = FALSE)
  if (type == "survival" && (!is.numeric(times) || !length(times) || any(!is.finite(times)) || any(times < 0))) {
    stop("Provide finite, non-negative 'times' for survival prediction.", call. = FALSE)
  }
  grid <- if (type == "risk") c(0, object$times) else times
  result <- NULL
  total <- 0
  for (j in which(object$weights > 0)) {
    curve <- surv_predict(object$models[[j]], newdata, "survival", times = grid)
    total <- total + object$weights[j]
    result <- if (is.null(result)) curve else result + (curve - result) * (object$weights[j] / total)
  }
  if (any(grid > object$max_time)) {
    result[, grid > object$max_time] <- NA_real_
    warning("Horizons beyond training follow-up return NA; no stack extrapolation was added.", call. = FALSE)
  }
  .validate_survival_prediction(result, nrow(newdata), grid, object)
  if (type == "survival") return(result)
  -drop((result[, -1L, drop = FALSE] + result[, -ncol(result), drop = FALSE]) %*% (diff(grid) / 2))
}

#' @rdname surv_stack
#' @param x A fitted stacking object.
#' @param ... Reserved; unused arguments are rejected.
#' @export
print.survlearning_stack <- function(x, ...) {
  if (length(list(...))) stop("Unused print argument(s).", call. = FALSE)
  cat("Survival stack: ", x$n, " training rows, ", x$events, " events\n", sep = "")
  print(data.frame(method = names(x$weights), weight = unname(x$weights)), row.names = FALSE)
  cat("OOF weight-training Brier loss: ", format(x$diagnostics$loss),
    " (not independent performance)\n", sep = "")
  invisible(x)
}
