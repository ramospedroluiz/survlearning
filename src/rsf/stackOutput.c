/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "stackOutput.h"
#include "importance.h"
#include "splitGreedy.h"
#include "sexpOutgoing.h"
#include "nativeUtil.h"
#include "nrutil.h"
#include "error.h"
void stackDefinedOutputObjects(char      mode,
                               char    **SL_RF_sexpString,
                               Node   ***pRF_root,
                               uint    **pRF_tLeafCount,
                               double  **pRF_proximity,
                               double  **pRF_distance,
                               double  **pRF_weight,
                               double  **p_imputation,
                               double ***pRF_sImputeResponsePtr,
                               double ***pRF_sImputePredictorPtr,
                               uint    **pRF_varUsed,
                               uint   ***pRF_varUsedPtr,
                               double  **p_splitDepth) {
  uint sexpIdentity;
  ulong localSize;
  uint xVimpSize;
  uint  obsSize;
  uint  mRecordSize;
  uint *mRecordIndex;
  double **responsePtr;
  double **predictorPtr;
  uint     rspSize;
  uint     dpthDimOne;
  double **ensembleDen;
  double **ensembleSRG;
  double **ensembleMRT;
  double **ensembleCIF;
  double **ensembleSRV;
  double **ensembleCLS;
  double **ensembleRGR;
  double **ensembleQNT;
  double ****ensembleSRGptr;
  double  ***ensembleMRTptr;
  double  ***ensembleSRVptr;
  double ****ensembleCIFptr;
  double ****ensembleCLSptr;
  double  ***ensembleRGRptr;
  double      ****ensembleQNTptr;
  uint         ***quantileStreamSize;
  LookUpInfo  ****quantileSearchTree;
  QuantileObj ****quantileHead;
  QuantileObj ****quantileTail;
  uint         ***quantileLinkLength;
  double ****ensembleSRGnum;
  double  ***ensembleMRTnum;
  double  ***ensembleSRVnum;
  double ****ensembleCIFnum;
  double ****ensembleCLSnum;
  double  ***ensembleRGRnum;
  char oobFlag, fullFlag;
  uint dimThree;
  uint i, j, k, m;
  xVimpSize      = 0;  
  dpthDimOne     = 0;  
  obsSize        = 0;  
  mRecordSize    = 0;  
  rspSize        = 0;  
  sexpIdentity   = 0;  
  responsePtr    = NULL;  
  predictorPtr   = NULL;  
  mRecordIndex   = NULL;  
  SL_RF_identityMembershipIndexSize = (SL_RF_bootstrapSize > SL_RF_observationSize ) ? SL_RF_bootstrapSize : SL_RF_observationSize;
  if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
    SL_RF_identityMembershipIndexSize = (SL_RF_bootstrapSize > SL_RF_observationSize ) ? SL_RF_bootstrapSize : SL_RF_observationSize;
  }
  else {
    SL_RF_identityMembershipIndexSize = (SL_RF_bootstrapSize > SL_RF_observationSize ) ? SL_RF_bootstrapSize : SL_RF_observationSize;
  }
  SL_RF_identityMembershipIndex = uivector(1, SL_RF_identityMembershipIndexSize);
  for (i = 1; i <= SL_RF_identityMembershipIndexSize; i++) {
    SL_RF_identityMembershipIndex[i] = i;
  }
  switch (mode) {
  case SL_RF_PRED:
    obsSize = SL_RF_fobservationSize;
    mRecordSize = SL_RF_fmRecordSize;
    rspSize = SL_RF_frSize;
    responsePtr  = SL_RF_fresponseIn;
    predictorPtr = SL_RF_fobservationIn;
    mRecordIndex = SL_RF_fmRecordIndex;
    SL_RF_stackCount = 0;
    SL_RF_stackCount ++;
    SL_RF_stackCount ++;
    if (SL_RF_opt & OPT_FENS) {
      if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
        SL_RF_stackCount += 3;
      }
      else {
        if (SL_RF_rTargetFactorCount > 0) {
          SL_RF_stackCount += 1;
        }
        if (SL_RF_rTargetNonFactorCount > 0) {
          SL_RF_stackCount += 1;
          if (SL_RF_opt & OPT_QUANTLE) {
            SL_RF_stackCount += 1;
          }
        }
      }
    }
    if (SL_RF_opt & OPT_PERF) {
      if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
        SL_RF_stackCount += 1;
      }
      else {
        if (SL_RF_rTargetFactorCount > 0) {
          SL_RF_stackCount += 1;
          if (SL_RF_optHigh & OPT_CSE) {
            SL_RF_stackCount += 1;
          }
        }
        if (SL_RF_rTargetNonFactorCount > 0) {
          SL_RF_stackCount += 1;
          if (SL_RF_optHigh & OPT_CSE) {
            SL_RF_stackCount += 1;
          }
        }
        if ((SL_RF_rTargetFactorCount > 0) || (SL_RF_rTargetNonFactorCount > 0)) {
          if (SL_RF_optHigh & OPT_CSE) {
            SL_RF_stackCount += 1;
          }
        }
      }
    }
    if (SL_RF_opt & OPT_PROX) {
      SL_RF_stackCount += 1;
    }
    if (SL_RF_optHigh & OPT_DIST) {
      SL_RF_stackCount += 1;
    }
    if (SL_RF_optHigh & OPT_WGHT) {
      SL_RF_stackCount += 1;
    }
    if (SL_RF_opt & OPT_MISS_OUT) {
      SL_RF_stackCount += 1;
    }
    if (SL_RF_opt & (OPT_SPLDPTH_1 | OPT_SPLDPTH_2)) {
      SL_RF_stackCount += 1;
    }
    if (SL_RF_opt & OPT_CASE_DPTH) {
      SL_RF_stackCount += 1;
    }
    if (SL_RF_opt & OPT_VIMP) {
      if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
        SL_RF_stackCount += 2;
      }
      else {
        if (SL_RF_rTargetFactorCount > 0) {
          SL_RF_stackCount += 2;
          if (SL_RF_optHigh & OPT_CSV) {
            SL_RF_stackCount += 1;
          }
        }
        if (SL_RF_rTargetNonFactorCount > 0) {
          SL_RF_stackCount += 2;
          if (SL_RF_optHigh & OPT_CSV) {
            SL_RF_stackCount += 1;
          }          
        }
        if ((SL_RF_rTargetFactorCount > 0) || (SL_RF_rTargetNonFactorCount > 0)) {
          if (SL_RF_optHigh & OPT_CSV) {
            SL_RF_stackCount += 1;
          }
        }
      }
    }
    if (SL_RF_optHigh & OPT_MEMB_PRUN) {
      SL_RF_stackCount += 1;
    }
    if (SL_RF_optHigh & OPT_MEMB_USER) {
      SL_RF_stackCount += 2;
    }
    break;
  default:
    obsSize = SL_RF_observationSize;
    mRecordSize = SL_RF_mRecordSize;
    rspSize = SL_RF_ySize;
    responsePtr  = SL_RF_responseIn;
    predictorPtr = SL_RF_observationIn;
    mRecordIndex = SL_RF_mRecordIndex;
    SL_RF_stackCount = 0;
    SL_RF_stackCount ++;
    if (SL_RF_opt & OPT_LEAF) {
      SL_RF_stackCount += 1;
    }
    if (SL_RF_opt & OPT_FENS) {
      if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
        SL_RF_stackCount += 3;
      }
      else {
        if (SL_RF_rTargetFactorCount > 0) {
          SL_RF_stackCount += 1;
        }
        if (SL_RF_rTargetNonFactorCount > 0) {
          SL_RF_stackCount += 1;
          if (SL_RF_opt & OPT_QUANTLE) {
            SL_RF_stackCount += 1;
          }
        }
      }
    }
    if (SL_RF_opt & OPT_OENS) {
      if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
        SL_RF_stackCount += 3;
      }
      else {
        if (SL_RF_rTargetFactorCount > 0) {
          SL_RF_stackCount += 1;
        }
        if (SL_RF_rTargetNonFactorCount > 0) {
          SL_RF_stackCount += 1;
          if (SL_RF_opt & OPT_QUANTLE) {
            SL_RF_stackCount += 1;
          }
        }
      }
    }
    if (SL_RF_opt & OPT_PERF) {
      if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
        SL_RF_stackCount += 1;
      }
      else {
        if (SL_RF_rTargetFactorCount > 0) {
          SL_RF_stackCount += 1;
          if (SL_RF_optHigh & OPT_CSE) {
            SL_RF_stackCount += 1;
          }
        }
        if (SL_RF_rTargetNonFactorCount > 0) {
          SL_RF_stackCount += 1;
          if (SL_RF_optHigh & OPT_CSE) {
            SL_RF_stackCount += 1;
          }
        }
        if ((SL_RF_rTargetFactorCount > 0) || (SL_RF_rTargetNonFactorCount > 0)) {
          if (SL_RF_optHigh & OPT_CSE) {
            SL_RF_stackCount += 1;
          }
        }
      }
    }
    if (SL_RF_opt & OPT_EMPR_RISK) {
      SL_RF_stackCount += 3;
    }
    if (SL_RF_opt & OPT_PROX) {
      SL_RF_stackCount += 1;
    }
    if (SL_RF_optHigh & OPT_DIST) {
      SL_RF_stackCount += 1;
    }
    if (SL_RF_optHigh & OPT_WGHT) {
      SL_RF_stackCount += 1;
    }
    if (SL_RF_opt & OPT_SEED) {
      if (SL_RF_opt & OPT_TREE) {
        SL_RF_stackCount += 1;
        uint bnpSize = getVimpRecoverySeedDimension(mode, SL_RF_opt);
        if (bnpSize > 0) {
          SL_RF_stackCount += 1;
        }
        SL_RF_stackCount += 1;
        SL_RF_stackCount += 4;
        SL_RF_stackCount += 5;
        SL_RF_stackCount += 1;
        SL_RF_stackCount += 1;
      }
      else {
        SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
        SL_RF_nativeError("\nRF-SRC:  S.E.X.P. TREE output request inconsistent.");
        SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
        SL_RF_nativeExit();
      }
    }
    if (SL_RF_opt & OPT_MISS_OUT) {
      SL_RF_stackCount += 1;
    }
    if (SL_RF_opt & (OPT_VARUSED_F | OPT_VARUSED_T)) {
      SL_RF_stackCount += 1;
    }
    if (SL_RF_opt & (OPT_SPLDPTH_1 | OPT_SPLDPTH_2)) {
      SL_RF_stackCount += 1;
    }
    if (SL_RF_opt & OPT_CASE_DPTH) {
      SL_RF_stackCount += 1;
    }
    if (SL_RF_opt & OPT_VIMP) {
      if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
        SL_RF_stackCount += 2;
      }
      else {
        if (SL_RF_rTargetFactorCount > 0) {
          SL_RF_stackCount += 2;
          if (SL_RF_optHigh & OPT_CSV) {
            SL_RF_stackCount += 1;
          }
        }
        if (SL_RF_rTargetNonFactorCount > 0) {
          SL_RF_stackCount += 2;
          if (SL_RF_optHigh & OPT_CSV) {
            SL_RF_stackCount += 1;
          }          
        }
        if ((SL_RF_rTargetFactorCount > 0) || (SL_RF_rTargetNonFactorCount > 0)) {
          if (SL_RF_optHigh & OPT_CSV) {
            SL_RF_stackCount += 1;
          }
        }
      }
    }
    if ((SL_RF_vtry > 0) && (SL_RF_vtryMode != SL_RF_VTRY_NULL)) {
      SL_RF_stackCount += 1;
      if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
        SL_RF_stackCount += 1;
      }
      else {
        if (SL_RF_rTargetFactorCount > 0) {
          SL_RF_stackCount += 1;
        }
        if (SL_RF_rTargetNonFactorCount > 0) {
          SL_RF_stackCount += 1;
        }
      }
    }
    if (SL_RF_optHigh & OPT_MEMB_PRUN) {
      SL_RF_stackCount += 1;
    }
    if (SL_RF_optHigh & OPT_MEMB_USER) {
      SL_RF_stackCount += 2;
    }
    if (SL_RF_optHigh & OPT_MEMB_OUTG) {
      SL_RF_stackCount += 10;
    }
    if (SL_RF_optHigh & OPT_TERM_OUTG) {
      if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
        SL_RF_stackCount += 1;
        if (!(SL_RF_opt & OPT_COMP_RISK)) {
          SL_RF_stackCount += 1;
          SL_RF_stackCount += 1;
        }
        else {
          SL_RF_stackCount += 2;
        }
      }
      else {
        if (SL_RF_rTargetNonFactorCount > 0) {
          SL_RF_stackCount += 1;
        }
        if (SL_RF_rTargetFactorCount > 0) {
          SL_RF_stackCount += 1;
        }
      }
    }
    if (SL_RF_optHigh & OPT_PART_PLOT) {
      if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
        SL_RF_stackCount += 1;
      }
      else {
        if (SL_RF_rTargetFactorCount > 0) {
          SL_RF_stackCount += 1;
        }
        if (SL_RF_rTargetNonFactorCount > 0) {
          SL_RF_stackCount += 1;
        }
      }
    }
    break;
  }  
  if (SL_RF_xMarginalSize > 0) {
    if (SL_RF_xMarginalSize <= SL_RF_xSize) {
      for (uint i = 1; i <= SL_RF_xMarginalSize; i++) {
        if ((SL_RF_xMarginal[i] < 1) || (SL_RF_xMarginal[i] > SL_RF_xSize)) {
          SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
          SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
          SL_RF_nativeError("\nRF-SRC:  Marginal predictor must be greater than zero and less than or equal to %10d:  %10d \n", SL_RF_xSize, SL_RF_xMarginal[i]);
          SL_RF_nativeExit();
        }
      }
    }
    else {
      SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
      SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
      SL_RF_nativeError("\nRF-SRC:  Number of marginal predictors must be greater than zero and less than or equal to %10d:  %10d \n", SL_RF_xSize, SL_RF_xMarginalSize);
      SL_RF_nativeExit();
    }
    SL_RF_utTermMembership      =  (uint ***) new_vvector(1, SL_RF_ntree, NRUTIL_UPTR2);
    SL_RF_utTermMembershipCount =  (uint **) new_vvector(1, SL_RF_ntree, NRUTIL_UPTR);
    SL_RF_utTermMembershipAlloc =  (uint **) new_vvector(1, SL_RF_ntree, NRUTIL_UPTR);
    SL_RF_xMarginalFlag         =  uivector(1, SL_RF_xSize);
    for (i = 1; i <= SL_RF_xSize; i++) {
      SL_RF_xMarginalFlag[i] = FALSE;
    }
    for (i = 1; i <= SL_RF_xMarginalSize; i++) {
      SL_RF_xMarginalFlag[SL_RF_xMarginal[i]] = TRUE;
    }
  }
  initProtect(SL_RF_stackCount);
  stackAuxiliaryInfoList(&SL_RF_snpAuxiliaryInfoList, SL_RF_stackCount);
  oobFlag = fullFlag = FALSE;
  if ((SL_RF_opt & OPT_FENS) || (SL_RF_opt & OPT_OENS)) {
    if (SL_RF_opt & OPT_FENS) {
      fullFlag = TRUE;
    }
    if (SL_RF_opt & OPT_OENS) {
      oobFlag = TRUE;
    }
    while ((oobFlag == TRUE) || (fullFlag == TRUE)) {
      ensembleDen    = NULL;
      ensembleSRG    = NULL;
      ensembleSRGptr = NULL;
      ensembleSRGnum = NULL;
      ensembleMRT    = NULL;
      ensembleMRTptr = NULL;
      ensembleMRTnum = NULL;
      ensembleSRV    = NULL;
      ensembleSRVptr = NULL;
      ensembleSRVnum = NULL;
      ensembleCIF    = NULL;
      ensembleCIFptr = NULL;
      ensembleCIFnum = NULL;
      ensembleCLS    = NULL;
      ensembleCLSptr = NULL;
      ensembleCLSnum = NULL;
      ensembleRGR    = NULL;
      ensembleRGRptr = NULL;
      ensembleRGRnum = NULL;
      ensembleQNT        = NULL;
      ensembleQNTptr     = NULL;
      quantileStreamSize = NULL;
      quantileSearchTree = NULL;
      quantileHead       = NULL;
      quantileTail       = NULL;
      quantileLinkLength = NULL;
      if (oobFlag == TRUE) {
        ensembleDen    = &SL_RF_oobEnsembleDen;
        ensembleSRG    = &SL_RF_oobEnsembleSRG_;
        ensembleSRGptr = &SL_RF_oobEnsembleSRGptr;
        ensembleSRGnum = &SL_RF_oobEnsembleSRGnum;
        ensembleMRT    = &SL_RF_oobEnsembleMRT_;
        ensembleMRTptr = &SL_RF_oobEnsembleMRTptr;
        ensembleMRTnum = &SL_RF_oobEnsembleMRTnum;
        ensembleSRV    = &SL_RF_oobEnsembleSRV_;
        ensembleSRVptr = &SL_RF_oobEnsembleSRVptr;
        ensembleSRVnum = &SL_RF_oobEnsembleSRVnum;
        ensembleCIF    = &SL_RF_oobEnsembleCIF_;
        ensembleCIFptr = &SL_RF_oobEnsembleCIFptr;
        ensembleCIFnum = &SL_RF_oobEnsembleCIFnum;
        ensembleCLS    = &SL_RF_oobEnsembleCLS_;
        ensembleCLSptr = &SL_RF_oobEnsembleCLSptr;
        ensembleCLSnum = &SL_RF_oobEnsembleCLSnum;
        ensembleRGR    = &SL_RF_oobEnsembleRGR_;
        ensembleRGRptr = &SL_RF_oobEnsembleRGRptr;
        ensembleRGRnum = &SL_RF_oobEnsembleRGRnum;
        ensembleQNT         = &SL_RF_oobEnsembleQNT_;
        ensembleQNTptr      = &SL_RF_oobEnsembleQNTptr;        
        quantileStreamSize  = &SL_RF_oobQuantileStreamSize;
        quantileSearchTree  = &SL_RF_oobQuantileSearchTree;
        quantileHead        = &SL_RF_oobQuantileHead;
        quantileTail        = &SL_RF_oobQuantileTail;
        quantileLinkLength  = &SL_RF_oobQuantileLinkLength;
      }
      else {
        ensembleDen    = &SL_RF_fullEnsembleDen;
        ensembleSRG    = &SL_RF_fullEnsembleSRG_;
        ensembleSRGptr = &SL_RF_fullEnsembleSRGptr;
        ensembleSRGnum = &SL_RF_fullEnsembleSRGnum;
        ensembleMRT    = &SL_RF_fullEnsembleMRT_;
        ensembleMRTptr = &SL_RF_fullEnsembleMRTptr;
        ensembleMRTnum = &SL_RF_fullEnsembleMRTnum;        
        ensembleSRV    = &SL_RF_fullEnsembleSRV_;
        ensembleSRVptr = &SL_RF_fullEnsembleSRVptr;
        ensembleSRVnum = &SL_RF_fullEnsembleSRVnum;
        ensembleCIF    = &SL_RF_fullEnsembleCIF_;
        ensembleCIFptr = &SL_RF_fullEnsembleCIFptr;
        ensembleCIFnum = &SL_RF_fullEnsembleCIFnum;
        ensembleCLS    = &SL_RF_fullEnsembleCLS_;
        ensembleCLSptr = &SL_RF_fullEnsembleCLSptr;
        ensembleCLSnum = &SL_RF_fullEnsembleCLSnum;
        ensembleRGR    = &SL_RF_fullEnsembleRGR_;
        ensembleRGRptr = &SL_RF_fullEnsembleRGRptr;
        ensembleRGRnum = &SL_RF_fullEnsembleRGRnum;
        ensembleQNT         = &SL_RF_fullEnsembleQNT_;
        ensembleQNTptr      = &SL_RF_fullEnsembleQNTptr;        
        quantileStreamSize  = &SL_RF_fullQuantileStreamSize;
        quantileSearchTree  = &SL_RF_fullQuantileSearchTree;
        quantileHead        = &SL_RF_fullQuantileHead;
        quantileTail        = &SL_RF_fullQuantileTail;
        quantileLinkLength  = &SL_RF_fullQuantileLinkLength;
      }
      *ensembleDen = dvector(1, obsSize);
      for (i = 1; i <= obsSize; i++) {
        (*ensembleDen)[i] = 0;
      }
      if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
        (oobFlag == TRUE) ? (sexpIdentity = SL_RF_OSRG_ID) : ((fullFlag == TRUE) ? sexpIdentity = SL_RF_ASRG_ID : TRUE);
        localSize = (ulong) SL_RF_eventTypeSize * SL_RF_sortedTimeInterestSize * obsSize;
        *ensembleSRG = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, sexpIdentity, localSize, 0, SL_RF_sexpString[sexpIdentity], ensembleSRGptr, 3, SL_RF_eventTypeSize, SL_RF_sortedTimeInterestSize, obsSize);
        *ensembleSRGnum = (double ***) new_vvector(1, SL_RF_eventTypeSize, NRUTIL_DPTR2);
        for (j = 1; j <= SL_RF_eventTypeSize; j++) {
          (*ensembleSRGnum)[j] = (double **) new_vvector(1, SL_RF_sortedTimeInterestSize, NRUTIL_DPTR);
          for (k = 1; k <= SL_RF_sortedTimeInterestSize; k++) {
            (*ensembleSRGnum)[j][k]  = dvector(1, obsSize);
            for (i = 1; i <= obsSize; i++) {
              (*ensembleSRGnum)[j][k][i] = 0.0;
            }
          }
        }
        (oobFlag == TRUE) ? (sexpIdentity = SL_RF_OMRT_ID) : ((fullFlag == TRUE) ? sexpIdentity = SL_RF_AMRT_ID: TRUE);
        localSize = (ulong) SL_RF_eventTypeSize * obsSize;
        *ensembleMRT = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, sexpIdentity, localSize, 0, SL_RF_sexpString[sexpIdentity], ensembleMRTptr, 2, SL_RF_eventTypeSize, obsSize);
        *ensembleMRTnum = (double **) new_vvector(1, SL_RF_eventTypeSize, NRUTIL_DPTR);
        for (j = 1; j <= SL_RF_eventTypeSize; j++) {
          (*ensembleMRTnum)[j] = dvector(1, obsSize);
          for (i = 1; i <= obsSize; i++) {
            (*ensembleMRTnum)[j][i] = 0.0;
          }
        }
        if (!(SL_RF_opt & OPT_COMP_RISK)) {
          (oobFlag == TRUE) ? (sexpIdentity = SL_RF_OSRV_ID) : ((fullFlag == TRUE) ? sexpIdentity = SL_RF_ASRV_ID: TRUE);
          localSize = (ulong) SL_RF_sortedTimeInterestSize * obsSize;
          *ensembleSRV = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, sexpIdentity, localSize, 0, SL_RF_sexpString[sexpIdentity], ensembleSRVptr, 2, SL_RF_sortedTimeInterestSize, obsSize);
          *ensembleSRVnum = (double **) new_vvector(1, SL_RF_sortedTimeInterestSize, NRUTIL_DPTR);
          for (j = 1; j <= SL_RF_sortedTimeInterestSize; j++) {
            (*ensembleSRVnum)[j]  = dvector(1, obsSize);
            for (i = 1; i <= obsSize; i++) {
              (*ensembleSRVnum)[j][i]  = 0.0;
            }
          }
        }  
        else {
          (oobFlag == TRUE) ? (sexpIdentity = SL_RF_OCIF_ID) : ((fullFlag == TRUE) ? sexpIdentity = SL_RF_ACIF_ID: TRUE);
          localSize = (ulong) SL_RF_eventTypeSize * SL_RF_sortedTimeInterestSize * obsSize;
          *ensembleCIF = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, sexpIdentity, localSize, 0, SL_RF_sexpString[sexpIdentity], ensembleCIFptr, 3, SL_RF_eventTypeSize, SL_RF_sortedTimeInterestSize, obsSize);
          *ensembleCIFnum = (double ***) new_vvector(1, SL_RF_eventTypeSize, NRUTIL_DPTR2);
          for (j = 1; j <= SL_RF_eventTypeSize; j++) {
            (*ensembleCIFnum)[j] = (double **) new_vvector(1, SL_RF_sortedTimeInterestSize, NRUTIL_DPTR);
            for (k = 1; k <= SL_RF_sortedTimeInterestSize; k++) {
              (*ensembleCIFnum)[j][k]  = dvector(1, obsSize);
              for (i = 1; i <= obsSize; i++) {
                (*ensembleCIFnum)[j][k][i] = 0.0;
              }
            }
          }
        }  
      }  
      else {
        if (SL_RF_rTargetFactorCount > 0) {
          (oobFlag == TRUE) ? (sexpIdentity = SL_RF_OCLS_ID) : ((fullFlag == TRUE) ? sexpIdentity = SL_RF_ACLS_ID: TRUE);
          localSize = 0;
          for (j = 1; j <= SL_RF_rTargetFactorCount; j++) {
            for (k = 1; k <= SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[j]]]; k++) {
              localSize += (ulong) obsSize;
            }
          }
          *ensembleCLS = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, sexpIdentity, localSize, 0, SL_RF_sexpString[sexpIdentity], ensembleCLSptr, 3, SL_RF_rTargetFactorCount, 0, obsSize);
          *ensembleCLSnum = (double ***) new_vvector(1, SL_RF_rTargetFactorCount, NRUTIL_DPTR2);
          localSize = 0;
          for (j = 1; j <= SL_RF_rTargetFactorCount; j++) {
            (*ensembleCLSnum)[j] = (double **) new_vvector(1, SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[j]]], NRUTIL_DPTR);
            for (k = 1; k <= SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[j]]]; k++) {
              (*ensembleCLSnum)[j][k]  = dvector(1, obsSize);
              localSize += (ulong) obsSize;
              for (i = 1; i <= obsSize; i++) {
                (*ensembleCLSnum)[j][k][i] = 0.0;
              }
            }
          }
        }
        if (SL_RF_rTargetNonFactorCount > 0) {
          (oobFlag == TRUE) ? (sexpIdentity = SL_RF_ORGR_ID) : ((fullFlag == TRUE) ? sexpIdentity = SL_RF_ARGR_ID: TRUE);
          localSize = (ulong) SL_RF_rTargetNonFactorCount * obsSize;
          *ensembleRGR = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, sexpIdentity, localSize, 0, SL_RF_sexpString[sexpIdentity], ensembleRGRptr, 2, SL_RF_rTargetNonFactorCount, obsSize);
          (*ensembleRGRnum) = (double **) new_vvector(1, SL_RF_rTargetNonFactorCount, NRUTIL_DPTR);
          for (j = 1; j <= SL_RF_rTargetNonFactorCount; j++) {
            (*ensembleRGRnum)[j] = dvector(1, obsSize);
            for (i = 1; i <= obsSize; i++) {
              (*ensembleRGRnum)[j][i] = 0.0;
            }
          }
          if (SL_RF_opt & OPT_QUANTLE) {
            if ((SL_RF_qEpsilon <= 0.0) || (SL_RF_qEpsilon >= 0.50)) {
              SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
              SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
              SL_RF_nativeError("\nRF-SRC:  Epsilon-approximate quantile threshold must be in the range (0, 1/2):  %10.4f \n", SL_RF_qEpsilon);
              SL_RF_nativeExit();
            }
            else {
              SL_RF_inv_2qEpsilon = 1 / (2 * SL_RF_qEpsilon);
            }
            (oobFlag == TRUE) ? (sexpIdentity = SL_RF_OQNT_ID) : ((fullFlag == TRUE) ? sexpIdentity = SL_RF_AQNT_ID: TRUE);
            localSize = (ulong) SL_RF_rTargetNonFactorCount * SL_RF_quantileSize * obsSize;
            *ensembleQNT = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, sexpIdentity, localSize, 0, SL_RF_sexpString[sexpIdentity], ensembleQNTptr, 3, SL_RF_rTargetNonFactorCount, SL_RF_quantileSize, obsSize);
            *quantileStreamSize = (uint **) new_vvector(1, SL_RF_rTargetNonFactorCount, NRUTIL_UPTR);
            for (j = 1; j <= SL_RF_rTargetNonFactorCount; j++) {
              (*quantileStreamSize)[j] = uivector(1, obsSize);
              for (i = 1; i <= obsSize; i++) {
                (*quantileStreamSize)[j][i] = 0;
              }
            }
            *quantileSearchTree = (LookUpInfo ***) new_vvector(1, SL_RF_rTargetNonFactorCount, NRUTIL_SPTR2);
            for (j = 1; j <= SL_RF_rTargetNonFactorCount; j++) {
              (*quantileSearchTree)[j] = (LookUpInfo **) new_vvector(1, obsSize, NRUTIL_SPTR);
              for (i = 1; i <= obsSize; i++) {
                (*quantileSearchTree)[j][i] = NULL;
              }
            }
            *quantileHead = (QuantileObj ***) new_vvector(1, SL_RF_rTargetNonFactorCount, NRUTIL_QPTR2);
            *quantileTail = (QuantileObj ***) new_vvector(1, SL_RF_rTargetNonFactorCount, NRUTIL_QPTR2);
            *quantileLinkLength = (uint **) new_vvector(1, SL_RF_rTargetNonFactorCount, NRUTIL_UPTR2);
            for (j = 1; j <= SL_RF_rTargetNonFactorCount; j++) {
              (*quantileHead)[j] = (QuantileObj **) new_vvector(1, obsSize, NRUTIL_QPTR);
              (*quantileTail)[j] = (QuantileObj **) new_vvector(1, obsSize, NRUTIL_QPTR);
              (*quantileLinkLength)[j] = uivector(1, obsSize);
              for (i = 1; i <= obsSize; i++) {
                (*quantileHead)[j][i] = NULL;
                (*quantileTail)[j][i] = NULL;
                (*quantileLinkLength)[j][i] = 0;
              }
            }
          }
        }
      }
      if (oobFlag == TRUE) {
        oobFlag = FALSE;
      }
      else {
        fullFlag = FALSE;
      }
    }  
  }
  if (SL_RF_opt & OPT_PERF) {
    if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
      {
        localSize = (ulong) SL_RF_ntree * SL_RF_eventTypeSize; 
        SL_RF_perfMRT_ = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_ER_SURV, localSize, SL_RF_nativeNaN, SL_RF_sexpString[SL_RF_ER_SURV], &SL_RF_perfMRTptr, 2, SL_RF_ntree, SL_RF_eventTypeSize);
      }
    }  
    else {
      if (SL_RF_rTargetFactorCount > 0) {
        localSize = 0;
        for (j = 1; j <= SL_RF_rTargetFactorCount; j++) {
          for (k = 1; k <= 1 + SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[j]]]; k++) {
            localSize += (ulong) SL_RF_ntree;
          }
        }
        SL_RF_perfCLS_ = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_ER_CLAS, localSize, SL_RF_nativeNaN, SL_RF_sexpString[SL_RF_ER_CLAS], &SL_RF_perfCLSptr, 3, SL_RF_ntree, SL_RF_rTargetFactorCount, -1);
        if (SL_RF_optHigh & OPT_CSE) {
          localSize = (ulong) SL_RF_rTargetFactorCount * obsSize;
          SL_RF_cseNumCLS_ = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_CSE_CLS, localSize, 0, SL_RF_sexpString[SL_RF_CSE_CLS], &SL_RF_cseNumCLSptr, 2, SL_RF_rTargetFactorCount, obsSize);
        }
      }
      if (SL_RF_rTargetNonFactorCount > 0) {
        localSize = (ulong) SL_RF_ntree * SL_RF_rTargetNonFactorCount;
        SL_RF_perfRGR_ = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_ER_REGR, localSize, SL_RF_nativeNaN, SL_RF_sexpString[SL_RF_ER_REGR], &SL_RF_perfRGRptr, 2, SL_RF_ntree, SL_RF_rTargetNonFactorCount);
        if (SL_RF_optHigh & OPT_CSE) {
          localSize = (ulong) SL_RF_rTargetNonFactorCount * obsSize;
          SL_RF_cseNumRGR_ = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_CSE_RGR, localSize, 0, SL_RF_sexpString[SL_RF_CSE_RGR], &SL_RF_cseNumRGRptr, 2, SL_RF_rTargetNonFactorCount, obsSize);
        } 
      }
      if ((SL_RF_rTargetFactorCount > 0) || (SL_RF_rTargetNonFactorCount > 0)) {
        if (SL_RF_optHigh & OPT_CSE) {
          SL_RF_cseDen_ = (uint*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_CSE_DEN, obsSize, 0, SL_RF_sexpString[SL_RF_CSE_DEN], &SL_RF_cseDENptr, 1, obsSize);
        }
      }
    }
  }  
  if (SL_RF_opt & OPT_VIMP) {
    SL_RF_vimpMRTstd = NULL;
    SL_RF_vimpCLSstd = NULL;
    SL_RF_vimpRGRstd = NULL;
    SL_RF_vimpEnsembleDen = NULL;
    SL_RF_blkEnsembleMRTnum = NULL;
    SL_RF_blkEnsembleCLSnum = NULL;
    SL_RF_blkEnsembleRGRnum = NULL;
    SL_RF_blkEnsembleDen    = NULL;
    SL_RF_vimpMRTblk = NULL;
    SL_RF_vimpCLSblk = NULL;
    SL_RF_vimpRGRblk = NULL;
    SL_RF_perfMRTblk = NULL;
    SL_RF_perfCLSblk = NULL;
    SL_RF_perfRGRblk = NULL;
    SL_RF_vimpMRTptr = NULL;
    SL_RF_vimpCLSptr = NULL;
    SL_RF_vimpRGRptr = NULL;
  }  
  if ((SL_RF_vtry > 0) && (SL_RF_vtryMode != SL_RF_VTRY_NULL)) {
    SL_RF_perfMRTblk = NULL;
    SL_RF_perfCLSblk = NULL;
    SL_RF_perfRGRblk = NULL;
    SL_RF_holdMRTptr = NULL;
    SL_RF_holdCLSptr = NULL;
    SL_RF_holdRGRptr = NULL;
    SL_RF_holdoutMap = NULL;
    SL_RF_runningHoldoutCount = NULL;
    SL_RF_blockSerialTreeIndex = NULL; 
  }
  if (SL_RF_opt & OPT_VIMP) {
    if (SL_RF_opt & OPT_VIMP_JOIN) {
      xVimpSize = 1;
    }
    else {
      xVimpSize = SL_RF_intrPredictorSize;
    }
    SL_RF_vimpMembership = (Terminal ****) new_vvector(1, xVimpSize, NRUTIL_NPTR3);
    for (k = 1; k <= xVimpSize; k++) {
      SL_RF_vimpMembership[k] = (Terminal ***) new_vvector(1,  SL_RF_ntree, NRUTIL_NPTR2);
    }
    for (k = 1; k <= xVimpSize; k++) {
      for (i = 1; i <= SL_RF_ntree; i++) {
        SL_RF_vimpMembership[k][i] = NULL;
      }
    }
    SL_RF_vimpEnsembleDen  = (double **) new_vvector(1, xVimpSize, NRUTIL_DPTR);
    for (j = 1; j <= xVimpSize; j++) {
      SL_RF_vimpEnsembleDen[j] = dvector(1, obsSize);
      for (i = 1; i <= obsSize; i++) {
        SL_RF_vimpEnsembleDen[j][i] = 0.0;
      }
    }
    SL_RF_blkEnsembleDen = dvector(1, obsSize);
    for (i = 1; i <= obsSize; i++) {
      SL_RF_blkEnsembleDen[i] = 0.0;
    }
    if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
      {
        localSize = (ulong) xVimpSize * SL_RF_eventTypeSize;
        SL_RF_vimpMRT_ = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_VMP_SRG, localSize, SL_RF_nativeNaN, SL_RF_sexpString[SL_RF_VMP_SRG], &SL_RF_vimpMRTptr, 2, xVimpSize, SL_RF_eventTypeSize);
        SL_RF_vimpMRTstd = (double ***) new_vvector(1, xVimpSize, NRUTIL_DPTR2);
        for (j = 1; j <= xVimpSize; j++) {
          SL_RF_vimpMRTstd[j]  = (double **) new_vvector(1, SL_RF_eventTypeSize, NRUTIL_DPTR);
          for (k = 1; k <= SL_RF_eventTypeSize; k++) {
            SL_RF_vimpMRTstd[j][k] = dvector(1, obsSize);
            for (m = 1; m <= obsSize; m++) {
              SL_RF_vimpMRTstd[j][k][m] = 0.0;
            }
          }
        }
        SL_RF_blkEnsembleMRTnum = (double **) new_vvector(1, SL_RF_eventTypeSize, NRUTIL_DPTR);
        for (j = 1; j <= SL_RF_eventTypeSize; j++) {
          SL_RF_blkEnsembleMRTnum[j] = dvector(1, obsSize);
          for (k = 1; k <= obsSize; k++) {
            SL_RF_blkEnsembleMRTnum[j][k] = 0.0;
          }
        }
        SL_RF_vimpMRTblk = (double ***) new_vvector(1, SL_RF_perfBlockCount, NRUTIL_DPTR2);
        for (i = 1; i <= SL_RF_perfBlockCount; i++) {
          SL_RF_vimpMRTblk[i]  = (double **) new_vvector(1, xVimpSize, NRUTIL_DPTR);
          for (j = 1; j <= xVimpSize; j++) {
            SL_RF_vimpMRTblk[i][j] = dvector(1, SL_RF_eventTypeSize);
            for (k = 1; k <= SL_RF_eventTypeSize; k++) {
              SL_RF_vimpMRTblk[i][j][k] = SL_RF_nativeNaN;
            }
          }
        }
        localSize = localSize / xVimpSize * SL_RF_perfBlockCount;
        SL_RF_perfBlockMRT_ = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_BLK_SRG, localSize, SL_RF_nativeNaN, SL_RF_sexpString[SL_RF_BLK_SRG], &SL_RF_perfMRTblk, 2, SL_RF_perfBlockCount, SL_RF_eventTypeSize);
      }
    }  
    else {
      if (SL_RF_rTargetFactorCount > 0) {
        localSize = 0;
        for (j = 1; j <= SL_RF_rTargetFactorCount; j++) {
          localSize += (ulong) 1 + SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[j]]];
        }
        localSize = (ulong) xVimpSize * localSize;
        SL_RF_vimpCLS_ = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_VMP_CLS, localSize, SL_RF_nativeNaN, SL_RF_sexpString[SL_RF_VMP_CLS], &SL_RF_vimpCLSptr, 3, xVimpSize, SL_RF_rTargetFactorCount, -1);
        SL_RF_vimpCLSstd = (double ****) new_vvector(1, xVimpSize, NRUTIL_DPTR3);        
        for (i = 1; i <= xVimpSize; i++) {
          SL_RF_vimpCLSstd[i] = (double ***) new_vvector(1, SL_RF_rTargetFactorCount, NRUTIL_DPTR2);
          for (j = 1; j <= SL_RF_rTargetFactorCount; j++) {
            SL_RF_vimpCLSstd[i][j]  = (double **) new_vvector(1, SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[j]]], NRUTIL_DPTR);
            for (k = 1; k <= SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[j]]]; k++) {
              SL_RF_vimpCLSstd[i][j][k] = dvector(1, obsSize);
              for (m = 1; m <= obsSize; m++) {
                SL_RF_vimpCLSstd[i][j][k][m] = 0.0;
              }
            }
          }
        }
        SL_RF_blkEnsembleCLSnum = (double ***) new_vvector(1, SL_RF_rTargetFactorCount, NRUTIL_DPTR2);
        for (j = 1; j <= SL_RF_rTargetFactorCount; j++) {
          SL_RF_blkEnsembleCLSnum[j] = (double **) new_vvector(1, SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[j]]], NRUTIL_DPTR);
          for (k = 1; k <= SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[j]]]; k++) {
            SL_RF_blkEnsembleCLSnum[j][k]  = dvector(1, obsSize);
            for (m = 1; m <= obsSize; m++) {
              SL_RF_blkEnsembleCLSnum[j][k][m]  = 0.0;
            }
          }
        }
        SL_RF_vimpCLSblk = (double ****) new_vvector(1, SL_RF_perfBlockCount, NRUTIL_DPTR3);
        for (i = 1; i <= SL_RF_perfBlockCount; i++) {
          SL_RF_vimpCLSblk[i] = (double ***) new_vvector(1, xVimpSize, NRUTIL_DPTR2);
          for (j = 1; j <= xVimpSize; j++) {            
            SL_RF_vimpCLSblk[i][j] = (double **) new_vvector(1, SL_RF_rTargetFactorCount, NRUTIL_DPTR);
            for (k = 1; k <= SL_RF_rTargetFactorCount; k++) {
              SL_RF_vimpCLSblk[i][j][k]  = dvector(1, 1 + SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[k]]]);
              for (m = 1; m <= 1 + SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[k]]]; m++) {
                SL_RF_vimpCLSblk[i][j][k][m]  = SL_RF_nativeNaN;
              }
            }
          }
        }
        localSize = localSize / xVimpSize * SL_RF_perfBlockCount;
        SL_RF_perfBlockCLS_ = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_BLK_CLS, localSize, SL_RF_nativeNaN, SL_RF_sexpString[SL_RF_BLK_CLS], &SL_RF_perfCLSblk, 3, SL_RF_perfBlockCount, SL_RF_rTargetFactorCount, -1);
        if (SL_RF_optHigh & OPT_CSV) {
          localSize = (ulong) xVimpSize * SL_RF_rTargetFactorCount * obsSize;
          SL_RF_csvNumCLS_ = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_CSV_CLS, localSize, 0, SL_RF_sexpString[SL_RF_CSV_CLS], &SL_RF_csvNumCLSptr, 3, xVimpSize, SL_RF_rTargetFactorCount, obsSize);
        }
      }
      if (SL_RF_rTargetNonFactorCount > 0) {
        localSize = (ulong) xVimpSize * SL_RF_rTargetNonFactorCount;
        SL_RF_vimpRGR_ = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_VMP_RGR, localSize, SL_RF_nativeNaN, SL_RF_sexpString[SL_RF_VMP_RGR], &SL_RF_vimpRGRptr, 2, xVimpSize, SL_RF_rTargetNonFactorCount);
        SL_RF_vimpRGRstd = (double ***) new_vvector(1, xVimpSize, NRUTIL_DPTR2);
        for (j = 1; j <= xVimpSize; j++) {
          SL_RF_vimpRGRstd[j]  = (double **) new_vvector(1, SL_RF_rTargetNonFactorCount, NRUTIL_DPTR);
          for (k = 1; k <= SL_RF_rTargetNonFactorCount; k++) {
            SL_RF_vimpRGRstd[j][k] = dvector(1, obsSize);
            for (m = 1; m <= obsSize; m++) {
              SL_RF_vimpRGRstd[j][k][m] = 0.0;
            }
          }
        }
        SL_RF_blkEnsembleRGRnum = (double **) new_vvector(1, SL_RF_rTargetNonFactorCount, NRUTIL_DPTR);
        for (j = 1; j <= SL_RF_rTargetNonFactorCount; j++) {            
          SL_RF_blkEnsembleRGRnum[j] = dvector(1, obsSize);   
          for (k= 1; k <= obsSize; k++) {
            SL_RF_blkEnsembleRGRnum[j][k] = 0.0;
          }
        }
        SL_RF_vimpRGRblk = (double ***) new_vvector(1, SL_RF_perfBlockCount, NRUTIL_DPTR2);
        for (i = 1; i <= SL_RF_perfBlockCount; i++) {
          SL_RF_vimpRGRblk[i] = (double **) new_vvector(1, xVimpSize, NRUTIL_DPTR);
          for (j = 1; j <= xVimpSize; j++) {            
            SL_RF_vimpRGRblk[i][j] = dvector(1, SL_RF_rTargetNonFactorCount);             
            for (k= 1; k <= SL_RF_rTargetNonFactorCount; k++) {
              SL_RF_vimpRGRblk[i][j][k] = SL_RF_nativeNaN;
            }
          }
        }
        localSize = localSize / xVimpSize * SL_RF_perfBlockCount;
        SL_RF_perfBlockRGR_ = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_BLK_RGR, localSize, SL_RF_nativeNaN, SL_RF_sexpString[SL_RF_BLK_RGR], &SL_RF_perfRGRblk, 2, SL_RF_perfBlockCount, SL_RF_rTargetNonFactorCount);
        if (SL_RF_optHigh & OPT_CSV) {
          localSize = (ulong) xVimpSize * SL_RF_rTargetNonFactorCount * obsSize;
          SL_RF_csvNumRGR_ = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_CSV_RGR, localSize, 0, SL_RF_sexpString[SL_RF_CSV_RGR], &SL_RF_csvNumRGRptr, 3, xVimpSize, SL_RF_rTargetNonFactorCount, obsSize);
        }
      }
      if ((SL_RF_rTargetFactorCount > 0) || (SL_RF_rTargetNonFactorCount > 0)) {
        if (SL_RF_optHigh & OPT_CSV) {
          localSize = (ulong) xVimpSize * obsSize;          
          SL_RF_csvDen_ = (uint*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_CSV_DEN, localSize, 0, SL_RF_sexpString[SL_RF_CSV_DEN], &SL_RF_csvDENptr, 2, xVimpSize, obsSize);
        }
      }
    }
  }  
  if ((SL_RF_vtry > 0) && (SL_RF_vtryMode != SL_RF_VTRY_NULL)) {
    localSize = (ulong) SL_RF_xSize;
    SL_RF_holdBLK_ = (uint*) stackAndProtect(mode, &SL_RF_nativeIndex,
                                          NATIVE_TYPE_INTEGER,
                                          SL_RF_HLDOUT_BLK,
                                          localSize,
                                          0,
                                          SL_RF_sexpString[SL_RF_HLDOUT_BLK],
                                          &SL_RF_holdBLKptr,
                                          1,
                                          SL_RF_xSize);
    SL_RF_holdoutMap = (uint **) new_vvector(1, SL_RF_xSize, NRUTIL_UPTR);
    uint *localBlockID = uivector(1, SL_RF_xSize);
    uint *iter = uivector(1, SL_RF_xSize);
    SL_RF_blockSerialTreeIndex = (uint ***) new_vvector(1, SL_RF_xSize, NRUTIL_UPTR2);
    SL_RF_runningHoldoutCount = (uint **) new_vvector(1, SL_RF_xSize, NRUTIL_UPTR);
    for (j = 1; j <= SL_RF_xSize; j++) {
      localBlockID[j]     = 0;
      iter[j]             = 0;
      SL_RF_blockSerialTreeIndex[j] = NULL;
      SL_RF_runningHoldoutCount[j]  = NULL;
      SL_RF_holdoutMap[j] = uivector(1, SL_RF_ntree);
    }
    for (j = 1; j <= SL_RF_xSize; j++) {
      for (i = 1; i <= SL_RF_ntree; i++) {
        if (SL_RF_vtryArray[i][j] > 0) {
          if (iter[j] == 0) {
            localBlockID[j] ++;
          }
          iter[j] ++;
          SL_RF_holdoutMap[j][i] = localBlockID[j];            
          if (iter[j] < SL_RF_vtryBlockSize) {
          }
          else {
            iter[j] = 0;
          }
        }
        else {
          SL_RF_holdoutMap[j][i] = 0;
        }
      }
    }
    for (j = 1; j <= SL_RF_xSize; j++) {
      if ((iter[j] > 0) && (iter[j] < SL_RF_vtryBlockSize)) {
        SL_RF_holdBLKptr[j] = localBlockID[j] - 1;
      }
      else {
        SL_RF_holdBLKptr[j] = localBlockID[j];
      }
    }
    free_uivector(localBlockID, 1, SL_RF_xSize);
    free_uivector(iter, 1, SL_RF_xSize);
    for (j = 1; j <= SL_RF_xSize; j++) {
      if (SL_RF_holdBLKptr[j] > 0) {
        SL_RF_blockSerialTreeIndex[j] = new_vvector(1, SL_RF_holdBLKptr[j], NRUTIL_UPTR);
        SL_RF_runningHoldoutCount[j]  = uivector(1, SL_RF_holdBLKptr[j]);
        for (k = 1; k <= SL_RF_holdBLKptr[j]; k++) {
          SL_RF_blockSerialTreeIndex[j][k] = uivector(1, SL_RF_vtryBlockSize);
          for (m = 1; m <= SL_RF_vtryBlockSize; m++) {
            SL_RF_blockSerialTreeIndex[j][k][m] = 0; 
          }
          SL_RF_runningHoldoutCount[j][k] = 0;
        }
      }
    }
    for (j = 1; j <= SL_RF_xSize; j++) {
      for (i = 1; i <= SL_RF_ntree; i++) {
        if ((SL_RF_vtryArray[i][j] > 0) && (SL_RF_holdoutMap[j][i] <= SL_RF_holdBLKptr[j])) {
          SL_RF_runningHoldoutCount[j][SL_RF_holdoutMap[j][i]] ++;
          SL_RF_blockSerialTreeIndex[j][SL_RF_holdoutMap[j][i]][SL_RF_runningHoldoutCount[j][SL_RF_holdoutMap[j][i]]] = i;
        }
      }
    }
    for (j = 1; j <= SL_RF_xSize; j++) {
      if (SL_RF_holdBLKptr[j] > 0) {
        for (k = 1; k <= SL_RF_holdBLKptr[j]; k++) {
          SL_RF_runningHoldoutCount[j][k] = 0;
        }
      }
    }
    xVimpSize = SL_RF_xSize;
    SL_RF_holdEnsembleDen  = (double ***) new_vvector(1, xVimpSize, NRUTIL_DPTR2);
    for (j = 1; j <= xVimpSize; j++) {
      if (SL_RF_holdBLKptr[j] > 0) {
        SL_RF_holdEnsembleDen[j] = (double **) new_vvector(1, SL_RF_holdBLKptr[j], NRUTIL_DPTR);
        for (k = 1; k <= SL_RF_holdBLKptr[j]; k++) {
          SL_RF_holdEnsembleDen[j][k] = NULL;
        }
      }
      else {
        SL_RF_holdEnsembleDen[j] = NULL;
      }
    }
    if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
      {
        localSize = 0;
        for (j = 1; j <= SL_RF_xSize; j++) {
          if (SL_RF_holdBLKptr[j] > 0) {
            localSize += (ulong) SL_RF_holdBLKptr[j] * SL_RF_eventTypeSize;
          }
        }
        SL_RF_holdMRT_ = (double*) stackAndProtect(mode,
                                                &SL_RF_nativeIndex,
                                                NATIVE_TYPE_NUMERIC,
                                                SL_RF_HLDOUT_SRG,
                                                localSize,
                                                SL_RF_nativeNaN,
                                                SL_RF_sexpString[SL_RF_HLDOUT_SRG],
                                                &SL_RF_holdMRTptr,
                                                3,
                                                xVimpSize,
                                                -3,  
                                                SL_RF_eventTypeSize);
        SL_RF_holdMRTstd = (double ****) new_vvector(1, xVimpSize, NRUTIL_DPTR3);
        for (j = 1; j <= xVimpSize; j++) {
          if (SL_RF_holdBLKptr[j] > 0) {
            SL_RF_holdMRTstd[j] = (double ***) new_vvector(1, SL_RF_holdBLKptr[j], NRUTIL_DPTR2);
            for (k = 1; k <= SL_RF_holdBLKptr[j]; k++) {
              SL_RF_holdMRTstd[j][k] = NULL;
            }
          }
          else {
            SL_RF_holdMRTstd[j]  = NULL;
          }
        }
      }
    }  
    else {
      if (SL_RF_rTargetFactorCount > 0) {
        ulong localSize2 = 0;
        for (i = 1; i <= SL_RF_rTargetFactorCount; i++) {
          localSize2 += (ulong) 1 + SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[i]]];
        }
        localSize = 0;
        for (j = 1; j <= SL_RF_xSize; j++) {
          if (SL_RF_holdBLKptr[j] > 0) {
            localSize += (ulong) SL_RF_holdBLKptr[j] * localSize2;
          }
        }
        SL_RF_holdCLS_ = (double*) stackAndProtect(mode,
                                                &SL_RF_nativeIndex,
                                                NATIVE_TYPE_NUMERIC,
                                                SL_RF_HLDOUT_CLS,
                                                localSize,
                                                SL_RF_nativeNaN,
                                                SL_RF_sexpString[SL_RF_HLDOUT_CLS],
                                                &SL_RF_holdCLSptr,
                                                4,
                                                xVimpSize,
                                                -3,   
                                                SL_RF_rTargetFactorCount,
                                                -1);  
        SL_RF_holdCLSstd = (double *****) new_vvector(1, xVimpSize, NRUTIL_DPTR4);        
        for (j = 1; j <= xVimpSize; j++) {
          if (SL_RF_holdBLKptr[j] > 0) {
            SL_RF_holdCLSstd[j] = (double ****) new_vvector(1, SL_RF_holdBLKptr[j], NRUTIL_DPTR3);
            for (k = 1; k <= SL_RF_holdBLKptr[j]; k++) {
              SL_RF_holdCLSstd[j][k] = NULL;
            }
          }
          else {
            SL_RF_holdCLSstd[j]  = NULL;
          }
        }
      }
      if (SL_RF_rTargetNonFactorCount > 0) {
        localSize = 0;
        for (j = 1; j <= SL_RF_xSize; j++) {
          if (SL_RF_holdBLKptr[j] > 0) {
            localSize += (ulong) SL_RF_holdBLKptr[j] * SL_RF_rTargetNonFactorCount;
          }
        }
        SL_RF_holdRGR_ = (double*) stackAndProtect(mode,
                                                &SL_RF_nativeIndex,
                                                NATIVE_TYPE_NUMERIC,
                                                SL_RF_HLDOUT_RGR,
                                                localSize,
                                                SL_RF_nativeNaN,
                                                SL_RF_sexpString[SL_RF_HLDOUT_RGR],
                                                &SL_RF_holdRGRptr,
                                                3,
                                                xVimpSize,
                                                -3,  
                                                SL_RF_rTargetNonFactorCount);
        SL_RF_holdRGRstd = (double ****) new_vvector(1, xVimpSize, NRUTIL_DPTR3);
        for (j = 1; j <= xVimpSize; j++) {
          if (SL_RF_holdBLKptr[j] > 0) {
            SL_RF_holdRGRstd[j] = (double ***) new_vvector(1, SL_RF_holdBLKptr[j], NRUTIL_DPTR2);
            for (k = 1; k <= SL_RF_holdBLKptr[j]; k++) {
              SL_RF_holdRGRstd[j][k] = NULL;
            }
          }
          else {
            SL_RF_holdRGRstd[j]  = NULL;
          }
        }
      }
    }
  }
  if (SL_RF_opt & OPT_PROX) {
    if ( !((mode == SL_RF_PRED) && (SL_RF_opt & OPT_PROX_IBG) && (SL_RF_opt & OPT_PROX_OOB)) ) {
      localSize = ((ulong) (obsSize + 1) * obsSize) >> 1;
      *pRF_proximity = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_PROX_ID, localSize, 0, SL_RF_sexpString[SL_RF_PROX_ID], NULL, 1, localSize);
      SL_RF_proximityDen = dvector(1, localSize);
      (*pRF_proximity) --;
      SL_RF_proximityPtr = (double **) new_vvector(1, obsSize, NRUTIL_DPTR);
      SL_RF_proximityDenPtr = (double **) new_vvector(1, obsSize, NRUTIL_DPTR);
      SL_RF_proximityPtr[1] = *pRF_proximity;
      SL_RF_proximityDenPtr[1] = SL_RF_proximityDen;    
      SL_RF_proximityPtr[1][1] = SL_RF_proximityDenPtr[1][1] = 0.0;
      for (i = 2; i <= obsSize; i++) {
        SL_RF_proximityPtr[i] = SL_RF_proximityPtr[i-1] + i - 1;
        SL_RF_proximityDenPtr[i] = SL_RF_proximityDenPtr[i-1] + i - 1;
        for (j = 1; j <= i; j++) {
          SL_RF_proximityPtr[i][j] = 0.0;
          SL_RF_proximityDenPtr[i][j] = 0.0;
        }
      }
    }
    else {
      localSize = (ulong) (SL_RF_fobservationSize * SL_RF_observationSize);
      *pRF_proximity = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_PROX_ID, localSize, 0, SL_RF_sexpString[SL_RF_PROX_ID], NULL, 1, localSize);
      SL_RF_proximityDen = dvector(1, localSize);
      (*pRF_proximity) --;
      SL_RF_proximityPtr = (double **) new_vvector(1, SL_RF_fobservationSize, NRUTIL_DPTR);
      SL_RF_proximityDenPtr = (double **) new_vvector(1, SL_RF_fobservationSize, NRUTIL_DPTR);
      for (i = 1; i <= SL_RF_fobservationSize; i++) {
        SL_RF_proximityPtr[i] = *pRF_proximity + (SL_RF_observationSize * (i - 1));
        SL_RF_proximityDenPtr[i] = SL_RF_proximityDen + (SL_RF_observationSize * (i - 1));
        for (j = 1; j <= SL_RF_observationSize; j++) {
          SL_RF_proximityPtr[i][j] = 0.0;
          SL_RF_proximityDenPtr[i][j] = 0.0;
        }
      }
    }
  }
  if (SL_RF_optHigh & OPT_DIST) {
    if ( !((mode == SL_RF_PRED) && (SL_RF_optHigh & OPT_DIST_IBG) && (SL_RF_optHigh & OPT_DIST_OOB)) ) {
      localSize = ((ulong) (obsSize + 1) * obsSize) >> 1;
      *pRF_distance = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_DIST_ID, localSize, 0, SL_RF_sexpString[SL_RF_DIST_ID], NULL, 1, localSize);
      SL_RF_distanceDen = dvector(1, localSize);
      (*pRF_distance) --;
      SL_RF_distancePtr = (double **) new_vvector(1, obsSize, NRUTIL_DPTR);
      SL_RF_distanceDenPtr = (double **) new_vvector(1, obsSize, NRUTIL_DPTR);
      SL_RF_distancePtr[1] = *pRF_distance;
      SL_RF_distanceDenPtr[1] = SL_RF_distanceDen;
      SL_RF_distancePtr[1][1] = SL_RF_distanceDenPtr[1][1] = 0.0;
      for (i = 2; i <= obsSize; i++) {
        SL_RF_distancePtr[i] = SL_RF_distancePtr[i-1] + i - 1;
        SL_RF_distanceDenPtr[i] = SL_RF_distanceDenPtr[i-1] + i - 1;
        for (j = 1; j <= i; j++) {
          SL_RF_distancePtr[i][j] = 0.0;
          SL_RF_distanceDenPtr[i][j] = 0.0;
        }
      }
    }
    else {
      localSize = (ulong) (SL_RF_fobservationSize * SL_RF_observationSize);
      *pRF_distance = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_DIST_ID, localSize, 0, SL_RF_sexpString[SL_RF_DIST_ID], NULL, 1, localSize);
      SL_RF_distanceDen = dvector(1, localSize);
      (*pRF_distance) --;
      SL_RF_distancePtr = (double **) new_vvector(1, SL_RF_fobservationSize, NRUTIL_DPTR);
      SL_RF_distanceDenPtr = (double **) new_vvector(1, SL_RF_fobservationSize, NRUTIL_DPTR);
      for (i = 1; i <= SL_RF_fobservationSize; i++) {
        SL_RF_distancePtr[i] = *pRF_distance + (SL_RF_observationSize * (i - 1));
        SL_RF_distanceDenPtr[i] = SL_RF_distanceDen + (SL_RF_observationSize * (i - 1));
        for (j = 1; j <= SL_RF_observationSize; j++) {
          SL_RF_distancePtr[i][j] = 0.0;
          SL_RF_distanceDenPtr[i][j] = 0.0;
        }
      }
    }
  }
  if (SL_RF_optHigh & OPT_WGHT) {
    localSize = (ulong) obsSize * SL_RF_observationSize;
    *pRF_weight = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_WGHT_ID, localSize, 0, SL_RF_sexpString[SL_RF_WGHT_ID], &SL_RF_weightPtr, 2, obsSize, SL_RF_observationSize);
    SL_RF_weightDenom = uivector(1, obsSize);
    for (k = 1; k <= obsSize; k++) {
      SL_RF_weightDenom[k] = 0.0;
    }
  }
  if (SL_RF_opt & OPT_LEAF) {
    localSize = (ulong) SL_RF_ntree;
    *pRF_tLeafCount = (uint*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_LEAF_CT, localSize, 0, SL_RF_sexpString[SL_RF_LEAF_CT], NULL, 1, localSize);
    (*pRF_tLeafCount) --;
    if (mode == SL_RF_GROW) {
      SL_RF_tLeafCount = *pRF_tLeafCount;
    }
    else {
      for (i = 1; i <= SL_RF_ntree; i++) {
        (*pRF_tLeafCount)[i] = SL_RF_tLeafCount[i];
      }
    }
  }
  if (SL_RF_opt & OPT_SEED) {
    localSize = (ulong) SL_RF_ntree;
    SL_RF_seed_ = (int*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_SEED_ID, localSize, 0, SL_RF_sexpString[SL_RF_SEED_ID], NULL, 1, localSize);
    SL_RF_seed_ --;
    for (i = 1; i <= SL_RF_ntree; i++) {
      SL_RF_seed_[i] = -1;
    }
    uint bnpSize = getVimpRecoverySeedDimension(mode, SL_RF_opt);
    if (bnpSize > 0) {
      localSize = (ulong) bnpSize;
      SL_RF_seedVimp_ = (int*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_SEED_VM, localSize, 0, SL_RF_sexpString[SL_RF_SEED_VM], NULL, 1, localSize);
      SL_RF_seedVimp_ --;
      for (i = 1; i <= bnpSize; i++) {
        SL_RF_seedVimp_[i] = -1;
      }
    }
    SL_RF_optLoGrow_ = (uint*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_OPT_LO_GROW, 1, 0, SL_RF_sexpString[SL_RF_OPT_LO_GROW], NULL, 1, 1);
    SL_RF_optLoGrow_ --;
    SL_RF_optLoGrow_[1] = SL_RF_optLoGrow = SL_RF_opt;
  }
  if (SL_RF_opt & OPT_MISS_OUT) {
    localSize = (ulong) (1 + rspSize + SL_RF_xSize) * mRecordSize;
    *p_imputation = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_MISS_ID, localSize, SL_RF_nativeNaN, SL_RF_sexpString[SL_RF_MISS_ID], &SL_RF_sImputeDataPtr, 2, 1 + rspSize + SL_RF_xSize, mRecordSize);
    if (rspSize > 0) {
      *pRF_sImputeResponsePtr = (double **) new_vvector(1, rspSize, NRUTIL_DPTR);
      for (i = 1; i <= rspSize; i++) {
        (*pRF_sImputeResponsePtr)[i]  = (*p_imputation)  + (i * mRecordSize) - 1;
      }
    }
    *pRF_sImputePredictorPtr = (double **) new_vvector(1, SL_RF_xSize, NRUTIL_DPTR);
    for (i = 1; i <= SL_RF_xSize; i++) {
      (*pRF_sImputePredictorPtr)[i]  = (*p_imputation)  + ((rspSize + i) * mRecordSize) - 1;
    }
    for (i = 1; i <= mRecordSize; i++) {
      (*p_imputation)[i-1] = (double) mRecordIndex[i];
      if (rspSize > 0) {
        for (j = 1; j <= rspSize; j++) {
          (*pRF_sImputeResponsePtr)[j][i] = responsePtr[j][mRecordIndex[i]];
        }
      }
      for (j = 1; j <= SL_RF_xSize; j++) {
        (*pRF_sImputePredictorPtr)[j][i] = predictorPtr[j][mRecordIndex[i]];
      }
    }
  }
  if (SL_RF_opt & (OPT_VARUSED_F | OPT_VARUSED_T)) {
    if (SL_RF_opt & OPT_VARUSED_T) {
      localSize = (ulong) SL_RF_ntree * SL_RF_xSize;
      *pRF_varUsed = (uint*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_VUSE_ID, localSize, 0, SL_RF_sexpString[SL_RF_VUSE_ID], pRF_varUsedPtr, 2, SL_RF_ntree, SL_RF_xSize);
    }
    else {
      localSize = (ulong) 1 * SL_RF_xSize;
      *pRF_varUsed = (uint*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_VUSE_ID, localSize, 0, SL_RF_sexpString[SL_RF_VUSE_ID], NULL, 1, localSize);
      *pRF_varUsedPtr = uimatrix(1, SL_RF_ntree, 1, SL_RF_xSize);
      for (i = 1; i <= SL_RF_ntree; i++) {
        for (j = 1; j <= SL_RF_xSize; j++) {
          (*pRF_varUsedPtr)[i][j] = 0;
        }
      }
    }
    (*pRF_varUsed) --;
  }
  if (SL_RF_opt & (OPT_SPLDPTH_1 | OPT_SPLDPTH_2)) {
    if (SL_RF_opt & OPT_SPLDPTH_1) {
      dpthDimOne = 1;
    }
    else {
      dpthDimOne = SL_RF_ntree;
    }
    localSize = (ulong) dpthDimOne * SL_RF_xSize * SL_RF_observationSize;
    *p_splitDepth = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_DPTH_ID, localSize, 0, SL_RF_sexpString[SL_RF_DPTH_ID], &SL_RF_splitDepthPtr, 3, dpthDimOne, SL_RF_xSize, SL_RF_observationSize);
  }
  if (SL_RF_opt & OPT_CASE_DPTH) {
    localSize = (ulong) SL_RF_ntree * obsSize;
    SL_RF_CASE_DPTH_ = (uint*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_CASE_DEPTH, localSize, 0, SL_RF_sexpString[SL_RF_CASE_DEPTH], &SL_RF_CASE_DPTH_ptr, 2, SL_RF_ntree, obsSize);
  }
  if (SL_RF_optHigh & OPT_MEMB_PRUN) {
    localSize = (ulong) SL_RF_ntree * obsSize;
    SL_RF_PRUN_ID_ = (uint*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_PRUN_ID, localSize, 0, SL_RF_sexpString[SL_RF_PRUN_ID], &SL_RF_PRUN_ID_ptr, 2, SL_RF_ntree, obsSize);
  }
  if (SL_RF_optHigh & OPT_MEMB_USER) {
    localSize = (ulong) SL_RF_ntree * obsSize;
    SL_RF_MEMB_ID_ = (uint*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_MEMB_ID, localSize, 0, SL_RF_sexpString[SL_RF_MEMB_ID], &SL_RF_MEMB_ID_ptr, 2, SL_RF_ntree, obsSize);
    localSize = (ulong) SL_RF_ntree * SL_RF_observationSize;
    SL_RF_BOOT_CT_ = (uint*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_BOOT_CT, localSize, 0, SL_RF_sexpString[SL_RF_BOOT_CT], &SL_RF_BOOT_CT_ptr, 2, SL_RF_ntree, SL_RF_observationSize);
  }
  if (SL_RF_optHigh & OPT_PART_PLOT) {
    SL_RF_partSURVptr = NULL;
    SL_RF_partCLASptr = NULL;
    SL_RF_partREGRptr = NULL;
    if ((SL_RF_partialXvar < 1) || (SL_RF_partialXvar > SL_RF_xSize)) {
      SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
      SL_RF_nativeError("\nRF-SRC:  Partial x-variable is out of range:  %10d ", SL_RF_partialXvar);
      SL_RF_nativeExit();
    }
    if (SL_RF_partialLength2 > 0) {
      for (i = 1; i <= SL_RF_partialLength2; i++) {
        if ((SL_RF_partialXvar2[i] < 1) || (SL_RF_partialXvar2[i] > SL_RF_xSize)) {
          SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
          SL_RF_nativeError("\nRF-SRC:  Second order partial x-variable is out of range:  idx = %10d, val =  %10d ", i, SL_RF_partialXvar2[i]);
          SL_RF_nativeExit();
        }
        if (SL_RF_partialXvar2[i] == SL_RF_partialXvar) {
          SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
          SL_RF_nativeError("\nRF-SRC:  First and Second order partial x-variables are identical:  idx = %10d, val =  %10d ", i, SL_RF_partialXvar2[i]);
          SL_RF_nativeExit();
        }
        for (j = i + 1; j <= SL_RF_partialLength2; j++) {
          if (SL_RF_partialXvar2[i] == SL_RF_partialXvar2[j]) {
            SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
            SL_RF_nativeError("\nRF-SRC:  Second order partial x-variables are not unique:  idx = %10d, idx =  %10d, val = %10d", i, j, SL_RF_partialXvar2[i]);
            SL_RF_nativeExit();
          }
        }
      }
    }
    SL_RF_partMembership = (Terminal ***) new_vvector(1, SL_RF_ntree, NRUTIL_TPTR2);
    for (i = 1; i <= SL_RF_ntree; i++) {
      SL_RF_partMembership[i] = NULL;
    }
    if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
      {
        SL_RF_partialTimeLength = SL_RF_sortedTimeInterestSize;
        SL_RF_partialTime = SL_RF_timeInterest;
        if ((SL_RF_partialTimeLength < 1) || (SL_RF_partialTimeLength > SL_RF_timeInterestSize)) {
          SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
          SL_RF_nativeError("\nRF-SRC:  Partial time length is out of range:  %10d ", SL_RF_partialTimeLength);
          SL_RF_nativeExit();
        }
        if (SL_RF_eventTypeSize > 1) {
          if ((SL_RF_partialType != SL_RF_PART_YRLS) &&
              (SL_RF_partialType != SL_RF_PART_CIFN) &&
              (SL_RF_partialType != SL_RF_PART_CHFN)) {
            SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
            SL_RF_nativeError("\nRF-SRC:  Partial type is out of range:  %10d ", SL_RF_partialType);
            SL_RF_nativeExit();
          }
        }
        else {
          if ((SL_RF_partialType != SL_RF_PART_MORT) &&
              (SL_RF_partialType != SL_RF_PART_NLSN) &&
              (SL_RF_partialType != SL_RF_PART_SURV)) {
            SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
            SL_RF_nativeError("\nRF-SRC:  Partial type is out of range:  %10d ", SL_RF_partialType);
            SL_RF_nativeExit();
          }
        }
        dimThree = SL_RF_partialTimeLength;
        if (!(SL_RF_opt & OPT_COMP_RISK)) {
          if (SL_RF_partialType == SL_RF_PART_MORT) {
            dimThree = 1;
          }
        }
        else {
          if (SL_RF_partialType == SL_RF_PART_YRLS) {
            dimThree = 1;
          }
        }
        localSize = (ulong) SL_RF_partialLength * SL_RF_eventTypeSize * dimThree * SL_RF_observationSize;
        SL_RF_partial_SURV_ = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_PART_SR, localSize, 0, SL_RF_sexpString[SL_RF_PART_SR], &SL_RF_partSURVptr, 4, SL_RF_partialLength, SL_RF_eventTypeSize, dimThree, SL_RF_observationSize);
      }
    }  
    else {
      if (SL_RF_rTargetFactorCount > 0) {
        localSize = 0;
        for (k = 1; k <= SL_RF_rTargetFactorCount; k++) {
          localSize += (ulong) 1 + SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[k]]];
        }
        localSize = (ulong) SL_RF_partialLength * localSize * SL_RF_observationSize;
        SL_RF_partial_CLAS_ = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_PART_CL, localSize, 0, SL_RF_sexpString[SL_RF_PART_CL], &SL_RF_partCLASptr, 4, SL_RF_partialLength, SL_RF_rTargetFactorCount, -1, SL_RF_observationSize);
      }
      if (SL_RF_rTargetNonFactorCount > 0) {
        localSize = (ulong) SL_RF_partialLength * SL_RF_rTargetNonFactorCount * SL_RF_observationSize;
        SL_RF_partial_REGR_ = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_PART_RG, localSize, 0, SL_RF_sexpString[SL_RF_PART_RG], &SL_RF_partREGRptr, 3, SL_RF_partialLength, SL_RF_rTargetNonFactorCount, SL_RF_observationSize);
      }
    }
  }  
  SL_RF_cpuTime_ = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_CPU_TIME, 1, 0, SL_RF_sexpString[SL_RF_CPU_TIME], NULL, 1, 1);
  SL_RF_cpuTime_ --;
}
void unstackDefinedOutputObjects(char mode) {
  uint obsSize;
  uint xVimpSize;
  ulong localSize;
  char oobFlag, fullFlag;
  uint rspSize;
  double        **ensembleDen;
  uint         ***quantileStreamSize;
  LookUpInfo  ****quantileSearchTree;
  QuantileObj ****quantileHead;
  QuantileObj ****quantileTail;
  uint         ***quantileLinkLength;
  double ****ensembleSRGnum;
  double  ***ensembleMRTnum;
  double  ***ensembleSRVnum;
  double ****ensembleCIFnum;
  double ****ensembleCLSnum;
  double  ***ensembleRGRnum;
  uint i, j, k;
  obsSize        = 0;  
  xVimpSize      = 0;  
  rspSize        = 0;  
  free_uivector(SL_RF_identityMembershipIndex, 1, SL_RF_identityMembershipIndexSize);
  switch (mode) {
  case SL_RF_PRED:
    obsSize = SL_RF_fobservationSize;
    rspSize = SL_RF_frSize;
    break;
  default:
    obsSize = SL_RF_observationSize;
    rspSize = SL_RF_ySize;
    break;
  }
  if (SL_RF_xMarginalSize > 0) {
    free_new_vvector(SL_RF_utTermMembership,      1, SL_RF_ntree, NRUTIL_UPTR2);
    free_new_vvector(SL_RF_utTermMembershipCount, 1, SL_RF_ntree, NRUTIL_UPTR);
    free_new_vvector(SL_RF_utTermMembershipAlloc, 1, SL_RF_ntree, NRUTIL_UPTR);
    free_uivector(SL_RF_xMarginalFlag, 1, SL_RF_xSize);
  }
  oobFlag = fullFlag = FALSE;
  if ((SL_RF_opt & OPT_FENS) || (SL_RF_opt & OPT_OENS)) {
    if (SL_RF_opt & OPT_FENS) {
      fullFlag = TRUE;
    }
    if (SL_RF_opt & OPT_OENS) {
      oobFlag = TRUE;
    }
    while ((oobFlag == TRUE) || (fullFlag == TRUE)) {
      if (oobFlag == TRUE) {
        ensembleDen    = &SL_RF_oobEnsembleDen;
        ensembleSRGnum = &SL_RF_oobEnsembleSRGnum;
        ensembleMRTnum = &SL_RF_oobEnsembleMRTnum;
        ensembleSRVnum = &SL_RF_oobEnsembleSRVnum;
        ensembleCIFnum = &SL_RF_oobEnsembleCIFnum;
        ensembleCLSnum = &SL_RF_oobEnsembleCLSnum;
        ensembleRGRnum = &SL_RF_oobEnsembleRGRnum;
        quantileStreamSize = &SL_RF_oobQuantileStreamSize;
        quantileSearchTree = &SL_RF_oobQuantileSearchTree;
        quantileHead       = &SL_RF_oobQuantileHead;
        quantileTail       = &SL_RF_oobQuantileTail;
        quantileLinkLength = &SL_RF_oobQuantileLinkLength;
      }
      else {
        ensembleDen    = &SL_RF_fullEnsembleDen;
        ensembleSRGnum = &SL_RF_fullEnsembleSRGnum;
        ensembleMRTnum = &SL_RF_fullEnsembleMRTnum;
        ensembleSRVnum = &SL_RF_fullEnsembleSRVnum;
        ensembleCIFnum = &SL_RF_fullEnsembleCIFnum;
        ensembleCLSnum = &SL_RF_fullEnsembleCLSnum;
        ensembleRGRnum = &SL_RF_fullEnsembleRGRnum;
        quantileStreamSize = &SL_RF_fullQuantileStreamSize;
        quantileSearchTree = &SL_RF_fullQuantileSearchTree;
        quantileHead       = &SL_RF_fullQuantileHead;
        quantileTail       = &SL_RF_fullQuantileTail;
        quantileLinkLength = &SL_RF_fullQuantileLinkLength;
      }
      free_dvector(*ensembleDen, 1, obsSize);
      if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
        for (j = 1; j <= SL_RF_eventTypeSize; j++) {
          for (k = 1; k <= SL_RF_sortedTimeInterestSize; k++) {
            free_dvector((*ensembleSRGnum)[j][k], 1, obsSize);
          }
          free_new_vvector((*ensembleSRGnum)[j], 1, SL_RF_sortedTimeInterestSize, NRUTIL_DPTR);
        }
        free_new_vvector(*ensembleSRGnum, 1, SL_RF_eventTypeSize, NRUTIL_DPTR2);
        for (j = 1; j <= SL_RF_eventTypeSize; j++) {
          free_dvector((*ensembleMRTnum)[j], 1, obsSize);
        }
        free_new_vvector(*ensembleMRTnum, 1, SL_RF_eventTypeSize, NRUTIL_DPTR);
        if (!(SL_RF_opt & OPT_COMP_RISK)) {
          for (k = 1; k <= SL_RF_sortedTimeInterestSize; k++) {
            free_dvector((*ensembleSRVnum)[k], 1, obsSize);
          }
          free_new_vvector(*ensembleSRVnum, 1, SL_RF_sortedTimeInterestSize, NRUTIL_DPTR);
        }
        else {
          for (j = 1; j <= SL_RF_eventTypeSize; j++) {
            for (k = 1; k <= SL_RF_sortedTimeInterestSize; k++) {
              free_dvector((*ensembleCIFnum)[j][k], 1, obsSize);
            }
            free_new_vvector((*ensembleCIFnum)[j], 1, SL_RF_sortedTimeInterestSize, NRUTIL_DPTR);
          }
          free_new_vvector(*ensembleCIFnum, 1, SL_RF_eventTypeSize, NRUTIL_DPTR2);            
        }  
      }  
      else {
        if (SL_RF_rTargetFactorCount > 0) {
          for (j = 1; j <= SL_RF_rTargetFactorCount; j++) {
            for (k = 1; k <= SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[j]]]; k++) {
              free_dvector((*ensembleCLSnum)[j][k], 1, obsSize);
            }
            free_new_vvector((*ensembleCLSnum)[j], 1, SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[j]]], NRUTIL_DPTR);
          }
          free_new_vvector(*ensembleCLSnum, 1, SL_RF_rTargetFactorCount, NRUTIL_DPTR2);
        }
        if (SL_RF_rTargetNonFactorCount > 0) {
          for (j = 1; j <= SL_RF_rTargetNonFactorCount; j++) {
            free_dvector((*ensembleRGRnum)[j], 1, obsSize);
          }
          free_new_vvector((*ensembleRGRnum), 1, SL_RF_rTargetNonFactorCount, NRUTIL_DPTR);        
          if (SL_RF_opt & OPT_QUANTLE) {
            for (j = 1; j <= SL_RF_rTargetNonFactorCount; j++) {
              free_uivector((*quantileStreamSize)[j], 1, obsSize);
            }
            free_new_vvector(*quantileStreamSize, 1, SL_RF_rTargetNonFactorCount, NRUTIL_UPTR);            
            for (j = 1; j <= SL_RF_rTargetNonFactorCount; j++) {
              for (i = 1; i <= obsSize; i++) {
                freeLookUpTree((*quantileSearchTree)[j][i]);
              }
              free_new_vvector((*quantileSearchTree)[j], 1, obsSize, NRUTIL_SPTR);
            }
            free_new_vvector(*quantileSearchTree, 1, SL_RF_rTargetNonFactorCount, NRUTIL_SPTR2);
            for (j = 1; j <= SL_RF_rTargetNonFactorCount; j++) {
              for (i = 1; i <= obsSize; i++) {
                freeQuantileObjList((*quantileHead)[j][i]);
              }
              free_new_vvector((*quantileHead)[j], 1, obsSize, NRUTIL_QPTR);
              free_new_vvector((*quantileTail)[j], 1, obsSize, NRUTIL_QPTR);
              free_uivector((*quantileLinkLength)[j], 1, obsSize);
            }
            free_new_vvector(*quantileHead, 1, SL_RF_rTargetNonFactorCount, NRUTIL_QPTR2);
            free_new_vvector(*quantileTail, 1, SL_RF_rTargetNonFactorCount, NRUTIL_QPTR2);
            free_new_vvector(*quantileLinkLength, 1, SL_RF_rTargetNonFactorCount, NRUTIL_UPTR2);
          }
        }
      }
      if (oobFlag == TRUE) {
        oobFlag = FALSE;
      }
      else {
        fullFlag = FALSE;
      }
    }  
  }
  if (SL_RF_opt & OPT_PERF) {
    if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
      {
      }
    }  
    else {
      if (SL_RF_rTargetFactorCount > 0) {
      }
      if (SL_RF_rTargetNonFactorCount > 0) {
      }
    }
  }  
  if (SL_RF_opt & OPT_VIMP) {
    if (SL_RF_opt & OPT_VIMP_JOIN) {
      xVimpSize = 1;
    }
    else {
      xVimpSize = SL_RF_intrPredictorSize;
    }
    for (k = 1; k <= xVimpSize; k++) {
      free_new_vvector(SL_RF_vimpMembership[k], 1,  SL_RF_ntree, NRUTIL_NPTR2);
    }
    free_new_vvector(SL_RF_vimpMembership, 1, xVimpSize, NRUTIL_NPTR3);
    for (j = 1; j <= xVimpSize; j++) {
      free_dvector(SL_RF_vimpEnsembleDen[j], 1, obsSize);
    }
    free_new_vvector(SL_RF_vimpEnsembleDen, 1, xVimpSize, NRUTIL_DPTR);
    free_dvector(SL_RF_blkEnsembleDen, 1, obsSize);
    if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
      {
        for (j = 1; j <= xVimpSize; j++) {
          for (k = 1; k <= SL_RF_eventTypeSize; k++) {
            free_dvector(SL_RF_vimpMRTstd[j][k], 1, obsSize);
          }
          free_new_vvector(SL_RF_vimpMRTstd[j], 1, SL_RF_eventTypeSize, NRUTIL_DPTR);
        }
        free_new_vvector(SL_RF_vimpMRTstd, 1, xVimpSize, NRUTIL_DPTR2);
        for (j = 1; j <= SL_RF_eventTypeSize; j++) {
          free_dvector(SL_RF_blkEnsembleMRTnum[j], 1, obsSize);
        }
        free_new_vvector(SL_RF_blkEnsembleMRTnum, 1, SL_RF_eventTypeSize, NRUTIL_DPTR);
        for (i = 1; i <= SL_RF_perfBlockCount; i++) {
          for (j = 1; j <= xVimpSize; j++) {
            free_dvector(SL_RF_vimpMRTblk[i][j], 1, SL_RF_eventTypeSize);
          }
          free_new_vvector(SL_RF_vimpMRTblk[i], 1, xVimpSize, NRUTIL_DPTR);
        }
        free_new_vvector(SL_RF_vimpMRTblk, 1, SL_RF_perfBlockCount, NRUTIL_DPTR2);
      }
    }  
    else {
      if (SL_RF_rTargetFactorCount > 0) {
        for (i = 1; i <= xVimpSize; i++) {
          for (j = 1; j <= SL_RF_rTargetFactorCount; j++) {
            for (k = 1; k <= SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[j]]]; k++) {
              free_dvector(SL_RF_vimpCLSstd[i][j][k], 1, obsSize);
            }
            free_new_vvector(SL_RF_vimpCLSstd[i][j], 1, SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[j]]], NRUTIL_DPTR);
          }
          free_new_vvector(SL_RF_vimpCLSstd[i], 1, SL_RF_rTargetFactorCount, NRUTIL_DPTR2);
        }
        free_new_vvector(SL_RF_vimpCLSstd, 1, xVimpSize, NRUTIL_DPTR3);        
        for (j = 1; j <= SL_RF_rTargetFactorCount; j++) {
          for (k = 1; k <= SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[j]]]; k++) {
            free_dvector(SL_RF_blkEnsembleCLSnum[j][k], 1, obsSize);
          }
          free_new_vvector(SL_RF_blkEnsembleCLSnum[j], 1, SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[j]]], NRUTIL_DPTR);
        }
        free_new_vvector(SL_RF_blkEnsembleCLSnum, 1, SL_RF_rTargetFactorCount, NRUTIL_DPTR2);
        for (i = 1; i <= SL_RF_perfBlockCount; i++) {
          for (j = 1; j <= xVimpSize; j++) {            
            for (k = 1; k <= SL_RF_rTargetFactorCount; k++) {
              free_dvector(SL_RF_vimpCLSblk[i][j][k], 1, 1 + SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[k]]]);
            }
            free_new_vvector(SL_RF_vimpCLSblk[i][j], 1, SL_RF_rTargetFactorCount, NRUTIL_DPTR);
          }
          free_new_vvector(SL_RF_vimpCLSblk[i], 1, xVimpSize, NRUTIL_DPTR2);
        }
        free_new_vvector(SL_RF_vimpCLSblk, 1, SL_RF_perfBlockCount, NRUTIL_DPTR3);
      }
      if (SL_RF_rTargetNonFactorCount > 0) {
        for (j = 1; j <= xVimpSize; j++) {
          for (k = 1; k <= SL_RF_rTargetNonFactorCount; k++) {
            free_dvector(SL_RF_vimpRGRstd[j][k], 1, obsSize);
          }
          free_new_vvector(SL_RF_vimpRGRstd[j], 1, SL_RF_rTargetNonFactorCount, NRUTIL_DPTR);
        }
        free_new_vvector(SL_RF_vimpRGRstd, 1, xVimpSize, NRUTIL_DPTR2);
        for (j = 1; j <= SL_RF_rTargetNonFactorCount; j++) {            
          free_dvector(SL_RF_blkEnsembleRGRnum[j], 1, obsSize);   
        }
        free_new_vvector(SL_RF_blkEnsembleRGRnum, 1, SL_RF_rTargetNonFactorCount, NRUTIL_DPTR);
        for (i = 1; i <= SL_RF_perfBlockCount; i++) {
          for (j = 1; j <= xVimpSize; j++) {            
            free_dvector(SL_RF_vimpRGRblk[i][j], 1, SL_RF_rTargetNonFactorCount);             
          }
          free_new_vvector(SL_RF_vimpRGRblk[i], 1, xVimpSize, NRUTIL_DPTR);
        }
        free_new_vvector(SL_RF_vimpRGRblk, 1, SL_RF_perfBlockCount, NRUTIL_DPTR2);
      }
    }
  }  
  if ((SL_RF_vtry > 0) && (SL_RF_vtryMode != SL_RF_VTRY_NULL)) {
    xVimpSize = SL_RF_xSize;
    for (j = 1; j <= xVimpSize; j++) {
      if (SL_RF_holdBLKptr[j] > 0) {
        free_new_vvector(SL_RF_holdEnsembleDen[j], 1, SL_RF_holdBLKptr[j], NRUTIL_DPTR);
      }
    }
    free_new_vvector(SL_RF_holdEnsembleDen, 1, xVimpSize, NRUTIL_DPTR2);
    if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
      {
        for (j = 1; j <= xVimpSize; j++) {
          if (SL_RF_holdBLKptr[j] > 0) {
            free_new_vvector(SL_RF_holdMRTstd[j], 1, SL_RF_holdBLKptr[j], NRUTIL_DPTR2);
          }
        }
        free_new_vvector(SL_RF_holdMRTstd, 1, xVimpSize, NRUTIL_DPTR3);
      }
    }  
    else {
      if (SL_RF_rTargetFactorCount > 0) {
        for (j = 1; j <= xVimpSize; j++) {
          if (SL_RF_holdBLKptr[j] > 0) {
            free_new_vvector(SL_RF_holdCLSstd[j], 1, SL_RF_holdBLKptr[j], NRUTIL_DPTR3);
          }
        }
        free_new_vvector(SL_RF_holdCLSstd, 1, xVimpSize, NRUTIL_DPTR4);        
      }
      if (SL_RF_rTargetNonFactorCount > 0) {
        for (j = 1; j <= xVimpSize; j++) {
          if (SL_RF_holdBLKptr[j] > 0) {
            free_new_vvector(SL_RF_holdRGRstd[j], 1, SL_RF_holdBLKptr[j], NRUTIL_DPTR2);
          }
        }
        free_new_vvector(SL_RF_holdRGRstd, 1, xVimpSize, NRUTIL_DPTR3);
      }
    }
      for (j = 1; j <= SL_RF_xSize; j++) {
        if (SL_RF_holdBLKptr[j] > 0) {
          free_uivector(SL_RF_runningHoldoutCount[j], 1, SL_RF_holdBLKptr[j]);
        }
      }
      free_new_vvector(SL_RF_runningHoldoutCount, 1, SL_RF_xSize, NRUTIL_UPTR);
      for (j = 1; j <= SL_RF_xSize; j++) {
        if (SL_RF_holdBLKptr[j] > 0) {
          for (k = 1; k <= SL_RF_holdBLKptr[j]; k++) {
            free_uivector(SL_RF_blockSerialTreeIndex[j][k], 1, SL_RF_vtryBlockSize);
          }
          free_new_vvector(SL_RF_blockSerialTreeIndex[j], 1, SL_RF_holdBLKptr[j], NRUTIL_UPTR);
        }
      }
      free_new_vvector(SL_RF_blockSerialTreeIndex, 1, SL_RF_xSize, NRUTIL_UPTR2);      
      for (j = 1; j <= SL_RF_xSize; j++) {
        free_uivector(SL_RF_holdoutMap[j], 1, SL_RF_ntree);
      }
      free_new_vvector(SL_RF_holdoutMap, 1, SL_RF_xSize, NRUTIL_UPTR);
  }
  if (SL_RF_opt & OPT_PROX) {
    if ( !((mode == SL_RF_PRED) && (SL_RF_opt & OPT_PROX_IBG) && (SL_RF_opt & OPT_PROX_OOB)) ) {
      localSize = ((obsSize + 1)  * obsSize) >> 1;
      free_dvector(SL_RF_proximityDen, 1, localSize);
      free_new_vvector(SL_RF_proximityPtr, 1, obsSize, NRUTIL_DPTR);
      free_new_vvector(SL_RF_proximityDenPtr, 1, obsSize, NRUTIL_DPTR);
    }
    else {
      localSize = (ulong) (SL_RF_fobservationSize * SL_RF_observationSize);
      free_dvector(SL_RF_proximityDen, 1, localSize);
      free_new_vvector(SL_RF_proximityPtr, 1, SL_RF_fobservationSize, NRUTIL_DPTR);
      free_new_vvector(SL_RF_proximityDenPtr, 1, SL_RF_fobservationSize, NRUTIL_DPTR);
    }
  }
  if (SL_RF_optHigh & OPT_DIST) {
    if ( !((mode == SL_RF_PRED) && (SL_RF_optHigh & OPT_DIST_IBG) && (SL_RF_optHigh & OPT_DIST_OOB)) ) {
      localSize = ((obsSize + 1)  * obsSize) >> 1;
      free_dvector(SL_RF_distanceDen, 1, localSize);
      free_new_vvector(SL_RF_distancePtr, 1, obsSize, NRUTIL_DPTR);
      free_new_vvector(SL_RF_distanceDenPtr, 1, obsSize, NRUTIL_DPTR);
    }
    else {
      localSize = (ulong) (SL_RF_fobservationSize * SL_RF_observationSize);
      free_dvector(SL_RF_distanceDen, 1, localSize);
      free_new_vvector(SL_RF_distancePtr, 1, SL_RF_fobservationSize, NRUTIL_DPTR);
      free_new_vvector(SL_RF_distanceDenPtr, 1, SL_RF_fobservationSize, NRUTIL_DPTR);
    }
  }
  if (SL_RF_optHigh & OPT_WGHT) {
    free_uivector(SL_RF_weightDenom, 1, obsSize);
  }
  if (SL_RF_opt & OPT_MISS_OUT) {
    if (rspSize > 0) {
      free_new_vvector(SL_RF_sImputeResponsePtr, 1, rspSize, NRUTIL_DPTR);
    }
    free_new_vvector(SL_RF_sImputePredictorPtr, 1, SL_RF_xSize, NRUTIL_DPTR);
  }
  if (SL_RF_opt & (OPT_VARUSED_F | OPT_VARUSED_T)) {
    if (SL_RF_opt & OPT_VARUSED_T) {
    }
    else {
      free_uimatrix(SL_RF_varUsedPtr, 1, SL_RF_ntree, 1, SL_RF_xSize);
    }
  }
  if (SL_RF_opt & (OPT_SPLDPTH_1 | OPT_SPLDPTH_2)) {
  }
  if (SL_RF_optHigh & OPT_MEMB_PRUN) {
  }
  if (SL_RF_opt & OPT_CASE_DPTH) {
  }
  if (SL_RF_optHigh & OPT_PART_PLOT) {
    free_new_vvector(SL_RF_partMembership, 1, SL_RF_ntree, NRUTIL_TPTR2);
    if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
      {
      }
    }
    else {
      if (SL_RF_rTargetFactorCount > 0) {
      }
      if (SL_RF_rTargetNonFactorCount > 0) {
      }
    }
  }
}
void stackForestObjectsPtrOnly(char mode) {
  uint i;
  if (SL_RF_opt & OPT_TREE) {
    SL_RF_totalNodeCount = 0;
    SL_RF_totalMWCPCount = 0;
    if (mode == SL_RF_GROW) {
      SL_RF_treeID_ptr     = (uint **) new_vvector(1, SL_RF_ntree, NRUTIL_UPTR);
      SL_RF_nodeID_ptr     = (uint **) new_vvector(1, SL_RF_ntree, NRUTIL_UPTR);
      SL_RF_nodeSZ_ptr     = (uint **) new_vvector(1, SL_RF_ntree, NRUTIL_UPTR);
      SL_RF_blnodeID_ptr   = (uint **) new_vvector(1, SL_RF_ntree, NRUTIL_UPTR);
      SL_RF_brnodeID_ptr   = (uint **) new_vvector(1, SL_RF_ntree, NRUTIL_UPTR);
      SL_RF_parmID_ptr     = (int ***)    new_vvector(1, SL_RF_ntree, NRUTIL_IPTR2);
      SL_RF_contPT_ptr     = (double ***) new_vvector(1, SL_RF_ntree, NRUTIL_DPTR2);
      SL_RF_mwcpSZ_ptr     = (uint ***)   new_vvector(1, SL_RF_ntree, NRUTIL_UPTR2);
      SL_RF_fsrecID_ptr    = (uint ***)   new_vvector(1, SL_RF_ntree, NRUTIL_UPTR2);
      SL_RF_mwcpCT_ptr     = (uint **)    new_vvector(1, SL_RF_ntree, NRUTIL_UPTR2);
      SL_RF_mwcpPT_ptr     = (uint ***)   new_vvector(1, SL_RF_ntree, NRUTIL_UPTR2);
      for (i = 1; i <= SL_RF_ntree; i++) {
        SL_RF_parmID_ptr[i]  = (int **)    new_vvector(1, 1, NRUTIL_IPTR);
        SL_RF_contPT_ptr[i]  = (double **) new_vvector(1, 1, NRUTIL_DPTR);
        SL_RF_mwcpSZ_ptr[i]  = (uint **)   new_vvector(1, 1, NRUTIL_UPTR);
        SL_RF_fsrecID_ptr[i] = (uint **)   new_vvector(1, 1, NRUTIL_UPTR);
        SL_RF_mwcpCT_ptr[i]  =  uivector(1, 1);
        SL_RF_mwcpPT_ptr[i]  = (uint **)   new_vvector(1, 1, NRUTIL_UPTR);
      }
      for (i = 1; i <= SL_RF_ntree; i++) {
        SL_RF_treeID_ptr[i]     = NULL;
        SL_RF_nodeID_ptr[i]     = NULL;
        SL_RF_nodeSZ_ptr[i]     = NULL;
        SL_RF_blnodeID_ptr[i]   = NULL;
        SL_RF_brnodeID_ptr[i]   = NULL;
        SL_RF_parmID_ptr[i][1]  = NULL;
        SL_RF_contPT_ptr[i][1]  = NULL;
        SL_RF_mwcpSZ_ptr[i][1]  = NULL;
        SL_RF_fsrecID_ptr[i][1] = NULL;
        SL_RF_mwcpCT_ptr[i][1]  = 0;
        SL_RF_mwcpPT_ptr[i][1]  = NULL;
      }
    }  
  }  
}
void unstackForestObjectsPtrOnly(char mode) {
  uint treeID;
  if (SL_RF_opt & OPT_TREE) {
    if (mode == SL_RF_GROW) {
      for (treeID = 1; treeID <= SL_RF_ntree; treeID++) {
        unstackTreeObjectsPtrOnly(treeID);
      }
      free_new_vvector(SL_RF_treeID_ptr,     1, SL_RF_ntree, NRUTIL_UPTR);
      free_new_vvector(SL_RF_nodeID_ptr,     1, SL_RF_ntree, NRUTIL_UPTR);
      free_new_vvector(SL_RF_nodeSZ_ptr,     1, SL_RF_ntree, NRUTIL_UPTR);
      free_new_vvector(SL_RF_blnodeID_ptr,   1, SL_RF_ntree, NRUTIL_UPTR);
      free_new_vvector(SL_RF_brnodeID_ptr,   1, SL_RF_ntree, NRUTIL_UPTR);
      free_new_vvector(SL_RF_parmID_ptr,  1,  SL_RF_ntree, NRUTIL_IPTR2);
      free_new_vvector(SL_RF_contPT_ptr,  1,  SL_RF_ntree, NRUTIL_DPTR2);
      free_new_vvector(SL_RF_mwcpSZ_ptr,  1,  SL_RF_ntree, NRUTIL_UPTR2);
      free_new_vvector(SL_RF_fsrecID_ptr, 1,  SL_RF_ntree, NRUTIL_UPTR2);
      free_new_vvector(SL_RF_mwcpCT_ptr, 1,  SL_RF_ntree, NRUTIL_UPTR2);
      free_new_vvector(SL_RF_mwcpPT_ptr, 1,  SL_RF_ntree, NRUTIL_UPTR2);
    }  
  }  
}
void stackTreeObjectsPtrOnly(char mode, uint treeID) {
  uint treeNodeCount;
  uint mwcpSize;
  uint treeMWCPCount;
  if (SL_RF_opt & OPT_TREE) {
    if (mode == SL_RF_GROW) {
      treeNodeCount = SL_RF_nodeCount[treeID];
      if (SL_RF_xFactorCount > 0) {
        mwcpSize = (SL_RF_xMaxFactorLevel >> (3 + ulog2(sizeof(uint)))) + ((SL_RF_xMaxFactorLevel & (MAX_EXACT_LEVEL - 1)) ? 1 : 0);
      }
      else {
        mwcpSize = 0;
      }
      treeMWCPCount = mwcpSize * treeNodeCount;
      SL_RF_treeID_ptr[treeID]     = uivector(1, treeNodeCount);
      SL_RF_nodeID_ptr[treeID]     = uivector(1, treeNodeCount);
      SL_RF_nodeSZ_ptr[treeID]     = uivector(1, treeNodeCount);
      SL_RF_blnodeID_ptr[treeID]   = uivector(1, treeNodeCount);
      SL_RF_brnodeID_ptr[treeID]   = uivector(1, treeNodeCount);
      SL_RF_parmID_ptr[treeID][1]  = ivector(1, treeNodeCount);
      SL_RF_contPT_ptr[treeID][1]  = dvector(1, treeNodeCount);
      SL_RF_mwcpSZ_ptr[treeID][1]  = uivector(1, treeNodeCount);
      SL_RF_fsrecID_ptr[treeID][1] = uivector(1, treeNodeCount);
      if (treeMWCPCount > 0) {
        SL_RF_mwcpPT_ptr[treeID][1]  = uivector(1, treeMWCPCount);
      }
    }  
  }  
}
void unstackTreeObjectsPtrOnly(uint treeID) {
  uint treeNodeCount;
  uint mwcpSize;
  uint treeMWCPCount;
  treeNodeCount = SL_RF_nodeCount[treeID];
  if (SL_RF_xFactorCount > 0) {
    mwcpSize = (SL_RF_xMaxFactorLevel >> (3 + ulog2(sizeof(uint)))) + ((SL_RF_xMaxFactorLevel & (MAX_EXACT_LEVEL - 1)) ? 1 : 0);
  }
  else {
    mwcpSize = 0;
  }
  treeMWCPCount = mwcpSize * treeNodeCount;
  free_uivector(SL_RF_treeID_ptr[treeID],     1, treeNodeCount);
  free_uivector(SL_RF_nodeID_ptr[treeID],     1, treeNodeCount);
  free_uivector(SL_RF_nodeSZ_ptr[treeID],     1, treeNodeCount);
  free_uivector(SL_RF_blnodeID_ptr[treeID],   1, treeNodeCount);
  free_uivector(SL_RF_brnodeID_ptr[treeID],   1, treeNodeCount);
  free_ivector(SL_RF_parmID_ptr[treeID][1], 1, treeNodeCount);
  free_dvector(SL_RF_contPT_ptr[treeID][1], 1, treeNodeCount);
  free_uivector(SL_RF_mwcpSZ_ptr[treeID][1], 1, treeNodeCount);
  free_uivector(SL_RF_fsrecID_ptr[treeID][1], 1, treeNodeCount);
  if (treeMWCPCount > 0) {
    free_uivector(SL_RF_mwcpPT_ptr[treeID][1], 1, treeMWCPCount);
  }
  free_new_vvector(SL_RF_parmID_ptr[treeID], 1, 1, NRUTIL_IPTR);
  free_new_vvector(SL_RF_contPT_ptr[treeID], 1, 1, NRUTIL_DPTR);
  free_new_vvector(SL_RF_mwcpSZ_ptr[treeID], 1, 1, NRUTIL_UPTR);
  free_new_vvector(SL_RF_fsrecID_ptr[treeID], 1, 1, NRUTIL_UPTR);
  free_new_vvector(SL_RF_mwcpPT_ptr[treeID], 1, 1, NRUTIL_UPTR);
  free_uivector(SL_RF_mwcpCT_ptr[treeID], 1, 1);
}
void stackForestObjectsOutput(char mode) {
  ulong  totalNodeCount;
  ulong *totalMWCPCount;
  char *resultStr;
  char *adjStr;
  uint asciiLengthOfHexPortion;
  uint i, j;
  if (SL_RF_opt & OPT_TREE) {
    j = 0;
    asciiLengthOfHexPortion = 0;
    while (j > 0) {
      asciiLengthOfHexPortion++;
      j = j >> 4;
    }
    resultStr = cvector(0, SL_RF_SEXP_ASCII_SIZE + asciiLengthOfHexPortion + 1);
    adjStr =    cvector(0, asciiLengthOfHexPortion + 1);
    if (mode == SL_RF_GROW) {
        totalMWCPCount     = ulvector(1, 1);
        SL_RF_parmID_  = (int **)    new_vvector(1, 1, NRUTIL_IPTR);
        SL_RF_contPT_  = (double **) new_vvector(1, 1, NRUTIL_DPTR);
        SL_RF_mwcpSZ_  = (uint **)   new_vvector(1, 1, NRUTIL_UPTR);
        SL_RF_fsrecID_ = (uint **)   new_vvector(1, 1, NRUTIL_UPTR);
        SL_RF_mwcpPT_  = (uint **)   new_vvector(1, 1, NRUTIL_UPTR);
        SL_RF_mwcpCT_  = (uint **)   new_vvector(1, 1, NRUTIL_UPTR);
        totalMWCPCount[1] = 0;
        for (i = 1; i <= SL_RF_ntree; i++) {
          totalMWCPCount[1] += SL_RF_mwcpCT_ptr[i][1];
        }
      SL_RF_totalNodeCount = 0;
      for (i = 1; i <= SL_RF_ntree; i++) {      
        SL_RF_totalNodeCount += SL_RF_nodeCount[i];
      }
      totalNodeCount = SL_RF_totalNodeCount;
      SL_RF_treeID_ = (uint*)   stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_TREE_ID, totalNodeCount, 0, SL_RF_sexpString[SL_RF_TREE_ID], NULL, 1, totalNodeCount);
      SL_RF_nodeID_ = (uint*)   stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_NODE_ID, totalNodeCount, 0, SL_RF_sexpString[SL_RF_NODE_ID], NULL, 1, totalNodeCount);
      SL_RF_nodeSZ_ = (uint*)   stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_NODE_SZ, totalNodeCount, 0, SL_RF_sexpString[SL_RF_NODE_SZ], NULL, 1, totalNodeCount);
      SL_RF_blnodeID_ = (uint*)   stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_BL_NODE_ID, totalNodeCount, 0, SL_RF_sexpString[SL_RF_BL_NODE_ID], NULL, 1, totalNodeCount);
      SL_RF_brnodeID_ = (uint*)   stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_BR_NODE_ID, totalNodeCount, 0, SL_RF_sexpString[SL_RF_BR_NODE_ID], NULL, 1, totalNodeCount);
      SL_RF_treeID_ --;
      SL_RF_nodeID_ --;
      SL_RF_nodeSZ_ --;
      SL_RF_blnodeID_ --;
      SL_RF_brnodeID_ --;
      SL_RF_parmID_[1]  = (int*)    stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_PARM_ID, totalNodeCount, 0, SL_RF_sexpString[SL_RF_PARM_ID], NULL, 1, totalNodeCount);
      SL_RF_contPT_[1]  = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_CONT_PT, totalNodeCount, 0, SL_RF_sexpString[SL_RF_CONT_PT], NULL, 1, totalNodeCount);
      SL_RF_mwcpSZ_[1]  = (uint*)   stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_MWCP_SZ, totalNodeCount, 0, SL_RF_sexpString[SL_RF_MWCP_SZ], NULL, 1, totalNodeCount);
      SL_RF_fsrecID_[1] = (uint*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_FS_REC_ID, totalNodeCount, 0, SL_RF_sexpString[SL_RF_FS_REC_ID], NULL, 1, totalNodeCount);
      SL_RF_parmID_[1]  --;
      SL_RF_contPT_[1]  --;
      SL_RF_mwcpSZ_[1]  --;
      SL_RF_fsrecID_[1] --;
      if (totalMWCPCount[1] > 0) {
        SL_RF_mwcpPT_[1] = (uint*)   stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_MWCP_PT, totalMWCPCount[1], 0, SL_RF_sexpString[SL_RF_MWCP_PT], NULL, 1, totalMWCPCount[1]);
        SL_RF_mwcpPT_[1] --;
      }
      else {
        SL_RF_mwcpPT_[1] = (uint*)   stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_MWCP_PT, 1, 0, SL_RF_sexpString[SL_RF_MWCP_PT], NULL, 1, 1);
      }
      SL_RF_mwcpCT_[1] = (uint*)   stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_MWCP_CT, (ulong) SL_RF_ntree, 0, SL_RF_sexpString[SL_RF_MWCP_CT], NULL, 1, SL_RF_ntree);
      SL_RF_mwcpCT_[1] --;
      free_ulvector(totalMWCPCount,     1, 1);
    }  
    free_cvector(resultStr, 0, SL_RF_SEXP_ASCII_SIZE + asciiLengthOfHexPortion + 1);
    free_cvector(adjStr, 0, asciiLengthOfHexPortion + 1);
  }  
}
void writeForestObjectsOutput(char mode) {
  ulong *totalMWCPCount;
  ulong offset;
  uint treeID;
  uint j;
  if (SL_RF_opt & OPT_TREE) {
    if (mode == SL_RF_GROW) {
      offset = 0;
      for (treeID = 1; treeID <= SL_RF_ntree; treeID++) {
        SL_RF_mwcpCT_[1][treeID] = SL_RF_mwcpCT_ptr[treeID][1];
        for (j = 1; j <= SL_RF_nodeCount[treeID]; j++) {
          offset ++;
          SL_RF_treeID_[offset] = SL_RF_treeID_ptr[treeID][j];
          SL_RF_nodeID_[offset] = SL_RF_nodeID_ptr[treeID][j];
          SL_RF_nodeSZ_[offset] = SL_RF_nodeSZ_ptr[treeID][j];
          SL_RF_blnodeID_[offset] = SL_RF_blnodeID_ptr[treeID][j];
          SL_RF_brnodeID_[offset] = SL_RF_brnodeID_ptr[treeID][j];
          SL_RF_parmID_[1][offset]  = SL_RF_parmID_ptr[treeID][1][j];
          SL_RF_contPT_[1][offset]  = SL_RF_contPT_ptr[treeID][1][j];
          SL_RF_mwcpSZ_[1][offset]  = SL_RF_mwcpSZ_ptr[treeID][1][j];
          SL_RF_fsrecID_[1][offset]  = SL_RF_fsrecID_ptr[treeID][1][j];
        }
      }
      if (offset != SL_RF_totalNodeCount) {
        SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
        SL_RF_nativeError("\nRF-SRC:  Inconsistent total node count encountered during writing of forest topology.");
        SL_RF_nativeError("\nRF-SRC:  Offset versus total was:  %20lu, %20lu", offset, SL_RF_totalNodeCount);
        SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
        SL_RF_nativeExit();
      }
      totalMWCPCount = ulvector(1, 1);
      totalMWCPCount[1] = 0;
      for (treeID = 1; treeID <= SL_RF_ntree; treeID++) {
        totalMWCPCount[1] += SL_RF_mwcpCT_[1][treeID];
      }
      offset = 0;
      for (treeID = 1; treeID <= SL_RF_ntree; treeID++) {
        for (j = 1; j <= SL_RF_mwcpCT_[1][treeID]; j++) {
          offset ++;
          SL_RF_mwcpPT_[1][offset] = SL_RF_mwcpPT_ptr[treeID][1][j];
        }
      }
      if (offset != totalMWCPCount[1]) {
        SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
        SL_RF_nativeError("\nRF-SRC:  Inconsistent MWCP count encountered during writing of forest topology.");
        SL_RF_nativeError("\nRF-SRC:  Offset versus total MWCP count was:  %20lu, %20lu", offset, totalMWCPCount[1]);
        SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
        SL_RF_nativeExit();
      }
      free_ulvector(totalMWCPCount, 1, 1);
    }  
  }  
}
void stackForestObjectsAuxOnly(char mode) {
  uint i;
  if (mode != SL_RF_GROW) {
    SL_RF_parmID_   = (int **)    new_vvector(1, 1, NRUTIL_UPTR);
    SL_RF_contPT_   = (double **) new_vvector(1, 1, NRUTIL_DPTR);
    SL_RF_mwcpSZ_   = (uint **)   new_vvector(1, 1, NRUTIL_UPTR);
    SL_RF_fsrecID_  = (uint **)  new_vvector(1, 1, NRUTIL_UPTR);
    SL_RF_mwcpPT_   = (uint **)   new_vvector(1, 1, NRUTIL_UPTR);
    SL_RF_mwcpCT_   = (uint **)   new_vvector(1, 1, NRUTIL_UPTR);
    SL_RF_restoreTreeID = uivector(1, SL_RF_ntree);
    SL_RF_restoreTreeOffset = ulvector(1, SL_RF_ntree);
    for (i = 1; i <= SL_RF_ntree; i++) {
      SL_RF_restoreTreeID[i] = 0;
      SL_RF_restoreTreeOffset[i] = 0;
    }
    SL_RF_restoreMWCPoffset = new_vvector(1, 1, NRUTIL_LPTR);
    SL_RF_restoreMWCPoffset[1] = ulvector(1, SL_RF_ntree);
    for (i = 1; i <= SL_RF_ntree; i++) {
      SL_RF_restoreMWCPoffset[1][i] = 0;
    }
    SL_RF_mwcpCT_[1] = uivector(1, SL_RF_ntree);
    for (i = 1; i <= SL_RF_ntree; i++) {
      SL_RF_mwcpCT_[1][i] = 0;
    }
  }
}
void unstackForestObjectsAuxOnly(char mode) {
  if (mode == SL_RF_GROW) {
    if (SL_RF_opt & OPT_TREE) {
      free_new_vvector(SL_RF_parmID_,  1, 1, NRUTIL_IPTR);
      free_new_vvector(SL_RF_contPT_,  1, 1, NRUTIL_DPTR);
      free_new_vvector(SL_RF_mwcpSZ_,  1, 1, NRUTIL_UPTR);
      free_new_vvector(SL_RF_fsrecID_, 1, 1, NRUTIL_UPTR);
      free_new_vvector(SL_RF_mwcpPT_,  1, 1, NRUTIL_UPTR);
      free_new_vvector(SL_RF_mwcpCT_,  1, 1, NRUTIL_UPTR);
    }
  }
  if (mode != SL_RF_GROW) {
    free_new_vvector(SL_RF_parmID_,  1, 1, NRUTIL_IPTR);
    free_new_vvector(SL_RF_contPT_,  1, 1, NRUTIL_DPTR);
    free_new_vvector(SL_RF_mwcpSZ_,  1, 1, NRUTIL_UPTR);
    free_new_vvector(SL_RF_fsrecID_, 1, 1, NRUTIL_UPTR);
    free_new_vvector(SL_RF_mwcpPT_,  1, 1, NRUTIL_UPTR);
    free_uivector(SL_RF_restoreTreeID, 1, SL_RF_ntree);
    free_ulvector(SL_RF_restoreTreeOffset, 1, SL_RF_ntree);
    free_ulvector(SL_RF_restoreMWCPoffset[1], 1, SL_RF_ntree);
    free_new_vvector(SL_RF_restoreMWCPoffset, 1, 1, NRUTIL_LPTR);
    free_uivector(SL_RF_mwcpCT_[1], 1, SL_RF_ntree);
    free_new_vvector(SL_RF_mwcpCT_, 1, 1, NRUTIL_UPTR);
  }
}
void verifyAndRegisterCustomSplitRules(void) {
  uint familyConstant;
  uint i, j;
  familyConstant = 0;  
  if (SL_RF_splitRule == CUST_SPLIT) {
    SL_RF_splitCustomIdx = (SL_RF_optHigh & OPT_SPLT_CUST) >> 8;
    for (i = 0; i < 4; i++) {
      for (j = 0; j < 16; j++) {
        customFunctionArray[i][j] = NULL;
      }
    }
    registerCustomFunctions();
    if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
      {
        if (!(SL_RF_opt & OPT_COMP_RISK)) {
          familyConstant = SURV_FAM;
        }
        else {
          familyConstant = CRSK_FAM;
        }
        if (customFunctionArray[familyConstant][SL_RF_splitCustomIdx] == NULL) {
          SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
          SL_RF_nativeError("\nRF-SRC:  Custom split rule not registered:  %10d", SL_RF_splitCustomIdx + 1);
          SL_RF_nativeError("\nRF-SRC:  Please register the rule and recompile the package.");
          SL_RF_nativeExit();
        }
      }
    }
    else {
      if (SL_RF_rTargetFactorCount > 0) {
        if (customFunctionArray[CLAS_FAM][SL_RF_splitCustomIdx] == NULL) {
          SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
          SL_RF_nativeError("\nRF-SRC:  Custom split rule not registered:  %10d", SL_RF_splitCustomIdx + 1);
          SL_RF_nativeError("\nRF-SRC:  Please register the rule and recompile the package.");
          SL_RF_nativeExit();
        }
      }
      if (SL_RF_rTargetNonFactorCount > 0) {
        if (customFunctionArray[REGR_FAM][SL_RF_splitCustomIdx] == NULL) {
          SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
          SL_RF_nativeError("\nRF-SRC:  Custom split rule not registered:  %10d", SL_RF_splitCustomIdx + 1);
          SL_RF_nativeError("\nRF-SRC:  Please register the rule and recompile the package.");
          SL_RF_nativeExit();
        }
      }
    }
  }
}
void stackAuxiliaryInfoList(SNPAuxiliaryInfo ***list, uint count) {
   *list = new_vvector(0, count, NRUTIL_XPTR);
   for (uint i = 0; i <= count; i++) {
     (*list)[i] = NULL;
  }
 }
void allocateAuxiliaryInfo(char   targetFlag,
                           char   type,
                           char  *stringIdentifier,
                           SNPAuxiliaryInfo **list,
                           uint   slot,
                           void  *snpPtr,
                           void  *auxiliaryArrayPtr,
                           uint   dimSize,
                           int   *dim) {
  uint dim1, dim2, dim3, dim4;
  ulong offset;
  uint stringLength;
  SNPAuxiliaryInfo *auxInfoPtr = (SNPAuxiliaryInfo*) gblock((size_t) sizeof(SNPAuxiliaryInfo));
  list[slot] = auxInfoPtr;
  auxInfoPtr -> slot = slot;
  auxInfoPtr -> type = type;
  stringLength = strlen(stringIdentifier) + 1;
  auxInfoPtr -> identity = cvector(1, stringLength);
  strcpy(auxInfoPtr -> identity, stringIdentifier);
  auxInfoPtr -> snpPtr = snpPtr;
  auxInfoPtr -> auxiliaryArrayPtr = auxiliaryArrayPtr;
  auxInfoPtr -> dimSize = dimSize;
  (auxInfoPtr -> dim) = ivector(1, dimSize);
  for (uint i = 1; i <= dimSize; i++) {
    (auxInfoPtr -> dim)[i] = dim[i];
  }
  switch(type) {
  case NATIVE_TYPE_NUMERIC:
    if (auxiliaryArrayPtr == NULL) {
    }
    else if (dimSize == 4) {
      offset = 0;
      dim1 = getAuxDim(targetFlag, dim, 0 , 1);
      *((double *****) auxiliaryArrayPtr) = (double ****) new_vvector(1, dim1, NRUTIL_DPTR3);
      for (uint i = 1; i <= dim1; i++) {
        dim2 = getAuxDim(targetFlag, dim, i , 2);
        if (dim2 > 0) {
          (*((double *****) auxiliaryArrayPtr))[i] = (double ***) new_vvector(1, dim2, NRUTIL_DPTR2);
          for (uint j = 1; j <= dim2; j++) {
            dim3 = getAuxDim(targetFlag, dim, j , 3);
            (*((double *****) auxiliaryArrayPtr))[i][j] = (double **) new_vvector(1, dim3, NRUTIL_DPTR);
            for (uint k = 1; k <= dim3; k++) {
              dim4 = getAuxDim(targetFlag, dim, k , 4);
              (*((double *****) auxiliaryArrayPtr))[i][j][k] = (double *) snpPtr + offset - 1;
              offset += dim4;
            }
          }
        }
      }
    }
    else if (dimSize == 3) {
      offset = 0;      
      dim1 = getAuxDim(targetFlag, dim, 0 , 1);
      *((double ****) auxiliaryArrayPtr) = (double ***) new_vvector(1, dim1, NRUTIL_DPTR2);
      for (uint i = 1; i <= dim1; i++) {
        dim2 = getAuxDim(targetFlag, dim, i , 2);
        if (dim2 > 0) {
          (*((double ****) auxiliaryArrayPtr))[i] = (double **) new_vvector(1, dim2, NRUTIL_DPTR);
          for (uint j = 1; j <= dim2; j++) {
            dim3 = getAuxDim(targetFlag, dim, j , 3);
            (*((double ****) auxiliaryArrayPtr))[i][j] = (double *) snpPtr + offset - 1;
            offset += dim3;
          }
        }
      }
    }
    else if (dimSize == 2) {
      offset = 0;
      dim1 = getAuxDim(targetFlag, dim, 0 , 1);
      *((double ***) auxiliaryArrayPtr) = (double **) new_vvector(1, dim1, NRUTIL_DPTR);
      for (uint i = 1; i <= dim1; i++) {
        dim2 = getAuxDim(targetFlag, dim, i , 2);
        (*((double ***) auxiliaryArrayPtr))[i] = (double *) snpPtr + offset - 1;
          offset += dim2;
      }
    }
    else if (dimSize == 1) {
      *((double **) auxiliaryArrayPtr) = (double *) snpPtr - 1;
    }
    else {
      SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
      SL_RF_nativeError("\nRF-SRC:  Invalid ( > 4 ) dimension in stackAndProtect() auxiliary arrays:  %10d", dimSize);
      SL_RF_nativeExit();
    }
    break;
  case NATIVE_TYPE_INTEGER:
    if (auxiliaryArrayPtr == NULL) {
    }
    else if (dimSize == 4) {
      offset = 0;
      dim1 = getAuxDim(targetFlag, dim, 0 , 1);
      *((uint *****) auxiliaryArrayPtr) = (uint ****) new_vvector(1, dim1, NRUTIL_UPTR3);
      for (uint i = 1; i <= dim1; i++) {
        dim2 = getAuxDim(targetFlag, dim, i , 2);
        (*((uint *****) auxiliaryArrayPtr))[i] = (uint ***) new_vvector(1, dim2, NRUTIL_UPTR2);
        for (uint j = 1; j <= dim2; j++) {
            dim3 = getAuxDim(targetFlag, dim, j , 3);
          (*((uint *****) auxiliaryArrayPtr))[i][j] = (uint **) new_vvector(1, dim3, NRUTIL_UPTR);
          for (uint k = 1; k <= dim3; k++) {
            dim4 = getAuxDim(targetFlag, dim, k , 4);
            (*((uint *****) auxiliaryArrayPtr))[i][j][k] = (uint *) snpPtr + offset - 1;
            offset += dim4;
          }
        }
      }
    }
    else if (dimSize == 3) {
      offset = 0;
      dim1 = getAuxDim(targetFlag, dim, 0 , 1);
      *((uint ****) auxiliaryArrayPtr) = (uint ***) new_vvector(1, dim1, NRUTIL_UPTR2);
      for (uint i = 1; i <= dim1; i++) {
        dim2 = getAuxDim(targetFlag, dim, i , 2);
        (*((uint ****) auxiliaryArrayPtr))[i] = (uint **) new_vvector(1, dim2, NRUTIL_UPTR);
        for (uint j = 1; j <= dim2; j++) {
          dim3 = getAuxDim(targetFlag, dim, j , 3);
          (*((uint ****) auxiliaryArrayPtr))[i][j] = (uint *) snpPtr + offset - 1;
            offset += dim3;
        }
      }
    }
    else if (dimSize == 2) {
      offset = 0;
      dim1 = getAuxDim(targetFlag, dim, 0 , 1);
      *((uint ***) auxiliaryArrayPtr) = (uint **) new_vvector(1, dim1, NRUTIL_UPTR);
      for (uint i = 1; i <= dim1; i++) {
        dim2 = getAuxDim(targetFlag, dim, i , 2);
        (*((uint ***) auxiliaryArrayPtr))[i] = (uint *) snpPtr + offset - 1;
          offset += dim2;
      }
    }
    else if (dimSize == 1) {
      *((uint **) auxiliaryArrayPtr) = (uint *) snpPtr - 1;
    }
    else {
      SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
      SL_RF_nativeError("\nRF-SRC:  Invalid ( > 4 ) dimension in stackAndProtect() auxiliary arrays:  %10d", dimSize);
      SL_RF_nativeExit();
    }
    break;
  }
}
uint getAuxDim(char flag, int *dim, uint iterIndex, uint slot) {
  uint result = 0;
  if (slot == 1) {
    result = dim[slot];
  }
  else if (dim[slot] >= 1) {
    result = dim[slot];
  }
  else if (dim[slot] == 0) {
    if (flag) {
      result = SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[iterIndex]]];
    }
    else {
      result = SL_RF_rFactorSize[iterIndex];
    }
  }
  else if (dim[slot] == -1) {
    if (flag) {
      result = 1 + SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[iterIndex]]];
    }
    else {
      result = 1 + SL_RF_rFactorSize[iterIndex];
    }
  }
  else if (dim[slot] == -2) {
    result = SL_RF_tLeafCount[iterIndex];
  }
  else if (dim[slot] == -3) {
    result = SL_RF_holdBLKptr[iterIndex];
  }
  else {
    SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    SL_RF_nativeError("\nRF-SRC:  Inconsistent internal dimension of auxiliary array in getAuxDim():  %10d", dim[slot]);
    SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
  }
  return result;
}
void unstackAuxiliaryInfoAndList(char targetFlag, SNPAuxiliaryInfo **list, uint count) {
  SNPAuxiliaryInfo *auxInfoPtr;
  int  *dim;
  uint  dimSize;
  uint  dim1, dim2, dim3;
  uint stringLength;
  for (uint ii = 0; ii < count; ii++) {
     auxInfoPtr = list[ii];
     if (auxInfoPtr != NULL) {
       dim = auxInfoPtr -> dim;
       dimSize = auxInfoPtr -> dimSize;
       stringLength = strlen(auxInfoPtr -> identity) + 1;
       free_cvector(auxInfoPtr -> identity, 1, stringLength);
       switch(auxInfoPtr -> type) {
       case NATIVE_TYPE_NUMERIC:
         if ((auxInfoPtr -> auxiliaryArrayPtr) == NULL) {
         }
         else if (dimSize == 4) {
           dim1 = getAuxDim(targetFlag, dim, 0 , 1);
           for (uint i = 1; i <= dim1; i++) {
             dim2 = getAuxDim(targetFlag, dim, i , 2);
             if (dim2 > 0) {
               for (uint j = 1; j <= dim2; j++) {
                 dim3 = getAuxDim(targetFlag, dim, j , 3);
                 free_new_vvector((*((double *****) (auxInfoPtr -> auxiliaryArrayPtr)))[i][j], 1, dim3, NRUTIL_DPTR);
               }
               free_new_vvector((*((double *****) (auxInfoPtr -> auxiliaryArrayPtr)))[i], 1, dim2, NRUTIL_DPTR2);
             }
           }
           free_new_vvector((*((double *****) (auxInfoPtr -> auxiliaryArrayPtr))), 1, dim1, NRUTIL_DPTR3);
         }
         else if (dimSize == 3) {
           dim1 = getAuxDim(targetFlag, dim, 0 , 1);
           for (uint i = 1; i <= dim1; i++) {
             dim2 = getAuxDim(targetFlag, dim, i , 2);
             if (dim2 > 0) {
               free_new_vvector((*((double ****) (auxInfoPtr -> auxiliaryArrayPtr)))[i], 1, dim2, NRUTIL_DPTR);
             }
           }
           free_new_vvector((*((double ****) (auxInfoPtr -> auxiliaryArrayPtr))), 1, dim1, NRUTIL_DPTR2);
         }
         else if (dimSize == 2) {
           dim1 = getAuxDim(targetFlag, dim, 0 , 1);
           free_new_vvector((*((double ***) (auxInfoPtr -> auxiliaryArrayPtr))), 1, dim1, NRUTIL_DPTR);
         }
         else if (dimSize == 1) {
         }
         break;
       case NATIVE_TYPE_INTEGER:
         if ((auxInfoPtr -> auxiliaryArrayPtr) == NULL) {
         }
         else if (dimSize == 4) {
           dim1 = getAuxDim(targetFlag, dim, 0 , 1);
           for (uint i = 1; i <= dim1; i++) {
             dim2 = getAuxDim(targetFlag, dim, i , 2);
             for (uint j = 1; j <= dim2; j++) {
               dim3 = getAuxDim(targetFlag, dim, j , 3);
               free_new_vvector((*((uint *****) (auxInfoPtr -> auxiliaryArrayPtr)))[i][j], 1, dim3, NRUTIL_UPTR);
             }
             free_new_vvector((*((uint *****) (auxInfoPtr -> auxiliaryArrayPtr)))[i], 1, dim2, NRUTIL_UPTR2);
           }
           free_new_vvector((*((uint *****) (auxInfoPtr -> auxiliaryArrayPtr))), 1,  dim1, NRUTIL_UPTR3);
         }
         else if (dimSize == 3) {
           dim1 = getAuxDim(targetFlag, dim, 0 , 1);
           for (uint i = 1; i <= dim1; i++) {
             dim2 = getAuxDim(targetFlag, dim, i , 2);             
             free_new_vvector((*((uint ****) (auxInfoPtr -> auxiliaryArrayPtr)))[i], 1, dim2, NRUTIL_UPTR);
           }
           free_new_vvector((*((uint ****) (auxInfoPtr -> auxiliaryArrayPtr))), 1, dim1, NRUTIL_UPTR2);
         }
         else if (dimSize == 2) {
           dim1 = getAuxDim(targetFlag, dim, 0 , 1);
           free_new_vvector((*((uint ***) (auxInfoPtr -> auxiliaryArrayPtr))), 1, dim1, NRUTIL_UPTR);
         }
         else if (dimSize == 1) {
         }
         break;
       }
       free_ivector(auxInfoPtr -> dim, 1, auxInfoPtr -> dimSize);
       free_gblock(auxInfoPtr, sizeof(SNPAuxiliaryInfo));
     }
   }
  free_new_vvector(list, 0, count, NRUTIL_XPTR);
}
