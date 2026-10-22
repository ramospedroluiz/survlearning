#' Evaluate Harrell's Concordance Index
#'
#' @param object A fitted or tuned survlearning object.
#' @param newdata Evaluation data including the outcome and predictors.
#' @return A single numeric C-index. Larger risk scores always mean earlier
#'   events. This is discrimination, not probability calibration or an event
#'   detection rate. Training-data scores are resubstitution estimates.
#'   An error is raised for non-finite scores or when no comparable pair exists,
#'   including a completely censored assessment sample.
#' @export
#' @examples
#' fit <- surv_fit(survival::Surv(time, status) ~ age + karno,
#'   survival::veteran, method = "cox")
#' surv_score(fit, survival::veteran)
surv_score <- function(object, newdata) {
  if (inherits(object, "survlearning_tune")) object <- object$best_model
  if (!inherits(object, "survlearning_fit")) {
    stop("'object' must come from surv_fit() or surv_tune().", call. = FALSE)
  }
  .cindex(.outcome(object$formula, newdata), surv_predict(object, newdata))
}

.cindex <- function(y, risk) {
  if (length(risk) != nrow(y) || any(!is.finite(risk))) {
    stop("Risk predictions must be finite and aligned with the outcome.", call. = FALSE)
  }
  score <- survival::concordance(y ~ risk, reverse = TRUE)$concordance
  if (!is.finite(score)) stop("C-index is undefined: no comparable pairs.", call. = FALSE)
  unname(score)
}
