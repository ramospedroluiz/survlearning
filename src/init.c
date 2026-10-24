#include <R.h>
#include <Rinternals.h>
#include <R_ext/Rdynload.h>
#include <R_ext/Visibility.h>
extern SEXP sl_gehan_fit(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP sl_svm_rank(SEXP, SEXP, SEXP);

extern SEXP sl_rfsrcCIndex(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP sl_rfsrcCIndexFenwick(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP sl_rfsrcDistance(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP sl_rfsrcGrow(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP,
    SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP,
    SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP,
    SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP);
extern SEXP sl_rfsrcPredict(SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP,
    SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP,
    SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP,
    SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP,
    SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP,
    SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP, SEXP,
    SEXP, SEXP, SEXP);

static const R_CallMethodDef CallEntries[] = {
    {"sl_svm_rank", (DL_FUNC) &sl_svm_rank, 3},
    {"sl_gehan_fit", (DL_FUNC) &sl_gehan_fit, 8},
    {"sl_rfsrcCIndex", (DL_FUNC) &sl_rfsrcCIndex, 8},
    {"sl_rfsrcCIndexFenwick", (DL_FUNC) &sl_rfsrcCIndexFenwick, 8},
    {"sl_rfsrcDistance", (DL_FUNC) &sl_rfsrcDistance, 9},
    {"sl_rfsrcGrow", (DL_FUNC) &sl_rfsrcGrow, 38},
    {"sl_rfsrcPredict", (DL_FUNC) &sl_rfsrcPredict, 63},
    {NULL, NULL, 0}
};

void attribute_visible R_init_survlearning(DllInfo *dll) {
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}
