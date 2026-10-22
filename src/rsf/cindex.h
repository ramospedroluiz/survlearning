/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */
#ifndef SL_RF_CINDEX_H
#define SL_RF_CINDEX_H
SEXP sl_rfsrcCIndex(SEXP sexp_traceFlag,
                 SEXP sexp_fastFlag,
                 SEXP sexp_size,
                 SEXP sexp_time,
                 SEXP sexp_censoring,
                 SEXP sexp_predicted,
                 SEXP sexp_denom,
                 SEXP sexp_weight);
SEXP sl_rfsrcCIndexFenwick(SEXP sexp_traceFlag,
                        SEXP sexp_eventType,
                        SEXP sexp_size,
                        SEXP sexp_time,
                        SEXP sexp_status,
                        SEXP sexp_predicted,
                        SEXP sexp_denom,
                        SEXP sexp_weight);
SEXP rfsrcTestSEXP(SEXP sexp_size);
#endif
