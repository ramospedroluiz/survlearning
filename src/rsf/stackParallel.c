/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "stackParallel.h"
#include "nrutil.h"
#ifdef _OPENMP
void stackLocksOpenMP(char mode) {
  uint i, j;
  omp_init_lock(&SL_RF_lockEnsbUpdtCount);
  omp_init_lock(&SL_RF_lockPerf);
  if (SL_RF_optHigh & OPT_PART_PLOT) {
    SL_RF_lockPartial = ompvector(1, SL_RF_observationSize);
    for (i = 1; i <= SL_RF_observationSize; i++) {
      omp_init_lock(&(SL_RF_lockPartial[i]));
    }
  }
  if (SL_RF_optHigh & OPT_WGHT) {
    uint gMembershipSize;
    if((SL_RF_optHigh & OPT_WGHT_IBG) && (SL_RF_optHigh & OPT_WGHT_OOB)) {
      switch (mode) {
      case SL_RF_PRED:
        gMembershipSize = SL_RF_fobservationSize;
        break;
      default:
        gMembershipSize = SL_RF_observationSize;
        break;
      }
    }
    else {
      gMembershipSize  = SL_RF_observationSize;
    }
    SL_RF_lockWeight = (omp_lock_t **) new_vvector(1, gMembershipSize, NRUTIL_OMPLPTR);
    for (i = 1; i <= gMembershipSize; i++) {
      SL_RF_lockWeight[i] = ompvector(1, SL_RF_observationSize);
    }
    for (i = 1; i <= gMembershipSize; i++) {
      for (j = 1; j <= SL_RF_observationSize; j++) {
        omp_init_lock(&(SL_RF_lockWeight[i][j]));
      }
    }
    SL_RF_lockWeightRow = ompvector(1, gMembershipSize);
    for (i = 1; i <= gMembershipSize; i++) {
      omp_init_lock(&(SL_RF_lockWeightRow[i]));
    }
  }
  if (SL_RF_opt & OPT_VIMP) {
      uint obsSize, xVimpSize;
      if (SL_RF_opt & OPT_VIMP_JOIN) {
        xVimpSize = 1;
      }
      else {
        xVimpSize = SL_RF_intrPredictorSize;
      }
      switch (mode) {
      case SL_RF_PRED:
        obsSize = SL_RF_fobservationSize;
        break;
      default:
        obsSize  = SL_RF_observationSize;
        break;
      }
      SL_RF_lockVimp = (omp_lock_t **) new_vvector(1, xVimpSize, NRUTIL_OMPLPTR);
      for (i = 1; i <= xVimpSize; i++) {
        SL_RF_lockVimp[i] = ompvector(1, obsSize);
      }
      for (i = 1; i <= xVimpSize; i++) {
        for (j = 1; j <= obsSize; j++) {
          omp_init_lock(&(SL_RF_lockVimp[i][j]));
        }
      }
      SL_RF_lockVimpRow = ompvector(1, obsSize);
      for (i = 1; i <= obsSize; i++) {
        omp_init_lock(&(SL_RF_lockVimpRow[i]));
      }
      SL_RF_lockVimpCol = ompvector(1, xVimpSize);
      for (i = 1; i <= xVimpSize; i++) {
        omp_init_lock(&(SL_RF_lockVimpCol[i]));
      }
  }
  if ((SL_RF_vtry > 0) && (SL_RF_vtryMode != SL_RF_VTRY_NULL)) {
    uint xVimpSize;
    xVimpSize = SL_RF_xSize;
    SL_RF_lockVimpHoldout = (omp_lock_t **) new_vvector(1, xVimpSize, NRUTIL_OMPLPTR);
    for (i = 1; i <= xVimpSize; i++) {
      if (SL_RF_holdBLKptr[i] > 0) {
        SL_RF_lockVimpHoldout[i] = ompvector(1, SL_RF_holdBLKptr[i]);
        for (j = 1; j <= SL_RF_holdBLKptr[i]; j++) {
          omp_init_lock(&(SL_RF_lockVimpHoldout[i][j]));
        }
      }
    }
  }
  if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
    omp_lock_t   **lockDENptr;
    uint obsSize;  
    char oobFlag, fullFlag;
    oobFlag = fullFlag = FALSE;
    switch (mode) {
    case SL_RF_PRED:
      if (SL_RF_opt & OPT_FENS) {
        fullFlag = TRUE;
      }
      break;
    default:
      if (SL_RF_opt & OPT_OENS) {
        oobFlag = TRUE;
      }
      if (SL_RF_opt & OPT_FENS) {
        fullFlag = TRUE;
      }
      break;
    }
    while ((oobFlag == TRUE) || (fullFlag == TRUE)) {
      if (oobFlag == TRUE) {
        lockDENptr = &SL_RF_lockDENoens;
        obsSize = SL_RF_observationSize;
      }
      else {
        lockDENptr = &SL_RF_lockDENfens;        
        obsSize = (mode == SL_RF_PRED) ? SL_RF_fobservationSize : SL_RF_observationSize;
      }
      *lockDENptr = ompvector(1, obsSize);
      for (i = 1; i <= obsSize; i++) {
        omp_init_lock(&((*lockDENptr)[i]));
      }
      if (oobFlag == TRUE) {
        oobFlag = FALSE;
      }
      else {
        fullFlag = FALSE;
      }
    }
  }
  else {
    char  potentiallyMixedMultivariate = FALSE;
    if (SL_RF_rTargetFactorCount > 0) {
      omp_lock_t   **lockDENptr;
      uint obsSize;  
      char oobFlag, fullFlag;
      oobFlag = fullFlag = FALSE;
      switch (mode) {
      case SL_RF_PRED:
        if (SL_RF_opt & OPT_FENS) {
          fullFlag = TRUE;
        }
        break;
      default:
        if (SL_RF_opt & OPT_OENS) {
          oobFlag = TRUE;
        }
        if (SL_RF_opt & OPT_FENS) {
          fullFlag = TRUE;
        }
        break;
      }
      while ((oobFlag == TRUE) || (fullFlag == TRUE)) {
        if (oobFlag == TRUE) {
          lockDENptr = &SL_RF_lockDENoens;
          obsSize = SL_RF_observationSize;
        }
        else {
          lockDENptr = &SL_RF_lockDENfens;
          obsSize = (mode == SL_RF_PRED) ? SL_RF_fobservationSize : SL_RF_observationSize;
        }
        if (!potentiallyMixedMultivariate) {
          *lockDENptr = ompvector(1, obsSize);
          for (i = 1; i <= obsSize; i++) {
            omp_init_lock(&((*lockDENptr)[i]));
          }
        }
        if (oobFlag == TRUE) {
          oobFlag = FALSE;
        }
        else {
          fullFlag = FALSE;
        }
      }
      potentiallyMixedMultivariate = TRUE;
    }
    if (SL_RF_rTargetNonFactorCount > 0) {
      omp_lock_t   **lockDENptr;
      omp_lock_t   **lockQNTptr;
      uint obsSize;  
      char oobFlag, fullFlag;
      oobFlag = fullFlag = FALSE;
      switch (mode) {
      case SL_RF_PRED:
        if (SL_RF_opt & OPT_FENS) {
          fullFlag = TRUE;
        }
        break;
      default:
        if (SL_RF_opt & OPT_OENS) {
          oobFlag = TRUE;
        }
        if (SL_RF_opt & OPT_FENS) {
          fullFlag = TRUE;
        }
        break;
      }
      while ((oobFlag == TRUE) || (fullFlag == TRUE)) {
        if (oobFlag == TRUE) {
          lockDENptr = &SL_RF_lockDENoens;
          lockQNTptr = &SL_RF_lockQNToens;
          obsSize = SL_RF_observationSize;
        }
        else {
          lockDENptr = &SL_RF_lockDENfens;
          lockQNTptr = &SL_RF_lockQNTfens;
          obsSize = (mode == SL_RF_PRED) ? SL_RF_fobservationSize : SL_RF_observationSize;
        }
        if (!potentiallyMixedMultivariate) {
          *lockDENptr = ompvector(1, obsSize);
          for (i = 1; i <= obsSize; i++) {
            omp_init_lock(&((*lockDENptr)[i]));
          }
        }
        *lockQNTptr = ompvector(1, obsSize);        
        for (i = 1; i <= obsSize; i++) {
          omp_init_lock(&((*lockQNTptr)[i]));
        }
        if (oobFlag == TRUE) {
          oobFlag = FALSE;
        }
        else {
          fullFlag = FALSE;
        }
      }
      potentiallyMixedMultivariate = TRUE;
    }
  }
}
#else
void stackLocksOpenMP(char mode) { }
#endif
#ifdef _OPENMP
void unstackLocksOpenMP(char mode) {
  uint i, j;
  omp_destroy_lock(&SL_RF_lockEnsbUpdtCount);
  omp_destroy_lock(&SL_RF_lockPerf);
  if (SL_RF_optHigh & OPT_PART_PLOT) {
    for (i = 1; i <= SL_RF_observationSize; i++) {
      omp_destroy_lock(&(SL_RF_lockPartial[i]));
    }
    free_ompvector(SL_RF_lockPartial, 1, SL_RF_observationSize);
  }
  if (SL_RF_optHigh & OPT_WGHT) {
    uint gMembershipSize;
    if((SL_RF_optHigh & OPT_WGHT_IBG) && (SL_RF_optHigh & OPT_WGHT_OOB)) {
      switch (mode) {
      case SL_RF_PRED:
        gMembershipSize = SL_RF_fobservationSize;
        break;
      default:
        gMembershipSize = SL_RF_observationSize;
        break;
      }
    }
    else {
      gMembershipSize  = SL_RF_observationSize;
    }
    for (i = 1; i <= gMembershipSize; i++) {
      for (j = 1; j <= SL_RF_observationSize; j++) {
        omp_destroy_lock(&(SL_RF_lockWeight[i][j]));
      }
    }
    for (i = 1; i <= gMembershipSize; i++) {    
      free_ompvector(SL_RF_lockWeight[i], 1, SL_RF_observationSize);
    }
    free_new_vvector(SL_RF_lockWeight, 1, gMembershipSize, NRUTIL_OMPLPTR);
    for (i = 1; i <= gMembershipSize; i++) {
      omp_destroy_lock(&(SL_RF_lockWeightRow[i]));
    }
    free_ompvector(SL_RF_lockWeightRow, 1, gMembershipSize);
  }
  if (SL_RF_opt & OPT_VIMP) {
      uint obsSize, xVimpSize;
      if (SL_RF_opt & OPT_VIMP_JOIN) {
        xVimpSize = 1;
      }
      else {
        xVimpSize = SL_RF_intrPredictorSize;
      }
      switch (mode) {
      case SL_RF_PRED:
        obsSize = SL_RF_fobservationSize;
        break;
      default:
        obsSize  = SL_RF_observationSize;
        break;
      }
      for (i = 1; i <= xVimpSize; i++) {
        for (j = 1; j <= obsSize; j++) {
          omp_destroy_lock(&(SL_RF_lockVimp[i][j]));
        }
      }
      for (i = 1; i <= xVimpSize; i++) {
        free_ompvector(SL_RF_lockVimp[i], 1, obsSize);
      }
      free_new_vvector(SL_RF_lockVimp, 1, xVimpSize, NRUTIL_OMPLPTR);
      for (i = 1; i <= obsSize; i++) {
        omp_destroy_lock(&(SL_RF_lockVimpRow[i]));
      }
      free_ompvector(SL_RF_lockVimpRow, 1, obsSize);
      for (i = 1; i <= xVimpSize; i++) {
        omp_destroy_lock(&(SL_RF_lockVimpCol[i]));
      }
      free_ompvector(SL_RF_lockVimpCol, 1, xVimpSize);
  }
  if ((SL_RF_vtry > 0) && (SL_RF_vtryMode != SL_RF_VTRY_NULL)) {
    uint xVimpSize;
    xVimpSize = SL_RF_xSize;
    for (i = 1; i <= xVimpSize; i++) {
      if (SL_RF_holdBLKptr[i] > 0) {
        for (j = 1; j <= SL_RF_holdBLKptr[i]; j++) {
          omp_destroy_lock(&(SL_RF_lockVimpHoldout[i][j]));
        }
        free_ompvector(SL_RF_lockVimpHoldout[i], 1, SL_RF_holdBLKptr[i]);
      }
    }
    free_new_vvector(SL_RF_lockVimpHoldout, 1, xVimpSize, NRUTIL_OMPLPTR);    
  }
  if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
    omp_lock_t   *lockDENptr;
    uint obsSize;  
    char oobFlag, fullFlag;
    oobFlag = fullFlag = FALSE;
    switch (mode) {
    case SL_RF_PRED:
      if (SL_RF_opt & OPT_FENS) {
        fullFlag = TRUE;
      }
      break;
    default:
      if (SL_RF_opt & OPT_OENS) {
        oobFlag = TRUE;
      }
      if (SL_RF_opt & OPT_FENS) {
        fullFlag = TRUE;
      }
      break;
    }
    while ((oobFlag == TRUE) || (fullFlag == TRUE)) {
      if (oobFlag == TRUE) {
        lockDENptr = SL_RF_lockDENoens;
        obsSize = SL_RF_observationSize;
      }
      else {
        lockDENptr = SL_RF_lockDENfens;
        obsSize = (mode == SL_RF_PRED) ? SL_RF_fobservationSize : SL_RF_observationSize;
      }
      for (i = 1; i <= obsSize; i++) {
        omp_destroy_lock(&(lockDENptr[i]));
      }
      free_ompvector(lockDENptr, 1, obsSize);
      if (oobFlag == TRUE) {
        oobFlag = FALSE;
      }
      else {
        fullFlag = FALSE;
      }
    }
  }
  else {
    char  potentiallyMixedMultivariate = FALSE;
    if (SL_RF_rTargetFactorCount > 0) {
      omp_lock_t   *lockDENptr;
      uint obsSize;  
      char oobFlag, fullFlag;
      oobFlag = fullFlag = FALSE;
      switch (mode) {
      case SL_RF_PRED:
        if (SL_RF_opt & OPT_FENS) {
          fullFlag = TRUE;
        }
        break;
      default:
        if (SL_RF_opt & OPT_OENS) {
          oobFlag = TRUE;
        }
        if (SL_RF_opt & OPT_FENS) {
          fullFlag = TRUE;
        }
        break;
      }
      while ((oobFlag == TRUE) || (fullFlag == TRUE)) {
        if (oobFlag == TRUE) {
          lockDENptr = SL_RF_lockDENoens;
          obsSize = SL_RF_observationSize;
        }
        else {
          lockDENptr = SL_RF_lockDENfens;
          obsSize = (mode == SL_RF_PRED) ? SL_RF_fobservationSize : SL_RF_observationSize;
        }
        if (!potentiallyMixedMultivariate) {
          for (i = 1; i <= obsSize; i++) {
            omp_destroy_lock(&(lockDENptr[i]));
          }
          free_ompvector(lockDENptr, 1, obsSize);
        }
        if (oobFlag == TRUE) {
          oobFlag = FALSE;
        }
        else {
          fullFlag = FALSE;
        }
      }
      potentiallyMixedMultivariate = TRUE;
    }
    if (SL_RF_rTargetNonFactorCount > 0) {
      omp_lock_t   *lockDENptr;
      omp_lock_t   *lockQNTptr;
      uint obsSize;  
      char oobFlag, fullFlag;
      oobFlag = fullFlag = FALSE;
      switch (mode) {
      case SL_RF_PRED:
        if (SL_RF_opt & OPT_FENS) {
          fullFlag = TRUE;
        }
        break;
      default:
        if (SL_RF_opt & OPT_OENS) {
          oobFlag = TRUE;
        }
        if (SL_RF_opt & OPT_FENS) {
          fullFlag = TRUE;
        }
        break;
      }
      while ((oobFlag == TRUE) || (fullFlag == TRUE)) {
        if (oobFlag == TRUE) {
          lockDENptr = SL_RF_lockDENoens;
          lockQNTptr = SL_RF_lockQNToens;
          obsSize = SL_RF_observationSize;
        }
        else {
          lockDENptr = SL_RF_lockDENfens;
          lockQNTptr = SL_RF_lockQNTfens;
          obsSize = (mode == SL_RF_PRED) ? SL_RF_fobservationSize : SL_RF_observationSize;
        }
        if (!potentiallyMixedMultivariate) {
          for (i = 1; i <= obsSize; i++) {
            omp_destroy_lock(&(lockDENptr[i]));
          }
          free_ompvector(lockDENptr, 1, obsSize);
        }
          for (i = 1; i <= obsSize; i++) {
            omp_destroy_lock(&(lockQNTptr[i]));
          }
          free_ompvector(lockQNTptr, 1, obsSize);
          if (oobFlag == TRUE) {
          oobFlag = FALSE;
        }
        else {
          fullFlag = FALSE;
        }
      }
      potentiallyMixedMultivariate = TRUE;
    }
  }
}
#else
void unstackLocksOpenMP(char mode) { }
#endif
