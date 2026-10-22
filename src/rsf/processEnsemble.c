/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "processEnsemble.h"
#include "rfsrcUtil.h"
#include "tree.h"
#include "treeUtil.h"
#include "marginal.h"
#include "importance.h"
#include "partial.h"
#include "factorOps.h"
#include "nrutil.h"
#include "nativeUtil.h"
#include "error.h"
void stackFactorInSitu(uint treeID) {
  uint j;
  if (SL_RF_rFactorCount + SL_RF_xFactorCount > 0) {
    SL_RF_factorList[treeID] = (Factor **) new_vvector(1, SL_RF_maxFactorLevel, NRUTIL_FPTR);
    for (j = 1; j <= SL_RF_maxFactorLevel; j++) {
      SL_RF_factorList[treeID][j] = NULL;
    }
    for (j = 1; j <= SL_RF_xFactorCount; j++) {
      if (SL_RF_factorList[treeID][SL_RF_xFactorSize[j]] == NULL) {
        SL_RF_factorList[treeID][SL_RF_xFactorSize[j]] = makeFactor(SL_RF_xFactorSize[j], FALSE);
      }
    }
    for (j = 1; j <= SL_RF_rFactorCount; j++) {
      if (SL_RF_factorList[treeID][SL_RF_rFactorSize[j]] == NULL) {
        SL_RF_factorList[treeID][SL_RF_rFactorSize[j]] = makeFactor(SL_RF_rFactorSize[j], FALSE);
      }
    }
  }
}
void unstackFactorInSitu(uint treeID) {
  uint j;
  if (SL_RF_rFactorCount + SL_RF_xFactorCount > 0) {
    if (SL_RF_factorList[treeID] != NULL) {
      for (j = 1; j <= SL_RF_maxFactorLevel; j++) {
        if (SL_RF_factorList[treeID][j] != NULL) {
          freeFactor(SL_RF_factorList[treeID][j]);
        }
      }
      free_new_vvector(SL_RF_factorList[treeID], 1, SL_RF_maxFactorLevel, NRUTIL_FPTR);
      SL_RF_factorList[treeID] = NULL;
    }
  }
}
void processEnsembleInSitu(char mode, char multImpFlag, uint b) {
  char perfFlag;
  double **responsePtr;
  char     rImputeFlag;
  uint lo, hi;
    if ((SL_RF_opt & OPT_PERF) ||
        (SL_RF_opt & OPT_OENS) ||
        (SL_RF_opt & OPT_FENS)) {
#ifdef _OPENMP
      omp_set_lock(&SL_RF_lockPerf);
#endif
      SL_RF_serialTreeIndex[++SL_RF_serialTreeID] = b;
      perfFlag = getPerfFlag(mode, SL_RF_serialTreeID);
      if (!perfFlag) {
#ifdef _OPENMP
        omp_unset_lock(&SL_RF_lockPerf);
#endif
      }
#ifdef _OPENMP
      omp_set_lock(&SL_RF_lockEnsbUpdtCount);
#endif
      SL_RF_ensbUpdtCount++;
#ifdef _OPENMP
      omp_unset_lock(&SL_RF_lockEnsbUpdtCount);
#endif
      updateEnsemble(mode, b);
      if (SL_RF_opt & OPT_VIMP) {
        if (SL_RF_opt & OPT_VIMP_JOIN) {
          stackVimpMembership(mode, & SL_RF_vimpMembership[1][b]);
          getVimpMembership(mode, b, SL_RF_vimpMembership[1][b], 0);
          updateEnsembleVimp(mode, b, SL_RF_vimpMembership[1][b], 1);
          unstackVimpMembership(mode, SL_RF_vimpMembership[1][b]);
        }
        else {
          for (uint p = 1; p <= SL_RF_intrPredictorSize; p++) {
            uint pp = SL_RF_intrPredictor[p];            
            stackVimpMembership(mode, & SL_RF_vimpMembership[p][b]);
            getVimpMembership(mode, b, SL_RF_vimpMembership[p][b], pp);
            updateEnsembleVimp(mode, b, SL_RF_vimpMembership[p][b], p);
            unstackVimpMembership(mode, SL_RF_vimpMembership[p][b]);
          }
        }
      }  
#ifdef _OPENMP
      omp_set_lock(&SL_RF_lockEnsbUpdtCount);
#endif
      SL_RF_ensbUpdtCount--;
#ifdef _OPENMP
      omp_unset_lock(&SL_RF_lockEnsbUpdtCount);
#endif
      if (perfFlag) {
        char ensbUpdtCountFlag = FALSE;
        while (!ensbUpdtCountFlag) {
#ifdef _OPENMP
          omp_set_lock(&SL_RF_lockEnsbUpdtCount);
#endif
          if (SL_RF_ensbUpdtCount == 0) {
            ensbUpdtCountFlag = TRUE;
          }
#ifdef _OPENMP
          omp_unset_lock(&SL_RF_lockEnsbUpdtCount);
#endif
        }
        normalizeEnsembleEstimates(mode, FALSE);
        rImputeFlag = stackAndImputePerfResponse(mode,
                                                 multImpFlag,
                                                 b,
                                                 1,
                                                 SL_RF_serialTreeID,
                                                 SL_RF_serialTreeIndex,
                                                 &responsePtr);
        summarizeFaithfulBlockPerformance(mode,
                                          b,
                                          SL_RF_serialTreeID,
                                          (mode == SL_RF_PRED) ? SL_RF_fullEnsembleMRTptr : SL_RF_oobEnsembleMRTptr,
                                          (mode == SL_RF_PRED) ? SL_RF_fullEnsembleCLSptr : SL_RF_oobEnsembleCLSptr,
                                          (mode == SL_RF_PRED) ? SL_RF_fullEnsembleRGRptr : SL_RF_oobEnsembleRGRptr,
                                          (mode == SL_RF_PRED) ? SL_RF_fullEnsembleDen    : SL_RF_oobEnsembleDen,
                                          responsePtr,
                                          SL_RF_perfMRTptr,
                                          SL_RF_perfCLSptr,
                                          SL_RF_perfRGRptr);
        unstackPerfResponse(mode, rImputeFlag, responsePtr);
        if (SL_RF_opt & OPT_VIMP) {
          SL_RF_serialBlockID ++;
          normalizeBlockedEnsembleEstimates(mode,
                                            SL_RF_blkEnsembleMRTnum,
                                            SL_RF_blkEnsembleCLSnum,
                                            SL_RF_blkEnsembleRGRnum,
                                            SL_RF_blkEnsembleDen);
          lo = ((SL_RF_serialBlockID - 1) * SL_RF_perfBlock) + 1;
          hi = SL_RF_serialBlockID * SL_RF_perfBlock;
          if (hi <= SL_RF_ntree) {
            rImputeFlag = stackAndImputePerfResponse(mode, multImpFlag, b, lo, hi, SL_RF_serialTreeIndex, &responsePtr);
            summarizeFaithfulBlockPerformance(mode,
                                              b,
                                              SL_RF_serialBlockID,
                                              SL_RF_blkEnsembleMRTnum,
                                              SL_RF_blkEnsembleCLSnum,
                                              SL_RF_blkEnsembleRGRnum,
                                              SL_RF_blkEnsembleDen,
                                              responsePtr,
                                              SL_RF_perfMRTblk,
                                              SL_RF_perfCLSblk,
                                              SL_RF_perfRGRblk);
            if (SL_RF_opt & OPT_VIMP_JOIN) {
              summarizePerturbedPerformance(mode, b, SL_RF_serialBlockID, 1, responsePtr);
            }
            else {
              for (uint p = 1; p <= SL_RF_intrPredictorSize; p++) {
                summarizePerturbedPerformance(mode, b, SL_RF_serialBlockID, p, responsePtr);
              }
            }
            unstackPerfResponse(mode, rImputeFlag, responsePtr);     
            resetBlockedEnsembleEstimates(mode);
          }
        }
#ifdef _OPENMP
        omp_unset_lock(&SL_RF_lockPerf);
#endif
      }  
    }  
    if (SL_RF_opt & (OPT_SPLDPTH_1 | OPT_SPLDPTH_2)) {
      updateSplitDepth(b, SL_RF_root[b], SL_RF_maxDepth[b]);
    }
    if (SL_RF_opt & OPT_CASE_DPTH) {
      updateCaseDepth(mode, b);
    }
    if (SL_RF_opt & (OPT_VARUSED_F | OPT_VARUSED_T)) {
      getVariablesUsed(b, SL_RF_root[b], SL_RF_varUsedPtr[b]);
    }
    if (SL_RF_optHigh & OPT_PART_PLOT) {
      getAndUpdatePartialMembership(b, SL_RF_root[b]);
    }
    if (SL_RF_xMarginalSize > 0) {
      getMarginalMembership(mode, b);
    }
    if (SL_RF_optHigh & OPT_WGHT) {
      updateWeight(mode, b);
    }
    if (SL_RF_optHigh & OPT_DIST) {
      if ((mode == SL_RF_PRED) && (SL_RF_optHigh & OPT_DIST_IBG) && (SL_RF_optHigh & OPT_DIST_OOB)) {
        updateDistanceHybrid(mode, b);
      }
      else {
        updateDistance(mode, b);
      }
    }
    if (SL_RF_opt & OPT_PROX) {
      if ((mode == SL_RF_PRED) && (SL_RF_opt & OPT_PROX_IBG) && (SL_RF_opt & OPT_PROX_OOB)) {
        updateProximityHybrid(mode, b);
      }
      else {
        updateProximity(mode, b);
      }
    }
    if (SL_RF_xMarginalSize > 0) {
      releaseMarginalMembership(mode, b);
    }
}
void processEnsemblePost(char mode) {
  char perfFlag;
  double **responsePtr;
  uint lo, hi;
  uint bb;
  if ((SL_RF_opt & OPT_PERF) ||
      (SL_RF_opt & OPT_OENS) ||
      (SL_RF_opt & OPT_FENS)) {
    if (getUserTraceFlag()) {
      SL_RF_userTimeStart = time(NULL);
      SL_RF_userTimeSplit = SL_RF_userTreeID = 0;
      SL_RF_nativePrint("\n");
    }
    for (bb = 1; bb <= SL_RF_getTreeCount; bb++) {
      uint b = SL_RF_getTreeIndex[bb]; 
      if (SL_RF_tLeafCount[b] > 0) {
        SL_RF_serialTreeIndex[++SL_RF_serialTreeID] = b;
        perfFlag = getPerfFlag(mode, SL_RF_serialTreeID);
        updateEnsemble(mode, b);
        if (SL_RF_opt & OPT_VIMP) {
          if (SL_RF_opt & OPT_VIMP_JOIN) {
            stackVimpMembership(mode, & SL_RF_vimpMembership[1][b]);              
            getVimpMembership(mode, b, SL_RF_vimpMembership[1][b], 0);
            updateEnsembleVimp(mode, b, SL_RF_vimpMembership[1][b], 1);
            unstackVimpMembership(mode, SL_RF_vimpMembership[1][b]);
          }
          else {
#ifdef _OPENMP
#pragma omp parallel for num_threads(SL_RF_numThreads)
#endif
            for (uint p = 1; p <= SL_RF_intrPredictorSize; p++) {
              uint pp = SL_RF_intrPredictor[p];
              stackVimpMembership(mode, & SL_RF_vimpMembership[p][b]);
              getVimpMembership(mode, b, SL_RF_vimpMembership[p][b], pp);
              updateEnsembleVimp(mode, b, SL_RF_vimpMembership[p][b], p);
              unstackVimpMembership(mode, SL_RF_vimpMembership[p][b]);
            }
          }
        }  
        if (perfFlag) {
          normalizeEnsembleEstimates(mode, FALSE);
          stackAndImputePerfResponse(mode,
                                     FALSE, 
                                     b,
                                     1,
                                     SL_RF_serialTreeID,
                                     SL_RF_serialTreeIndex,
                                     &responsePtr);
          summarizeFaithfulBlockPerformance(mode,
                                            b,
                                            SL_RF_serialTreeID,
                                            (mode == SL_RF_PRED) ? SL_RF_fullEnsembleMRTptr : SL_RF_oobEnsembleMRTptr,
                                            (mode == SL_RF_PRED) ? SL_RF_fullEnsembleCLSptr : SL_RF_oobEnsembleCLSptr,
                                            (mode == SL_RF_PRED) ? SL_RF_fullEnsembleRGRptr : SL_RF_oobEnsembleRGRptr,
                                            (mode == SL_RF_PRED) ? SL_RF_fullEnsembleDen    : SL_RF_oobEnsembleDen,
                                            responsePtr,
                                            SL_RF_perfMRTptr,
                                            SL_RF_perfCLSptr,
                                            SL_RF_perfRGRptr);
          unstackPerfResponse(mode, FALSE, responsePtr);
          if (SL_RF_opt & OPT_VIMP) {
            SL_RF_serialBlockID ++;
            normalizeBlockedEnsembleEstimates(mode,
                                              SL_RF_blkEnsembleMRTnum,
                                              SL_RF_blkEnsembleCLSnum,
                                              SL_RF_blkEnsembleRGRnum,
                                              SL_RF_blkEnsembleDen);
            lo = ((SL_RF_serialBlockID - 1) * SL_RF_perfBlock) + 1;
            hi = SL_RF_serialBlockID * SL_RF_perfBlock;
            if (hi <= SL_RF_ntree) {
              stackAndImputePerfResponse(mode,
                                         FALSE,
                                         b,
                                         lo,
                                         hi,
                                         SL_RF_serialTreeIndex,
                                         &responsePtr);
              summarizeFaithfulBlockPerformance(mode,
                                                b,
                                                SL_RF_serialBlockID,
                                                SL_RF_blkEnsembleMRTnum,
                                                SL_RF_blkEnsembleCLSnum,
                                                SL_RF_blkEnsembleRGRnum,
                                                SL_RF_blkEnsembleDen,
                                                responsePtr,
                                                SL_RF_perfMRTblk,
                                                SL_RF_perfCLSblk,
                                                SL_RF_perfRGRblk);
              if (SL_RF_opt & OPT_VIMP_JOIN) {
                summarizePerturbedPerformance(mode, b, SL_RF_serialBlockID, 1, responsePtr);
              }
              else {
#ifdef _OPENMP
#pragma omp parallel for num_threads(SL_RF_numThreads)
#endif
                for (uint p = 1; p <= SL_RF_intrPredictorSize; p++) {
                  summarizePerturbedPerformance(mode, b, SL_RF_serialBlockID, p, responsePtr);
                }
              }
              unstackPerfResponse(mode, FALSE, responsePtr);     
              resetBlockedEnsembleEstimates(mode);
            }  
          }  
        }  
      }  
      if (getUserTraceFlag()) {
#ifdef _OPENMP
#pragma omp critical (_update_timer)
#endif
        { 
          double userTimeElapsedFromStart;
          double userTimeElapsedFromSplit;
          double userTimeRemaining;
          time_t current;
          SL_RF_userTreeID++;
          current = time(NULL);
          userTimeElapsedFromSplit = (double) (current - SL_RF_userTimeSplit);
          if ((userTimeElapsedFromSplit) > (double) getUserTraceFlag()) {
            userTimeElapsedFromStart = (double) (current - SL_RF_userTimeStart);
            userTimeRemaining = (userTimeElapsedFromStart / SL_RF_userTreeID * SL_RF_ntree) - userTimeElapsedFromStart;
            SL_RF_nativePrint("Ensembles Processed:  %6d,    Time Remaining (sec):  %6.0f \n", SL_RF_userTreeID, ceil(userTimeRemaining));
            SL_RF_userTimeSplit = current;
          }
        }  
      }
    }  
  }  
  if (SL_RF_opt & (OPT_SPLDPTH_1 | OPT_SPLDPTH_2)) {
#ifdef _OPENMP
#pragma omp parallel for num_threads(SL_RF_numThreads)
#endif
    for (bb = 1; bb <= SL_RF_getTreeCount; bb++) {
      uint b = SL_RF_getTreeIndex[bb];
      if (SL_RF_tLeafCount[b] > 0) {
        updateSplitDepth(b, SL_RF_root[b], SL_RF_maxDepth[b]);
      }
    }
  }
  if (SL_RF_opt & OPT_CASE_DPTH) {
#ifdef _OPENMP
#pragma omp parallel for num_threads(SL_RF_numThreads)
#endif
    for (bb = 1; bb <= SL_RF_getTreeCount; bb++) {
      uint b = SL_RF_getTreeIndex[bb];
      if (SL_RF_tLeafCount[b] > 0) {
        updateCaseDepth(mode, b);
      }
    }
  }
  if (SL_RF_opt & (OPT_VARUSED_F | OPT_VARUSED_T)) {
#ifdef _OPENMP
#pragma omp parallel for num_threads(SL_RF_numThreads)
#endif
    for (bb = 1; bb <= SL_RF_getTreeCount; bb++) {
      uint b = SL_RF_getTreeIndex[bb];      
      if (SL_RF_tLeafCount[b] > 0) {
        getVariablesUsed(b, SL_RF_root[b], SL_RF_varUsedPtr[b]);
      }
    }
  }
  if (SL_RF_optHigh & OPT_PART_PLOT) {
#ifdef _OPENMP
#pragma omp parallel for num_threads(SL_RF_numThreads)
#endif
    for (bb = 1; bb <= SL_RF_getTreeCount; bb++) {
      uint b = SL_RF_getTreeIndex[bb];
      if (SL_RF_tLeafCount[b] > 0) {
        getAndUpdatePartialMembership(b, SL_RF_root[b]);
      }
    }
  }
  for (bb = 1; bb <= SL_RF_getTreeCount; bb++) {
    uint b = SL_RF_getTreeIndex[bb];
    if (SL_RF_tLeafCount[b] > 0) {
      if (SL_RF_xMarginalSize > 0) {
        getMarginalMembership(mode, b);
      }
      if (SL_RF_optHigh & OPT_WGHT) {
        updateWeight(mode, b);
      }
      if (SL_RF_optHigh & OPT_DIST) {
        if ((mode == SL_RF_PRED) && (SL_RF_optHigh & OPT_DIST_IBG) && (SL_RF_optHigh & OPT_DIST_OOB)) {
          updateDistanceHybrid(mode, b);
        }
        else {
          updateDistance(mode, b);
        }
      }
      if (SL_RF_opt & OPT_PROX) {
        if ((mode == SL_RF_PRED) && (SL_RF_opt & OPT_PROX_IBG) && (SL_RF_opt & OPT_PROX_OOB)) {
          updateProximityHybrid(mode, b);
        }
        else {
          updateProximity(mode, b);
        }
      }
      if (SL_RF_xMarginalSize > 0) {
        releaseMarginalMembership(mode, b);
      }
    }  
  }  
}
void processEnsembleHoldout(uint xVarIdx, uint b) {
  Terminal *terminalNode;
  double   *denomPtr;  
  uint  *membershipIndex;
  uint   membershipSize;
  uint   obsSize;
  uint   blockID;
  uint i, j, k, m, ii;
  obsSize = SL_RF_observationSize;
    blockID = SL_RF_holdoutMap[xVarIdx][b];
    if (blockID > 0) {
      if (blockID <= SL_RF_holdBLKptr[xVarIdx]) {
#ifdef _OPENMP
        omp_set_lock(&(SL_RF_lockVimpHoldout[xVarIdx][blockID]));
#endif
        if (SL_RF_holdEnsembleDen[xVarIdx][blockID] == NULL) {
          SL_RF_holdEnsembleDen[xVarIdx][blockID] = dvector(1, obsSize);
          for (m = 1; m <= obsSize; m++) {
            SL_RF_holdEnsembleDen[xVarIdx][blockID][m] = 0;
          }
        }
        if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
          {
            if (SL_RF_holdMRTstd[xVarIdx][blockID] == NULL) {
              SL_RF_holdMRTstd[xVarIdx][blockID] = (double **) new_vvector(1, SL_RF_eventTypeSize, NRUTIL_DPTR);
              for (k = 1; k <= SL_RF_eventTypeSize; k++) {
                SL_RF_holdMRTstd[xVarIdx][blockID][k] = dvector(1, obsSize);
                for (m = 1; m <= obsSize; m++) {
                  SL_RF_holdMRTstd[xVarIdx][blockID][k][m] = 0.0;
                }
              }
            }
          }
        }
        else {
          if (SL_RF_rTargetFactorCount > 0) {
            if (SL_RF_holdCLSstd[xVarIdx][blockID] == NULL) {
              SL_RF_holdCLSstd[xVarIdx][blockID] = (double ***) new_vvector(1, SL_RF_rTargetFactorCount, NRUTIL_DPTR2);
              for (j = 1; j <= SL_RF_rTargetFactorCount; j++) {
                SL_RF_holdCLSstd[xVarIdx][blockID][j]  = (double **) new_vvector(1, SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[j]]], NRUTIL_DPTR);
                for (k = 1; k <= SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[j]]]; k++) {
                  SL_RF_holdCLSstd[xVarIdx][blockID][j][k] = dvector(1, obsSize);
                  for (m = 1; m <= obsSize; m++) {
                    SL_RF_holdCLSstd[xVarIdx][blockID][j][k][m] = 0.0;
                  }
                }
              }
            }
          }
          if (SL_RF_rTargetNonFactorCount > 0) {
            if (SL_RF_holdRGRstd[xVarIdx][blockID] == NULL) {
              SL_RF_holdRGRstd[xVarIdx][blockID] = (double **) new_vvector(1, SL_RF_rTargetNonFactorCount, NRUTIL_DPTR);
              for (j = 1; j <= SL_RF_rTargetNonFactorCount; j++) {
                SL_RF_holdRGRstd[xVarIdx][blockID][j] = dvector(1, obsSize);
                for (m = 1; m <= obsSize; m++) {
                  SL_RF_holdRGRstd[xVarIdx][blockID][j][m] = 0.0;
                }
              }
            }
          }
        }
        membershipSize  = SL_RF_oobSize[b];
        membershipIndex = SL_RF_oobMembershipIndex[b];
        denomPtr = SL_RF_holdEnsembleDen[xVarIdx][blockID];
        for (i = 1; i <= membershipSize; i++) {
          ii = membershipIndex[i];
          terminalNode = SL_RF_tTermMembership[b][ii];
          if ((terminalNode -> membrCount) > 0) {
            denomPtr[ii] ++;
            if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
              {
                for (k = 1; k <= SL_RF_eventTypeSize; k++) {
                  SL_RF_holdMRTstd[xVarIdx][blockID][k][ii] += terminalNode -> mortality[k];
                }
              }
            }
            else {
              if (SL_RF_rTargetFactorCount > 0) {
                for (j = 1; j <= SL_RF_rTargetFactorCount; j++) {
                  for (k = 1; k <= SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[j]]]; k++) {
                    SL_RF_holdCLSstd[xVarIdx][blockID][j][k][ii] += (double) (terminalNode -> multiClassProb)[SL_RF_rFactorMap[SL_RF_rTargetFactor[j]]][k] / (double) (terminalNode -> membrCount);
                  }
                }
              }
              if (SL_RF_rTargetNonFactorCount > 0) {
                for (j=1; j <= SL_RF_rTargetNonFactorCount; j++) {
                  SL_RF_holdRGRstd[xVarIdx][blockID][j][ii] += (terminalNode -> meanResponse)[SL_RF_rNonFactorMap[SL_RF_rTargetNonFactor[j]]];
                }
              }
            }
          }
          else {
              SL_RF_nativePrint("\nRF-SRC:  *** ERROR *** ");
              SL_RF_nativePrint("\nRF-SRC:  NA encountered for VIMP outcome in terminal node:  %10d", terminalNode -> nodeID);
              SL_RF_nativePrint("\nRF-SRC:  Please Contact Technical Support.");
              SL_RF_nativeExit();
          }
        }  
        SL_RF_runningHoldoutCount[xVarIdx][blockID] ++;
        if (SL_RF_runningHoldoutCount[xVarIdx][blockID] == SL_RF_vtryBlockSize) {
          double **responsePtr;
          char     rImputeFlag;
          rImputeFlag = stackAndImputePerfResponse(SL_RF_GROW,
                                                   FALSE, 
                                                   b,
                                                   1,
                                                   SL_RF_vtryBlockSize,
                                                   SL_RF_blockSerialTreeIndex[xVarIdx][blockID],
                                                   &responsePtr);
          if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
            {
              normalizeBlockedEnsembleEstimates(SL_RF_GROW,
                                                SL_RF_holdMRTstd[xVarIdx][blockID],
                                                NULL,
                                                NULL,
                                                SL_RF_holdEnsembleDen[xVarIdx][blockID]);
              summarizeHoldoutBlockPerformance(SL_RF_GROW,
                                               b,
                                               xVarIdx,
                                               blockID,
                                               responsePtr,
                                               SL_RF_holdMRTstd[xVarIdx][blockID],
                                               NULL,
                                               NULL,
                                               SL_RF_holdEnsembleDen[xVarIdx][blockID],
                                               SL_RF_holdMRTptr[xVarIdx][blockID],
                                               NULL,
                                               NULL);
              if (SL_RF_holdMRTstd[xVarIdx][blockID] != NULL) {
                for (k = 1; k <= SL_RF_eventTypeSize; k++) {
                  free_dvector(SL_RF_holdMRTstd[xVarIdx][blockID][k], 1, obsSize);
                }
                free_new_vvector(SL_RF_holdMRTstd[xVarIdx][blockID], 1, SL_RF_eventTypeSize, NRUTIL_DPTR);
                SL_RF_holdMRTstd[xVarIdx][blockID] = NULL;
              }
            }
          }
          else {
            if (SL_RF_rTargetFactorCount > 0) {
              normalizeBlockedEnsembleEstimates(SL_RF_GROW,
                                                NULL,
                                                SL_RF_holdCLSstd[xVarIdx][blockID],
                                                NULL,
                                                SL_RF_holdEnsembleDen[xVarIdx][blockID]);
              summarizeHoldoutBlockPerformance(SL_RF_GROW,
                                               b,
                                               xVarIdx,
                                               blockID,
                                               responsePtr,
                                               NULL,
                                               SL_RF_holdCLSstd[xVarIdx][blockID],
                                               NULL,
                                               SL_RF_holdEnsembleDen[xVarIdx][blockID],
                                               NULL,
                                               SL_RF_holdCLSptr[xVarIdx][blockID],
                                               NULL);
              if (SL_RF_holdCLSstd[xVarIdx][blockID] != NULL) {
                for (j = 1; j <= SL_RF_rTargetFactorCount; j++) {
                  for (k = 1; k <= SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[j]]]; k++) {
                    free_dvector(SL_RF_holdCLSstd[xVarIdx][blockID][j][k], 1, obsSize);
                  }
                  free_new_vvector(SL_RF_holdCLSstd[xVarIdx][blockID][j], 1, SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[j]]], NRUTIL_DPTR);
                }
                free_new_vvector(SL_RF_holdCLSstd[xVarIdx][blockID], 1, SL_RF_rTargetFactorCount, NRUTIL_DPTR2);
                SL_RF_holdCLSstd[xVarIdx][blockID] = NULL;
              }
            }
            if (SL_RF_rTargetNonFactorCount > 0) {
              normalizeBlockedEnsembleEstimates(SL_RF_GROW,
                                                NULL,
                                                NULL,
                                                SL_RF_holdRGRstd[xVarIdx][blockID],
                                                SL_RF_holdEnsembleDen[xVarIdx][blockID]);
              summarizeHoldoutBlockPerformance(SL_RF_GROW,
                                               b,
                                               xVarIdx,
                                               blockID,
                                               responsePtr,
                                               NULL,
                                               NULL,
                                               SL_RF_holdRGRstd[xVarIdx][blockID],
                                               SL_RF_holdEnsembleDen[xVarIdx][blockID],
                                               NULL,
                                               NULL,
                                               SL_RF_holdRGRptr[xVarIdx][blockID]);
              if (SL_RF_holdRGRstd[xVarIdx][blockID] != NULL) {
                for (j = 1; j <= SL_RF_rTargetNonFactorCount; j++) {
                  free_dvector(SL_RF_holdRGRstd[xVarIdx][blockID][j], 1, obsSize);
                }
                free_new_vvector(SL_RF_holdRGRstd[xVarIdx][blockID], 1, SL_RF_rTargetNonFactorCount, NRUTIL_DPTR);
                SL_RF_holdRGRstd[xVarIdx][blockID] = NULL;
              }
            }
          }
          if (SL_RF_holdEnsembleDen[xVarIdx][blockID] != NULL) {
            free_dvector(SL_RF_holdEnsembleDen[xVarIdx][blockID], 1, obsSize);
            SL_RF_holdEnsembleDen[xVarIdx][blockID] = NULL;
          }
          unstackPerfResponse(SL_RF_GROW, rImputeFlag, responsePtr);                
        } 
#ifdef _OPENMP
        omp_unset_lock(&(SL_RF_lockVimpHoldout[xVarIdx][blockID]));
#endif
      }  
    }  
}
void processEnsembleHoldoutPost(uint bb) {
  if (SL_RF_tLeafCount[SL_RF_getTreeIndex[bb]] > 0) {
    for (uint p = 1; p <= SL_RF_xSize; p++) {
      processEnsembleHoldout(p, SL_RF_getTreeIndex[bb]);
    }
  }
}
