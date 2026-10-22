/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "entry.h"
#include "entryGeneric.h"
#include "nativeUtil.h"
#include "rfsrc.h"
#include "stackOutput.h"
SEXP sl_rfsrcGrow(SEXP traceFlag,
               SEXP seedPtr,
               SEXP optLow,
               SEXP optHigh,
               SEXP optSup,
               SEXP splitRule,
               SEXP nsplit,
               SEXP mtry,
               SEXP lot,
               SEXP baseLearn,
               SEXP vtry,
               SEXP vtryArray,
               SEXP vtryExperimental,
               SEXP ytry,
               SEXP nodeSize,
               SEXP nodeDepth,
               SEXP crWeightSize,
               SEXP crWeight,
               SEXP vimpThreshold,
               SEXP ntree,
               SEXP observationSize,
               SEXP yInfo,
               SEXP yLevels,
               SEXP yData,
               SEXP xInfo,
               SEXP xLevels,
               SEXP xData,
               SEXP sampleInfo,
               SEXP xWeightStat,
               SEXP yWeight,
               SEXP xWeight,
               SEXP timeInterest,
               SEXP nImpute,
               SEXP perfBlock,
               SEXP quantileInfo,
               SEXP qStarPlus,
               SEXP unoWeight,
               SEXP numThreads) {
  clock_t cpuTimeStart = clock();
  setUserTraceFlag(INTEGER(traceFlag)[0]);
  setNativeGlobalEnv(&SL_RF_nativeIndex, &SL_RF_stackCount);
  int seedValue           = INTEGER(seedPtr)[0];
  SL_RF_opt                  = INTEGER(optLow)[0];
  SL_RF_optHigh              = INTEGER(optHigh)[0];
  SL_RF_optSup               = INTEGER(optSup)[0];
  SL_RF_splitRule            = INTEGER(splitRule)[0];
  SL_RF_nsplit               = INTEGER(nsplit)[0];
  SL_RF_mtry                 = INTEGER(mtry)[0];
  SL_RF_ytry                 = INTEGER(ytry)[0];
  SL_RF_nodeSize             = INTEGER(nodeSize)[0];
  SL_RF_nodeDepth            = INTEGER(nodeDepth)[0];
  SL_RF_crWeight             = NULL;
  SL_RF_crWeightSize         = INTEGER(crWeightSize)[0];
  if (SL_RF_crWeightSize > 0) {
    SL_RF_crWeight             = REAL(crWeight); SL_RF_crWeight--;
  }
  SL_RF_vimpThreshold        = REAL(vimpThreshold)[0];
  SL_RF_ntree                = INTEGER(ntree)[0];
  SL_RF_observationSize      = INTEGER(observationSize)[0];
  SL_RF_ySize                = INTEGER(VECTOR_ELT(yInfo, 0))[0];
  if(VECTOR_ELT(yInfo, 1) != R_NilValue) {
    SL_RF_rType = (char *) copy1DObject(VECTOR_ELT(yInfo, 1), NATIVE_TYPE_CHARACTER, SL_RF_ySize, TRUE);
  }
  else {
    SL_RF_rType = NULL;
  }
  if(VECTOR_ELT(yInfo, 2) != R_NilValue) {
    SL_RF_rLevelsMax         = (uint *) INTEGER(VECTOR_ELT(yInfo, 2)); SL_RF_rLevelsMax--;
  }
  else {
    SL_RF_rLevelsMax         = NULL;
  }
  if(VECTOR_ELT(yInfo, 3) != R_NilValue) {
    SL_RF_rLevelsCnt         = (uint *) INTEGER(VECTOR_ELT(yInfo, 3)); SL_RF_rLevelsCnt--;
  }
  else {
    SL_RF_rLevelsCnt         = NULL;
  }
  if(VECTOR_ELT(yInfo, 4) != R_NilValue) {
    SL_RF_subjIn             =  (uint *) INTEGER(VECTOR_ELT(yInfo, 4));  SL_RF_subjIn --;
  }
  else {
    SL_RF_subjIn             = NULL;
  }
  if(VECTOR_ELT(yInfo, 5) != R_NilValue) {
    SL_RF_eventTypeSize      =  INTEGER(VECTOR_ELT(yInfo, 5))[0];
  }
  else {
    SL_RF_eventTypeSize      = 0;
  }
  if(VECTOR_ELT(yInfo, 6) != R_NilValue) {
    if (SL_RF_eventTypeSize > 0) {
      SL_RF_eventType        =  (uint *) INTEGER(VECTOR_ELT(yInfo, 6));  SL_RF_eventType --;
    }
    else {
      SL_RF_eventType        = NULL;
    }
  }
  else {
    SL_RF_eventType          = NULL;
  }
  SL_RF_rLevelsSEXP = yLevels;
  SL_RF_rLevels = NULL;
  if(SL_RF_ySize > 0) {
    SL_RF_responseIn         = (double **) copy2DObject(yData, NATIVE_TYPE_NUMERIC, TRUE, SL_RF_ySize, SL_RF_observationSize);
  }
  else {
    SL_RF_responseIn = NULL;
  }
  SL_RF_xSize                 = INTEGER(VECTOR_ELT(xInfo, 0))[0];
  if(VECTOR_ELT(xInfo, 1) != R_NilValue) {  
    SL_RF_xType                = (char *) copy1DObject(VECTOR_ELT(xInfo, 1), NATIVE_TYPE_CHARACTER, SL_RF_xSize, TRUE);
  }
  else {
    SL_RF_xType = NULL;
  }
  if(VECTOR_ELT(xInfo, 2) != R_NilValue) {  
    SL_RF_xLevelsMax           = (uint *) INTEGER(VECTOR_ELT(xInfo, 2)); SL_RF_xLevelsMax --;
  }
  else {
    SL_RF_xLevelsMax = NULL;
  }
  if(VECTOR_ELT(xInfo, 3) != R_NilValue) {
    SL_RF_xLevelsCnt         = (uint *) INTEGER(VECTOR_ELT(xInfo, 3)); SL_RF_xLevelsCnt --;
  }
  else {
    SL_RF_xLevelsCnt         = NULL;
  }
  if(VECTOR_ELT(xInfo, 4) != R_NilValue) {
    SL_RF_xtType             = (uint *) INTEGER(VECTOR_ELT(xInfo, 4)); SL_RF_xtType --;
  }
  else {
    SL_RF_xtType             = NULL;
  }
  if(VECTOR_ELT(xInfo, 5) != R_NilValue) {
    SL_RF_stType             = (uint *) INTEGER(VECTOR_ELT(xInfo, 5)); SL_RF_stType --;
  }
  else {
    SL_RF_stType             = NULL;
  }
  SL_RF_xLevelsSEXP = xLevels;
  SL_RF_xLevels = NULL;
  if (SL_RF_xSize > 0) {
    SL_RF_observationIn      = (double **) copy2DObject(xData, NATIVE_TYPE_NUMERIC, TRUE, SL_RF_xSize, SL_RF_observationSize);
  }
  else {
    SL_RF_observationIn = NULL;
  }
  SL_RF_subjSize             = SL_RF_observationSize;
  SL_RF_subjWeight           = NULL;
  SL_RF_bootstrapSize        = SL_RF_observationSize;
  SL_RF_bootstrapIn          = NULL;
  if(VECTOR_ELT(sampleInfo, 0) != R_NilValue) {
    SL_RF_subjSize = INTEGER(VECTOR_ELT(sampleInfo, 0))[0];
    if(VECTOR_ELT(sampleInfo, 1) != R_NilValue) {
      SL_RF_subjWeight = REAL(VECTOR_ELT(sampleInfo, 1)); SL_RF_subjWeight--;
    }
    if(VECTOR_ELT(sampleInfo, 2) != R_NilValue) {  
      SL_RF_bootstrapSize        = INTEGER(VECTOR_ELT(sampleInfo, 2))[0];
      if(VECTOR_ELT(sampleInfo, 3) != R_NilValue) {
        SL_RF_bootstrapIn = (uint **) copy2DObject(VECTOR_ELT(sampleInfo, 3), NATIVE_TYPE_INTEGER, (SL_RF_opt & OPT_BOOT_TYP1) && (SL_RF_opt & OPT_BOOT_TYP2), SL_RF_ntree, SL_RF_subjSize);
      }
    }
    else {
      SL_RF_bootstrapSize        = SL_RF_subjSize;
    }
  }
  SL_RF_xWeightStat          = REAL(xWeightStat);  SL_RF_xWeightStat--;
  if (yWeight != R_NilValue) {
    SL_RF_yWeight              = REAL(yWeight);  SL_RF_yWeight--;
  }
  else {
    SL_RF_yWeight = NULL;
  }
  SL_RF_xWeight              = REAL(xWeight);  SL_RF_xWeight--;
  SL_RF_timeInterestSize = INTEGER(VECTOR_ELT(timeInterest, 0))[0];
  if (VECTOR_ELT(timeInterest, 1) != R_NilValue) {
    SL_RF_timeInterest         = (double *) REAL(VECTOR_ELT(timeInterest, 1));
    SL_RF_timeInterest --;
  }
  else {
    SL_RF_timeInterest = NULL;
  }
  SL_RF_nImpute              = INTEGER(nImpute)[0];
  SL_RF_perfBlock            = INTEGER(perfBlock)[0];
  SL_RF_quantileSize = INTEGER(VECTOR_ELT(quantileInfo, 0))[0];
  if (VECTOR_ELT(quantileInfo, 1) != R_NilValue) {
    SL_RF_quantile = (double *) REAL(VECTOR_ELT(quantileInfo, 1));
    SL_RF_quantile --;
  }
  else {
    SL_RF_quantile = NULL;
  }
  SL_RF_qEpsilon = REAL(VECTOR_ELT(quantileInfo, 2))[0];
  SL_RF_qStarPlus = NULL;
  if(SL_RF_ySize > 0) {
    if (qStarPlus != R_NilValue) {
      SL_RF_qStarPlus         = (double **) copy2DObject(qStarPlus, NATIVE_TYPE_NUMERIC, TRUE, SL_RF_ySize, SL_RF_ySize);
    }
  }
  SL_RF_vtry                = INTEGER(vtry)[0];
  SL_RF_vtryArray           = (uint **) copy2DObject(vtryArray, NATIVE_TYPE_INTEGER, SL_RF_vtry > 0, SL_RF_ntree, SL_RF_xSize);
  SL_RF_vtryMode            = SL_RF_VTRY_NULL;
  SL_RF_vtryBlockSize       = 0;
  if (vtryExperimental != R_NilValue) {
    SL_RF_vtry = INTEGER(VECTOR_ELT(vtryExperimental, 0))[0];
    if (SL_RF_vtry > 0) {
      if (VECTOR_ELT(vtryExperimental, 1) != R_NilValue) {
        SL_RF_vtryArray = (uint **) copy2DObject(VECTOR_ELT(vtryExperimental, 1), NATIVE_TYPE_INTEGER, SL_RF_vtry > 0, SL_RF_ntree, SL_RF_xSize);
        SL_RF_vtryBlockSize = INTEGER(VECTOR_ELT(vtryExperimental, 2))[0]; 
        SL_RF_vtryMode  = INTEGER(VECTOR_ELT(vtryExperimental, 3))[0];
      }
      else {
        SL_RF_vtry = 0;
      }
    }
  }
  if (unoWeight != R_NilValue) {  
    SL_RF_unoWeight            = REAL(unoWeight);  SL_RF_unoWeight--;
  }
  else {
    SL_RF_unoWeight = NULL;
  }
  SL_RF_numThreads           = INTEGER(numThreads)[0];
  processDefaultGrow();
  rfsrc(SL_RF_GROW, seedValue);
  free_1DObject(SL_RF_rType, NATIVE_TYPE_CHARACTER, SL_RF_ySize);
  free_1DObject(SL_RF_xType, NATIVE_TYPE_CHARACTER, SL_RF_xSize);
  free_2DObject(SL_RF_responseIn, NATIVE_TYPE_NUMERIC, SL_RF_ySize > 0, SL_RF_ySize, SL_RF_observationSize);
  free_2DObject(SL_RF_bootstrapIn, NATIVE_TYPE_INTEGER, (SL_RF_opt & OPT_BOOT_TYP1) && (SL_RF_opt & OPT_BOOT_TYP2), SL_RF_ntree, SL_RF_subjSize);
  free_2DObject(SL_RF_observationIn, NATIVE_TYPE_NUMERIC, TRUE, SL_RF_xSize, SL_RF_observationSize);  
  free_2DObject(SL_RF_qStarPlus, NATIVE_TYPE_NUMERIC, SL_RF_qStarPlus != NULL, SL_RF_ySize, SL_RF_ySize);
  free_2DObject(SL_RF_vtryArray, NATIVE_TYPE_INTEGER, SL_RF_vtry > 0, SL_RF_ntree, SL_RF_xSize);  
  SL_RF_cpuTime_[1] = (double) (clock() - cpuTimeStart) / CLOCKS_PER_SEC;
  R_ReleaseObject(SL_RF_sexpVector[SL_RF_OUTP_ID]);
  R_ReleaseObject(SL_RF_sexpVector[SL_RF_STRG_ID]);  
  return SL_RF_sexpVector[SL_RF_OUTP_ID];
}
SEXP sl_rfsrcPredict(SEXP traceFlag,
                  SEXP seedPtr,
                  SEXP optLow,
                  SEXP optHigh,
                  SEXP vimpThreshold,
                  SEXP ntree,
                  SEXP observationSize,
                  SEXP yInfo,
                  SEXP yLevels,
                  SEXP yData,
                  SEXP xInfo,
                  SEXP xLevels,
                  SEXP xData,
                  SEXP sampleInfo,
                  SEXP timeInterest,
                  SEXP totalNodeCount,
                  SEXP tLeafCount,
                  SEXP seedInfo,
                  SEXP hdim,
                  SEXP baseLearn,
                  SEXP treeID,
                  SEXP nodeID,
                  SEXP nodeSZ,
                  SEXP brnodeID,
                  SEXP hc_zero,
                  SEXP hc_oneAugIntr,
                  SEXP hc_oneAugSyth,
                  SEXP hc_one,
                  SEXP hc_parmID,
                  SEXP hc_contPT,
                  SEXP hc_contPTR,
                  SEXP hc_mwcpSZ,
                  SEXP hc_fsrecID,
                  SEXP hc_mwcpPT,
                  SEXP hc_augmXone,
                  SEXP hc_augmXtwo,
                  SEXP hc_augmXS,
                  SEXP hc_augmSythTop,
                  SEXP tnRMBR,
                  SEXP tnAMBR,
                  SEXP tnRCNT,
                  SEXP tnACNT,
                  SEXP tnSURV,
                  SEXP tnMORT,
                  SEXP tnNLSN,
                  SEXP tnCSHZ,
                  SEXP tnCIFN,
                  SEXP tnREGR,
                  SEXP tnCLAS,
                  SEXP yTarget,
                  SEXP ptnCount,
                  SEXP xMarginalInfo,
                  SEXP intrPredictorInfo,
                  SEXP partial,
                  SEXP fobservationSize,
                  SEXP frSize,
                  SEXP frData,
                  SEXP fxData,
                  SEXP perfBlock,
                  SEXP quantile,
                  SEXP getTree,
                  SEXP unoWeight,
                  SEXP numThreads) {
  char mode;
  clock_t cpuTimeStart = clock();
  setUserTraceFlag(INTEGER(traceFlag)[0]);
  setNativeGlobalEnv(&SL_RF_nativeIndex, &SL_RF_stackCount);
  int seedValue           = INTEGER(seedPtr)[0];
  SL_RF_opt                  = INTEGER(optLow)[0];
  SL_RF_optHigh              = INTEGER(optHigh)[0];
  SL_RF_vimpThreshold        = REAL(vimpThreshold)[0];
  SL_RF_ntree                = INTEGER(ntree)[0];
  SL_RF_observationSize      = INTEGER(observationSize)[0];
  SL_RF_ySize                = INTEGER(VECTOR_ELT(yInfo, 0))[0];
  if(VECTOR_ELT(yInfo, 1) != R_NilValue) {
    SL_RF_rType = (char *) copy1DObject(VECTOR_ELT(yInfo, 1), NATIVE_TYPE_CHARACTER, SL_RF_ySize, TRUE);
  }
  else {
    SL_RF_rType = NULL;
  }
  if(VECTOR_ELT(yInfo, 2) != R_NilValue) {
    SL_RF_rLevelsMax         = (uint *) INTEGER(VECTOR_ELT(yInfo, 2)); SL_RF_rLevelsMax--;
  }
  else {
    SL_RF_rLevelsMax         = NULL;
  }
  if(VECTOR_ELT(yInfo, 3) != R_NilValue) {
    SL_RF_rLevelsCnt         = (uint *) INTEGER(VECTOR_ELT(yInfo, 3)); SL_RF_rLevelsCnt--;
  }
  else {
    SL_RF_rLevelsCnt = NULL;
  }
  if(VECTOR_ELT(yInfo, 4) != R_NilValue) {
    SL_RF_subjIn             =  (uint *) INTEGER(VECTOR_ELT(yInfo, 4));  SL_RF_subjIn --;
  }
  else {
    SL_RF_subjIn             = NULL;
  }
  if(VECTOR_ELT(yInfo, 5) != R_NilValue) {
    SL_RF_eventTypeSize      =  INTEGER(VECTOR_ELT(yInfo, 5))[0];
  }
  else {
    SL_RF_eventTypeSize      = 0;
  }
  if(VECTOR_ELT(yInfo, 6) != R_NilValue) {
    if (SL_RF_eventTypeSize > 0) {
      SL_RF_eventType        =  (uint *) INTEGER(VECTOR_ELT(yInfo, 6));  SL_RF_eventType --;
    }
    else {
      SL_RF_eventType        = NULL;
    }
  }
  else {
    SL_RF_eventType          = NULL;
  }
  SL_RF_rLevelsSEXP = yLevels;
  SL_RF_rLevels = NULL;
  if(SL_RF_ySize > 0) {
    SL_RF_responseIn           = (double **) copy2DObject(yData, NATIVE_TYPE_NUMERIC, TRUE, SL_RF_ySize, SL_RF_observationSize);
  }
  else {
    SL_RF_responseIn = NULL;
  }
  SL_RF_xSize                 = INTEGER(VECTOR_ELT(xInfo, 0))[0];
  if(VECTOR_ELT(xInfo, 1) != R_NilValue) {
    SL_RF_xType                = (char *) copy1DObject(VECTOR_ELT(xInfo, 1), NATIVE_TYPE_CHARACTER, SL_RF_xSize, TRUE);
  }
  else {
    SL_RF_xType = NULL;
  }
  if(VECTOR_ELT(xInfo, 2) != R_NilValue) {  
    SL_RF_xLevelsMax           = (uint *) INTEGER(VECTOR_ELT(xInfo, 2)); SL_RF_xLevelsMax--;
  }
  else {
    SL_RF_xLevelsMax = NULL;
  }
  if(VECTOR_ELT(xInfo, 3) != R_NilValue) {
    SL_RF_xLevelsCnt           = (uint *) INTEGER(VECTOR_ELT(xInfo, 3)); SL_RF_xLevelsCnt --;
  }
  else {
    SL_RF_xLevelsCnt = NULL;
  }
  SL_RF_xtType = NULL;
  SL_RF_stType = NULL;
  SL_RF_xLevelsSEXP = xLevels;
  SL_RF_xLevels = NULL;
  if (xData != R_NilValue) {
    SL_RF_observationIn      = (double **) copy2DObject(xData, NATIVE_TYPE_NUMERIC, TRUE, SL_RF_xSize, SL_RF_observationSize);
  }
  else {
    SL_RF_observationIn = NULL;
  }
  SL_RF_subjSize             = 0;
  SL_RF_subjWeight           = NULL;
  SL_RF_bootstrapSize        = 0;
  SL_RF_bootstrapIn          = NULL;
  if(VECTOR_ELT(sampleInfo, 0) != R_NilValue) {
    SL_RF_subjSize = INTEGER(VECTOR_ELT(sampleInfo, 0))[0];
    if(VECTOR_ELT(sampleInfo, 1) != R_NilValue) {
      SL_RF_subjWeight = REAL(VECTOR_ELT(sampleInfo, 1)); SL_RF_subjWeight--;
    }
    if(VECTOR_ELT(sampleInfo, 2) != R_NilValue) {  
      SL_RF_bootstrapSize        = INTEGER(VECTOR_ELT(sampleInfo, 2))[0];
      if(VECTOR_ELT(sampleInfo, 3) != R_NilValue) {
        SL_RF_bootstrapIn = (uint **) copy2DObject(VECTOR_ELT(sampleInfo, 3), NATIVE_TYPE_INTEGER, (SL_RF_opt & OPT_BOOT_TYP1) && (SL_RF_opt & OPT_BOOT_TYP2), SL_RF_ntree, SL_RF_subjSize);
      }
    }
    else {
      SL_RF_bootstrapSize        = SL_RF_subjSize;
    }
  }
  SL_RF_timeInterestSize = INTEGER(VECTOR_ELT(timeInterest, 0))[0];
  if (VECTOR_ELT(timeInterest, 1) != R_NilValue) {
    SL_RF_timeInterest         = (double *) REAL(VECTOR_ELT(timeInterest, 1));
    SL_RF_timeInterest --;
  }
  else {
    SL_RF_timeInterest = NULL;
  }
  SL_RF_totalNodeCount       = INTEGER(totalNodeCount)[0];
  SL_RF_tLeafCount           = (uint *) INTEGER(tLeafCount); SL_RF_tLeafCount --;
  SL_RF_seed_                = (int *) INTEGER(VECTOR_ELT(seedInfo, 0)); SL_RF_seed_ --;
  if (VECTOR_ELT(seedInfo, 1) != R_NilValue) {
    SL_RF_seedVimp_          = (int *) INTEGER(VECTOR_ELT(seedInfo, 1)); SL_RF_seedVimp_ --;
  }
  else {
    SL_RF_seedVimp_          = NULL;
  }
  SL_RF_optLoGrow            = (uint) INTEGER(VECTOR_ELT(seedInfo, 2))[0]; 
  SL_RF_treeID_              = (uint *) INTEGER(treeID);   SL_RF_treeID_ --;
  SL_RF_nodeID_              = (uint *) INTEGER(nodeID);   SL_RF_nodeID_ --;
  SL_RF_nodeSZ_              = (uint *) INTEGER(nodeSZ);   SL_RF_nodeSZ_ --;
  SL_RF_brnodeID_            = (uint *) INTEGER(brnodeID); SL_RF_brnodeID_ --;
  SL_RF_RMBR_ID_             = (uint *) INTEGER(tnRMBR);
  SL_RF_AMBR_ID_             = (uint *) INTEGER(tnAMBR);
  SL_RF_TN_RCNT_             = (uint *) INTEGER(tnRCNT);
  SL_RF_TN_ACNT_             = (uint *) INTEGER(tnACNT);
  SL_RF_perfBlock            = INTEGER(perfBlock)[0];
  SL_RF_quantileSize = INTEGER(VECTOR_ELT(quantile, 0))[0];
  if (VECTOR_ELT(quantile, 1) != R_NilValue) {
    SL_RF_quantile = (double *) REAL(VECTOR_ELT(quantile, 1));
    SL_RF_quantile --;
  }
  else {
    SL_RF_quantile = NULL;
  }
  SL_RF_qEpsilon = REAL(VECTOR_ELT(quantile, 2))[0];
  SL_RF_numThreads           = INTEGER(numThreads)[0];
  SL_RF_ptnCount             = INTEGER(ptnCount)[0];
  SL_RF_rTargetCount         = INTEGER(VECTOR_ELT(yTarget, 0))[0];
  if (VECTOR_ELT(yTarget, 1) != R_NilValue) {
    SL_RF_rTarget         = (uint *) INTEGER(VECTOR_ELT(yTarget, 1));
    SL_RF_rTarget --;
  }
  else {
    SL_RF_rTarget = NULL;
  }
  SL_RF_intrPredictorSize = INTEGER(VECTOR_ELT(intrPredictorInfo, 0))[0];
  if (VECTOR_ELT(intrPredictorInfo, 1) != R_NilValue) {
    SL_RF_intrPredictor = (uint *) INTEGER(VECTOR_ELT(intrPredictorInfo, 1));
    SL_RF_intrPredictor --;
  }
  else {
    SL_RF_intrPredictor = NULL;
  }
  if (xMarginalInfo != R_NilValue) {
    SL_RF_xMarginalSize     = INTEGER(VECTOR_ELT(xMarginalInfo, 0))[0];
    if (VECTOR_ELT(xMarginalInfo, 1) != R_NilValue) {
      SL_RF_xMarginal         = (uint *) INTEGER(VECTOR_ELT(xMarginalInfo, 1));
      SL_RF_xMarginal --;
    }
    else {
      SL_RF_xMarginal = NULL;
    }
  }
  else {
    SL_RF_xMarginalSize = 0;
    SL_RF_xMarginal = NULL;
  }
  SL_RF_partialType          = INTEGER(VECTOR_ELT(partial, 0))[0];
  SL_RF_partialXvar          = INTEGER(VECTOR_ELT(partial, 1))[0];
  SL_RF_partialLength        = INTEGER(VECTOR_ELT(partial, 2))[0];
  if (SL_RF_partialLength > 0) {
    SL_RF_partialValue         = REAL(VECTOR_ELT(partial, 3)); SL_RF_partialValue --;
  }
  else {
    SL_RF_partialValue = NULL;
  }
  SL_RF_partialLength2       = INTEGER(VECTOR_ELT(partial, 4))[0];
  if (SL_RF_partialLength2 > 0) {
    SL_RF_partialXvar2         = (uint *) INTEGER(VECTOR_ELT(partial, 5)); SL_RF_partialXvar2 --;
  }
  else {
    SL_RF_partialXvar2 = NULL;
  }
  if (SL_RF_partialLength2 > 0) {
    SL_RF_partialValue2        = REAL(VECTOR_ELT(partial, 6)); SL_RF_partialValue2 --;
  }
  else {
    SL_RF_partialValue2 = NULL;
  }
  SL_RF_fobservationSize     = INTEGER(fobservationSize)[0];
  SL_RF_frSize               = INTEGER(frSize)[0];
  SL_RF_fresponseIn          = (double **) copy2DObject(frData, NATIVE_TYPE_NUMERIC, SL_RF_frSize > 0, SL_RF_frSize, SL_RF_fobservationSize);
  SL_RF_fobservationIn       = (double **) copy2DObject(fxData, NATIVE_TYPE_NUMERIC, SL_RF_fobservationSize > 0, SL_RF_xSize, SL_RF_fobservationSize);
  if (unoWeight != R_NilValue) {  
    SL_RF_unoWeight            = REAL(unoWeight);  SL_RF_unoWeight--;
  }
  else {
    SL_RF_unoWeight = NULL;
  }
  SL_RF_getTree = (uint *) INTEGER(getTree);  SL_RF_getTree --;
  SL_RF_TN_SURV_ = REAL(tnSURV);
  SL_RF_TN_MORT_ = REAL(tnMORT);
  SL_RF_TN_NLSN_ = REAL(tnNLSN);
  SL_RF_TN_CSHZ_ = REAL(tnCSHZ);
  SL_RF_TN_CIFN_ = REAL(tnCIFN);
  SL_RF_TN_REGR_ = REAL(tnREGR);  
  SL_RF_TN_CLAS_ = (uint *) INTEGER(tnCLAS);
  processDefaultPredict();
  mode = (SL_RF_fobservationSize > 0)? SL_RF_PRED : SL_RF_REST;  
  stackForestObjectsAuxOnly(mode);
  SL_RF_parmID_[1]              = (int *) INTEGER(VECTOR_ELT(hc_zero, 0));   SL_RF_parmID_[1]  --;
  SL_RF_contPT_[1]              =             REAL(VECTOR_ELT(hc_zero, 1));  SL_RF_contPT_[1]  --;
  SL_RF_mwcpSZ_[1]              = (uint *) INTEGER(VECTOR_ELT(hc_zero, 2));  SL_RF_mwcpSZ_[1]  --;
  SL_RF_fsrecID_[1]             = (uint *) INTEGER(VECTOR_ELT(hc_zero, 3));  SL_RF_fsrecID_[1] --;
  if (VECTOR_ELT(hc_zero, 4) != R_NilValue) {
    SL_RF_mwcpPT_[1]            = (uint *) INTEGER(VECTOR_ELT(hc_zero, 4));  SL_RF_mwcpPT_[1]  --;
  }
  else {
    SL_RF_mwcpPT_[1] = NULL;
  }
  rfsrc(mode, seedValue);
  unstackForestObjectsAuxOnly(mode);
  free_1DObject(SL_RF_rType, NATIVE_TYPE_CHARACTER, SL_RF_ySize);
  free_1DObject(SL_RF_xType, NATIVE_TYPE_CHARACTER, SL_RF_xSize);
  if (SL_RF_responseIn != NULL) free_2DObject(SL_RF_responseIn, NATIVE_TYPE_NUMERIC, SL_RF_ySize > 0, SL_RF_ySize, SL_RF_observationSize);
  if (SL_RF_observationIn != NULL) free_2DObject(SL_RF_observationIn, NATIVE_TYPE_NUMERIC, TRUE, SL_RF_xSize, SL_RF_observationSize);
  free_2DObject(SL_RF_bootstrapIn, NATIVE_TYPE_INTEGER, (SL_RF_opt & OPT_BOOT_TYP1) && (SL_RF_opt & OPT_BOOT_TYP2), SL_RF_ntree, SL_RF_subjSize);
  free_2DObject(SL_RF_fresponseIn, NATIVE_TYPE_NUMERIC, SL_RF_frSize > 0, SL_RF_frSize, SL_RF_fobservationSize);
  free_2DObject(SL_RF_fobservationIn, NATIVE_TYPE_NUMERIC, SL_RF_fobservationSize > 0 , SL_RF_xSize, SL_RF_fobservationSize);
  SL_RF_cpuTime_[1] = (double) (clock() - cpuTimeStart) / CLOCKS_PER_SEC;
  R_ReleaseObject(SL_RF_sexpVector[SL_RF_OUTP_ID]);
  R_ReleaseObject(SL_RF_sexpVector[SL_RF_STRG_ID]);
  return SL_RF_sexpVector[SL_RF_OUTP_ID];
}
