#' Explicit, Reusable Survival Resampling Plans
#'
#' @param formula Two-sided formula with a right-censored survival response.
#' @param data Data frame. Only outcomes are inspected, allowing missing predictors.
#' @param folds Number of assessment folds, at least two.
#' @param repeats Number of repetitions, at least one.
#' @param type One of `"stratified"`, `"grouped"`, or `"temporal"`.
#' @param group Column name or row-aligned group vector for grouped splitting.
#' @param time Column name or row-aligned numeric, Date, or POSIXct vector giving
#'   observation order, not the survival response, for temporal splitting.
#' @param seed NULL (default) uses the caller's RNG. An integer >= 0 supplies
#'   a local seed and restores the caller's RNG state.
#' @details
#' Stratification allocates events and censored observations separately. Grouped
#' folds balance group sizes without dividing groups. Temporal plans partition
#' ordered unique times into `folds + 1` blocks: block one is initial training;
#' each subsequent block is assessed using all preceding blocks. Time ties are
#' never divided. Temporal plans require `repeats = 1`: deterministic duplicate
#' splits must not be counted as additional validation replicates. Chronological
#' row order alone does not establish when labels or predictors became available.
#'
#' Splits contain integer `analysis` and `assessment` row indices. The plan
#' stores `data.n`, formula, outcomes, row names, structural vectors, and
#' sample/event diagnostics. Consumers recheck signatures and disjointness.
#' The signature detects changed/reordered outcomes and row names, not arbitrary
#' predictor changes. Preserve row order. No-event splits are retained, not redrawn.
#' @return A `survlearning_resamples` list with `splits`, `diagnostics`, and
#'   metadata. `fold_id` is available for a single complete CV partition.
#' @export
#' @examples
#' plan <- surv_resamples(survival::Surv(time, status) ~ age + karno,
#'   survival::veteran, folds = 3)
#' plan$diagnostics
surv_resamples <- function(formula, data, folds = 5L, repeats = 1L,
                           type = "stratified", group = NULL, time = NULL,
                           seed = NULL) {
  .integer_scalar(folds, "folds", 2L)
  .integer_scalar(repeats, "repeats", 1L)
  .validate_seed(seed)
  type <- match.arg(type, c("stratified", "grouped", "temporal"))
  if (type == "temporal" && repeats > 1L) {
    stop("Temporal repeats > 1 duplicate deterministic splits; use repeats = 1.", call. = FALSE)
  }
  y <- .outcome(formula, data, require_event = FALSE)
  n <- nrow(data)
  if (folds > n) stop("Use fewer folds than observations.", call. = FALSE)
  groups <- ordering <- group_name <- time_name <- NULL
  if (type == "grouped") {
    group_name <- .resample_column(group, data)
    groups <- .resample_vector(group, data, "group")
    if (length(unique(groups)) < folds) {
      stop("Grouped resampling needs at least 'folds' distinct groups.", call. = FALSE)
    }
  }
  if (type == "temporal") {
    time_name <- .resample_column(time, data)
    ordering <- .resample_vector(time, data, "time")
    if (!(is.numeric(ordering) || inherits(ordering, c("Date", "POSIXt"))) ||
        any(!is.finite(as.numeric(ordering)))) {
      stop("'time' must contain finite numeric, Date, or POSIXct values.", call. = FALSE)
    }
    ordering <- as.numeric(ordering)
    if (length(unique(ordering)) < folds + 1L) {
      stop("Temporal resampling needs at least folds + 1 distinct times.", call. = FALSE)
    }
  }
  splits <- .with_seed(seed, {
    out <- vector("list", folds * repeats)
    for (r in seq_len(repeats)) {
      if (type == "stratified") {
        ids <- integer(n)
        for (status in c(0, 1)) {
          index <- which(y[, 2L] == status)
          if (length(index)) {
            allocation <- rep(seq_len(folds), length.out = length(index))
            ids[index] <- allocation[sample.int(length(index))]
          }
        }
        # Sparse strata can leave empty folds. Move one row, never omit it.
        for (empty in setdiff(seq_len(folds), ids)) {
          donor <- which.max(tabulate(ids, nbins = folds))
          ids[which(ids == donor)[1L]] <- empty
        }
      } else if (type == "grouped") {
        labels <- unique(groups)
        group_id <- match(groups, labels)
        sizes <- tabulate(group_id, nbins = length(labels))
        shuffled <- sample.int(length(labels))
        visit <- shuffled[order(-sizes[shuffled])]
        loads <- integer(folds)
        allocation <- integer(length(labels))
        for (g in visit) {
          choices <- which(loads == min(loads))
          chosen <- choices[sample.int(length(choices), 1L)]
          allocation[g] <- chosen
          loads[chosen] <- loads[chosen] + sizes[g]
        }
        ids <- allocation[group_id]
      } else {
        unique_times <- sort(unique(ordering))
        counts <- tabulate(rep(seq_len(folds + 1L), length.out = length(unique_times)),
          nbins = folds + 1L)
        blocks <- rep(seq_len(folds + 1L), times = counts)
        ids <- blocks[match(ordering, unique_times)]
      }
      for (k in seq_len(folds)) {
        index <- (r - 1L) * folds + k
        out[[index]] <- list(id = paste0("Repeat", r, ".Fold", k),
          repeat_id = r, fold = k,
          analysis = if (type == "temporal") which(ids <= k) else which(ids != k),
          assessment = which(ids == if (type == "temporal") k + 1L else k))
      }
    }
    out
  })
  plan <- structure(list(splits = splits, data.n = n, formula = formula,
    outcome = .resample_signature(y), row_names = row.names(data),
    type = type, folds = as.integer(folds), repeats = as.integer(repeats),
    seed = seed, group = groups, time = ordering,
    group_name = group_name, time_name = time_name), class = "survlearning_resamples")
  .validate_resamples(plan, formula, data)
}

.resample_column <- function(x, data) {
  if (is.character(x) && length(x) == 1L && !is.na(x) && x %in% names(data)) x else NULL
}

.resample_vector <- function(x, data, name) {
  column <- .resample_column(x, data)
  if (!is.null(column)) x <- data[[column]]
  if (is.null(x) || !is.atomic(x) || !is.null(dim(x)) ||
      length(x) != nrow(data) || anyNA(x)) {
    stop("'", name, "' must be a column name or a non-missing row-aligned vector.", call. = FALSE)
  }
  x
}

.resample_signature <- function(y) {
  unname(cbind(as.numeric(y[, 1L]), as.numeric(y[, 2L])))
}

.validate_resamples <- function(plan, formula, data) {
  if (!inherits(plan, "survlearning_resamples") || !is.list(plan) ||
      length(plan$data.n) != 1L || !is.numeric(plan$data.n) ||
      !is.finite(plan$data.n) || plan$data.n != nrow(data)) {
    stop("'resamples' must be a surv_resamples() plan with matching data.n.", call. = FALSE)
  }
  y <- .outcome(formula, data, require_event = FALSE)
  if (!identical(plan$outcome, .resample_signature(y)) ||
      !identical(plan$row_names, row.names(data)) ||
      !identical(deparse(plan$formula), deparse(formula))) {
    stop("Resampling formula/outcome signature or row order does not match 'data'.", call. = FALSE)
  }
  if (!is.character(plan$type) || length(plan$type) != 1L || is.na(plan$type) ||
      !plan$type %in% c("stratified", "grouped", "temporal") ||
      !is.list(plan$splits) || !length(plan$splits)) {
    stop("Invalid resampling type or empty splits.", call. = FALSE)
  }
  if (plan$type == "grouped") {
    groups <- .resample_vector(plan$group, data, "group")
    if (!is.null(plan$group_name) && !identical(groups, data[[plan$group_name]])) {
      stop("Resampling group values do not match 'data'.", call. = FALSE)
    }
  }
  if (plan$type == "temporal") {
    ordering <- .resample_vector(plan$time, data, "time")
    if (!is.numeric(ordering) || any(!is.finite(ordering)) ||
        (!is.null(plan$time_name) && !identical(ordering, as.numeric(data[[plan$time_name]])))) {
      stop("Resampling time values do not match 'data'.", call. = FALSE)
    }
  }
  diagnostics <- vector("list", length(plan$splits))
  ids <- character(length(plan$splits))
  for (i in seq_along(plan$splits)) {
    split <- plan$splits[[i]]
    if (!is.list(split) || !is.character(split$id) || length(split$id) != 1L ||
        is.na(split$id) || !nzchar(split$id)) {
      stop("Each resampling split needs a non-empty unique id.", call. = FALSE)
    }
    .integer_scalar(split$repeat_id, "repeat_id", 1L)
    .integer_scalar(split$fold, "fold", 1L)
    for (name in c("analysis", "assessment")) {
      x <- split[[name]]
      if (!is.numeric(x) || !length(x) || anyNA(x) || any(!is.finite(x)) ||
          any(x != floor(x)) || any(x < 1L | x > nrow(data)) || anyDuplicated(x)) {
        stop("Split '", split$id, "' has invalid ", name, " indices.", call. = FALSE)
      }
    }
    a <- split$analysis
    b <- split$assessment
    if (length(intersect(a, b))) stop("Analysis and assessment indices must be disjoint.", call. = FALSE)
    if (plan$type == "grouped" && length(intersect(groups[a], groups[b]))) {
      stop("Analysis and assessment groups must be disjoint.", call. = FALSE)
    }
    if (plan$type == "temporal" && max(ordering[a]) >= min(ordering[b])) {
      stop("Temporal analysis must strictly precede assessment, respecting time ties.", call. = FALSE)
    }
    ids[i] <- split$id
    na <- sum(y[a, 2L])
    nb <- sum(y[b, 2L])
    diagnostic <- c(if (!na) "No observed events in analysis.",
      if (!nb) "No observed events in assessment.")
    diagnostics[[i]] <- data.frame(resample = split$id,
      repeat_id = split$repeat_id, fold = split$fold,
      n_analysis = length(a), events_analysis = na,
      n_assessment = length(b), events_assessment = nb,
      diagnostic = paste(diagnostic, collapse = " "), stringsAsFactors = FALSE)
  }
  if (anyDuplicated(ids)) stop("Resampling split ids must be unique.", call. = FALSE)
  names(plan$splits) <- ids
  plan$diagnostics <- do.call(rbind, diagnostics)
  membership <- fold_id <- integer(nrow(data))
  for (split in plan$splits) {
    membership[split$assessment] <- membership[split$assessment] + 1L
    fold_id[split$assessment] <- split$fold
  }
  plan$fold_id <- if (all(membership == 1L)) fold_id else NULL
  plan
}

#' Nested, Paired Validation of Survival Learning Methods
#'
#' @inheritParams surv_resamples
#' @param methods Character vector of method identifiers.
#' @param grids Named list of explicit grids, one per method. Missing entries
#'   use one fixed configuration; no search ranges are invented.
#' @param params Named list of method-specific parameter lists.
#' @param resamples Explicit outer plan from [surv_resamples()].
#' @param inner_folds Number of inner folds when `tune = TRUE`.
#' @param tune Whether to tune on each outer analysis sample only.
#' @param metrics Metrics reported on outer assessments. Their order never
#'   changes parameter selection.
#' @param metric Scalar inner selection objective (default "harrell"), independent
#'   of metrics. For AUC/Brier selection provide one selection_time when outer
#'   evaluation uses several times.
#' @param selection_time Optional single horizon for AUC/Brier tuning only.
#' @param times Evaluation horizons passed to [surv_evaluate()].
#' @param tau Truncation horizon passed to [surv_evaluate()].
#' @param impute `"none"` or `"median"`; learned separately within each training fit.
#' @param censoring A formula/list censoring specification refitted on each
#'   analysis sample, including each inner fold. NULL retains marginal IPCW.
#' @details
#' All methods share outer and inner splits. Outer assessments never enter
#' parameter selection or preprocessing. Nested tuning with grouped or temporal
#' outer plans is explicitly rejected. Use `tune = FALSE` with prespecified
#' parameters for these plans; ordinary inner CV would violate their structure.
#' Explicit optimizer `reported_failure` diagnostics invalidate a fit. Warnings
#' alone do not; `not_reported` is not interpreted as confirmed convergence.
#'
#' Undefined and unsupported evaluations remain NA with diagnostics. An analysis
#' sample without events cannot be fitted; an assessment without events can
#' still have meaningful Brier scores. Warnings remain visible and are recorded.
#' Summary `mean`/`sd` use each method's finite splits; `mean_common`/`sd_common`
#' use the intersection across all methods. Do not compare available-case means
#' as if they used identical samples. Pairwise summaries use each pair's own
#' intersection and describe method-one minus method-two. All SDs are descriptive;
#' no standard errors or confidence intervals are invented from correlated folds.
#' @return A `survlearning_validate` list with `results`, `summary`,
#'   `paired_summary`, `warnings`, `failures`, `resamples`, `inner_resamples`, and
#'   `tuning` diagnostics. No final deployment model is fitted.
#' @export
#' @examples
#' validation <- surv_validate(survival::Surv(time, status) ~ age + karno,
#'   survival::veteran, folds = 3, inner_folds = 2)
#' validation$summary
surv_validate <- function(formula, data, methods = c("cox", "aft"),
                          grids = NULL, params = NULL, resamples = NULL,
                          folds = 5L, repeats = 1L, inner_folds = 3L,
                          tune = TRUE, metrics = "harrell", times = NULL,
                          tau = NULL, seed = NULL, impute = "none", censoring = NULL,
                          metric = "harrell", selection_time = NULL) {
  .validate_seed(seed)
  if (!is.character(methods) || !length(methods) || anyNA(methods) || anyDuplicated(methods)) {
    stop("'methods' must be distinct method names.", call. = FALSE)
  }
  methods <- vapply(methods, .method, character(1), USE.NAMES = FALSE)
  metrics <- .resample_metrics(metrics)
  metric <- .resample_metrics(metric)
  if (length(metric) != 1L) stop("Choose one scalar selection 'metric'.", call. = FALSE)
  .resample_preflight(metrics, times, tau)
  selection_times <- if (metric %in% c("auc", "brier") && !is.null(selection_time)) selection_time else times
  if (!is.null(selection_time) && !metric %in% c("auc", "brier")) {
    stop("'selection_time' is only used for AUC/Brier parameter selection.", call. = FALSE)
  }
  impute <- match.arg(impute, c("none", "median"))
  if (!is.logical(tune) || length(tune) != 1L || is.na(tune)) stop("'tune' must be TRUE or FALSE.", call. = FALSE)
  grids <- .resample_method_lists(grids, methods, "grids")
  params <- .resample_method_lists(params, methods, "params")
  if (!tune && length(grids)) stop("'grids' requires tune = TRUE.", call. = FALSE)
  if (is.null(resamples)) resamples <- surv_resamples(formula, data, folds, repeats, seed = seed)
  else resamples <- .validate_resamples(resamples, formula, data)
  if (tune && resamples$type != "stratified") {
    stop("Nested tuning for grouped or temporal resamples is unsupported; use tune = FALSE with prespecified parameters.", call. = FALSE)
  }
  if (tune) {
    .resample_preflight(metric, selection_times, tau, scalar = TRUE)
    .integer_scalar(inner_folds, "inner_folds", 2L)
    for (method in methods) .tune_grid(grids[[method]], method)
  }
  rows <- logs <- list()
  inner <- tuning <- vector("list", length(resamples$splits))
  names(inner) <- names(tuning) <- names(resamples$splits)
  template <- .resample_metric_template(metrics, times, tau)
  for (i in seq_along(resamples$splits)) {
    split <- resamples$splits[[i]]
    diagnostics <- resamples$diagnostics[i, , drop = FALSE]
    analysis <- data[split$analysis, , drop = FALSE]
    assessment <- data[split$assessment, , drop = FALSE]
    if (tune) {
      inner[[i]] <- .resample_capture(surv_resamples(formula, analysis, folds = inner_folds, seed = seed))
    }
    tuning[[i]] <- stats::setNames(vector("list", length(methods)), methods)
    for (method in methods) {
      start <- proc.time()[["elapsed"]]
      checked <- .resample_capture({
        .resample_check_events(diagnostics, method)
        setting <- params[[method]]
        if (is.null(setting)) setting <- list()
        if (tune) {
          if (nzchar(inner[[i]]$error)) stop(inner[[i]]$error, call. = FALSE)
          tuned <- surv_tune(formula, analysis, method = method,
            grid = grids[[method]], params = setting, seed = seed,
            resamples = inner[[i]]$value, metric = metric, times = selection_times, tau = tau,
            impute = impute, censoring = censoring)
          tuning[[i]][[method]] <- list(best_params = tuned$best_params,
            results = tuned$results, fold_results = tuned$fold_results)
          fitted <- tuned$best_model
        } else fitted <- .resample_fit(formula, analysis, method, setting, seed, impute)
        censor_fit <- .resample_censoring(censoring, analysis)
        list(evaluation = .resample_evaluate(fitted, assessment, metrics, times, tau, censor_fit), params = fitted$params)
      })
      if (nzchar(checked$error)) {
        evaluated <- template
        evaluated$estimate <- NA_real_
        evaluated$error <- evaluated$diagnostic <- checked$error
        evaluated$status <- "error"
        setting <- list()
        if (inherits(checked$condition, "survlearning_tune_error")) {
          tuning[[i]][[method]] <- list(results = checked$condition$results,
            fold_results = checked$condition$fold_results)
        }
      } else {
        evaluated <- checked$value$evaluation
        setting <- checked$value$params
      }
      evaluated$method <- method
      evaluated$resample <- split$id
      evaluated$repeat_id <- split$repeat_id
      evaluated$fold <- split$fold
      evaluated$n_analysis <- diagnostics$n_analysis
      evaluated$events_analysis <- diagnostics$events_analysis
      evaluated$n_assessment <- diagnostics$n_assessment
      evaluated$events_assessment <- diagnostics$events_assessment
      evaluated$seconds <- unname(proc.time()[["elapsed"]] - start)
      evaluated$warning <- paste(checked$warnings, collapse = "\n")
      evaluated$params <- rep(list(setting), nrow(evaluated))
      rows[[length(rows) + 1L]] <- evaluated
      if (length(checked$warnings)) logs[[length(logs) + 1L]] <- data.frame(
        method = method, resample = split$id, stage = "outer", warning = checked$warnings,
        stringsAsFactors = FALSE)
    }
  }
  results <- do.call(rbind, rows)
  row.names(results) <- NULL
  summaries <- .resample_summaries(results, methods)
  structure(list(results = results, summary = summaries$summary, paired_summary = summaries$paired,
    resamples = resamples,
    inner_resamples = lapply(inner, function(x) if (is.null(x)) NULL else x$value),
    tuning = tuning, warnings = if (length(logs)) do.call(rbind, logs) else
      data.frame(method = character(), resample = character(), stage = character(), warning = character()),
    failures = results[!is.finite(results$estimate), , drop = FALSE],
    methods = methods, metrics = metrics, seed = seed, impute = impute, tune = tune,
    censoring = censoring, metric = metric, selection_time = selection_time),
    class = "survlearning_validate")
}

.resample_method_lists <- function(x, methods, name) {
  if (is.null(x)) return(list())
  if (!is.list(x) || (length(x) && (is.null(names(x)) || anyNA(names(x)) ||
      any(!nzchar(names(x))) || anyDuplicated(names(x)))) || length(setdiff(names(x), methods))) {
    stop("'", name, "' must be a named list indexed by requested methods.", call. = FALSE)
  }
  x
}

.resample_metrics <- function(metrics) {
  if (!is.character(metrics) || !length(metrics) || anyNA(metrics) ||
      any(!nzchar(metrics)) || anyDuplicated(metrics)) stop("Choose distinct, non-empty metric names.", call. = FALSE)
  if (any(!metrics %in% c("harrell", "uno", "ipcw_c", "auc", "brier", "ibs"))) {
    stop("Unknown metric; choose harrell, uno, ipcw_c, auc, brier, or ibs.", call. = FALSE)
  }
  metrics
}

.resample_preflight <- function(metrics, times, tau, scalar = FALSE) {
  if (any(metrics %in% c("auc", "brier", "ibs"))) .evaluation_times(times)
  if ("ibs" %in% metrics && (length(times) < 2L || any(diff(times) <= 0))) {
    stop("IBS requires at least two strictly increasing horizons.", call. = FALSE)
  }
  if (scalar && any(metrics %in% c("auc", "brier")) && length(times) != 1L) {
    stop("Tuning requires one scalar metric horizon; supply a single time or selection_time.", call. = FALSE)
  }
  if (any(metrics %in% c("uno", "ipcw_c")) &&
      (!is.numeric(tau) || length(tau) != 1L || !is.finite(tau) || tau <= 0)) {
    stop("Uno/ipcw_c requires an explicit finite positive tau.", call. = FALSE)
  }
  invisible(NULL)
}

.resample_capture <- function(code) {
  warnings <- character()
  error <- ""
  condition <- NULL
  value <- tryCatch(withCallingHandlers(force(code), warning = function(w) {
    warnings <<- c(warnings, conditionMessage(w))
  }), error = function(e) {
    error <<- conditionMessage(e)
    condition <<- e
    NULL
  })
  list(value = value, error = error, condition = condition, warnings = warnings)
}

.resample_fit <- function(formula, data, method, params, seed, impute) {
  args <- list(formula = formula, data = data, method = method, params = params, seed = seed)
  if (impute != "none") args$impute <- impute
  fitted <- do.call(surv_fit, args)
  .resample_check_convergence(fitted)
  fitted
}

.resample_check_convergence <- function(fitted) {
  convergence <- fitted$diagnostics$convergence
  if (identical(convergence$status, "reported_failure")) {
    stop("Engine reported optimizer failure (code: ",
      paste(convergence$code, collapse = ", "), ").", call. = FALSE)
  }
  invisible(fitted)
}

.resample_check_events <- function(diagnostics, method = NULL) {
  if (!diagnostics$events_analysis && !identical(method, "km")) {
    stop("No observed events in analysis.", call. = FALSE)
  }
}

.resample_metric_template <- function(metrics, times, tau) {
  do.call(rbind, lapply(metrics, function(metric) {
    horizon <- if (metric %in% c("brier", "auc") && length(times)) times else NA_real_
    if (metric %in% c("uno", "ipcw_c") && length(tau) == 1L) horizon <- tau
    data.frame(metric = metric, time = as.numeric(horizon), stringsAsFactors = FALSE)
  }))
}

.resample_evaluate <- function(object, newdata, metrics, times, tau, censoring_model = NULL) {
  rows <- lapply(metrics, function(metric) {
    checked <- .resample_capture({
      if (metric == "harrell") {
        data.frame(metric = metric, time = NA_real_, estimate = surv_score(object, newdata),
          status = "ok", message = "")
      } else {
        args <- list(object = object, newdata = newdata, metrics = metric)
        if (!is.null(censoring_model)) args$censoring_model <- censoring_model
        if (metric %in% c("uno", "ipcw_c")) args$tau <- tau
        if (metric %in% c("auc", "brier", "ibs")) args$times <- times
        do.call(surv_evaluate, args)
      }
    })
    if (nzchar(checked$error)) {
      out <- .resample_metric_template(metric, times, tau)
      out$estimate <- NA_real_
      out$error <- out$diagnostic <- checked$error
      out$status <- "error"
      return(out)
    }
    out <- checked$value
    if (!is.data.frame(out) || !all(c("metric", "time", "estimate") %in% names(out)) ||
        !nrow(out) || anyNA(out$metric) || any(out$metric != metric) ||
        !is.numeric(out$estimate) || !is.numeric(out$time)) {
      stop("surv_evaluate() must return metric, time, and numeric estimate columns.", call. = FALSE)
    }
    if (!"diagnostic" %in% names(out)) {
      out$diagnostic <- if ("message" %in% names(out)) as.character(out$message) else ""
    }
    if (!"error" %in% names(out)) out$error <- ""
    if (!"status" %in% names(out)) out$status <- ifelse(is.finite(out$estimate), "ok", "undefined")
    out$diagnostic[is.na(out$diagnostic)] <- ""
    out$error[is.na(out$error)] <- ""
    invalid <- !is.finite(out$estimate)
    out$estimate[invalid] <- NA_real_
    out$diagnostic[invalid & !nzchar(out$diagnostic)] <- "Metric unavailable or unsupported."
    out$error[invalid & !nzchar(out$error)] <- out$diagnostic[invalid & !nzchar(out$error)]
    out[c("metric", "time", "estimate", "error", "diagnostic", "status")]
  })
  do.call(rbind, rows)
}

.resample_summaries <- function(results, methods) {
  keys <- unique(results[c("metric", "time")])
  summary <- paired <- list()
  for (i in seq_len(nrow(keys))) {
    same_time <- if (is.na(keys$time[i])) is.na(results$time) else
      !is.na(results$time) & results$time == keys$time[i]
    rows <- results[results$metric == keys$metric[i] & same_time, , drop = FALSE]
    ids <- unique(results$resample)
    scores <- matrix(NA_real_, nrow = length(ids), ncol = length(methods), dimnames = list(ids, methods))
    for (j in seq_len(nrow(rows))) scores[rows$resample[j], rows$method[j]] <- rows$estimate[j]
    complete <- apply(is.finite(scores), 1L, all)
    for (method in methods) {
      values <- scores[is.finite(scores[, method]), method]
      common <- scores[complete, method]
      summary[[length(summary) + 1L]] <- data.frame(method = method,
        metric = keys$metric[i], time = keys$time[i], n_total = length(ids),
        n_valid = length(values), n_paired = length(common),
        mean = if (length(values)) mean(values) else NA_real_,
        sd = if (length(values) > 1L) stats::sd(values) else NA_real_,
        mean_common = if (length(common)) mean(common) else NA_real_,
        sd_common = if (length(common) > 1L) stats::sd(common) else NA_real_)
    }
    if (length(methods) > 1L) for (a in seq_len(length(methods) - 1L)) {
      for (b in seq.int(a + 1L, length(methods))) {
        pair_complete <- is.finite(scores[, a]) & is.finite(scores[, b])
        difference <- scores[pair_complete, a] - scores[pair_complete, b]
        paired[[length(paired) + 1L]] <- data.frame(method1 = methods[a], method2 = methods[b],
          metric = keys$metric[i], time = keys$time[i], n_paired = length(difference),
          mean_difference = if (length(difference)) mean(difference) else NA_real_,
          sd_difference = if (length(difference) > 1L) stats::sd(difference) else NA_real_)
      }
    }
  }
  list(summary = do.call(rbind, summary), paired = if (length(paired)) do.call(rbind, paired) else
    data.frame(method1 = character(), method2 = character(), metric = character(), time = numeric(),
      n_paired = integer(), mean_difference = numeric(), sd_difference = numeric()))
}
