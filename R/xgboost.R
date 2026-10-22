.fit_xgboost <- function(method, x, y, params, seed) {
  if (method == "xgboost_cox") {
    labels <- ifelse(y[, 2L] == 1, y[, 1L], -y[, 1L])
    train <- xgboost::xgb.DMatrix(x, label = labels, nthread = params$threads)
  } else {
    train <- xgboost::xgb.DMatrix(x, label_lower_bound = y[, 1L],
      label_upper_bound = ifelse(y[, 2L] == 1, y[, 1L], Inf), nthread = params$threads)
  }
  settings <- list(objective = if (method == "xgboost_cox") "survival:cox" else "survival:aft",
    tree_method = "hist", max_depth = params$depth, eta = params$learning_rate,
    min_child_weight = params$min_child_weight, reg_lambda = params$leaf_penalty,
    nthread = params$threads, seed = seed, verbosity = 0L)
  if (method == "xgboost_aft") {
    settings$aft_loss_distribution <- params$distribution
    settings$aft_loss_distribution_scale <- params$scale
  }
  xgboost::xgb.train(settings, train, nrounds = params$trees, verbose = 0L)
}

.xgboost_margin <- function(engine, x) {
  lp <- as.numeric(stats::predict(engine, newdata = x, outputmargin = TRUE))
  if (any(!is.finite(lp))) stop("XGBoost log risks/locations must be finite.", call. = FALSE)
  lp
}

.xgboost_baseline <- function(engine, x, y) {
  lp <- .xgboost_margin(engine, x)
  if (length(lp) != nrow(y) || any(!is.finite(lp))) {
    stop("XGBoost baseline requires one finite margin per training row.", call. = FALSE)
  }
  # Center before log accumulation so a large common margin cannot erase log counts.
  center <- min(lp) / 2 + max(lp) / 2
  lp <- lp - center
  order <- order(y[, 1L])
  time <- y[order, 1L]
  first <- !duplicated(time)
  log_at_risk <- rev(.neural_logcumsumexp(rev(lp[order])))[first]
  events <- as.numeric(rowsum(y[order, 2L], cumsum(first), reorder = FALSE))
  # Zero-event times contribute -Inf, including late censor-only risk sets.
  log_increment <- log(events) - log_at_risk
  list(time = time[first], logcumhaz = .neural_logcumsumexp(log_increment),
    center = center, ties = "breslow")
}

.xgboost_survival <- function(object, x, times) {
  lp <- .xgboost_margin(object$engine, x)
  if (any(!is.finite(lp))) stop("XGBoost log risks/locations must be finite.", call. = FALSE)
  if (object$method == "xgboost_cox") {
    baseline <- object$baseline
    if (!is.null(baseline$logcumhaz)) {
      log_hazard <- baseline$logcumhaz
    } else {
      # Preserve finite saved baselines; a corrupted old baseline cannot be recovered here.
      if (any(!is.finite(baseline$cumhaz)) || any(baseline$cumhaz < 0)) {
        stop("Saved XGBoost baseline is non-finite; rebuild it from the original training data or refit the model.", call. = FALSE)
      }
      log_hazard <- log(baseline$cumhaz)
    }
    cumulative <- c(-Inf, log_hazard)[findInterval(times, baseline$time) + 1L]
    out <- exp(-exp(outer(lp - baseline$center, cumulative, "+")))
    out[, cumulative == -Inf] <- 1
    return(out)
  }
  z <- outer(lp, log(times), function(location, time) (time - location) / object$params$scale)
  switch(object$params$distribution,
    normal = stats::pnorm(z, lower.tail = FALSE),
    logistic = stats::plogis(z, lower.tail = FALSE),
    extreme = exp(-exp(z)))
}

.xgboost_median <- function(object, x) {
  error_median <- if (object$params$distribution == "extreme") log(log(2)) else 0
  exp(.xgboost_margin(object$engine, x) + object$params$scale * error_median)
}
