.svm_kernel <- function(x, kernel, newx = NULL) {
  if (is.null(newx)) newx <- x
  if (kernel == "linear") return(tcrossprod(x, newx))
  if (kernel != "additive") stop("Unknown SVM kernel.", call. = FALSE)
  span <- vapply(seq_len(ncol(x)), function(j) max(x[, j]) - min(x[, j]), 0)
  if (!length(span) || any(!is.finite(span) | span <= 0)) {
    stop("The additive kernel cannot be applied to a constant column or non-finite range.", call. = FALSE)
  }
  out <- matrix(0, nrow(x), nrow(newx))
  for (j in seq_len(ncol(x))) out <- out + 1 - abs(outer(x[, j], newx[, j], `-`)) / span[j]
  out
}

.svm_memory_check <- function(estimate, params) {
  if (estimate > params$memory_limit_mb) {
    stop(sprintf(paste0("SVM workspace planning estimate %.0f MiB exceeds memory_limit_mb = %.0f. ",
      "No rows were removed. Raise the limit only on a machine with sufficient RAM."),
      estimate, params$memory_limit_mb), call. = FALSE)
  }
}

#' Predict Encoded Censored SVM Responses
#' @param object A `survlearning_svm` engine obtained as the engine of a fitted
#'   SVM. Use [surv_predict()] for original, unencoded predictors.
#' @param newdata Numeric matrix or data frame of finite encoded and standardized
#'   predictors, with the training column names and at least one row.
#' @param ... Reserved for S3 compatibility; extra arguments are rejected.
#' @return A list with numeric vector `predicted`, one value per row. These
#'   censored-regression scores are not calibrated survival times. Their negative
#'   is the package's increasing-risk score. Missing/invalid columns cause errors.
#'   Failed engines and engines from the former conditioned QP require refitting.
#' @export
predict.survlearning_svm <- function(object, newdata, ...) {
  if (length(list(...))) stop("Unused SVM prediction argument(s).", call. = FALSE)
  if (!identical(object$formulation, "censored_primal")) {
    stop("This SVM used the former conditioned quadratic program; refit with the censored primal.",
      call. = FALSE)
  }
  if (isFALSE(object$diagnostics$feasible) ||
      identical(object$diagnostics$status, "reported_failure") ||
      (!is.null(object$convergence) && object$convergence != 0)) {
    stop("Cannot predict from an SVM engine marked as infeasible or failed; refit the model.",
      call. = FALSE)
  }
  if (!is.data.frame(newdata) && !is.matrix(newdata)) {
    stop("Provide an encoded predictor data frame or matrix.", call. = FALSE)
  }
  columns <- colnames(object$sv)
  if (!is.null(columns)) {
    if (!all(columns %in% colnames(newdata))) stop("Encoded SVM predictors are missing.", call. = FALSE)
    newdata <- newdata[, columns, drop = FALSE]
  }
  x <- as.matrix(newdata)
  if (!is.numeric(x) || !nrow(x) || ncol(x) != ncol(object$sv) || any(!is.finite(x))) {
    stop("Encoded SVM predictors must be finite numeric values with the training columns.",
      call. = FALSE)
  }
  list(predicted = .svm_primal_predict(object, x))
}
