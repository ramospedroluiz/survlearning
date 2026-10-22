#' Censoring-Aware Evaluation of Survival Predictions
#'
#' @param object A fitted or tuned survlearning object.
#' @param newdata Evaluation data including outcomes and predictors.
#' @param metrics One or more of `"harrell"`, `"uno"`, `"auc"`, `"brier"`,
#'   `"ibs"`, and `"ipcw_c"`. Higher concordance/AUC and lower Brier/IBS are better.
#' @param times Finite non-negative horizons for AUC/Brier. IBS requires an
#'   explicit, strictly increasing grid of at least two supported horizons.
#' @param tau Required finite positive truncation horizon for Uno or ipcw_c.
#' @param censoring_data Optional reference data.frame. Only the outcome side
#'   of the fitted formula is evaluated, so predictors are unnecessary. Defaults
#'   to `object$training_outcome`; evaluation outcomes are never substituted.
#' @param min_g Minimum acceptable censoring survival, in `(0, 1]`.
#'   Unsupported horizons or smaller G values cause errors, not weight clipping.
#' @param n_boot Number of conditional test-case bootstrap replicates; zero
#'   disables confidence intervals. The fitted model and censoring KM stay fixed.
#' @param conf_level Confidence level for percentile bootstrap intervals.
#' @param seed NULL (default) uses the caller's RNG for bootstrap sampling.
#'   An integer >= 0 supplies a local seed and restores the caller's RNG state.
#' @param censoring_model Optional training/reference fit from [surv_censor_fit()].
#'   Mutually exclusive with censoring_data. Conditional G is used for Brier,
#'   IBS, weighted-case-and-control AUC, and ipcw_c. Uno remains the historical
#'   marginal-G statistic and is rejected with a conditional censoring model.
#' @details
#' Only right-censoring is supported, with event = I(T <= C). The reference
#' censoring distribution uses reverse Kaplan-Meier: at tied times, observed
#' events depart before censoring, so the censoring hazard denominator is
#' n_risk - d_event, not n_risk. Merely swapping the event indicator in an
#' ordinary KM is incorrect at event/censor ties. Event weights use the left
#' limit G(T-) = P(C >= T); controls use G(t) = P(C > t), after censoring at t.
#' These marginal IPCW estimates require independent censoring, also independent
#' of predictors, and a transportable censoring distribution between reference
#' and evaluation populations. A supplied conditional G instead requires
#' conditional independent censoring given its predictors, a correctly specified
#' censoring model, transportability, and adequate positive support.
#'
#' Harrell uses [survival::concordance()] with its tie convention (including
#' event/censor pairs tied in time) and numerical time-tie correction. Uno uses
#' strictly ordered pairs T_i < T_j with event i at T_i <= tau, weighted by
#' 1/G(T_i-)^2. Risk ties receive half credit. Uno requires a declared supported
#' tau; full follow-up is never chosen automatically. Its rank-tree calculation
#' takes O(n log n) time and O(n) memory.
#' The separately named ipcw_c uses strictly ordered pairs with weight
#' 1 / (G(T_i- | x_i) * G(T_i | x_j)). It is not labeled Uno under conditional
#' censoring. Equal-time outcomes are excluded and risk ties receive half
#' credit. Its time-group calculation avoids a dense pair matrix, but is
#' slower than the marginal Uno rank tree.
#'
#' Cumulative/dynamic AUC uses native risk scores (larger means earlier events),
#' weighted cases with T <= t and an event, and controls with T > t. Case and
#' control weights are 1/G(T-) and 1/G(t); risk ties receive half credit. It does
#' not require survival probabilities and uses sorted controls, not a pair matrix.
#' Brier requires native S(t) and is
#' `sum(I(T <= t, event) * S(t)^2 / G(T-) +
#' I(T > t) * (1 - S(t))^2 / G(t)) / n`.
#' Censored observations at or before t contribute zero but remain in n.
#' IBS is the trapezoidal Brier integral over the supplied grid divided by its
#' last minus first time. No implicit grid or zero-time interval is added.
#'
#' Probability predictions are never synthesized from risk scores. Horizons
#' cannot exceed reference or evaluation follow-up. Undefined metrics (e.g.,
#' AUC without cases/controls) return NA with diagnostics. Bootstrap intervals
#' condition on the fitted model and estimated G, not training uncertainty.
#' Undefined bootstrap replicates are counted and reported; intervals using
#' fewer than all replicates have an explicit diagnostic status.
#' @return A data.frame with class `survlearning_evaluation`, columns `metric`,
#'   `time`, `estimate`, `lower`, `upper`, `n`, `events`, `status`, `message`.
#'   Harrell and IBS have NA time; Uno has time tau. Attributes `censoring`,
#'   `settings`, and `assumptions` record G, its source, grids, and limitations.
#' @export
#' @examples
#' fit <- surv_fit(Surv(time, status) ~ age + karno, survival::veteran)
#' surv_evaluate(fit, survival::veteran, c("harrell", "auc", "brier"), times = 30)
surv_evaluate <- function(object, newdata, metrics = "harrell", times = NULL,
    tau = NULL, censoring_data = NULL, min_g = 0.05, n_boot = 0L,
    conf_level = .95, seed = NULL, censoring_model = NULL) {
  object <- .evaluation_object(object)
  choices <- c("harrell", "uno", "auc", "brier", "ibs", "ipcw_c")
  if (!is.character(metrics) || !length(metrics) || anyNA(metrics) ||
      any(!metrics %in% choices) || anyDuplicated(metrics)) {
    stop("'metrics' must contain distinct names from: ", paste(choices, collapse = ", "),
      ".", call. = FALSE)
  }
  .integer_scalar(n_boot, "n_boot")
  .validate_seed(seed)
  .evaluation_fraction(conf_level, "conf_level")
  .evaluation_fraction(min_g, "min_g", inclusive = TRUE)
  y <- .outcome(object$formula, newdata, require_event = FALSE)
  n <- nrow(y)
  horizons <- any(metrics %in% c("auc", "brier", "ibs"))
  if (horizons) {
    .evaluation_times(times)
    if ("ibs" %in% metrics && (length(times) < 2L || any(diff(times) <= 0))) {
      stop("IBS requires an explicit strictly increasing 'times' grid of at least two horizons.",
        call. = FALSE)
    }
    .evaluation_followup(times, max(y[, 1L]), "evaluation")
  } else if (!is.null(times)) {
    stop("'times' is only used by auc, brier, or ibs.", call. = FALSE)
  }
  if (any(c("uno", "ipcw_c") %in% metrics)) {
    if (!is.numeric(tau) || length(tau) != 1L || !is.finite(tau) || tau <= 0) {
      stop("Uno/ipcw_c requires an explicit finite positive 'tau'.", call. = FALSE)
    }
    .evaluation_followup(tau, max(y[, 1L]), "evaluation")
  } else if (!is.null(tau)) {
    stop("'tau' is only used by uno or ipcw_c.", call. = FALSE)
  }
  ipcw <- any(metrics != "harrell")
  censoring <- NULL
  conditional <- FALSE
  if (!is.null(censoring_model)) {
    if (!inherits(censoring_model, "survlearning_censoring")) {
      stop("'censoring_model' must come from surv_censor_fit().", call. = FALSE)
    }
    if (!is.null(censoring_data)) stop("Supply censoring_model or censoring_data, not both.", call. = FALSE)
    conditional <- censoring_model$method != "km"
    if (conditional && "uno" %in% metrics) {
      stop("Uno uses marginal G; choose metric = 'ipcw_c' for conditional pair weights.", call. = FALSE)
    }
  }
  if (ipcw || !is.null(censoring_data)) {
    if (!is.null(censoring_model)) censoring <- censoring_model else {
      reference <- if (is.null(censoring_data)) object$training_outcome else
        .outcome(object$formula, censoring_data, require_event = FALSE)
      .evaluation_reference(reference)
      censoring <- .evaluation_censoring(reference)
      censoring$source <- if (is.null(censoring_data)) "training_outcome" else "censoring_data"
    }
    requested <- c(if (horizons) times, if (any(c("uno", "ipcw_c") %in% metrics)) tau)
    .evaluation_followup(requested, censoring$max_time, "censoring reference")
    if (!conditional) .evaluation_g_support(.evaluation_g(censoring, requested), min_g)
  }
  risk <- NULL
  if (any(metrics %in% c("harrell", "uno", "auc", "ipcw_c"))) {
    risk <- surv_predict(object, newdata, type = "risk")
    if (!is.numeric(risk) || length(risk) != n || !is.null(dim(risk)) ||
        any(!is.finite(risk))) {
      stop("Risk predictions must be finite and aligned with evaluation outcomes.", call. = FALSE)
    }
  }
  probabilities <- NULL
  if (any(metrics %in% c("brier", "ibs"))) {
    probabilities <- .evaluation_probabilities(object, newdata, times)
  }
  g_event <- rep(1, n)
  g_time <- if (horizons) rep(1, length(times)) else numeric()
  if (ipcw) {
    needed <- y[, 2L] == 1 & y[, 1L] <= max(c(times, tau))
    if (any(needed)) g_event[needed] <- if (conditional) surv_censor_predict(censoring,
      newdata[needed, , drop = FALSE], y[needed, 1L], left = TRUE, pairwise = TRUE) else
      .evaluation_g(censoring, y[needed, 1L], left = TRUE)
    .evaluation_g_support(g_event[needed], min_g)
    if (horizons) {
      if (conditional) {
        g_time <- matrix(1, n, length(times))
        for (j in seq_along(times)) {
          controls <- y[, 1L] > times[j]
          if (any(controls)) {
            g_time[controls, j] <- surv_censor_predict(censoring,
              newdata[controls, , drop = FALSE], times[j])[, 1L]
            .evaluation_g_support(g_time[controls, j], min_g)
          }
        }
      } else g_time <- .evaluation_g(censoring, times)
    }
  }
  calculate <- function(index) .evaluation_values(y[index, , drop = FALSE],
    if (!is.null(risk)) risk[index] else NULL,
    if (!is.null(probabilities)) probabilities[index, , drop = FALSE] else NULL,
    metrics, times, tau, g_event[index],
    if (is.matrix(g_time)) g_time[index, , drop = FALSE] else g_time,
    pair_context = list(censoring = censoring, newdata = newdata[index, , drop = FALSE],
      conditional = conditional, min_g = min_g))
  out <- calculate(seq_len(n))
  out$lower <- out$upper <- NA_real_
  valid <- integer(nrow(out))
  if (n_boot > 0L) {
    boot <- .with_seed(seed, {
      values <- matrix(NA_real_, nrow = n_boot, ncol = nrow(out))
      for (b in seq_len(n_boot)) {
        values[b, ] <- calculate(sample.int(n, n, replace = TRUE))$estimate
      }
      values
    })
    valid <- colSums(is.finite(boot))
    for (j in seq_len(nrow(out))) {
      if (!is.finite(out$estimate[j])) next
      if (valid[j] >= 2L) {
        ci <- stats::quantile(boot[is.finite(boot[, j]), j],
          c((1 - conf_level) / 2, (1 + conf_level) / 2), names = FALSE)
        out$lower[j] <- ci[1L]
        out$upper[j] <- ci[2L]
      }
      if (valid[j] < n_boot || valid[j] < 2L) {
        out$status[j] <- if (valid[j] < 2L) "insufficient_bootstrap" else "partial_bootstrap"
        out$message[j] <- paste0(valid[j], "/", n_boot,
          " finite bootstrap replicates; intervals require at least two.")
      }
    }
  }
  out$n <- n
  out$events <- sum(y[, 2L])
  out <- out[, c("metric", "time", "estimate", "lower", "upper", "n",
    "events", "status", "message")]
  row.names(out) <- NULL
  class(out) <- c("survlearning_evaluation", "data.frame")
  attr(out, "censoring") <- censoring
  attr(out, "settings") <- list(metrics = metrics, times = times, tau = tau,
    min_g = min_g, n_boot = n_boot, conf_level = conf_level, seed = seed,
    bootstrap = "conditional test-case percentile; fixed event and censoring fits",
    bootstrap_valid = valid, ibs_interval = if ("ibs" %in% metrics) range(times) else NULL)
  attr(out, "assumptions") <- .evaluation_assumptions(conditional)
  out
}

.evaluation_object <- function(object) {
  if (inherits(object, "survlearning_tune")) object <- object$best_model
  if (!inherits(object, "survlearning_fit")) {
    stop("'object' must come from surv_fit() or surv_tune().", call. = FALSE)
  }
  object
}

.evaluation_fraction <- function(x, name, inclusive = FALSE) {
  if (!is.numeric(x) || length(x) != 1L || !is.finite(x) || x <= 0 ||
      if (inclusive) x > 1 else x >= 1) {
    stop("'", name, "' must be in ", if (inclusive) "(0, 1]." else "(0, 1).",
      call. = FALSE)
  }
}

.evaluation_times <- function(times) {
  if (!is.numeric(times) || !length(times) || any(!is.finite(times)) ||
      any(times < 0) || anyDuplicated(times)) {
    stop("Provide distinct finite non-negative 'times'.", call. = FALSE)
  }
}

.evaluation_followup <- function(times, maximum, source) {
  if (any(times > maximum)) {
    stop("Horizons exceed ", source, " follow-up (", format(maximum),
      "); no tail extrapolation is supported.", call. = FALSE)
  }
}

.evaluation_reference <- function(y) {
  if (is.null(y)) {
    stop("IPCW requires object$training_outcome or explicit 'censoring_data'; refit older objects.",
      call. = FALSE)
  }
  if (!inherits(y, "Surv") || !identical(attr(y, "type"), "right") ||
      ncol(y) != 2L || !nrow(y) || any(!is.finite(y)) ||
      any(y[, 1L] <= 0) || any(!y[, 2L] %in% c(0, 1))) {
    stop("Censoring reference must be a finite right-censored Surv outcome with positive times and 0/1 events.",
      call. = FALSE)
  }
}

.evaluation_censoring <- function(y) {
  fit <- survival::survfit(y ~ 1, se.fit = FALSE, timefix = FALSE)
  g_risk <- fit$n.risk - fit$n.event
  factor <- rep(1, length(fit$time))
  censored <- fit$n.censor > 0
  # Under event = I(T <= C), tied events leave before the censoring phase.
  # With no censors the factor is one, including a final all-event risk set.
  factor[censored] <- 1 - fit$n.censor[censored] / g_risk[censored]
  list(method = "marginal reverse Kaplan-Meier; events before tied censoring",
    n = nrow(y), events = sum(y[, 2L]), max_time = max(y[, 1L]),
    table = data.frame(time = fit$time, g = cumprod(factor), n_risk = fit$n.risk,
      events = fit$n.event, g_risk = g_risk, censored = fit$n.censor),
    event_limit = "G(T-)", horizon_limit = "G(t)",
    ties = "event = I(T <= C); censoring denominator n_risk - d_event; exact time equality")
}

.evaluation_g <- function(censoring, times, left = FALSE) {
  table <- censoring$table
  index <- findInterval(times, table$time)
  if (left) {
    tied <- index > 0L
    tied[tied] <- times[tied] == table$time[index[tied]]
    index[tied] <- index[tied] - 1L
  }
  c(1, table$g)[index + 1L]
}

.evaluation_g_support <- function(g, min_g) {
  if (any(!is.finite(g) | g <= 0 | g < min_g)) {
    stop("Insufficient censoring G support: required G is zero or below 'min_g'. Choose earlier horizons or a justified reference; IPCW is not clipped.",
      call. = FALSE)
  }
}

.evaluation_probabilities <- function(object, newdata, times) {
  p <- surv_predict(object, newdata, type = "survival", times = times)
  if (!is.matrix(p) || !is.numeric(p) ||
      !identical(dim(p), c(nrow(newdata), length(times))) ||
      any(!is.finite(p)) || any(p < 0 | p > 1)) {
    stop("Native survival probabilities must be a finite n-by-times matrix in [0, 1] at supported horizons.",
      call. = FALSE)
  }
  if (length(times) > 1L) {
    ordered <- p[, order(times), drop = FALSE]
    if (any(ordered[, -1L, drop = FALSE] >
        ordered[, -ncol(ordered), drop = FALSE] + 1e-8)) {
      stop("Native survival probabilities must be non-increasing over time.", call. = FALSE)
    }
  }
  p
}

.evaluation_assumptions <- function(conditional = FALSE) {
  c("Only right-censoring; no competing risks, delayed entry, or interval censoring.",
    if (conditional) "Conditional IPCW assumes T independent of C given the censoring predictors, a correctly specified G(t|x), and transportable reference G." else
      "Marginal IPCW assumes censoring independent of event time and predictors, with transportable reference G.",
    "No silent IPCW clipping; positive censoring support is required.",
    "Evaluation on training observations is resubstitution, not external validation.")
}

.evaluation_values <- function(y, risk, p, metrics, times, tau, g_event, g_time,
                               pair_context = NULL) {
  rows <- list()
  add <- function(metric, time, value, message) {
    rows[[length(rows) + 1L]] <<- data.frame(metric = metric, time = time,
      estimate = value, status = if (is.finite(value)) "ok" else "undefined",
      message = if (is.finite(value)) "" else message, stringsAsFactors = FALSE)
  }
  brier <- NULL
  if (any(metrics %in% c("brier", "ibs"))) {
    brier <- vapply(seq_along(times), function(j) {
      cases <- y[, 2L] == 1 & y[, 1L] <= times[j]
      controls <- y[, 1L] > times[j]
      if (!any(cases | controls)) return(NA_real_)
      (sum(p[cases, j]^2 / g_event[cases]) +
        sum((1 - p[controls, j])^2 / if (is.matrix(g_time)) g_time[controls, j] else g_time[j])) / nrow(y)
    }, numeric(1))
  }
  for (metric in metrics) {
    if (metric == "harrell") {
      value <- if (nrow(y) < 2L || !sum(y[, 2L])) NA_real_ else
        unname(survival::concordance(y ~ risk, reverse = TRUE)$concordance)
      if (!is.finite(value)) value <- NA_real_
      add(metric, NA_real_, value, "No comparable pairs for Harrell concordance.")
    } else if (metric == "uno") {
      w <- ifelse(y[, 2L] == 1 & y[, 1L] <= tau, 1 / g_event^2, 0)
      add(metric, tau, .evaluation_uno(y[, 1L], risk, w),
        "No strictly ordered comparable pairs with an event at or before tau.")
    } else if (metric == "auc") {
      for (j in seq_along(times)) {
        cases <- y[, 2L] == 1 & y[, 1L] <= times[j]
        controls <- y[, 1L] > times[j]
        value <- .evaluation_auc(risk[cases], risk[controls], 1 / g_event[cases],
          if (is.matrix(g_time)) 1 / g_time[controls, j] else rep(1, sum(controls)))
        add(metric, times[j], value, "AUC needs observed cases by the horizon and controls beyond it.")
      }
    } else if (metric == "ipcw_c") {
      add(metric, tau, .evaluation_ipcw_pairs(y, risk, tau, g_event, pair_context),
        "No strictly ordered comparable pairs with supported conditional weights.")
    } else if (metric == "brier") {
      for (j in seq_along(times)) add(metric, times[j], brier[j],
        "No observed cases or controls at the Brier horizon.")
    } else {
      value <- sum(diff(times) * (brier[-1L] + brier[-length(brier)]) / 2) /
        (times[length(times)] - times[1L])
      add(metric, NA_real_, value, "IBS requires defined Brier scores over the entire grid.")
    }
  }
  do.call(rbind, rows)
}

.evaluation_auc <- function(cases, controls, weights, control_weights = rep(1, length(controls))) {
  if (!length(cases) || !length(controls)) return(NA_real_)
  ordering <- order(controls)
  controls <- controls[ordering]
  control_weights <- control_weights[ordering]
  # findInterval includes equal controls; subtract half of their multiplicity.
  right <- findInterval(cases, controls)
  levels <- unique(controls)
  counts <- as.vector(rowsum(control_weights, match(controls, levels), reorder = FALSE))
  tied <- counts[match(cases, levels)]
  tied[is.na(tied)] <- 0
  below_or_equal <- c(0, cumsum(control_weights))[right + 1L]
  sum(weights * (below_or_equal - tied / 2)) / (sum(weights) * sum(control_weights))
}

.evaluation_ipcw_pairs <- function(y, risk, tau, g_event, context) {
  numerator <- denominator <- 0
  event <- y[, 2L] == 1 & y[, 1L] <= tau
  for (time in sort(unique(y[event, 1L]))) {
    cases <- which(event & y[, 1L] == time)
    controls <- which(y[, 1L] > time)
    if (!length(controls)) next
    g <- if (context$conditional) surv_censor_predict(context$censoring,
      context$newdata[controls, , drop = FALSE], time)[, 1L] else
      rep(.evaluation_g(context$censoring, time), length(controls))
    .evaluation_g_support(g, context$min_g)
    case_weights <- 1 / g_event[cases]
    control_weights <- 1 / g
    mass <- sum(case_weights) * sum(control_weights)
    numerator <- numerator + mass * .evaluation_auc(risk[cases], risk[controls],
      case_weights, control_weights)
    denominator <- denominator + mass
  }
  if (denominator > 0) numerator / denominator else NA_real_
}

.evaluation_uno <- function(time, risk, weights) {
  ranks <- match(risk, sort(unique(risk)))
  m <- max(ranks)
  tree <- numeric(m)
  sorted <- order(time, decreasing = TRUE)
  sizes <- rle(time[sorted])$lengths
  end <- cumsum(sizes)
  start <- c(1L, utils::head(end, -1L) + 1L)
  numerator <- denominator <- later <- 0
  for (block in seq_along(start)) {
    index <- sorted[start[block]:end[block]]
    cases <- index[weights[index] > 0]
    for (i in cases) {
      k <- ranks[i]
      below <- equal_or_below <- 0
      while (k > 0L) {
        equal_or_below <- equal_or_below + tree[k]
        k <- k - bitwAnd(k, -k)
      }
      k <- ranks[i] - 1L
      while (k > 0L) {
        below <- below + tree[k]
        k <- k - bitwAnd(k, -k)
      }
      numerator <- numerator + weights[i] * (below + equal_or_below) / 2
      denominator <- denominator + weights[i] * later
    }
    # Insert the whole block only after querying: no equal-time Uno pairs.
    for (i in index) {
      k <- ranks[i]
      while (k <= m) {
        tree[k] <- tree[k] + 1
        k <- k + bitwAnd(k, -k)
      }
    }
    later <- later + length(index)
  }
  if (denominator > 0) numerator / denominator else NA_real_
}

#' Grouped Survival Calibration
#'
#' @param object A fitted or tuned survlearning object with native probabilities.
#' @param newdata Evaluation data including outcomes and predictors.
#' @param times Distinct finite non-negative supported horizons.
#' @param groups Requested number of quantile groups at each horizon.
#' @param censoring_model Optional training/reference fit from [surv_censor_fit()].
#'   When supplied, use normalized IPCW case/control calibration instead of KM.
#' @param min_g Minimum censoring survival; smaller values cause errors, not clipping.
#' @param n_boot Assessment bootstrap repetitions for IPCW intervals; default 0
#'   reports point estimates only. The event/G fits and original groups stay fixed.
#' @param conf_level Interval confidence level.
#' @param seed NULL (default) uses the caller's RNG; an integer >= 0 supplies
#'   a local bootstrap seed and restores the caller's RNG state.
#' @details Groups are formed separately at each horizon using empirical
#'   midrank quantile bins in predicted survival. Identical probabilities are
#'   never split; fewer than
#'   `groups` groups can result. Without a censoring model, observed survival
#'   is a group-specific KM, with log-log Greenwood intervals at `conf_level`
#'   from [survival::survfit()]. No KM
#'   tail extrapolation is performed. Groups with fewer than 10 observations,
#'   fewer than 3 events by the horizon, or fewer than 5 at risk trigger a low
#'   support warning. Grouping is descriptive, not a test of calibration or a
#'   recalibration procedure; within-group censoring must be independent.
#'   Without censoring_model, covariate-dependent censoring is not adjusted.
#'   With a supplied model, observed survival is the normalized IPCW control
#'   mass divided by case plus control mass. Censoring assumptions, positive
#'   support and reference transportability are required. Bootstrap intervals
#'   exclude event/G fitting and group-selection uncertainty. Risk scores are not converted
#'   to fake probabilities. Training-data calibration is resubstitution.
#' @return A `survlearning_calibration` data.frame with `time`, `group`,
#'   `predicted` (mean predicted survival), `observed` (KM survival without a
#'   censoring model, normalized IPCW survival otherwise), `lower`,
#'   `upper`, `n`, `events` (events by the horizon), `n_risk` (just before the
#'   horizon), `status`, and `message`. Unsupported group tails are NA.
#'   The IPCW branch also returns `effective_n` (squared total weight divided
#'   by the sum of squared weights) and `bootstrap_valid` (number of usable
#'   bootstrap estimates). Its intervals are percentile bootstrap intervals
#'   at `conf_level`, or NA when `n_boot = 0` or bootstrap support is inadequate.
#' @export
#' @examples
#' fit <- surv_fit(Surv(time, status) ~ age + karno, survival::veteran)
#' calibration <- surv_calibrate(fit, survival::veteran, times = 30, groups = 3)
#' plot(calibration)
surv_calibrate <- function(object, newdata, times, groups = 5L,
    censoring_model = NULL, min_g = .05, n_boot = 0L, conf_level = .95, seed = NULL) {
  object <- .evaluation_object(object)
  .evaluation_times(times)
  .integer_scalar(groups, "groups", 1L)
  .evaluation_fraction(conf_level, "conf_level")
  .evaluation_fraction(min_g, "min_g", inclusive = TRUE)
  .integer_scalar(n_boot, "n_boot")
  .validate_seed(seed)
  if (n_boot == 1L) stop("Use n_boot = 0 or at least two bootstrap repetitions.", call. = FALSE)
  y <- .outcome(object$formula, newdata, require_event = FALSE)
  .evaluation_followup(times, max(y[, 1L]), "evaluation")
  p <- .evaluation_probabilities(object, newdata, times)
  if (!is.null(censoring_model)) return(.calibration_ipcw(object, newdata, y, p,
    times, groups, censoring_model, min_g, n_boot, conf_level, seed))
  if (n_boot > 0L) stop("Bootstrap calibration requires an explicit censoring_model; ordinary KM uses Greenwood intervals.", call. = FALSE)
  rows <- list()
  for (j in seq_along(times)) {
    bins <- min(groups, nrow(y))
    group <- floor((rank(p[, j], ties.method = "average") - .5) * bins / nrow(y)) + 1L
    occupied <- sort(unique(group))
    for (g in seq_along(occupied)) {
      index <- which(group == occupied[g])
      outcome <- y[index, , drop = FALSE]
      fit <- survival::survfit(outcome ~ 1, conf.int = conf_level, conf.type = "log-log",
        timefix = FALSE)
      at <- findInterval(times[j], fit$time)
      supported <- times[j] <= max(outcome[, 1L])
      observed <- lower <- upper <- NA_real_
      if (supported) {
        observed <- c(1, fit$surv)[at + 1L]
        lower <- c(1, fit$lower)[at + 1L]
        upper <- c(1, fit$upper)[at + 1L]
      }
      n <- length(index)
      events <- sum(outcome[, 2L] == 1 & outcome[, 1L] <= times[j])
      n_risk <- sum(outcome[, 1L] >= times[j])
      reasons <- c(if (!supported) "Horizon exceeds group follow-up; KM tail not extrapolated.",
        if (n < 10L) "Fewer than 10 subjects.",
        if (events < 3L) "Fewer than 3 events by horizon.",
        if (n_risk < 5L) "Fewer than 5 at risk.",
        if (length(occupied) < groups) "Fewer groups than requested; tied predictions are not split.")
      rows[[length(rows) + 1L]] <- data.frame(time = times[j], group = g,
        predicted = mean(p[index, j]), observed = observed, lower = lower,
        upper = upper, n = n, events = events, n_risk = n_risk,
        status = if (!supported) "unsupported" else if (length(reasons)) "low_support" else "ok",
        message = paste(reasons, collapse = " "), stringsAsFactors = FALSE)
    }
  }
  out <- do.call(rbind, rows)
  row.names(out) <- NULL
  if (any(out$status != "ok")) {
    warning("Low support or collapsed groups in survival calibration; inspect status and message.",
      call. = FALSE)
  }
  class(out) <- c("survlearning_calibration", "data.frame")
  attr(out, "settings") <- list(times = times, groups = groups,
    conf_level = conf_level, conf_type = "log-log", estimator = "group-specific KM",
    grouping = "per-horizon predicted survival midrank quantile bins; ties kept together")
  attr(out, "assumptions") <- c("Independent censoring within each calibration group; no covariate-conditional adjustment.",
    "Descriptive grouped calibration; intervals condition on the fitted model and observed grouping.",
    "Training-data calibration is resubstitution, not external validation.")
  out
}

#' Plot Grouped Survival Calibration
#'
#' @param x A result from [surv_calibrate()].
#' @param time Optional single horizon to plot; NULL plots all horizons.
#' @param xlab,ylab Axis labels.
#' @param xlim,ylim Axis ranges, defaulting to `[0, 1]`.
#' @param ... Additional arguments to [graphics::plot.default()].
#' @return Invisibly, `x`.
#' @export
plot.survlearning_calibration <- function(x, time = NULL,
    xlab = "Mean predicted survival", ylab = NULL,
    xlim = c(0, 1), ylim = c(0, 1), ...) {
  selected <- x
  if (is.null(ylab)) ylab <- if (identical(attr(x, "settings")$estimator, "normalized IPCW"))
    "IPCW observed survival" else "Kaplan-Meier survival"
  if (!is.null(time)) {
    if (!is.numeric(time) || length(time) != 1L || !is.finite(time) ||
        !time %in% x$time) stop("Select a 'time' present in the calibration result.", call. = FALSE)
    selected <- x[x$time == time, , drop = FALSE]
  }
  horizons <- unique(selected$time)
  color <- match(selected$time, horizons)
  graphics::plot(NA_real_, NA_real_, xlab = xlab, ylab = ylab,
    xlim = xlim, ylim = ylim, ...)
  graphics::abline(a = 0, b = 1, lty = 2, col = "grey60")
  valid <- is.finite(selected$observed)
  interval <- valid & is.finite(selected$lower) & is.finite(selected$upper)
  graphics::segments(selected$predicted[interval], selected$lower[interval],
    selected$predicted[interval], selected$upper[interval], col = color[interval])
  graphics::points(selected$predicted[valid], selected$observed[valid], pch = 19,
    col = color[valid])
  if (length(horizons) > 1L) graphics::legend("topleft", legend = paste0("t = ", horizons),
    col = seq_along(horizons), pch = 19, bty = "n")
  invisible(x)
}
