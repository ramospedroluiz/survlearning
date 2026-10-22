/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "stack.h"
#include "impute.h"
#include "nrutil.h"
#include "error.h"
/*
#include "nativeUtil.h"
*/
void stackAndInitializeTimeAndSubjectArrays(char mode) {
  uint i, j;
  uint leadingIndex;
  uint adjObsSize;
  if (!(SL_RF_opt & OPT_ANON)) {
    SL_RF_masterTime  = dvector(1, SL_RF_observationSize);
    SL_RF_masterTimeIndexIn  = uivector(1, SL_RF_observationSize);
    SL_RF_masterTimeSize = 0;
    for (j = 1; j <= SL_RF_observationSize; j++) {
      if (!SL_RF_nativeIsNaN(SL_RF_responseIn[SL_RF_timeIndex][j])) {
        SL_RF_masterTimeSize ++;
        SL_RF_masterTime[SL_RF_masterTimeSize] = SL_RF_responseIn[SL_RF_timeIndex][j];
      }
    }
    adjObsSize = SL_RF_observationSize;
    qksort(SL_RF_masterTime, SL_RF_masterTimeSize);
    leadingIndex = 1;
    for (i=2; i <= SL_RF_masterTimeSize; i++) {
      if (SL_RF_masterTime[i] > SL_RF_masterTime[leadingIndex]) {
        leadingIndex++;
        SL_RF_masterTime[leadingIndex] = SL_RF_masterTime[i];
      }
    }
    SL_RF_masterTimeSize = leadingIndex;
    for (i= SL_RF_masterTimeSize + 1; i <= adjObsSize; i++) {
      SL_RF_masterTime[i] = 0;
    }
  }
  if (!(SL_RF_opt & OPT_IMPU_ONLY)) {
    qksort(SL_RF_timeInterest, SL_RF_timeInterestSize);
    SL_RF_sortedTimeInterestSize = 1;
    for (i=2; i <= SL_RF_timeInterestSize; i++) {
      if (SL_RF_timeInterest[i] > SL_RF_timeInterest[SL_RF_sortedTimeInterestSize]) {
        (SL_RF_sortedTimeInterestSize) ++;
        SL_RF_timeInterest[SL_RF_sortedTimeInterestSize] = SL_RF_timeInterest[i];
      }
    }
    if (SL_RF_sortedTimeInterestSize != SL_RF_timeInterestSize) {
      SL_RF_nativePrint("\nRF-SRC:  *** WARNING *** ");
      SL_RF_nativePrint("\nRF-SRC:  Time points of interest are not unique.");
      SL_RF_nativePrint("\nRF-SRC:  Any ensemble matricies will be");
      SL_RF_nativePrint("\nRF-SRC:  resized as [N'] x [n], where N' is the");
      SL_RF_nativePrint("\nRF-SRC:  unique time points of interest and n is");
      SL_RF_nativePrint("\nRF-SRC:  number of observations in the data.");
    }
    for (i = SL_RF_sortedTimeInterestSize + 1; i <= SL_RF_timeInterestSize; i++) {
      SL_RF_timeInterest[i] = 0;
    }
  }  
}
void unstackTimeAndSubjectArrays(char mode) {
  if (!(SL_RF_opt & OPT_ANON)) {
    free_dvector(SL_RF_masterTime, 1, SL_RF_observationSize);
    free_uivector(SL_RF_masterTimeIndexIn, 1, SL_RF_observationSize);
  }
}
void stackFactorArrays(char mode) {
  uint i, k;
  stackFactorGeneric(TRUE,
                     SL_RF_ySize,
                     SL_RF_rType,
                     &SL_RF_rFactorMap,
                     &SL_RF_rFactorCount,
                     &SL_RF_rFactorIndex,
                     &SL_RF_rFactorSize,
                     &SL_RF_rNonFactorMap,
                     &SL_RF_rNonFactorCount,
                     &SL_RF_rNonFactorIndex);
  stackFactorGeneric(FALSE,
                     SL_RF_xSize,
                     SL_RF_xType,
                     &SL_RF_xFactorMap,
                     &SL_RF_xFactorCount,
                     &SL_RF_xFactorIndex,
                     &SL_RF_xFactorSize,
                     &SL_RF_xNonFactorMap,
                     &SL_RF_xNonFactorCount,
                     &SL_RF_xNonFactorIndex);
  if (SL_RF_xFactorCount > 0) {
    
    SL_RF_xLevels = (uint **) new_vvector(1, SL_RF_xFactorCount, NRUTIL_UPTR);  
    for (k = 1; k <= SL_RF_xFactorCount; k++) {
      if (SL_RF_xLevelsCnt[k] > 0) {
        SL_RF_xLevels[k] = (uint *) INTEGER(VECTOR_ELT(SL_RF_xLevelsSEXP, k-1));
        SL_RF_xLevels[k] --;
      }
      else {
        SL_RF_nativeError("\nRF-SRC: *** ERROR *** ");
        SL_RF_nativeError("\nRF-SRC: Inconsistent zero-level count in factor:  compressed-index = %10d, x-index = %10d", k, SL_RF_xFactorIndex[k]);
        SL_RF_nativeError("\nRF-SRC: Please Contact Technical Support.");
        SL_RF_nativeExit();
      }
    }
    
    /*
    SL_RF_xLevels = (uint **) copy2DObject(SL_RF_xLevelsJNIE, NATIVE_TYPE_INTEGER, &SL_RF_nat2DInfoListSize);
    for (k = 1; k <= SL_RF_xFactorCount; k++) {    
      if (SL_RF_xLevelsCnt[k] > 0) {
      }
      else {
        SL_RF_nativeError("\nRF-SRC: *** ERROR *** ");
        SL_RF_nativeError("\nRF-SRC: Inconsistent zero-level count in factor:  compressed-index = %10d, x-index = %10d", k, SL_RF_xFactorIndex[k]);
        SL_RF_nativeError("\nRF-SRC: Please Contact Technical Support.");
        SL_RF_nativeExit();
      }
    }
    */    
    /*
    SL_RF_xLevels = (uint **) copyXDObject2D(SL_RF_xLevelsPYTH, &SL_RF_natXDInfoListSize);
    for (k = 1; k <= SL_RF_xFactorCount; k++) {
      if (SL_RF_xLevelsCnt[k] > 0) {
      }
      else {
        SL_RF_nativeError("\nRF-SRC: *** ERROR *** ");
        SL_RF_nativeError("\nRF-SRC: Inconsistent zero-level count in factor:  compressed-index = %10d, x-index = %10d", k, SL_RF_xFactorIndex[k]);
        SL_RF_nativeError("\nRF-SRC: Please Contact Technical Support.");
        SL_RF_nativeExit();
      }
    }
    */    
  }
  if (SL_RF_ySize == 0) {
    SL_RF_rTarget = NULL;
    SL_RF_rTargetCount = 0;
  }
  else {
    if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
      SL_RF_rTarget = NULL;
      SL_RF_rTargetCount = 0;
    }
    else {
      if (mode == SL_RF_GROW) {
        SL_RF_rTargetCount = SL_RF_ySize;
        SL_RF_rTarget = uivector(1 , SL_RF_rTargetCount);
        for (i = 1; i <= SL_RF_rTargetCount; i++) {
          SL_RF_rTarget[i] = i;
        }
      }
      else {
      }
      SL_RF_rTargetFactor    = uivector(1, SL_RF_rTargetCount);
      SL_RF_rTargetNonFactor = uivector(1, SL_RF_rTargetCount);
      SL_RF_rTargetFactorCount = SL_RF_rTargetNonFactorCount = 0;
      for (i = 1; i <= SL_RF_rTargetCount; i++) {
        if ((SL_RF_rTarget[i] < 1) || (SL_RF_rTarget[i] > SL_RF_ySize)) {
          SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
          SL_RF_nativeError("\nRF-SRC:  Target response is out of range for [C+], [R+], [M+]:  %10d %10d ", i, SL_RF_rTarget[i]);
          SL_RF_nativeExit();
        }
        if ((SL_RF_rType[SL_RF_rTarget[i]] == 'B') ||
            (SL_RF_rType[SL_RF_rTarget[i]] == 'I') ||
            (SL_RF_rType[SL_RF_rTarget[i]] == 'C')) {
          SL_RF_rTargetFactor[++SL_RF_rTargetFactorCount] = SL_RF_rTarget[i];
        }
        else {
          SL_RF_rTargetNonFactor[++SL_RF_rTargetNonFactorCount] = SL_RF_rTarget[i];
        }
      }
    }  
  }  
}
void stackFactorGeneric(char    respFlag,
                        uint    size,
                        char   *type,
                        uint  **p_factorMap,
                        uint   *factorCount,
                        uint  **p_factorIndex,
                        uint  **p_factorSize,
                        uint  **p_nonfactorMap,
                        uint   *nonfactorCount,
                        uint  **p_nonfactorIndex) {
  uint i, j;
  if (size > 0) {
    *p_factorMap    = uivector(1, size);
    *p_nonfactorMap = uivector(1, size);
    *factorCount    = 0;
    *nonfactorCount = 0;
    for (i = 1; i <= size; i++) {
      (*p_factorMap)[i]    = 0;
      (*p_nonfactorMap)[i] = 0;
      if ((type[i] == 'B') ||
          ((type[i] == 'I') && respFlag) ||
          (type[i] == 'C')) {
        (*factorCount) ++;
        (*p_factorMap)[i] = *factorCount;
      }
      else {
        (*nonfactorCount) ++;
        (*p_nonfactorMap)[i] = *nonfactorCount;
      }
    }
    if (*factorCount > 0) {
      *p_factorIndex = uivector(1, *factorCount);
      j = 0;
      for (i = 1; i <= size; i++) {
        if ((*p_factorMap)[i] > 0) {
          (*p_factorIndex)[++j] = i;
        }
      }
      *p_factorSize = uivector(1, *factorCount);
    }
    if (*nonfactorCount > 0) {
      *p_nonfactorIndex = uivector(1, *nonfactorCount);
      j = 0;
      for (i = 1; i <= size; i++) {
        if ((*p_nonfactorMap)[i] > 0) {
          (*p_nonfactorIndex)[++j] = i;
        }
      }
    }
  }
  else {
    *factorCount    = 0;
    *nonfactorCount = 0;
  }
}
void unstackFactorArrays(char mode) {
  if (SL_RF_ySize > 0) {
    free_uivector(SL_RF_rFactorMap, 1, SL_RF_ySize);
    if (SL_RF_rFactorCount > 0) {
      free_uivector(SL_RF_rFactorIndex, 1, SL_RF_rFactorCount);
      free_uivector(SL_RF_rFactorSize, 1, SL_RF_rFactorCount);
    }
    free_uivector(SL_RF_rNonFactorMap, 1, SL_RF_ySize);
    if (SL_RF_rNonFactorCount > 0) {
      free_uivector(SL_RF_rNonFactorIndex, 1, SL_RF_rNonFactorCount);
    }
  }
  free_uivector(SL_RF_xFactorMap, 1, SL_RF_xSize);
  if (SL_RF_xFactorCount > 0) {
    free_uivector(SL_RF_xFactorIndex, 1, SL_RF_xFactorCount);
    free_uivector(SL_RF_xFactorSize, 1, SL_RF_xFactorCount);
    
    free_new_vvector(SL_RF_xLevels, 1, SL_RF_xFactorCount, NRUTIL_UPTR);  
    
    /*
    SL_RF_xLevels = NULL;
    */
    /*
    SL_RF_xLevels = NULL;
    */
  }
  free_uivector(SL_RF_xNonFactorMap, 1, SL_RF_xSize);
  if (SL_RF_xNonFactorCount > 0) {
    free_uivector(SL_RF_xNonFactorIndex, 1, SL_RF_xNonFactorCount);
  }
  if ((SL_RF_rFactorCount + SL_RF_xFactorCount) > 0) {
    free_new_vvector(SL_RF_factorList, 1, SL_RF_ntree, NRUTIL_FPTR2);
  }
  if (SL_RF_ySize == 0) {
  }
  else {
    if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
    }
    else {
      free_uivector(SL_RF_rTargetFactor, 1, SL_RF_rTargetCount);
      free_uivector(SL_RF_rTargetNonFactor, 1, SL_RF_rTargetCount);
      if (mode == SL_RF_GROW) {
        free_uivector(SL_RF_rTarget, 1 , SL_RF_rTargetCount);
      }
      else {
      }
    }
  }
}
char stackMissingArraysPhase1(char mode) {
  char result;
  char mFlag;
  uint i, j;
  result = TRUE;
  if (!(SL_RF_opt & OPT_ANON)) {
    if (!(SL_RF_optHigh & OPT_DATA_PASG)) {
      for (j = 1; j <= SL_RF_ySize; j++) {
        for (i = 1; i <= SL_RF_observationSize; i++) {
          if (!SL_RF_nativeIsNaN(SL_RF_responseIn[j][i])) {
            if (!isfinite(SL_RF_responseIn[j][i])) {
              result = FALSE;
              SL_RF_nativePrint("\nRF-SRC:  train response elements must not be plus or minus infinity:  [%10d, %10d] = %12.4f \n", j, i, SL_RF_responseIn[j][i]);
            }
          }
        }
        if (result == FALSE) {
          SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
          SL_RF_nativeError("\nRF-SRC:  Plus or Minus Infinity detected.");
          SL_RF_nativeExit();
        }
      }  
      for (j = 1; j <= SL_RF_xSize; j++) {
        for (i = 1; i <= SL_RF_observationSize; i++) {
          if (!SL_RF_nativeIsNaN(SL_RF_observationIn[j][i])) {
            if (!isfinite(SL_RF_observationIn[j][i])) {
              result = FALSE;
              SL_RF_nativeError("\nRF-SRC:  train x-variable elements must not be plus or minus infinity:  [%10d, %10d] = %12.4f \n", j, i, SL_RF_observationIn[j][i]);
            }
          }
        }
        if (result == FALSE) {
          SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
          SL_RF_nativeError("\nRF-SRC:  Plus or Minus Infinity detected.");
          SL_RF_nativeExit();
        }
      }
    }
    if (!(SL_RF_optHigh & OPT_DATA_PASG)) {
      for (j = 1; j <= SL_RF_ySize; j++) {
        if (j == SL_RF_timeIndex) {
          for (i = 1; i <= SL_RF_observationSize; i++) {
            if (!SL_RF_nativeIsNaN(SL_RF_responseIn[SL_RF_timeIndex][i])) {
              if (SL_RF_responseIn[SL_RF_timeIndex][i] < 0) {
                result = FALSE;
                SL_RF_nativePrint("\nRF-SRC:  train time elements must be greater than or equal to zero or NA:  [%10d] = %12.4f \n", i, SL_RF_responseIn[SL_RF_timeIndex][i]);
              }
            }
          }
        }
        if (j == SL_RF_statusIndex) {
          for (i = 1; i <= SL_RF_observationSize; i++) {
            if (!SL_RF_nativeIsNaN(SL_RF_responseIn[SL_RF_statusIndex][i])) {
              if (SL_RF_responseIn[SL_RF_statusIndex][i] < 0) {
                result = FALSE;
                SL_RF_nativePrint("\nRF-SRC:  train status elements must be greater than or equal to zero or NA:  [%10d] = %12.4f \n", i, SL_RF_responseIn[SL_RF_statusIndex][i]);
              }
            }
          }
        }
        if (j == SL_RF_statusIndex) {
          mFlag = FALSE;
          for (i = 1; i <= SL_RF_observationSize; i++) {
            if (!SL_RF_nativeIsNaN(SL_RF_responseIn[SL_RF_statusIndex][i])) {
              if (SL_RF_responseIn[SL_RF_statusIndex][i] >= 0) {
                mFlag = TRUE;
                i = SL_RF_observationSize;
              }
            }
          }
          if (mFlag == FALSE) {
            SL_RF_nativePrint("\nRF-SRC:  All train status elements are censored or missing. \n");
            result = FALSE;
          }
        }
        if ((SL_RF_statusIndex == 0) && (SL_RF_timeIndex == 0)) {
          mFlag = FALSE;
          for (i = 1; i <= SL_RF_observationSize; i++) {
            if (!SL_RF_nativeIsNaN(SL_RF_responseIn[j][i])) {
              mFlag = TRUE;
              i = SL_RF_observationSize;
            }
          }
          if (mFlag == FALSE) {
            SL_RF_nativePrint("\nRF-SRC:  All train outcome/response elements are missing for:  %10d \n", j);
            result = FALSE;
          }
        }
        if (result == FALSE) {
          SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
          SL_RF_nativeError("\nRF-SRC:  Missingness verification failed.");
          SL_RF_nativeExit();
        }
      }  
    }
    SL_RF_response = (double ***) new_vvector(1, SL_RF_ntree, NRUTIL_DPTR2);
    if (SL_RF_ySize > 0) {
      for (i = 1 ; i <= SL_RF_ntree; i++) {
        SL_RF_response[i] = SL_RF_responseIn;
      }
      SL_RF_time = NULL;
      SL_RF_masterTimeIndex = NULL;
      if (SL_RF_statusIndex > 0) {
        SL_RF_time = (double **) new_vvector(1, SL_RF_ntree, NRUTIL_DPTR);
        SL_RF_masterTimeIndex = (uint **) new_vvector(1, SL_RF_ntree, NRUTIL_UPTR);
        for (i = 1 ; i <= SL_RF_ntree; i++) {
          SL_RF_time[i] = SL_RF_responseIn[SL_RF_timeIndex];
          SL_RF_masterTimeIndex[i] = SL_RF_masterTimeIndexIn;
        }
        updateTimeIndexArray(0,
                             NULL,
                             SL_RF_observationSize,
                             SL_RF_responseIn[SL_RF_timeIndex],
                             TRUE,
                             FALSE,
                             SL_RF_masterTimeIndexIn);
        SL_RF_status = (double **) new_vvector(1, SL_RF_ntree, NRUTIL_DPTR);
        for (i = 1 ; i <= SL_RF_ntree; i++) {
          SL_RF_status[i] = SL_RF_responseIn[SL_RF_statusIndex];
        }
      }
    }
    else {
      for (i = 1 ; i <= SL_RF_ntree; i++) {
        SL_RF_response[i] = NULL;
      }
    }
    SL_RF_observation = (double ***) new_vvector(1, SL_RF_ntree, NRUTIL_DPTR2);
    for (i = 1 ; i <= SL_RF_ntree; i++) {
      SL_RF_observation[i] = SL_RF_observationIn;
    }
    if (SL_RF_optHigh & OPT_DATA_PASG) {
      SL_RF_mStatusFlag = SL_RF_mTimeFlag = SL_RF_mResponseFlag = SL_RF_mPredictorFlag = FALSE;
      SL_RF_mRecordSize = 0;
      SL_RF_mRecordMap = NULL;
    }
    else {
      SL_RF_mRecordMap = uivector(1, SL_RF_observationSize);
      SL_RF_mRecordSize = getRecordMap(SL_RF_mRecordMap,
                                    SL_RF_observationSize,
                                    SL_RF_responseIn,
                                    SL_RF_observationIn);
      if (SL_RF_mRecordSize == 0) {
        SL_RF_mStatusFlag = SL_RF_mTimeFlag = SL_RF_mResponseFlag = SL_RF_mPredictorFlag = FALSE;
      }
      else {
        SL_RF_optHigh = SL_RF_optHigh & (~OPT_MEMB_INCG);
        SL_RF_optHigh = SL_RF_optHigh & (~OPT_TERM_INCG);
        stackMissingSignatures(SL_RF_observationSize,
                               SL_RF_ySize,
                               SL_RF_responseIn,
                               SL_RF_observationIn,
                               SL_RF_mRecordMap,
                               SL_RF_mRecordSize,
                               & SL_RF_mRecordIndex,
                               & SL_RF_mpIndexSize,
                               & SL_RF_mpSign,
                               & SL_RF_mpIndex,
                               & SL_RF_mrFactorSize,
                               & SL_RF_mrFactorIndex,
                               & SL_RF_mxFactorSize,
                               & SL_RF_mxFactorIndex,
                               & SL_RF_mTimeFlag,
                               & SL_RF_mStatusFlag,
                               & SL_RF_mResponseFlag,
                               & SL_RF_mPredictorFlag);
        if (SL_RF_mResponseFlag == TRUE) {
          for (i = 1 ; i <= SL_RF_ntree; i++) {
            SL_RF_response[i] = NULL;
            if (SL_RF_timeIndex > 0) {
              SL_RF_time[i] = NULL;
              SL_RF_masterTimeIndex[i] = NULL;
            }
            if (SL_RF_statusIndex > 0) {
              SL_RF_status[i] = NULL;
            }
          }
        }
        if (SL_RF_mPredictorFlag == TRUE) {
          for (i = 1 ; i <= SL_RF_ntree; i++) {
            SL_RF_observation[i] = NULL;
          }
        }
      }  
    }  
  }  
  else {
    SL_RF_mStatusFlag = SL_RF_mTimeFlag = SL_RF_mResponseFlag = SL_RF_mPredictorFlag = FALSE;
    SL_RF_mRecordSize = 0;
    SL_RF_mRecordMap = NULL;
  }
  if (mode == SL_RF_PRED) {
    if (!(SL_RF_optHigh & OPT_DATA_PASP)) {
      if (SL_RF_frSize > 0) {
        for (j = 1; j <= SL_RF_ySize; j++) {      
          for (i = 1 ; i <= SL_RF_fobservationSize; i++) {
            if (!SL_RF_nativeIsNaN(SL_RF_fresponseIn[j][i])) {
              if (!isfinite(SL_RF_fresponseIn[j][i])) {
                result = FALSE;
                SL_RF_nativeError("\nRF-SRC:  test response elements must not be plus or minus infinity:  [%10d, %10d] = %12.4f \n", j, i, SL_RF_fresponseIn[j][i]);
              }
            }
          }
          if (result == FALSE) {
            SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
            SL_RF_nativeError("\nRF-SRC:  Plus or Minus Infinity detected.");
            SL_RF_nativeExit();
          }
        }
      }
      for (j = 1; j <= SL_RF_xSize; j++) {      
        for (i = 1; i <= SL_RF_fobservationSize; i++) {
          if (!SL_RF_nativeIsNaN(SL_RF_fobservationIn[j][i])) {
            if (!isfinite(SL_RF_fobservationIn[j][i])) {
              result = FALSE;
              SL_RF_nativeError("\nRF-SRC:  test x-variable elements must not be plus or minus infinity:  [%10d, %10d] = %12.4f \n", j, i, SL_RF_fobservationIn[j][i]);
            }
          }
        }
        if (result == FALSE) {
          SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
          SL_RF_nativeError("\nRF-SRC:  Plus or Minus Infinity detected.");
          SL_RF_nativeExit();
        }
      }
    }
    if (!(SL_RF_optHigh & OPT_DATA_PASP)) {    
      if (SL_RF_frSize > 0) {
        if (SL_RF_timeIndex > 0) {
          for (i = 1 ; i <= SL_RF_fobservationSize; i++) {
            if (!SL_RF_nativeIsNaN(SL_RF_fresponseIn[SL_RF_timeIndex][i])) {
              if (SL_RF_fresponseIn[SL_RF_timeIndex][i] < 0) {
                result = FALSE;
                SL_RF_nativeError("\nRF-SRC:  PRED time elements must be greater than or equal to zero or NA:  [%10d] = %12.4f \n", i, SL_RF_fresponseIn[SL_RF_timeIndex][i]);
              }
            }
          }
        }
      }
      if (SL_RF_frSize > 0) {
        if (SL_RF_statusIndex > 0) {
          for (i = 1 ; i <= SL_RF_fobservationSize; i++) {
            if (!SL_RF_nativeIsNaN(SL_RF_fresponseIn[SL_RF_statusIndex][i])) {
              if (SL_RF_fresponseIn[SL_RF_statusIndex][i] < 0) {
                result = FALSE;
                SL_RF_nativeError("\nRF-SRC:  PRED status elements must be greater than or equal to zero or NA:  [%10d] = %12.4f \n", i, SL_RF_fresponseIn[SL_RF_statusIndex][i]);
              }
            }
          }
        }
      }
      if (result == FALSE) {
        SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
        SL_RF_nativeError("\nRF-SRC:  Missingness verification failed.");
        SL_RF_nativeExit();
      }
    }
    SL_RF_fobservation = (double ***) new_vvector(1, SL_RF_ntree, NRUTIL_DPTR2);
    for (i = 1 ; i <= SL_RF_ntree; i++) {
      SL_RF_fobservation[i] = SL_RF_fobservationIn;
    }
    SL_RF_fresponse = (double ***) new_vvector(1, SL_RF_ntree, NRUTIL_DPTR2);
    if (SL_RF_frSize > 0) {
      for (i = 1 ; i <= SL_RF_ntree; i++) {
        SL_RF_fresponse[i] = SL_RF_fresponseIn;
      }
      if (SL_RF_timeIndex > 0) {
        SL_RF_ftime = (double **) new_vvector(1, SL_RF_ntree, NRUTIL_DPTR);
        for (i = 1 ; i <= SL_RF_ntree; i++) {
          SL_RF_ftime[i] = SL_RF_fresponseIn[SL_RF_timeIndex];
        }
      }
      if (SL_RF_statusIndex > 0) {
        SL_RF_fstatus = (double **) new_vvector(1, SL_RF_ntree, NRUTIL_DPTR);
        for (i = 1 ; i <= SL_RF_ntree; i++) {
          SL_RF_fstatus[i] = SL_RF_fresponseIn[SL_RF_statusIndex];
        }
      }
    }
    else {
      for (i = 1 ; i <= SL_RF_ntree; i++) {
        SL_RF_fresponse[i] = NULL;
      }
    }
    if (SL_RF_optHigh & OPT_DATA_PASP) {
      SL_RF_fmStatusFlag = SL_RF_fmTimeFlag = SL_RF_fmResponseFlag = SL_RF_fmPredictorFlag = FALSE;
      SL_RF_fmRecordSize = 0;
      SL_RF_fmRecordMap = NULL;
    }
    else {
    SL_RF_fmRecordMap = uivector(1, SL_RF_fobservationSize);
    SL_RF_fmRecordSize = getRecordMap(SL_RF_fmRecordMap,
                                 SL_RF_fobservationSize,
                                 SL_RF_fresponseIn,
                                 SL_RF_fobservationIn);
    if (SL_RF_fmRecordSize == 0) {
      SL_RF_fmStatusFlag = SL_RF_fmTimeFlag = SL_RF_fmResponseFlag = SL_RF_fmPredictorFlag = FALSE;
    }  
    else {
      if (SL_RF_opt & OPT_ANON) {
      }
      else {
      }
      stackMissingSignatures(SL_RF_fobservationSize,
                             SL_RF_frSize,
                             SL_RF_fresponseIn,
                             SL_RF_fobservationIn,
                             SL_RF_fmRecordMap,
                             SL_RF_fmRecordSize,
                             & SL_RF_fmRecordIndex,
                             & SL_RF_fmpIndexSize,
                             & SL_RF_fmpSign,
                             & SL_RF_fmpIndex,
                             & SL_RF_fmrFactorSize,
                             & SL_RF_fmrFactorIndex,
                             & SL_RF_fmxFactorSize,
                             & SL_RF_fmxFactorIndex,
                             & SL_RF_fmTimeFlag,
                             & SL_RF_fmStatusFlag,
                             & SL_RF_fmResponseFlag,
                             & SL_RF_fmPredictorFlag);
      if (FALSE) {
        if (SL_RF_frSize > 0) {
          if (SL_RF_fmResponseFlag == TRUE) {
            for (i = 1 ; i <= SL_RF_ntree; i++) {
              SL_RF_fresponse[i] = NULL;
              if (SL_RF_timeIndex > 0) {
                SL_RF_ftime[i] = NULL;
              }
              if (SL_RF_statusIndex > 0) {
                SL_RF_fstatus[i] = NULL;
              }
            }
          }
        }
        if (SL_RF_fmPredictorFlag == TRUE) {
          for (i = 1 ; i <= SL_RF_ntree; i++) {
            SL_RF_fobservation[i] = NULL;
          }
        }
      }
    }  
    }  
  }  
  return result;
}
char stackMissingArraysPhase2(char mode) {
  char result;
  char mFlag;
  char dualUseFlag;
  uint recordSize;
  uint i, j;
  result = TRUE;
  if (SL_RF_opt & OPT_ANON) {
    result = FALSE;
    if (SL_RF_fmResponseFlag == TRUE) {
      SL_RF_opt = SL_RF_opt & (~OPT_PERF);
    }
    if (SL_RF_fmPredictorFlag == TRUE) {
      if (SL_RF_optHigh & OPT_JIT_TOP) {
      }
      else {
        SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
        SL_RF_nativeError("\nRF-SRC:  An anonymous forest with missingness in the test data requires the JIT flag to be asserted");
        SL_RF_nativeError("\nRF-SRC:  Please adjust your script accordingly.");
        SL_RF_nativeExit();
      }      
    }
  }
  if (SL_RF_optHigh & OPT_JIT_TOP) {
    result = FALSE;
  }
  if (result) {
    dualUseFlag = FALSE;
    switch (mode) {
    case SL_RF_PRED:
      if (SL_RF_fmRecordSize > 0) {
        recordSize = SL_RF_fmRecordSize;
        dualUseFlag = TRUE;
        mFlag = ACTIVE;
      }
      else {
        SL_RF_opt = SL_RF_opt & (~OPT_MISS_OUT);
      }
      break;
    default:
      SL_RF_fmRecordSize = 0;
      if (SL_RF_mRecordSize > 0) {
        recordSize = SL_RF_mRecordSize;
        dualUseFlag = TRUE;
        mFlag = FALSE;
      }
      else {
        SL_RF_opt = SL_RF_opt & (~OPT_MISS_OUT);
        SL_RF_nImpute = 1;
      }
      break;
    }  
    if (dualUseFlag == TRUE) {
      SL_RF_dmRecordBootFlag = cmatrix(1, SL_RF_ntree, 1, recordSize);
      for (j = 1; j <= SL_RF_ntree; j++) {
        for (i = 1; i <= recordSize; i++) {
          SL_RF_dmRecordBootFlag[j][i] = mFlag;
        }
      }
    }
  }  
  else {
    SL_RF_opt = SL_RF_opt & (~OPT_MISS_OUT);    
  }
  return result;
}
void unstackMissingArrays(char mode) {
  char dualUseFlag;
  uint recordSize;
  if (!(SL_RF_opt & OPT_ANON)) {  
    free_new_vvector(SL_RF_response, 1, SL_RF_ntree, NRUTIL_DPTR2);
    if (SL_RF_ySize > 0) {
      if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
        free_new_vvector(SL_RF_time, 1, SL_RF_ntree, NRUTIL_DPTR);
        free_new_vvector(SL_RF_masterTimeIndex, 1, SL_RF_ntree, NRUTIL_UPTR);
        free_new_vvector(SL_RF_status, 1, SL_RF_ntree, NRUTIL_DPTR);
      }
    }
    free_new_vvector(SL_RF_observation, 1, SL_RF_ntree, NRUTIL_DPTR2);
    if (SL_RF_optHigh & OPT_DATA_PASG) {
    }
    else {
      free_uivector(SL_RF_mRecordMap, 1, SL_RF_observationSize);
    }
    if (SL_RF_mRecordSize == 0) {
    }
    else {
      unstackMissingSignatures(SL_RF_ySize,
                               SL_RF_mRecordSize,
                               SL_RF_mRecordIndex,
                               SL_RF_mpIndexSize,
                               SL_RF_mpSign,
                               SL_RF_mpIndex,
                               SL_RF_mrFactorSize,
                               SL_RF_mrFactorIndex,
                               SL_RF_mxFactorSize,
                               SL_RF_mxFactorIndex);
    }
  }  
  else {
  }  
  if (mode == SL_RF_PRED) {
    free_new_vvector(SL_RF_fobservation, 1, SL_RF_ntree, NRUTIL_DPTR2);
    free_new_vvector(SL_RF_fresponse, 1, SL_RF_ntree, NRUTIL_DPTR2);
    if (SL_RF_frSize > 0) {
      if (SL_RF_timeIndex > 0) {
        free_new_vvector(SL_RF_ftime, 1, SL_RF_ntree, NRUTIL_DPTR);
      }
      if (SL_RF_statusIndex > 0) {
        free_new_vvector(SL_RF_fstatus, 1, SL_RF_ntree, NRUTIL_DPTR);
      }
    }
    if (SL_RF_optHigh & OPT_DATA_PASP) {
    }
    else {
      free_uivector(SL_RF_fmRecordMap, 1, SL_RF_fobservationSize);
    }
    if (SL_RF_fmRecordSize == 0) {
    }
    else {
      unstackMissingSignatures(SL_RF_frSize,
                               SL_RF_fmRecordSize,
                               SL_RF_fmRecordIndex,
                               SL_RF_fmpIndexSize,
                               SL_RF_fmpSign,
                               SL_RF_fmpIndex,
                               SL_RF_fmrFactorSize,
                               SL_RF_fmrFactorIndex,
                               SL_RF_fmxFactorSize,
                               SL_RF_fmxFactorIndex);
    }
  }
  if (SL_RF_opt & OPT_MISS_OUT) {
    dualUseFlag = FALSE;
    switch (mode) {
    case SL_RF_PRED:
      if (SL_RF_fmRecordSize > 0) {
        dualUseFlag = TRUE;
        recordSize = SL_RF_fmRecordSize;
      }
      break;
    default:
      if (SL_RF_mRecordSize > 0) {
        dualUseFlag = TRUE;
        recordSize = SL_RF_mRecordSize;
      }
      break;
    }  
    if (dualUseFlag == TRUE) {
      free_cmatrix(SL_RF_dmRecordBootFlag, 1, SL_RF_ntree, 1, recordSize);
    }
  }
}
void stackMissingSignatures(uint     obsSize,
                            uint     rspSize,
                            double **responsePtr,
                            double **predictorPtr,
                            uint    *recordMap,
                            uint     recordSize,
                            uint   **p_recordIndex,
                            uint    *p_pIndexSize,
                            int   ***p_pSign,
                            int    **p_pIndex,
                            uint    *pRF_mrFactorSize,
                            uint   **pRF_mrFactorIndex,
                            uint    *pRF_mxFactorSize,
                            uint   **pRF_mxFactorIndex,
                            char    *pRF_mTimeFlag,
                            char    *pRF_mStatusFlag,
                            char    *pRF_mResponseFlag,
                            char    *pRF_mPredictorFlag) {
  uint i, j, p;
  if (recordSize < 1) {
    SL_RF_nativePrint("\nRF-SRC:  *** ERROR *** ");
    SL_RF_nativePrint("\nRF-SRC:  Attempt to allocate for missingness in its absence.");
    SL_RF_nativePrint("\nRF-SRC:  Please Contact Technical Support.");
    SL_RF_nativeExit();
  }
  *p_recordIndex = uivector(1, recordSize);
  i = 0;
  for (j = 1; j <= obsSize; j++) {
    if (recordMap[j] > 0) {
      i++;
      (*p_recordIndex)[i] = j;
    }
  }
  *p_pSign = imatrix(1, rspSize + SL_RF_xSize, 1, recordSize);
  for (j = 1; j <= recordSize; j++) {
    for (i = 1; i <= rspSize + SL_RF_xSize; i++) {
      (*p_pSign)[i][j] = 0;
    }
  }
  for (j = 1; j <= recordSize; j++) {
    for (i = 1; i <= rspSize; i++) {
      if (SL_RF_nativeIsNaN(responsePtr[i][(*p_recordIndex)[j]])) {
        (*p_pSign)[i][j] = 1;
      }
    }
    for (i = 1; i <= SL_RF_xSize; i++) {
      if (SL_RF_nativeIsNaN(predictorPtr[i][(*p_recordIndex)[j]])) {
        (*p_pSign)[rspSize + i][j] = 1;
      }
    }
  }
  *pRF_mStatusFlag = *pRF_mTimeFlag = *pRF_mResponseFlag = *pRF_mPredictorFlag = FALSE;
  *p_pIndex = ivector(1, rspSize + SL_RF_xSize);
  *p_pIndexSize = 0;
  for (i = 1; i <= rspSize; i++) {
    (*p_pIndex)[i] = 0;
    for (j = 1; j <= recordSize; j++) {
      if ((*p_pSign)[i][j] == 1) {
        (*p_pIndexSize) ++;
        (*p_pIndex)[*p_pIndexSize] = - i;
        *pRF_mResponseFlag = TRUE;
        if (i == SL_RF_timeIndex) {
          *pRF_mTimeFlag = TRUE;
        }
        else if (i == SL_RF_statusIndex) {
          *pRF_mStatusFlag = TRUE;
        }
        j = recordSize;
      }
    }
  }  
  for (i = rspSize + 1; i <= rspSize + SL_RF_xSize; i++) {
    (*p_pIndex)[i] = 0;
    for (j = 1; j <= recordSize; j++) {
      if ((*p_pSign)[i][j] == 1) {
        (*p_pIndexSize) ++;
        (*p_pIndex)[*p_pIndexSize] =  i - rspSize;
        *pRF_mPredictorFlag = TRUE;
        j = recordSize;
      }
    }
  }  
  if (rspSize > 0) {
    *pRF_mrFactorIndex = uivector(1, rspSize);
    for (p = 1; p <= rspSize; p++) {
      (*pRF_mrFactorIndex)[p] = 0;
    }
  }
  *pRF_mxFactorIndex = uivector(1, SL_RF_xSize);
  for (p = 1; p <= SL_RF_xSize; p++) {
    (*pRF_mxFactorIndex)[p] = 0;
  }
  *pRF_mrFactorSize = *pRF_mxFactorSize = 0;
  for (p = 1; p <= *p_pIndexSize; p++) {
    if ((*p_pIndex)[p] < 0) {
      if ((SL_RF_rType[(uint) abs((*p_pIndex)[p])] == 'B') ||
          (SL_RF_rType[(uint) abs((*p_pIndex)[p])] == 'I') ||
          (SL_RF_rType[(uint) abs((*p_pIndex)[p])] == 'C')) {
        (*pRF_mrFactorSize) ++;
        (*pRF_mrFactorIndex)[*pRF_mrFactorSize] = (uint) abs((*p_pIndex)[p]);
      }
    }
    else {
      if ((SL_RF_xType[(*p_pIndex)[p]] == 'B') ||
          (SL_RF_xType[(*p_pIndex)[p]] == 'C')) {
        (*pRF_mxFactorSize) ++;
        (*pRF_mxFactorIndex)[*pRF_mxFactorSize] = (*p_pIndex)[p];
      }
    }
  }
}
void unstackMissingSignatures(uint      rspSize,
                              uint      recordSize,
                              uint     *recordIndex,
                              uint      vIndexSize,
                              int     **vSign,
                              int      *vIndex,
                              uint      mrFactorSize,
                              uint     *mrFactorIndex,
                              uint      mxFactorSize,
                              uint     *mxFactorIndex) {
  if (recordSize == 0) {
    SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    SL_RF_nativeError("\nRF-SRC:  Attempt to deallocate for missingness in its absence.");
    SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
    SL_RF_nativeExit();
  }
  free_uivector(recordIndex, 1, recordSize);
  free_imatrix(vSign, 1, rspSize + SL_RF_xSize, 1, recordSize);
  free_ivector(vIndex, 1, rspSize + SL_RF_xSize);
  if (rspSize > 0) {
    free_uivector(mrFactorIndex, 1, rspSize);
  }
  free_uivector(mxFactorIndex, 1, SL_RF_xSize);
}
void initializeFactorArrays(char mode) {
  uint j;
  if (SL_RF_rFactorCount + SL_RF_xFactorCount > 0) {
    SL_RF_rMaxFactorLevel = 0;
    for (j = 1; j <= SL_RF_rFactorCount; j++) {
      SL_RF_rFactorSize[j] = SL_RF_rLevelsMax[SL_RF_rFactorIndex[j]];
      if (SL_RF_rMaxFactorLevel < SL_RF_rFactorSize[j]) {
        SL_RF_rMaxFactorLevel = SL_RF_rFactorSize[j];
      }
    }
    SL_RF_xMaxFactorLevel = 0;
    for (j = 1; j <= SL_RF_xFactorCount; j++) {
      SL_RF_xFactorSize[j] = SL_RF_xLevelsMax[SL_RF_xFactorIndex[j]];
      if (SL_RF_xMaxFactorLevel < SL_RF_xFactorSize[j]) {
        SL_RF_xMaxFactorLevel = SL_RF_xFactorSize[j];
      }
    }
    SL_RF_maxFactorLevel = (SL_RF_xMaxFactorLevel > SL_RF_rMaxFactorLevel) ? SL_RF_xMaxFactorLevel : SL_RF_rMaxFactorLevel;
    SL_RF_factorList = (Factor ***) new_vvector(1, SL_RF_ntree, NRUTIL_FPTR2);
    for (j = 1; j <= SL_RF_ntree; j++) {
      SL_RF_factorList[j] = NULL;
    }
  }
}
char stackCompetingArrays(char mode) {
  uint obsSize;
  double  *statusPtr;
  uint    *mRecordMap;
  int    **mpSign;
  uint     mRecordSize;
  char eventSubsetFlag;
  char statusFlag;
  uint *eventCounter;
  uint i, j;
  if (SL_RF_statusIndex == 0) {
    SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    SL_RF_nativeError("\nRF-SRC:  Attempt to stack competing risk structures in the absence of SURV data.");
    SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
    SL_RF_nativeExit();
  }
  switch (mode) {
  case SL_RF_GROW:
    if ((SL_RF_splitRule == SURV_CR_LAU) || (SL_RF_splitRule == SURV_CR_GEN)) {
      if (SL_RF_eventTypeSize > 1) {
        SL_RF_opt = SL_RF_opt | OPT_COMP_RISK;
      }
      else {
        SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
        SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
        SL_RF_nativeError("\nRF-SRC:  Competing Risk analysis has been requested.");
        SL_RF_nativeError("\nRF-SRC:  The train data set does not contain competing risks.");
        SL_RF_nativeExit();
      }
    }
    else {
      if (SL_RF_splitRule == CUST_SPLIT) {
        if (SL_RF_eventTypeSize > 1) {
          SL_RF_opt = SL_RF_opt | OPT_COMP_RISK;
        }
        else {
          SL_RF_opt = SL_RF_opt & (~OPT_COMP_RISK);
        }
      }
      else {
        SL_RF_opt = SL_RF_opt & (~OPT_COMP_RISK);
      }
    }
    break;
  default:
    break;
  }
  if (SL_RF_eventTypeSize == 0) {
    if ((SL_RF_opt & OPT_OUTC_TYPE) && !(SL_RF_opt & OPT_PERF) && !(SL_RF_opt & OPT_VIMP)) {
      SL_RF_opt                  = SL_RF_opt & (~OPT_OENS);
      SL_RF_opt                  = SL_RF_opt & (~OPT_FENS);
    }
    else {
      SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
      SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
      SL_RF_nativeError("\nRF-SRC:  Performance or vimp has been requested.");
      SL_RF_nativeError("\nRF-SRC:  The train or pseudo-train data set does not contain any events.");
      SL_RF_nativeExit();
    }
  }
  else {
    hpsortui(SL_RF_eventType, SL_RF_eventTypeSize);
    SL_RF_eventTypeIndex  = uivector(1, SL_RF_eventType[SL_RF_eventTypeSize]);
    for (j = 1; j <= SL_RF_eventType[SL_RF_eventTypeSize]; j++) {
      SL_RF_eventTypeIndex[j] = 0;
    }
    for (j = 1; j <= SL_RF_eventTypeSize; j++) {
      SL_RF_eventTypeIndex[SL_RF_eventType[j]] = j;
    }
  }
  switch (mode) {
  case SL_RF_GROW:
    if (SL_RF_splitRule == RAND_SPLIT) {
      if (SL_RF_eventTypeSize == 1) {
      }
      else {
        SL_RF_opt = SL_RF_opt | OPT_COMP_RISK;
      }
    }
    if (SL_RF_splitRule == SURV_CR_LAU) {
      if (SL_RF_eventTypeSize == 1) {
        SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
        SL_RF_nativeError("\nRF-SRC:  Split rule specified is for Competing Risk scenarios only.");
        SL_RF_nativeError("\nRF-SRC:  The data set does not contain multiple events.");
        SL_RF_nativeExit();
      }
      if(SL_RF_crWeightSize != SL_RF_eventTypeSize) {
        SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
        SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
        SL_RF_nativeError("\nRF-SRC:  Competing risk weight vector must be of size equal to number of event types:  %12d != %12d \n", SL_RF_crWeightSize, SL_RF_eventTypeSize);
        SL_RF_nativeExit();
      }
      i = 0;
      for (j = 1; j <= SL_RF_eventTypeSize; j++) {
        if(SL_RF_crWeight[j] < 0) {
          SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
          SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
          SL_RF_nativeError("\nRF-SRC:  Competing risk weight elements must be greater than or equal to zero:  %12.4f \n", SL_RF_crWeight[j]);
          SL_RF_nativeExit();
        }
        else {
          if(SL_RF_crWeight[j] == 0) {
            i ++;
          }
        }
      }
      if (i == SL_RF_eventTypeSize) {
        SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
        SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
        SL_RF_nativeError("\nRF-SRC:  Competing risk weight elements are all zero. \n");
        SL_RF_nativeExit();
      }
    }
    break;
  default:
    if (SL_RF_opt & OPT_COMP_RISK) {
      if (SL_RF_eventTypeSize == 1) {
        SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
        SL_RF_nativeError("\nRF-SRC:  CR analysis has been specified in !GROW mode.");
        SL_RF_nativeError("\nRF-SRC:  However, the GROW data set does not contain multiple events.");
        SL_RF_nativeExit();
      }
    }
    break;
  }
  switch (mode) {
  case SL_RF_PRED:
    if (SL_RF_frSize > 0) {
      getEventInfo(mode);
    }
    else {
      SL_RF_feventTypeSize = SL_RF_mStatusSize = 0;
    }
    break;
  default:
    getEventInfo(mode);
    break;
  } 
  if (SL_RF_eventTypeSize > 1) {
    if (mode == SL_RF_PRED) {
      if (SL_RF_feventTypeSize > 0) {
        eventSubsetFlag = TRUE;
      }
      else {
        eventSubsetFlag = FALSE;
      }
    }
    else if (mode == SL_RF_REST) {
      if (!(SL_RF_opt & OPT_ANON)) {
        eventSubsetFlag = TRUE;
      }
      else {
        eventSubsetFlag = FALSE;
      }
    }
    else {
      eventSubsetFlag = TRUE;        
    }
  }
  else {
    eventSubsetFlag = FALSE;
  }
  if (eventSubsetFlag == TRUE) {
    switch (mode) {
    case SL_RF_PRED:
      obsSize    = SL_RF_fobservationSize;
      statusPtr  = SL_RF_fresponseIn[SL_RF_statusIndex];
      mpSign     = SL_RF_fmpSign;
      mRecordMap = SL_RF_fmRecordMap;
      mRecordSize = SL_RF_fmRecordSize;
      break;
    default:
      obsSize    = SL_RF_observationSize;
      statusPtr  = SL_RF_responseIn[SL_RF_statusIndex];
      mpSign     = SL_RF_mpSign;
      mRecordMap = SL_RF_mRecordMap;
      mRecordSize = SL_RF_mRecordSize;
      break;
    }
    SL_RF_eIndividualSize = uivector(1, SL_RF_eventTypeSize);
    for (j = 1; j <= SL_RF_eventTypeSize; j++) {
      SL_RF_eIndividualSize[j] = 0;
    }
    if (mRecordSize > 0) {
      for (i = 1; i <= obsSize; i++) {
        statusFlag = FALSE;
        if (mRecordMap[i] == 0) {
          statusFlag = TRUE;
        }
        else {
          if (mpSign[SL_RF_statusIndex][mRecordMap[i]] == 0) {
            statusFlag = TRUE;
          }
        }
        if (statusFlag == TRUE) {
          if ((uint) statusPtr[i] > 0) {
            SL_RF_eIndividualSize[SL_RF_eventTypeIndex[(uint) statusPtr[i]]] ++;
          }
          else {
            for (j=1; j <= SL_RF_eventTypeSize; j++) {
              SL_RF_eIndividualSize[j] ++;
            }
          }
        }
      } 
    }
    else {
      for (i = 1; i <= obsSize; i++) {
        if ((uint) statusPtr[i] > 0) {
          SL_RF_eIndividualSize[SL_RF_eventTypeIndex[(uint) statusPtr[i]]] ++;
        }
        else {
          for (j=1; j <= SL_RF_eventTypeSize; j++) {
            SL_RF_eIndividualSize[j] ++;
          }
        }
      } 
    }
    SL_RF_eIndividualIn = (uint **) new_vvector(1, SL_RF_eventTypeSize, NRUTIL_UPTR);
    for (j = 1; j <= SL_RF_eventTypeSize; j++) {
      SL_RF_eIndividualIn[j] = uivector(1, SL_RF_eIndividualSize[j] + SL_RF_mStatusSize + 1);
    }
    eventCounter = uivector(1, SL_RF_eventTypeSize);
    for (j = 1; j <= SL_RF_eventTypeSize; j++) {
      eventCounter[j] = 0;
    }
    if (mRecordSize > 0) {    
      for (i = 1; i <= obsSize; i++) {
        statusFlag = FALSE;
        if (mRecordMap[i] == 0) {
          statusFlag = TRUE;
        }
        else {
          if (mpSign[SL_RF_statusIndex][mRecordMap[i]] == 0) {
            statusFlag = TRUE;
          }
        }
        if (statusFlag == TRUE) {
          if ((uint) statusPtr[i] > 0) {
            j = SL_RF_eventTypeIndex[(uint) statusPtr[i]];
            eventCounter[j] ++;
            SL_RF_eIndividualIn[j][eventCounter[j]] = i;
          }
          else {
            for (j=1; j <= SL_RF_eventTypeSize; j++) {
              eventCounter[j] ++;
              SL_RF_eIndividualIn[j][eventCounter[j]] = i;
            }
          }
        }
      }
    }
    else {
      for (i = 1; i <= obsSize; i++) {
        if ((uint) statusPtr[i] > 0) {
          j = SL_RF_eventTypeIndex[(uint) statusPtr[i]];
          eventCounter[j] ++;
          SL_RF_eIndividualIn[j][eventCounter[j]] = i;
        }
        else {
          for (j=1; j <= SL_RF_eventTypeSize; j++) {
            eventCounter[j] ++;
            SL_RF_eIndividualIn[j][eventCounter[j]] = i;
          }
        }
      }
    }
    free_uivector(eventCounter, 1, SL_RF_eventTypeSize);
  }  
  return TRUE;
}
void getEventInfo(char mode) {
  uint    obsSize;
  double *status;
  uint   *mRecordMap;
  int   **mpSign;
  uint    mRecordSize;
  uint statusFlag;
  uint leadingIndex;
  uint i, j;
  uint jgrow;
  if (SL_RF_statusIndex == 0) {
    SL_RF_nativeError("\nRF-SRC: *** ERROR *** ");
    SL_RF_nativeError("\nRF-SRC: Attempt to stack competing risk structures in the absence of SURV data.");
    SL_RF_nativeError("\nRF-SRC: Please Contact Technical Support.");
    SL_RF_nativeExit();
  }
  if (mode == SL_RF_PRED) {
    obsSize    = SL_RF_fobservationSize;
    status     = SL_RF_fresponseIn[SL_RF_statusIndex];
    mRecordMap = SL_RF_fmRecordMap;
    mpSign     = SL_RF_fmpSign;
    mRecordSize = SL_RF_fmRecordSize;
  }
  else {
    obsSize    = SL_RF_observationSize;
    status     = SL_RF_responseIn[SL_RF_statusIndex];
    mRecordMap = SL_RF_mRecordMap;
    mpSign     = SL_RF_mpSign;
    mRecordSize = SL_RF_mRecordSize;    
  }
  SL_RF_mStatusSize = 0;
  uint *eventTypeLocal = uivector(1, obsSize);
  uint eventTypeSizeLocal = 0;
  if (mRecordSize > 0) {
    for (i = 1; i <= obsSize; i++) {
      eventTypeLocal[i] = 0;
      statusFlag = FALSE;
      if (mRecordMap[i] == 0) {
        statusFlag = TRUE;
      }
      else {
        if (mpSign[SL_RF_statusIndex][mRecordMap[i]] == 0) {
          statusFlag = TRUE;
        }
      }
      if (statusFlag == TRUE) {
        if ((uint) status[i] > 0) {
          eventTypeSizeLocal ++;
          eventTypeLocal[eventTypeSizeLocal] = (uint) status[i];
        } 
        else {
        }
      }
      else {
        SL_RF_mStatusSize ++;
      }
    }  
  }
  else {
    for (i = 1; i <= obsSize; i++) {
      eventTypeLocal[i] = 0;
      if ((uint) status[i] > 0) {
        eventTypeSizeLocal ++;
        eventTypeLocal[eventTypeSizeLocal] = (uint) status[i];
      } 
      else {
      }
    }
  }
  if (mode == SL_RF_PRED) {
    if(eventTypeSizeLocal > 0) {
      hpsortui(eventTypeLocal, eventTypeSizeLocal);
      leadingIndex = 1;
      for (i = 2; i <= eventTypeSizeLocal; i++) {
        if (eventTypeLocal[i] > eventTypeLocal[leadingIndex]) {
          leadingIndex++;
          eventTypeLocal[leadingIndex] = eventTypeLocal[i];
        }
      }
      eventTypeSizeLocal = leadingIndex;
    }
    if (eventTypeSizeLocal > 0) {
      SL_RF_feventTypeSize = eventTypeSizeLocal;
    }
    else {
      SL_RF_feventTypeSize = 0;
    }
    if (SL_RF_feventTypeSize == 0) {
      if (!(SL_RF_opt & OPT_PERF) && !(SL_RF_opt & OPT_VIMP)) {
      }
      else {
        SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
        SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
        SL_RF_nativeError("\nRF-SRC:  Performance or vimp has been requested.");
        SL_RF_nativeError("\nRF-SRC:  The test or pseudo-train data set does not contain any events.");
        SL_RF_nativeExit();
      }
    }
    else {
      char consistencyFlag = TRUE;
      if (SL_RF_eventTypeSize > 1) {
        for (j = 1; j <= SL_RF_feventTypeSize; j++) {
          for (jgrow = 1; jgrow <= SL_RF_eventTypeSize; jgrow++) {
            if (eventTypeLocal[j] != SL_RF_eventType[jgrow]) {
              if (jgrow == SL_RF_eventTypeSize) {
                consistencyFlag = FALSE;
              }
            }
            else {
              jgrow = SL_RF_eventTypeSize;
            }
          }
        }
      }
      if (consistencyFlag == FALSE) {
        SL_RF_nativeError("\nRF-SRC: *** ERROR *** ");
        SL_RF_nativeError("\nRF-SRC: Unknown event type encountered in PRED mode. ");
        SL_RF_nativeError("\nRF-SRC: Please Contact Technical Support.");
        SL_RF_nativeExit();
      }
    }
  }
  free_uivector(eventTypeLocal, 1, obsSize);
}
void unstackCompetingArrays(char mode) {
  char eventSubsetFlag;
  uint j;
    if (SL_RF_statusIndex == 0) {
      SL_RF_nativeError("\nRF-SRC: *** ERROR *** ");
      SL_RF_nativeError("\nRF-SRC: Attempt to unstack competing risk structures in the absence of SURV data.");
      SL_RF_nativeError("\nRF-SRC: Please Contact Technical Support.");
      SL_RF_nativeExit();
    }
    if (SL_RF_eventTypeSize == 0) {
    }
    else {
      free_uivector(SL_RF_eventTypeIndex, 1, SL_RF_eventType[SL_RF_eventTypeSize]);
    }
    if (SL_RF_eventTypeSize > 1) {
      if (mode == SL_RF_PRED) {
        if (SL_RF_feventTypeSize > 0) {
          eventSubsetFlag = TRUE;
        }
        else {
          eventSubsetFlag = FALSE;
        }
      }
      else if (mode == SL_RF_REST) {
        if (!(SL_RF_opt & OPT_ANON)) {
          eventSubsetFlag = TRUE;
        }
        else {
          eventSubsetFlag = FALSE;
        }
      }
      else {
        eventSubsetFlag = TRUE;        
      }
    }
    else {
      eventSubsetFlag = FALSE;
    }
    if (eventSubsetFlag == TRUE) {
      for (j = 1; j <= SL_RF_eventTypeSize; j++) {
        free_uivector(SL_RF_eIndividualIn[j], 1, SL_RF_eIndividualSize[j] + SL_RF_mStatusSize + 1);
      }
      free_new_vvector(SL_RF_eIndividualIn, 1, SL_RF_eventTypeSize, NRUTIL_UPTR);
      free_uivector(SL_RF_eIndividualSize, 1, SL_RF_eventTypeSize);
    }  
}
char stackClassificationArrays(char mode) {
  uint  minorityClassID, minorityClassCnt;
  uint  majorityClassID, majorityClassCnt;
  uint i, j, k;
  if (SL_RF_rFactorCount == 0) {
    SL_RF_nativeError("\nRF-SRC: *** ERROR *** ");
    SL_RF_nativeError("\nRF-SRC: Attempt to stack classification structures in the absence of CLAS data.");
    SL_RF_nativeError("\nRF-SRC: Please Contact Technical Support.");
    SL_RF_nativeExit();
  }
  SL_RF_classLevelSize = uivector(1, SL_RF_rFactorCount);
  SL_RF_classLevel = (uint **) new_vvector(1, SL_RF_rFactorCount, NRUTIL_UPTR);
  SL_RF_rFactorMinorityFlag = cvector(1, SL_RF_rFactorCount);
  
  SL_RF_rLevels = (uint **) new_vvector(1, SL_RF_rFactorCount, NRUTIL_UPTR);  
  for (k = 1; k <= SL_RF_rFactorCount; k++) {
    if (SL_RF_rLevelsCnt[k] > 0) {
      SL_RF_classLevelSize[k] = SL_RF_rLevelsCnt[k];
      SL_RF_rLevels[k] = (uint *) INTEGER(VECTOR_ELT(SL_RF_rLevelsSEXP, k-1));
      SL_RF_rLevels[k] --;
      SL_RF_classLevel[k] = SL_RF_rLevels[k];
    }
    else {
      SL_RF_nativeError("\nRF-SRC: *** ERROR *** ");
      SL_RF_nativeError("\nRF-SRC: Inconsistent zero-level count in factor:  compressed-index = %10d, y-index = %10d", k, SL_RF_rFactorIndex[k]);
      SL_RF_nativeError("\nRF-SRC: Please Contact Technical Support.");
      SL_RF_nativeExit();
    }
  }
  
  /*
  SL_RF_rLevels = (uint **) copy2DObject(SL_RF_rLevelsJNIE, NATIVE_TYPE_INTEGER, &SL_RF_nat2DInfoListSize);
  for (k = 1; k <= SL_RF_rFactorCount; k++) {
    if (SL_RF_rLevelsCnt[k] > 0) {
      SL_RF_classLevelSize[k] = SL_RF_rLevelsCnt[k];
      SL_RF_classLevel[k] = SL_RF_rLevels[k];
    }
    else {
      SL_RF_nativeError("\nRF-SRC: *** ERROR *** ");
      SL_RF_nativeError("\nRF-SRC: Inconsistent zero-level count in factor:  compressed-index = %10d, y-index = %10d", k, SL_RF_rFactorIndex[k]);
      SL_RF_nativeError("\nRF-SRC: Please Contact Technical Support.");
      SL_RF_nativeExit();
    }
  }
  */
  /*
  SL_RF_rLevels = (uint **) copyXDObject2D(SL_RF_rLevelsPYTH, &SL_RF_natXDInfoListSize);
  for (k = 1; k <= SL_RF_rFactorCount; k++) {
    if (SL_RF_rLevelsCnt[k] > 0) {
      SL_RF_classLevelSize[k] = SL_RF_rLevelsCnt[k];
      SL_RF_classLevel[k] = SL_RF_rLevels[k];
    }
    else {
      SL_RF_nativeError("\nRF-SRC: *** ERROR *** ");
      SL_RF_nativeError("\nRF-SRC: Inconsistent zero-level count in factor:  compressed-index = %10d, y-index = %10d", k, SL_RF_rFactorIndex[k]);
      SL_RF_nativeError("\nRF-SRC: Please Contact Technical Support.");
      SL_RF_nativeExit();
    }
  }
  */
  SL_RF_classLevelIndex = (uint **) new_vvector(1, SL_RF_rFactorCount, NRUTIL_UPTR);
  for (k = 1; k <= SL_RF_rFactorCount; k++) {
    SL_RF_rFactorMinorityFlag[k] = FALSE;
    SL_RF_classLevelIndex[k] = uivector(1, SL_RF_rFactorSize[k]);
    for (j = 1; j <= SL_RF_rFactorSize[k]; j++) {
      SL_RF_classLevelIndex[k][j] = 0;
    }
    for (j = 1; j <= SL_RF_classLevelSize[k]; j++) {
      SL_RF_classLevelIndex[k][SL_RF_classLevel[k][j]] = j;
    }
  }  
  if (SL_RF_opt & OPT_PERF) {
    if (SL_RF_opt & OPT_CLAS_RFQ) {
      SL_RF_rFactorMinority = uivector(1, SL_RF_rFactorCount);
      SL_RF_rFactorMajority = uivector(1, SL_RF_rFactorCount);
      SL_RF_rFactorThreshold = dvector(1, SL_RF_rFactorCount);
      uint totalCount;
      for (j = 1; j <= SL_RF_rFactorCount; j++) {
        uint *levelCount = uivector(1, SL_RF_rFactorSize[j]);
        totalCount = 0;
        for (k = 1; k <= SL_RF_rFactorSize[j]; k++) {
          levelCount[k] = 0;
        }
        for (i = 1; i <= SL_RF_observationSize; i++) {
          if (!SL_RF_nativeIsNaN(SL_RF_responseIn[SL_RF_rFactorIndex[j]][i])) {
            levelCount[(uint) SL_RF_responseIn[SL_RF_rFactorIndex[j]][i]] ++;
            totalCount ++;
          }
        }
        minorityClassCnt = levelCount[1];
        minorityClassID = 1;
        for (k = 1; k <= SL_RF_rFactorSize[j]; k++) {
          if (levelCount[k] > 0) {
            if (levelCount[k] < minorityClassCnt) {
              minorityClassCnt = levelCount[k];
              minorityClassID = k;
            }
          }
        }
        SL_RF_rFactorMinority[j] = minorityClassID;
        majorityClassCnt = levelCount[1];
        majorityClassID = 1;
        for (k = 1; k <= SL_RF_rFactorSize[j]; k++) {
          if (levelCount[k] >= majorityClassCnt) {
            majorityClassCnt = levelCount[k];
            majorityClassID = k;
          }
        }
        SL_RF_rFactorMajority[j] = majorityClassID;
        SL_RF_rFactorThreshold[j] = (double) levelCount[SL_RF_rFactorMinority[j]] / totalCount;
        free_uivector(levelCount, 1, SL_RF_rFactorSize[j]);
      }
    }
    for (j = 1; j <= SL_RF_rFactorCount; j++) {
      if (SL_RF_rFactorSize[j] == 2) {
        SL_RF_rFactorMinorityFlag[j] = TRUE;
      }
    }
  }
  if (mode == SL_RF_PRED) {
    SL_RF_rFactorSizeTest = uivector(1, SL_RF_rFactorCount);
    if (SL_RF_frSize > 0) {
      for (k = 1; k <= SL_RF_rFactorCount; k++) {
        SL_RF_rFactorSizeTest[k] = SL_RF_rFactorSize[k];
        for (i = 1; i <= SL_RF_fobservationSize; i++) {
          if (!SL_RF_nativeIsNaN(SL_RF_fresponseIn[SL_RF_rFactorIndex[k]][i])) {
            if ((uint) SL_RF_fresponseIn[SL_RF_rFactorIndex[k]][i] > SL_RF_rFactorSize[k]) {
              SL_RF_rFactorSizeTest[k] = (uint) SL_RF_fresponseIn[SL_RF_rFactorIndex[k]][i];
            }
          }
        }
      }
    }
  }
  return TRUE;
}
void unstackClassificationArrays(char mode) {
  uint k;
  if (SL_RF_rFactorCount == 0) {
    SL_RF_nativeError("\nRF-SRC: *** ERROR *** ");
    SL_RF_nativeError("\nRF-SRC: Attempt to unstack classification structures in the absence of CLAS data.");
    SL_RF_nativeError("\nRF-SRC: Please Contact Technical Support.");
    SL_RF_nativeExit();
  }
  for (k = 1; k <= SL_RF_rFactorCount; k++) {
    free_uivector(SL_RF_classLevelIndex[k], 1, SL_RF_rFactorSize[k]);
  }
  free_new_vvector(SL_RF_classLevelIndex, 1, SL_RF_rFactorCount, NRUTIL_UPTR);
  free_uivector(SL_RF_classLevelSize, 1, SL_RF_rFactorCount);
  free_new_vvector(SL_RF_classLevel, 1, SL_RF_rFactorCount, NRUTIL_UPTR);
  free_cvector(SL_RF_rFactorMinorityFlag, 1, SL_RF_rFactorCount);
  
  free_new_vvector(SL_RF_rLevels, 1, SL_RF_rFactorCount, NRUTIL_UPTR);  
  
  if (SL_RF_opt & OPT_PERF) {
    if (SL_RF_opt & OPT_CLAS_RFQ) {
      free_dvector(SL_RF_rFactorThreshold, 1, SL_RF_rFactorCount);
      free_uivector(SL_RF_rFactorMinority, 1, SL_RF_rFactorCount);
      free_uivector(SL_RF_rFactorMajority, 1, SL_RF_rFactorCount);
    }
  }
  if (mode == SL_RF_PRED) {
    free_uivector(SL_RF_rFactorSizeTest, 1, SL_RF_rFactorCount);
  }
}
