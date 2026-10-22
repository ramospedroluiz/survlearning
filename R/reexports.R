#' Survival Response Constructor
#'
#' Re-exported from survival for use in model formulas after loading only
#' survlearning. This package currently accepts right-censored responses with
#' positive times and event coding 0/1. See \code{\link[survival:Surv]{survival::Surv()}} for the complete
#' constructor documentation.
#' @inheritParams survival::Surv
#' @return An object of class \code{Surv}, implemented as a numeric matrix with
#'   one row per observation. A right-censored response has two columns, the
#'   observed time and event status, with a \code{type = "right"} attribute.
#'   Time units are those of the inputs. Other constructor types have the
#'   structure documented by survival, but are rejected by this package's
#'   modeling interface. The constructor does not fit a model or write files.
#' @name Surv
#' @rdname Surv
#' @importFrom survival Surv
#' @export
survival::Surv
