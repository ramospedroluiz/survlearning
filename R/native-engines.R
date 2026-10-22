#' @useDynLib survlearning, .registration = TRUE
NULL

.fit_rsf <- function(formula, data, params, seed) {
  fit <- .rsf_core$fit(formula, data = data, ntree = params$trees,
    mtry = params$mtry, nodesize = params$min_node, nsplit = params$splits,
    splitrule = "logrank", importance = FALSE, seed = seed)
  class(fit) <- c("survlearning_rsf", class(fit))
  fit
}

#' @rdname surv_predict
#' @export
predict.survlearning_rsf <- function(object, newdata, ...) {
  .predict_rsf(object, newdata, ...)
}

.predict_rsf <- function(object, newdata, ...) {
  class(object) <- setdiff(class(object), "survlearning_rsf")
  .rsf_core$predict(object, newdata = newdata, ...)
}
