.tune_portable_formula <- function(formula, data) {
  if (!inherits(formula, "formula")) stop("Use a formula for portable tuning.", call. = FALSE)
  allowed <- c("Surv", "~", "+", "-", "*", "/", "^", ":", "(", "I", "log", "log1p",
    "sqrt", "exp", "poly", "ns", "bs", "factor", "as.factor", "scale", "==", "!=", ">", "<", ">=", "<=", "&", "|", "!")
  unknown <- setdiff(all.vars(formula), c(names(data), "."))
  if (length(unknown)) stop("Parallel/checkpoint formulas cannot capture external variables: ",
    paste(unknown, collapse = ", "), ". Put them in data or use serial tuning without checkpoints.", call. = FALSE)
  bindings <- new.env(parent = baseenv())
  walk <- function(expression) {
    if (!is.call(expression)) return(invisible(NULL))
    head <- expression[[1L]]
    if (is.call(head) && identical(head[[1L]], as.name("::"))) {
      package <- as.character(head[[2L]])
      name <- as.character(head[[3L]])
      if (!package %in% c("base", "stats", "survival", "splines") || !name %in% allowed) {
        stop("Unsupported function in parallel/checkpoint formula; use standard base/stats/survival/splines functions.", call. = FALSE)
      }
    } else {
      name <- as.character(head)
      if (length(name) != 1L || !name %in% allowed) stop("Custom formula functions are not portable for parallel/checkpoint tuning.", call. = FALSE)
      resolved <- get(name, envir = environment(formula), mode = "function", inherits = TRUE)
      canonical <- lapply(c("base", "stats", "survival", "splines"), function(package) {
        tryCatch(getExportedValue(package, name), error = function(e) NULL)
      })
      if (!any(vapply(canonical, identical, logical(1), y = resolved))) {
        stop("Shadowed/custom formula functions are not portable for parallel/checkpoint tuning.", call. = FALSE)
      }
      assign(name, resolved, envir = bindings)
    }
    for (argument in as.list(expression)[-1L]) walk(argument)
    invisible(NULL)
  }
  walk(formula)
  environment(formula) <- bindings
  formula
}

.tune_digest <- function(value) {
  path <- tempfile("survlearning-hash-")
  on.exit(unlink(path), add = TRUE)
  saveRDS(value, path, compress = FALSE, version = 2L)
  unname(tools::md5sum(path))
}

.tune_atomic_save <- function(value, target) {
  temporary <- tempfile(".survlearning-write-", tmpdir = dirname(target))
  backup <- NULL
  on.exit(unlink(temporary), add = TRUE)
  saveRDS(value, temporary, compress = FALSE)
  if (file.exists(target)) {
    backup <- tempfile(".survlearning-previous-", tmpdir = dirname(target))
    if (!file.rename(target, backup)) stop("Could not replace checkpoint file: ", target, call. = FALSE)
  }
  if (!file.rename(temporary, target)) {
    if (!is.null(backup)) file.rename(backup, target)
    stop("Could not commit checkpoint file: ", target, call. = FALSE)
  }
  if (!is.null(backup)) unlink(backup)
  invisible(NULL)
}

.tune_owns_lock <- function(state) {
  owner <- file.path(state$lock, "owner.rds")
  file.exists(owner) && identical(tryCatch(readRDS(owner), error = function(e) NULL), state$token)
}

.tune_commit <- function(value, target, state) {
  if (!.tune_owns_lock(state)) stop("Search no longer owns its checkpoint lock; no record was written.", call. = FALSE)
  .tune_atomic_save(value, target)
}

.tune_unlock <- function(state) {
  if (is.null(state)) return(invisible(NULL))
  lock <- normalizePath(state$lock, winslash = "/", mustWork = FALSE)
  root <- normalizePath(state$path, winslash = "/", mustWork = TRUE)
  expected <- normalizePath(file.path(root, ".survlearning-lock"), winslash = "/", mustWork = FALSE)
  if (!identical(tolower(lock), tolower(expected))) {
    stop("Refusing checkpoint lock cleanup outside its owned directory.", call. = FALSE)
  }
  if (file.exists(file.path(lock, "owner.rds")) && !.tune_owns_lock(state)) {
    stop("Refusing to remove another run's checkpoint lock.", call. = FALSE)
  }
  unlink(lock, recursive = TRUE)
  invisible(NULL)
}

.tune_checkpoint <- function(path, context, resume) {
  if (is.null(path)) return(NULL)
  if (!is.character(path) || length(path) != 1L || is.na(path) || !nzchar(path)) {
    stop("'checkpoint' must be one directory path or NULL.", call. = FALSE)
  }
  if (!dir.exists(path) && !dir.create(path, recursive = TRUE)) stop("Cannot create checkpoint directory.", call. = FALSE)
  path <- normalizePath(path, winslash = "/", mustWork = TRUE)
  lock <- file.path(path, ".survlearning-lock")
  if (!dir.create(lock, showWarnings = FALSE)) {
    stop("Checkpoint is locked. Do not share it between runs. After a hard process crash, verify no run is active before removing: ", lock, call. = FALSE)
  }
  state <- list(path = path, lock = lock, token = paste(Sys.getpid(), basename(tempfile()), sep = "-"))
  success <- FALSE
  on.exit(if (!success) .tune_unlock(state), add = TRUE)
  saveRDS(state$token, file.path(lock, "owner.rds"))
  dependencies <- unique(c("survlearning", "survival", .method_specs()[[context$method]]$engine,
    if (context$method == "svm") c("Matrix", "clarabel", "osqp")))
  versions <- stats::setNames(vapply(dependencies, function(package) {
    tryCatch(as.character(utils::packageVersion(package)), error = function(e) "not_installed")
  }, character(1)), dependencies)
  specification <- context
  specification$checkpoint <- NULL
  specification$formula <- paste(deparse(context$formula, width.cutoff = 500L), collapse = "\n")
  specification$resamples$formula <- paste(deparse(context$resamples$formula, width.cutoff = 500L), collapse = "\n")
  if (!is.null(specification$censoring)) {
    if (inherits(specification$censoring, "formula")) specification$censoring <- list(formula = specification$censoring)
    specification$censoring$formula <- paste(deparse(specification$censoring$formula, width.cutoff = 500L), collapse = "\n")
  }
  state$fingerprint <- .tune_digest(list(schema = 1L, specification = specification,
    versions = versions, R = R.version.string, platform = R.version$platform,
    RNG = RNGkind(), contrasts = getOption("contrasts")))
  metadata <- file.path(path, "metadata.rds")
  if (file.exists(metadata)) {
    old <- tryCatch(readRDS(metadata), error = function(e) stop("Unreadable checkpoint metadata.", call. = FALSE))
    if (!resume) stop("Checkpoint already exists; use resume = TRUE or a new directory.", call. = FALSE)
    if (!identical(old$fingerprint, state$fingerprint)) {
      stop("Checkpoint mismatch: data, row order, formula, folds, settings, or software versions changed. Use a new directory.", call. = FALSE)
    }
  } else {
    other <- setdiff(list.files(path, all.files = TRUE, no.. = TRUE), ".survlearning-lock")
    if (length(other)) stop("Refusing an unowned nonempty checkpoint directory.", call. = FALSE)
    .tune_atomic_save(list(schema = 1L, fingerprint = state$fingerprint,
      versions = versions, created = Sys.time()), metadata)
  }
  success <- TRUE
  state
}

.tune_record_path <- function(state, candidate, fold) {
  file.path(state$path, sprintf("candidate-%05d-fold-%05d.rds", candidate, fold))
}

.tune_read_record <- function(state, candidate, fold, resample) {
  if (is.null(state)) return(NULL)
  path <- .tune_record_path(state, candidate, fold)
  if (!file.exists(path)) return(NULL)
  record <- tryCatch(readRDS(path), error = function(e) stop("Unreadable checkpoint record: ", path, call. = FALSE))
  row <- record$row
  if (!identical(record$fingerprint, state$fingerprint) || !is.data.frame(row) || nrow(row) != 1L ||
      !all(c("candidate", "resample", "estimate", "error", "warning", "seconds") %in% names(row)) ||
      row$candidate != candidate || !identical(as.character(row$resample), as.character(resample))) {
    stop("Invalid or incompatible checkpoint record: ", path, call. = FALSE)
  }
  row
}

.tune_bundle <- function(bundle, context, state) {
  rows <- list()
  prepared <- svm_cache <- NULL
  current_group <- NULL
  fold <- bundle$fold[1L]
  split <- context$resamples$splits[[fold]]
  diagnostics <- context$resamples$diagnostics[fold, , drop = FALSE]
  analysis <- context$data[split$analysis, , drop = FALSE]
  assessment <- context$data[split$assessment, , drop = FALSE]
  censor_fit <- .resample_capture(.resample_censoring(context$censoring, analysis))
  for (j in seq_len(nrow(bundle))) {
    candidate <- bundle$candidate[j]
    setting <- .tune_setting(context$params, context$grid, candidate)
    group <- context$groups[candidate]
    if (context$shared && !identical(current_group, group)) {
      svm_cache <- new.env(parent = emptyenv())
      current_group <- group
    }
    start <- proc.time()[["elapsed"]]
    cache_hit <- FALSE
    checked <- .resample_capture({
      .resample_check_events(diagnostics, context$method)
      if (context$shared) {
        if (is.null(prepared)) prepared <- .prepare_design(context$formula, analysis, context$impute)
        fitted <- .surv_fit_prepared(context$formula, context$formula, prepared,
          context$method, setting, context$seed, context$impute, svm_cache)
        cache_hit <- fitted$engine$diagnostics$cache_hit
        .resample_check_convergence(fitted)
      } else fitted <- .resample_fit(context$formula, analysis, context$method,
        setting, context$seed, context$impute)
      if (nzchar(censor_fit$error)) stop(censor_fit$error, call. = FALSE)
      evaluation <- .resample_evaluate(fitted, assessment, context$metric,
        context$times, context$tau, censor_fit$value)
      if (nrow(evaluation) != 1L) stop("Tuning requires a scalar metric per split; choose one evaluation horizon.", call. = FALSE)
      if (!is.finite(evaluation$estimate)) stop(evaluation$diagnostic, call. = FALSE)
      evaluation
    })
    row <- data.frame(candidate = candidate, fold = split$fold,
      cindex = if (context$metric == "harrell" && !nzchar(checked$error)) checked$value$estimate else NA_real_,
      seconds = unname(proc.time()[["elapsed"]] - start), error = checked$error,
      resample = split$id, repeat_id = split$repeat_id, metric = context$metric,
      time = if (nzchar(checked$error)) NA_real_ else checked$value$time,
      estimate = if (nzchar(checked$error)) NA_real_ else checked$value$estimate,
      warning = paste(unique(c(censor_fit$warnings, checked$warnings)), collapse = "\n"),
      n_analysis = diagnostics$n_analysis, events_analysis = diagnostics$events_analysis,
      n_assessment = diagnostics$n_assessment, events_assessment = diagnostics$events_assessment,
      cache_hit = cache_hit, censoring_n = if (is.null(censor_fit$value)) NA_integer_ else censor_fit$value$n,
      stringsAsFactors = FALSE)
    if (!is.null(state)) .tune_commit(list(fingerprint = state$fingerprint, row = row),
      .tune_record_path(state, candidate, fold), state)
    rows[[as.character((candidate - 1L) * length(context$resamples$splits) + fold)]] <- row
  }
  rows
}

.tune_worker <- function(bundle, context, state) .tune_bundle(bundle, context, state)

.tune_execute <- function(context, jobs, state, workers, retry_failed, progress) {
  details <- vector("list", nrow(jobs))
  pending <- rep(TRUE, nrow(jobs))
  for (i in seq_len(nrow(jobs))) {
    candidate <- jobs$candidate[i]
    fold <- jobs$fold[i]
    record <- .tune_read_record(state, candidate, fold, context$resamples$splits[[fold]]$id)
    if (!is.null(record) && !(retry_failed && nzchar(record$error))) {
      index <- (candidate - 1L) * length(context$resamples$splits) + fold
      details[[index]] <- record
      pending[i] <- FALSE
    }
  }
  reused <- sum(!pending)
  bundles <- split(jobs[pending, , drop = FALSE], jobs$fold[pending])
  actual_workers <- if (length(bundles)) min(workers, length(bundles)) else 0L
  if (progress) message("Tuning: ", reused, "/", nrow(jobs), " checkpoint records reused.")
  if (actual_workers > 1L) {
    cluster <- .with_seed(context$seed, parallel::makePSOCKcluster(actual_workers,
      rscript_args = "--vanilla"))
    on.exit(parallel::stopCluster(cluster), add = TRUE)
    parallel::clusterCall(cluster, function(paths, version, kinds, contrasts) {
      .libPaths(paths)
      do.call(RNGkind, as.list(kinds))
      options(contrasts = contrasts)
      if (!requireNamespace("survlearning", quietly = TRUE) ||
          as.character(utils::packageVersion("survlearning")) != version) {
        stop("Install the current survlearning version before PSOCK tuning.")
      }
      TRUE
    }, .libPaths(), as.character(utils::packageVersion("survlearning")), RNGkind(), getOption("contrasts"))
  }
  done <- reused
  if (length(bundles)) for (start in seq.int(1L, length(bundles), by = max(1L, actual_workers))) {
    batch <- bundles[start:min(length(bundles), start + max(1L, actual_workers) - 1L)]
    computed <- if (actual_workers > 1L) parallel::parLapply(cluster, batch, .tune_worker,
      context = context, state = state) else lapply(batch, .tune_bundle, context = context, state = state)
    for (bundle in computed) for (index in names(bundle)) {
      details[[as.integer(index)]] <- bundle[[index]]
      if (actual_workers > 1L && nzchar(bundle[[index]]$warning)) warning(bundle[[index]]$warning, call. = FALSE)
      done <- done + 1L
    }
    if (progress) message("Tuning: ", done, "/", nrow(jobs), " candidate-fold fits completed.")
  }
  list(details = details, execution = list(workers_requested = workers,
    workers_used = actual_workers, resumed_jobs = reused, computed_jobs = nrow(jobs) - reused,
    checkpoint = if (is.null(state)) NULL else state$path,
    resumption = "same data, row order, grid, resamples, metric, settings, R/platform and engine versions"))
}
