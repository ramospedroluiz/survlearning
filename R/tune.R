#' Tune a Survival Learning Model With a Common Validation Protocol
#'
#' @inheritParams surv_fit
#' @param grid Data frame of candidates or named list expanded as a Cartesian
#'   grid. No automatic search range is imposed. NULL means one fixed setting.
#' @param folds Number of event-stratified validation folds (default 5).
#' @param resamples Reusable plan from [surv_resamples()]. When NULL, historical
#'   event-stratified allocation and minimum event counts apply.
#' @param metric Scalar tuning objective. Non-Harrell metrics are delegated to
#'   [surv_evaluate()]. Brier and integrated Brier (`"ibs"`) are minimized;
#'   concordance and AUC are maximized.
#' @param times Evaluation horizons. Time-specific tuning requires one horizon.
#' @param tau Truncation horizon passed to [surv_evaluate()].
#' @param impute `"none"` or `"median"`, learned within every training fit.
#' @param cache Reuse SVM preprocessing, sparse penalties and constraint matrices
#'   within each analysis split and kernel (default TRUE). Never shared across
#'   splits; at most one kernel workspace is retained. FALSE provides a cold
#'   reference path. Other methods are unaffected.
#' @param censoring NULL for marginal IPCW, or a formula/list specification
#'   refitted by [surv_censor_fit()] on each analysis fold. Never a global fit.
#' @param workers Number of local PSOCK workers. Default 1 is serial. Work is
#'   grouped by analysis fold, retaining within-fold SVM matrix reuse.
#' @param checkpoint NULL, or a directory owned by this search. Individual
#'   candidate-fold records are committed atomically; no fold models are stored.
#' @param resume Reuse compatible completed records (default TRUE).
#' @param retry_failed Recompute failed checkpoint records (default FALSE).
#' @param progress Report completion after each serial fold or parallel wave.
#' @details
#' All candidates share assessment indices. Any failed or undefined split makes
#' a candidate ineligible. Errors and warnings remain in `fold_results`;
#' warnings, including convergence warnings, are not suppressed. Ties select the
#' first grid row. Explicit engine `reported_failure` diagnostics invalidate a
#' fit; warnings alone and `not_reported` convergence do not. The winner is
#' refitted on the supplied data. This selection
#' criterion is not an unbiased estimate of the search procedure; use
#' [surv_validate()] or an external test set for final evaluation.
#'
#' Historical `fold_id`, `cindex`, `mean_cindex`, and `sd_cindex` fields remain
#' for default Harrell tuning. For other metrics concordance fields are NA; use
#' `estimate`, `mean_estimate`, and `sd_estimate`. With repeated or temporal plans
#' `fold_id` is NULL; use explicit indices. All-candidate failure raises a
#' `survlearning_tune_error` retaining `results`, `fold_results`, and `resamples`.
#' Parallel/checkpoint formulas support standard base/stats/survival/splines
#' functions and data columns, not custom closures or captured constants.
#' Checkpoints match all data/predictors/order, formulas, folds, settings and
#' recorded R/platform/engine versions. Workers and progress can change when
#' resuming. Use only one live search per directory. Ordinary interrupts/errors
#' release the lock; after a hard process crash verify no run is active before
#' manually removing the reported lock directory. Incomplete/corrupt records
#' cannot silently win. Warnings/errors are retained. A completed winning model
#' is also cached. Resuming missing fits rebuilds numerical workspaces; no solver
#' warm-start history is persisted, so numerical tolerances still apply.
#' Parallelism duplicates data/workspaces across workers. Avoid nested engine
#' threads, and choose workers according to available memory. No universal
#' speedup or automatic memory cap is promised. The same per-fit seeds and
#' stable result ordering are used across schedules. With seed = NULL a search
#' seed is drawn once from the caller's RNG and stored in the result. Checkpoint
#' searches require an explicit seed so that their identity is stable on resume.
#' @return A `survlearning_tune` with historical `best_model`, `best_params`,
#'   `results`, `fold_results`, `grid`, `fold_id`, `seed`, and `best_candidate`,
#'   plus `resamples`, `metric`, warnings and failure diagnostics.
#' @export
#' @examples
#' tuned <- surv_tune(survival::Surv(time, status) ~ age + karno,
#'   survival::veteran, method = "aft",
#'   grid = list(distribution = c("weibull", "lognormal")), folds = 3)
#' tuned$results
surv_tune <- function(formula, data, method = "cox", grid = NULL, params = list(),
                      folds = 5L, seed = NULL, resamples = NULL,
                      metric = "harrell", times = NULL, tau = NULL,
                      impute = "none", cache = TRUE, censoring = NULL,
                      workers = 1L, checkpoint = NULL, resume = TRUE,
                      retry_failed = FALSE, progress = FALSE) {
  search_start <- proc.time()[["elapsed"]]
  method <- .method(method)
  .validate_seed(seed)
  if (is.null(seed)) {
    if (!is.null(checkpoint)) stop("Supply an explicit 'seed' for resumable checkpoint tuning.", call. = FALSE)
    seed <- sample.int(.Machine$integer.max, 1L)
  }
  metric <- .resample_metrics(metric)
  if (length(metric) != 1L) stop("Choose one scalar tuning metric.", call. = FALSE)
  .resample_preflight(metric, times, tau, scalar = TRUE)
  impute <- match.arg(impute, c("none", "median"))
  .integer_scalar(workers, "workers", 1L)
  for (value in list(resume, retry_failed, progress)) {
    if (!is.logical(value) || length(value) != 1L || is.na(value)) {
      stop("'resume', 'retry_failed', and 'progress' must be TRUE or FALSE.", call. = FALSE)
    }
  }
  if (!is.logical(cache) || length(cache) != 1L || is.na(cache)) {
    stop("'cache' must be TRUE or FALSE.", call. = FALSE)
  }
  if (is.null(resamples)) {
    .integer_scalar(folds, "folds", 2L)
    y <- .outcome(formula, data, require_event = FALSE)
    if ((sum(y[, 2L]) < folds && !(method == "km" && metric %in% c("brier", "ibs"))) || nrow(data) < 2L * folds) {
      stop("Use fewer folds: every fold needs an event and at least two rows overall.", call. = FALSE)
    }
    resamples <- surv_resamples(formula, data, folds = folds, seed = seed)
  } else resamples <- .validate_resamples(resamples, formula, data)
  grid <- .tune_grid(grid, method)
  if (!is.list(params) || (length(params) && (is.null(names(params)) ||
      anyNA(names(params)) || any(!nzchar(names(params))) || anyDuplicated(names(params))))) {
    stop("'params' must be a named list with unique names.", call. = FALSE)
  }
  if (workers > 1L || !is.null(checkpoint)) {
    formula <- .tune_portable_formula(formula, data)
    if (!is.null(censoring)) {
      if (inherits(censoring, "formula")) censoring <- list(formula = censoring, method = "cox")
      if (!is.list(censoring) || is.null(censoring$formula) ||
          any(!names(censoring) %in% c("formula", "method", "impute"))) {
        stop("Portable censoring must be a formula/list specification, not a global fitted model.", call. = FALSE)
      }
      censoring$formula <- .tune_portable_formula(censoring$formula, data)
    }
  }
  if (workers > 1L) for (i in seq_len(nrow(grid))) {
    setting <- .tune_setting(params, grid, i)
    if (!is.null(setting$threads) && setting$threads != 1L) {
      stop("Use threads = 1 inside models when workers > 1; nested parallelism is not enabled.", call. = FALSE)
    }
  }
  jobs <- expand.grid(fold = seq_along(resamples$splits), candidate = seq_len(nrow(grid)))
  shared <- method %in% c("svm", .svm_learning_methods()) && cache
  groups <- rep("", nrow(grid))
  if (shared) groups <- vapply(seq_len(nrow(grid)), function(i) {
    setting <- .tune_setting(params, grid, i)
    paste(if (is.null(setting$solver)) .method_specs()$svm$defaults$solver else setting$solver,
      if (is.null(setting$kernel)) "linear" else setting$kernel,
      if (is.null(setting$collapse_duplicates)) TRUE else setting$collapse_duplicates, sep = "_")
  }, "")
  if (shared) jobs <- jobs[order(jobs$fold, groups[jobs$candidate], jobs$candidate), ]
  context <- list(formula = formula, data = data, method = method, params = params,
    grid = grid, resamples = resamples, seed = seed, metric = metric, times = times,
    tau = tau, impute = impute, shared = shared, groups = groups, censoring = censoring)
  state <- .tune_checkpoint(checkpoint, context, resume)
  on.exit(.tune_unlock(state), add = TRUE)
  execution <- .tune_execute(context, jobs, state, workers, retry_failed, progress)
  if (!is.null(state) && !execution$execution$computed_jobs && file.exists(file.path(state$path, "winner.rds"))) {
    saved <- readRDS(file.path(state$path, "winner.rds"))
    if (!identical(saved$fingerprint, state$fingerprint) || !inherits(saved$model, "survlearning_tune")) {
      stop("Invalid cached winning model; use a new checkpoint directory.", call. = FALSE)
    }
    if (!identical(saved$model$fold_results, do.call(rbind, execution$details))) {
      stop("Cached winner and fold records disagree; use a new checkpoint directory.", call. = FALSE)
    }
    saved$model$execution <- execution$execution
    saved$model$execution$refit_reused <- TRUE
    saved$model$execution$elapsed_seconds <- unname(proc.time()[["elapsed"]] - search_start)
    return(saved$model)
  }
  fold_results <- do.call(rbind, execution$details)
  results <- data.frame(candidate = seq_len(nrow(grid)), grid,
    mean_cindex = NA_real_, sd_cindex = NA_real_, failed_folds = 0L, seconds = 0,
    mean_estimate = NA_real_, sd_estimate = NA_real_, metric = metric, check.names = FALSE)
  for (candidate in seq_len(nrow(grid))) {
    rows <- fold_results[fold_results$candidate == candidate, , drop = FALSE]
    results$failed_folds[candidate] <- sum(!is.finite(rows$estimate))
    results$seconds[candidate] <- sum(rows$seconds)
    if (!results$failed_folds[candidate]) {
      results$mean_estimate[candidate] <- mean(rows$estimate)
      results$sd_estimate[candidate] <- stats::sd(rows$estimate)
      if (metric == "harrell") {
        results$mean_cindex[candidate] <- results$mean_estimate[candidate]
        results$sd_cindex[candidate] <- results$sd_estimate[candidate]
      }
    }
  }
  if (!any(is.finite(results$mean_estimate))) {
    first <- fold_results$error[which(nzchar(fold_results$error))[1L]]
    stop(structure(list(message = paste0("All tuning candidates failed. First error: ", first),
      call = NULL, results = results, fold_results = fold_results, resamples = resamples),
      class = c("survlearning_tune_error", "error", "condition")))
  }
  objective <- results$mean_estimate
  maximize <- !metric %in% c("brier", "ibs", "integrated_brier")
  best <- if (maximize) which.max(replace(objective, is.na(objective), -Inf)) else
    which.min(replace(objective, is.na(objective), Inf))
  best_params <- .tune_setting(params, grid, best)
  refit <- .resample_capture(.resample_fit(formula, data, method, best_params, seed, impute))
  if (nzchar(refit$error)) {
    stop(structure(list(message = paste0("Winning candidate refit failed: ", refit$error),
      call = NULL, results = results, fold_results = fold_results, resamples = resamples,
      refit_warnings = refit$warnings), class = c("survlearning_tune_error", "error", "condition")))
  }
  best_model <- refit$value
  out <- structure(list(method = method, best_model = best_model, best_params = best_model$params,
    results = results, fold_results = fold_results, grid = grid,
    fold_id = resamples$fold_id, seed = seed, best_candidate = best,
    resamples = resamples, metric = metric, times = times, tau = tau, impute = impute,
    warnings = fold_results[nzchar(fold_results$warning), , drop = FALSE],
    failures = fold_results[!is.finite(fold_results$estimate), , drop = FALSE],
    refit_warnings = refit$warnings, cache = shared, censoring = censoring,
    execution = c(execution$execution, list(refit_reused = FALSE)),
    search = list(candidates = nrow(grid), hyperparameter_search = nrow(grid) > 1L,
      description = if (nrow(grid) > 1L) "Explicit candidate search" else
        "One fixed configuration: cross-validation without hyperparameter search")),
    class = "survlearning_tune")
  out$execution$elapsed_seconds <- unname(proc.time()[["elapsed"]] - search_start)
  if (!is.null(state)) .tune_commit(list(fingerprint = state$fingerprint, model = out),
    file.path(state$path, "winner.rds"), state)
  out
}

.tune_grid <- function(grid, method) {
  if (is.null(grid) || (is.list(grid) && !length(grid) && !is.data.frame(grid))) grid <- data.frame(row.names = 1L)
  if (is.list(grid) && !is.data.frame(grid)) {
    grid <- expand.grid(grid, KEEP.OUT.ATTRS = FALSE, stringsAsFactors = FALSE)
  }
  if (!is.data.frame(grid) || !nrow(grid) || anyNA(names(grid)) ||
      any(!nzchar(names(grid))) || anyDuplicated(names(grid))) {
    stop("'grid' must have named parameters and at least one candidate, or be NULL.", call. = FALSE)
  }
  for (name in names(grid)) if (is.factor(grid[[name]])) grid[[name]] <- as.character(grid[[name]])
  unknown <- setdiff(names(grid), names(.method_specs()[[method]]$defaults))
  if (length(unknown)) stop("Unknown tuning parameter(s): ", paste(unknown, collapse = ", "), ".", call. = FALSE)
  grid
}

.tune_setting <- function(params, grid, candidate) {
  for (key in names(grid)) params[key] <- list(grid[[key]][candidate])
  params
}

#' Print a Survival Learning Search
#' @param x A `survlearning_tune` object from [surv_tune()].
#' @param ... Reserved for S3 compatibility.
#' @return Invisibly returns `x` unchanged, after printing search information
#'   and the selected parameter list to the console.
#' @export
print.survlearning_tune <- function(x, ...) {
  legacy <- is.null(x$metric) || x$metric == "harrell"
  score <- if (legacy) x$results$mean_cindex[x$best_candidate] else x$results$mean_estimate[x$best_candidate]
  folds <- if (is.null(x$resamples)) max(x$fold_id) else length(x$resamples$splits)
  cat("SurvLearning tuning\n", " Method: ", x$method, "\n", " Candidates: ",
    nrow(x$grid), "\n", " Folds: ", folds, "\n",
    if (legacy) " Best mean validation C-index: " else paste0(" Best mean validation ", x$metric, ": "),
    format(round(score, 4)), "\n", sep = "")
  if (nrow(x$grid) == 1L) cat(" One fixed configuration; no hyperparameter search.\n")
  print(x$best_params)
  invisible(x)
}
