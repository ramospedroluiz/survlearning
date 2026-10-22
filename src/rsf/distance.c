/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "distance.h"
#include "stackOutput.h"
#include "nativeUtil.h"
#include "nrutil.h"
#include "error.h"
SEXP sl_rfsrcDistance(SEXP sexp_metricType,
                   SEXP sexp_n,
                   SEXP sexp_p,
                   SEXP sexp_x,
                   SEXP sexp_sizeIJ,
                   SEXP sexp_rowI,
                   SEXP sexp_rowJ,
                   SEXP sexp_numThreads,
                   SEXP sexp_traceFlag) {
  uint    traceFlag   = INTEGER(sexp_traceFlag)[0];
  setUserTraceFlag(traceFlag);
  setNativeGlobalEnv(&SL_RF_nativeIndex, &SL_RF_stackCount);
  uint    metricType  = INTEGER(sexp_metricType)[0];
  uint    n           = INTEGER(sexp_n)[0];
  uint    p           = INTEGER(sexp_p)[0];
  double *x           = REAL(sexp_x);
  uint    sizeIJ      = INTEGER(sexp_sizeIJ)[0];
  SL_RF_numThreads       = INTEGER(sexp_numThreads)[0];
  uint    *rowI;
  uint    *rowJ;
  double **xMatrix;
  double *dist;
  uint size;
  uint i, j, k;
  char  *sexpString[3] = {
    "",              
    "",              
    "distance"       
  };
  if (metricType != SL_RF_DISTANCE_EUCLIDEAN) {
    SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
    SL_RF_nativeError("\nRF-SRC:  Distance metric is invalid:  %10d \n", metricType);
    SL_RF_nativeExit();
  }
  if (n < 2) {
    SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
    SL_RF_nativeError("\nRF-SRC:  Matrix must have more than one (1) row:  %10d \n", n);
    SL_RF_nativeExit();
  }
#ifdef _OPENMP
  if (SL_RF_numThreads < 0) {
    SL_RF_numThreads = omp_get_max_threads();
  }
  else {
    SL_RF_numThreads = (SL_RF_numThreads < omp_get_max_threads()) ? (SL_RF_numThreads) : (omp_get_max_threads());
  }
#endif
  if (sizeIJ > 0) {
    rowI        = (uint*) INTEGER(sexp_rowI); rowI--;
    rowJ        = (uint*) INTEGER(sexp_rowJ); rowJ--;
    size = sizeIJ;
  }
  else {
    size = (n * (n-1)) >> 1;
    rowI = uivector(1, size);
    rowJ = uivector(1, size);
    k = 0;
    for (i = 1; i <= n; i++) {
      for (j = 1; j < i; j++) {
        k++;
        rowI[k] = i;
        rowJ[k] = j;
      }
    }
  }
  SL_RF_stackCount = 1;
  initProtect(SL_RF_stackCount);
  stackAuxiliaryInfoList(&SL_RF_snpAuxiliaryInfoList, SL_RF_stackCount);
  dist = (double*) stackAndProtect(SL_RF_GROW, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, 2, size, 0, sexpString[2], NULL, 1, size);
  dist --;
  xMatrix = (double **) new_vvector(1, p, NRUTIL_DPTR);
  for (i = 1; i <= p; i++) {
    xMatrix[i] = (x + ((i-1) * n) - 1);
  }
#ifdef _OPENMP
#pragma omp parallel for num_threads(SL_RF_numThreads)
#endif
  for (k = 1; k <= size; k++) {
    dist[k] = euclidean(n, p, rowI[k], rowJ[k], xMatrix);
  }
  free_new_vvector(xMatrix, 1, p, NRUTIL_DPTR);
  if (sizeIJ > 0) {
  }
  else {
    free_uivector(rowI, 1, size);
    free_uivector(rowJ, 1, size);
  }
  unstackAuxiliaryInfoAndList(FALSE, SL_RF_snpAuxiliaryInfoList, SL_RF_stackCount);
  R_ReleaseObject(SL_RF_sexpVector[SL_RF_OUTP_ID]);
  R_ReleaseObject(SL_RF_sexpVector[SL_RF_STRG_ID]);
  return SL_RF_sexpVector[SL_RF_OUTP_ID];
}
double euclidean(uint n, uint p, uint i, uint j, double **x) {
  double result;
  double difference;
  uint   k;
  result = 0.0;
  for (k = 1; k <= p; k++) {
    difference = x[k][i] - x[k][j];
    result += (difference * difference);
  }
  result = sqrt(result);
  return result;
}
