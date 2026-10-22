.outcome <- function(formula, data, require_event = FALSE) {
  if (!inherits(formula, "formula") || length(formula) != 3L) {
    stop("Use a two-sided formula: Surv(time, event) ~ predictors.", call. = FALSE)
  }
  if (!is.data.frame(data) || !nrow(data)) {
    stop("'data' must be a non-empty data.frame.", call. = FALSE)
  }
  response <- formula
  response[[3L]] <- 1
  frame <- stats::model.frame(response, data, na.action = stats::na.fail)
  y <- stats::model.response(frame)
  if (!inherits(y, "Surv") || attr(y, "type") != "right" || ncol(y) != 2L) {
    stop("Only right-censored Surv(time, event) responses are supported.", call. = FALSE)
  }
  if (any(!is.finite(y)) || any(y[, 1] <= 0)) {
    stop("Times must be finite and strictly positive; missing values are not imputed.", call. = FALSE)
  }
  # Surv() can infer 1/2 coding. Reject it rather than silently recoding an event.
  if (!is.call(formula[[2L]])) {
    stop("Use an explicit Surv(time, event) response in the formula.", call. = FALSE)
  }
  response_call <- match.call(survival::Surv, formula[[2L]])
  event_call <- if (!is.null(response_call$event)) response_call$event else response_call$time2
  if (is.null(event_call)) stop("Provide an explicit 0/1 event indicator.", call. = FALSE)
  event <- eval(event_call, data, environment(formula))
  if (!(is.numeric(event) || is.logical(event)) || anyNA(event) ||
      any(!event %in% c(0, 1))) {
    stop("Encode the event as 1 = event and 0 = right-censored.", call. = FALSE)
  }
  if (require_event && !sum(y[, 2])) {
    stop("At least one observed event is required.", call. = FALSE)
  }
  y
}

.survival_frame <- function(formula, data, require_event = TRUE) {
  .outcome(formula, data, require_event = require_event)
  if (nrow(data) < 2L) stop("Training needs at least two rows.", call. = FALSE)
  stats::model.frame(formula, data, na.action = stats::na.fail)
}

.fit_imputation <- function(formula, data, impute) {
  impute <- match.arg(impute, c("none", "median"))
  if (impute == "none") return(list(data = data, values = list()))
  variables <- all.vars(stats::delete.response(stats::terms(formula, data = data)))
  values <- list()
  for (name in variables) {
    x <- data[[name]]
    if (is.null(x)) next
    observed <- x[!is.na(x)]
    if (!length(observed)) {
      stop("Cannot impute predictor '", name, "': all training values are missing.", call. = FALSE)
    }
    if (is.numeric(x) && !is.object(x)) {
      if (any(!is.finite(observed))) stop("Predictors must be finite.", call. = FALSE)
      value <- stats::median(observed)
    } else if (is.factor(x) || is.character(x) || is.logical(x)) {
      choices <- sort(unique(as.character(observed)))
      value <- choices[which.max(tabulate(match(as.character(observed), choices)))]
      if (is.logical(x)) value <- identical(value, "TRUE")
    } else {
      stop("Median/mode imputation supports numeric, factor, character and logical predictors only.", call. = FALSE)
    }
    values[[name]] <- value
    data[[name]][is.na(x)] <- value
  }
  list(data = data, values = values)
}

.apply_imputation <- function(data, values) {
  for (name in names(values)) {
    if (!name %in% names(data)) next
    data[[name]][is.na(data[[name]])] <- values[[name]]
  }
  data
}

.prepare_design <- function(formula, data, impute = "none", allow_empty = FALSE,
                            require_event = TRUE) {
  .outcome(formula, data, require_event = require_event)
  imputation <- .fit_imputation(formula, data, impute)
  data <- imputation$data
  frame <- .survival_frame(formula, data, require_event = require_event)
  terms <- stats::delete.response(attr(frame, "terms"))
  if (.contains_call(formula[[3L]], c("strata", "cluster", "frailty", "tt"))) {
    stop("Strata, clusters, frailties, and time-dependent effects are not supported in this version.", call. = FALSE)
  }
  if (!attr(terms, "intercept") || length(attr(terms, "offset"))) {
    stop("This version requires the default intercept convention and does not support offsets.", call. = FALSE)
  }
  predictors <- frame[-1L]
  for (name in names(predictors)) {
    if (is.character(predictors[[name]])) predictors[[name]] <- factor(predictors[[name]])
    if (is.factor(predictors[[name]]) && nlevels(predictors[[name]]) < 2L) {
      stop("Factor '", name, "' needs at least two levels in training.", call. = FALSE)
    }
  }
  attr(predictors, "terms") <- terms
  x <- stats::model.matrix(terms, predictors)
  contrasts <- attr(x, "contrasts")
  xlevels <- lapply(predictors[vapply(predictors, is.factor, logical(1))], levels)
  x <- x[, colnames(x) != "(Intercept)", drop = FALSE]
  if ((!ncol(x) && !allow_empty) || any(!is.finite(x))) {
    stop("Provide at least one finite predictor; missing values are not imputed.", call. = FALSE)
  }
  moments <- .design_moments(x)
  center <- moments$center
  scale <- moments$scale
  keep <- is.finite(scale) & scale > 0
  if (!any(keep) && !allow_empty) stop("No varying predictors remain in training.", call. = FALSE)
  columns <- colnames(x)[keep]
  x <- x[, keep, drop = FALSE]
  design <- list(terms = terms, xlevels = xlevels, contrasts = contrasts,
    columns = columns, names = if (ncol(x)) paste0("x", seq_len(ncol(x))) else character(),
    center = center[keep], scale = scale[keep], imputation = imputation$values)
  colnames(x) <- design$names
  list(x = x, y = stats::model.response(frame), design = design)
}

.new_design <- function(design, newdata) {
  if (!is.data.frame(newdata) || !nrow(newdata)) {
    stop("'newdata' must be a non-empty data.frame.", call. = FALSE)
  }
  newdata <- .apply_imputation(newdata, design$imputation)
  frame <- stats::model.frame(design$terms, newdata, na.action = stats::na.fail,
    xlev = design$xlevels)
  x <- stats::model.matrix(design$terms, frame, contrasts.arg = design$contrasts)
  if (!all(design$columns %in% colnames(x)) || any(!is.finite(x))) {
    stop("New predictors must match the finite training design.", call. = FALSE)
  }
  x <- x[, design$columns, drop = FALSE]
  colnames(x) <- design$names
  x
}

.scaled_design <- function(x, design) {
  out <- sweep(sweep(x, 2L, design$center, "-"), 2L, design$scale, "/")
  if (any(!is.finite(out))) {
    # Division first avoids overflowing a representable standardized difference.
    out <- sweep(sweep(x, 2L, design$scale, "/"), 2L,
      design$center / design$scale, "-")
  }
  if (any(!is.finite(out))) stop("Standardized predictors are not representable as finite doubles.", call. = FALSE)
  out
}

.design_moments <- function(x) {
  center <- scale <- stats::setNames(numeric(ncol(x)), colnames(x))
  for (j in seq_len(ncol(x))) {
    column <- x[, j]
    if (all(column == column[1L])) {
      center[j] <- column[1L]
      scale[j] <- 0
    } else if (max(abs(column)) >= sqrt(.Machine$double.xmin) &&
               max(abs(column)) < sqrt(.Machine$double.xmax / nrow(x))) {
      center[j] <- mean(column)
      scale[j] <- stats::sd(column)
    } else {
      magnitude <- max(abs(column))
      unit <- column / magnitude
      center[j] <- mean(unit) * magnitude
      scale[j] <- stats::sd(unit) * magnitude
    }
    if (scale[j] == 0 && any(column != column[1L])) {
      stop("Positive predictor standard deviation is below the representable double range.", call. = FALSE)
    }
  }
  if (any(!is.finite(center)) || any(!is.finite(scale))) {
    stop("Predictor mean or standard deviation is not representable as a finite double.", call. = FALSE)
  }
  list(center = center, scale = scale)
}

.engine_data <- function(x, y = NULL) {
  out <- as.data.frame(x)
  if (!is.null(y)) {
    out$.sl_time <- as.numeric(y[, 1L])
    out$.sl_event <- as.integer(y[, 2L])
  }
  out
}

.engine_formula <- function() Surv(.sl_time, .sl_event) ~ .

.glmnet_design <- function(x) {
  # glmnet requires at least two columns; a zero column adds no fitted effect.
  if (ncol(x) == 1L) cbind(x, .sl_padding = 0) else x
}

.contains_call <- function(expr, names) {
  if (!is.call(expr)) return(FALSE)
  head <- expr[[1L]]
  name <- if (is.symbol(head)) as.character(head) else ""
  if (is.call(head) && identical(head[[1L]], as.name("::"))) {
    name <- as.character(head[[3L]])
  }
  name %in% names || any(vapply(as.list(expr)[-1L], .contains_call,
    logical(1), names = names))
}
