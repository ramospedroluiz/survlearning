#' @importFrom Rcpp evalCpp
NULL

.fit_gehan <- function(x, y, params, alpha) {
  singular_value <- if (min(dim(x)) == 1L) sqrt(sum(x^2)) else irlba::irlba(x, 1L)$d
  eta <- nrow(x) * singular_value^2
  if (!is.finite(eta) || eta <= 0) stop("Gehan ADMM needs a positive finite design norm.", call. = FALSE)
  result <- .Call("sl_gehan_fit", x, log(y[, 1L]), as.numeric(y[, 2L]),
    params$lambda, alpha, eta, params$max_iter, params$tolerance, PACKAGE = "survlearning")
  coefficients <- matrix(as.numeric(result$beta), ncol = 1L, dimnames = list(colnames(x), NULL))
  result$beta <- NULL
  if (!result$converged) warning("Gehan ADMM reached max_iter without satisfying both residual tolerances.", call. = FALSE)
  structure(list(beta = coefficients, lambda = params$lambda, alpha = alpha,
    standardize = FALSE, diagnostics = result,
    opt = list(convergence = if (result$converged) 0L else 1L)), class = "survlearning_gehan")
}

.gehan_coefficients <- function(fit, lambda) {
  position <- match(lambda, fit$lambda)
  if (is.na(position)) stop("The fitted Gehan penalty does not match the requested lambda.", call. = FALSE)
  as.numeric(fit$beta[, position])
}
