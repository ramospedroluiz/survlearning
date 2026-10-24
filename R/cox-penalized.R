.fit_penalized_cox <- function(x, y, lambda, alpha) {
  # A weak single-lambda cold start can fail numerically in highly censored data.
  initial <- glmnet::glmnet(x, y, family = "cox", alpha = alpha,
    standardize = TRUE, cox.ties = "breslow")
  upper <- max(c(initial$lambda, lambda))
  path <- if (upper == lambda) lambda else
    exp(seq(log(upper), log(lambda), length.out = 100L))
  path[length(path)] <- lambda
  fit <- glmnet::glmnet(x, y, family = "cox", alpha = alpha,
    lambda = path, standardize = TRUE, cox.ties = "breslow")
  if (fit$jerr != 0L || !any(fit$lambda == lambda)) {
    stop("Penalized Cox did not reach the requested lambda on its warm-start path.", call. = FALSE)
  }
  fit
}
