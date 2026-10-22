/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "rfsrc.h"
#include "stackPreDefined.h"
#include "splitUtil.h"
#include "splitGreedy.h"
#include "splitMult.h"
#include "stackOutput.h"
#include "stackParallel.h"
#include "tree.h"
#include "rfsrcUtil.h"
#include "treeUtil.h"
#include "treeJIT.h"
#include "impute.h"
#include "sexpOutgoing.h"
#include "importance.h"
#include "importanceRand.h"
#include "importanceAnti.h"
#include "importancePerm.h"
#include "partial.h"
#include "splitRegr.h"
#include "splitClas.h"
#include "stack.h"
#include "stackOutputQQ.h"
#include "classification.h"
#include "nodeOps.h"
#include "random.h"
#include "nrutil.h"
#include "nativeUtil.h"
#include "error.h"
void rfsrc(char mode, int seedValue) {
  uint   adj;
  ulong *mwcpOffset;
  uint previousTreeID;
  uint r;
  uint b, p;
  uint seedValueLC;
  seedValueLC    = 0; 
  if (seedValue >= 0) {
    SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
    SL_RF_nativeError("\nRF-SRC:  Random seed must be less than zero.  \n");
    SL_RF_nativeExit();
  }
  if (SL_RF_nImpute < 1) {
    SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
    SL_RF_nativeError("\nRF-SRC:  Number imputations must be greater than zero:  %10d \n", SL_RF_ntree);
    SL_RF_nativeExit();
  }
  if (SL_RF_ntree < 1) {
    SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
    SL_RF_nativeError("\nRF-SRC:  Number of bootstrap iterations must be greater than zero:  %10d \n", SL_RF_ntree);
    SL_RF_nativeExit();
  }
  if (SL_RF_observationSize < 1) {
    SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
    SL_RF_nativeError("\nRF-SRC:  Number of individuals must be greater than one:  %10d \n", SL_RF_observationSize);
    SL_RF_nativeExit();
  }
  if (SL_RF_xSize < 1) {
    SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
    SL_RF_nativeError("\nRF-SRC:  Number of parameters must be greater than zero:  %10d \n", SL_RF_xSize);
    SL_RF_nativeExit();
  }
  if ((SL_RF_perfBlock < 1) || (SL_RF_perfBlock > SL_RF_ntree)) {
    SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
    SL_RF_nativeError("\nRF-SRC:  Invalid value specified for error block count:  %10d \n", SL_RF_perfBlock);
    SL_RF_nativeExit();
  }
#ifdef _OPENMP
  if (SL_RF_numThreads < 0) {
    SL_RF_numThreads = omp_get_max_threads();
  }
  else if (SL_RF_numThreads == 0) {
    SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
    SL_RF_nativeError("\nRF-SRC:  Number of threads must not be zero:  %10d \n", SL_RF_numThreads);
    SL_RF_nativeExit();
  }
  else {
    SL_RF_numThreads = (SL_RF_numThreads < omp_get_max_threads()) ? (SL_RF_numThreads) : (omp_get_max_threads());
  }
#endif
  stackIncomingArrays(mode);
  stackPreDefinedCommonArrays(mode,
                              &SL_RF_nodeMembership,  
                              &SL_RF_tTermMembership,
                              &SL_RF_tTermList,
                              &SL_RF_root);
  switch (mode) {
  case SL_RF_PRED:
    stackPreDefinedPredictArrays();
    break;
  case SL_RF_REST:
    stackPreDefinedRestoreArrays();
    break;
  default:
    stackPreDefinedGrowthArrays();
    break;
  }
  if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
    stackAndInitializeTimeAndSubjectArrays(mode);
  }
  stackFactorArrays(mode);
  initializeFactorArrays(mode);
  stackMissingArraysPhase1(mode);
  if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
      stackCompetingArrays(mode);
  }
  if (SL_RF_rFactorCount > 0) {
    stackClassificationArrays(mode);
  }
  SL_RF_perfBlockCount = (uint) floor(((double) SL_RF_ntree) / SL_RF_perfBlock);
  freeNode = & freeNodeGeneric;
  switch (mode) {
  case SL_RF_GROW:
    stackForestObjectsPtrOnly(mode);
    getPreSplitResult = & getPreSplitResultGeneric;
    stackRandomCovariates   = & stackRandomCovariatesGeneric;
    unstackRandomCovariates = & unstackRandomCovariatesGeneric;
    selectRandomCovariates  = & selectRandomCovariatesGeneric;
    stackRandomResponses   = & stackRandomResponsesGeneric;
    unstackRandomResponses = & unstackRandomResponsesGeneric;
    selectRandomResponses  = & selectRandomResponsesGenericVector;
    virtuallySplitNode = & virtuallySplitNodeGeneric;
    updateMaximumSplit = & updateMaximumSplitGeneric;
    forkAndUpdate = & forkAndUpdateGeneric;
    freeNode = & freeNodeGeneric;
    unstackSplitVector = & unstackSplitVectorGeneric;
    growTree = & growTreeRecursive;
    regressionXwghtSplit = & regressionXwghtSplitCur;
    classificationXwghtSplit = & classificationXwghtSplitCur;
    multivariateSplit        = & multivariateSplitNew;
    unsupervisedSplit        = & unsupervisedSplitMiss;
    randomSplit = & randomSplitGeneric;
    if (SL_RF_mRecordSize == 0) {
      getPreSplitResult = & getPreSplitResultNoMiss;
      multivariateSplit        = & multivariateSplitNew3;
      unsupervisedSplit        = & unsupervisedSplitNew;
    }
    if (SL_RF_yWeightType == SL_RF_WGHT_UNIFORM){
      stackRandomResponses   = & stackRandomResponsesSimple;
      unstackRandomResponses = & unstackRandomResponsesSimple;
      selectRandomResponses  = & selectRandomResponsesSimpleVector;
    }
    SL_RF_optHigh = SL_RF_optHigh & (~OPT_JIT_TOP);
    SL_RF_opt = SL_RF_opt & (~OPT_ANON);
    acquireTree = &acquireTreeGeneric;
    antiMembership = &antiMembershipGeneric;
    randomMembership = &randomMembershipGeneric;
    getMembership = &getMembershipGeneric;
    partialMembership = &partialMembershipGeneric;
    SL_RF_splitRuleObj = makeSplitRuleObj(SL_RF_splitRule);
    break;
  default:
    if ((mode == SL_RF_PRED) && (SL_RF_optHigh & OPT_DIST) && (SL_RF_optHigh & OPT_DIST_IBG) && (SL_RF_optHigh & OPT_DIST_OOB)) {
      SL_RF_optHigh = SL_RF_optHigh & (~OPT_JIT_TOP);
    }
    acquireTree = &acquireTreeGeneric;
    antiMembership = &antiMembershipGeneric;
    randomMembership = &randomMembershipGeneric;
    getMembership = &getMembershipGeneric;
    partialMembership = &partialMembershipGeneric;      
    if (SL_RF_opt & OPT_ANON) {
      if (mode == SL_RF_REST) {
        SL_RF_optHigh = SL_RF_optHigh | (~OPT_JIT_TOP);
      }
    }
    if (SL_RF_optHigh & OPT_JIT_TOP) {
      if ((SL_RF_mRecordSize == 0) &&
          (SL_RF_optHigh & OPT_MEMB_INCG) &&
          (SL_RF_optHigh & OPT_TERM_INCG) &&
          !(SL_RF_optHigh & OPT_MEMB_PRUN) &&
          !(SL_RF_optHigh & OPT_PART_PLOT) &&
          !(SL_RF_xMarginalSize > 0)) {
        acquireTree = &acquireTreeJIT;
        antiMembership = &antiMembershipJIT;
        randomMembership = &randomMembershipJIT;
        getMembership = &getMembershipJIT;
        partialMembership = &partialMembershipJIT;
        if (SL_RF_fmResponseFlag) {
          SL_RF_opt = SL_RF_opt & (~OPT_PERF);
        }
      }
      else {
        SL_RF_optHigh = SL_RF_optHigh & (~OPT_JIT_TOP);
      }
    }
    else {
    }
    if (mode == SL_RF_REST) {
      if (!(SL_RF_optHigh & OPT_JIT_TOP)) {
        if (SL_RF_mRecordSize > 0) {
          SL_RF_optHigh = SL_RF_optHigh & (~OPT_MEMB_INCG);
          SL_RF_optHigh = SL_RF_optHigh & (~OPT_TERM_INCG);
        }
      }
    }
    if (mode == SL_RF_PRED) {
      if (!(SL_RF_optHigh & OPT_JIT_TOP)) {
        if (SL_RF_fmRecordSize > 0) {
          SL_RF_optHigh = SL_RF_optHigh & (~OPT_MEMB_INCG);
          SL_RF_optHigh = SL_RF_optHigh & (~OPT_TERM_INCG);
        }
      }
    }
    adj = 1;
    mwcpOffset = ulvector(1, adj);
    for (uint j = 1; j <= adj; j++) {
      mwcpOffset[j] = 0;
    }
    previousTreeID = b = 0;
    for (ulong ui = 1; ui <= SL_RF_totalNodeCount; ui++) {
      if ((SL_RF_treeID_[ui] > 0) && (SL_RF_treeID_[ui] <= SL_RF_ntree)) {
        if (SL_RF_treeID_[ui] != previousTreeID) {
          previousTreeID = SL_RF_restoreTreeID[++b] = SL_RF_treeID_[ui];
          SL_RF_restoreTreeOffset[SL_RF_treeID_[ui]] = ui;
        }
        SL_RF_nodeCount[SL_RF_treeID_[ui]] ++;
        for (uint j = 1; j <= adj; j++) {
          SL_RF_mwcpCT_[j][SL_RF_treeID_[ui]] += SL_RF_mwcpSZ_[j][ui];
        }
      }
      else {
        SL_RF_nativeError("\nRF-SRC:  Diagnostic Trace of Tree Record:  \n");
        SL_RF_nativeError("\nRF-SRC:      treeID     nodeID ");
        SL_RF_nativeError("\nRF-SRC:  %10d %10d \n", SL_RF_treeID_[ui], SL_RF_nodeID_[ui]);
        SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
        SL_RF_nativeError("\nRF-SRC:  Invalid forest input record at line:  %20lu", ui);
        SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
        SL_RF_nativeExit();
      }
    }
    for (b = 1; b <= SL_RF_ntree; b++) {
      for (uint j = 1; j <= adj; j++) {
        if (SL_RF_mwcpCT_[j][SL_RF_restoreTreeID[b]] > 0) {
          SL_RF_restoreMWCPoffset[j][SL_RF_restoreTreeID[b]] = mwcpOffset[j];
          mwcpOffset[j] = mwcpOffset[j] + SL_RF_mwcpCT_[j][SL_RF_restoreTreeID[b]];
        }
        else {
          SL_RF_restoreMWCPoffset[j][SL_RF_restoreTreeID[b]] = 0;
        }
      }
    }
    free_ulvector(mwcpOffset, 1, adj);
    SL_RF_totalTerminalCount = 0;
    for (b = 1; b <= SL_RF_ntree; b++) {
      SL_RF_totalTerminalCount += (ulong) SL_RF_tLeafCount[b];
    }
    break;
  }
  getConditionalClassificationIndex = &getConditionalClassificationIndexGrow;
  getGMeanIndex = &getGMeanIndexGrow;
  if (mode == SL_RF_PRED) {
    getConditionalClassificationIndex = &getConditionalClassificationIndexPred;
    getGMeanIndex = &getGMeanIndexPred;
  }
  stackMissingArraysPhase2(mode);
  stackDefinedOutputObjects(mode,
                            SL_RF_sexpString,
                            & SL_RF_root,
                            & SL_RF_tLeafCount_,
                            & SL_RF_proximity_,
                            & SL_RF_distance_,
                            & SL_RF_weight_,
                            & SL_RF_imputation_,
                            & SL_RF_sImputeResponsePtr,
                            & SL_RF_sImputePredictorPtr,
                            & SL_RF_varUsed_,
                            & SL_RF_varUsedPtr,
                            & SL_RF_splitDepth_);
  verifyAndRegisterCustomSplitRules();
  if ((SL_RF_optHigh & OPT_MEMB_INCG) || (SL_RF_optHigh & OPT_TERM_INCG)) {
    SL_RF_incStackCount = 0;
    stackAuxiliaryInfoList(&SL_RF_incAuxiliaryInfoList, 8);
  }
  else {
    SL_RF_incStackCount = 0;
    SL_RF_incAuxiliaryInfoList = NULL;
  }
  stackTNQualitativeObjectsKnown(mode,
                                 & SL_RF_RMBR_ID_,
                                 & SL_RF_AMBR_ID_,
                                 & SL_RF_TN_RCNT_,
                                 & SL_RF_TN_ACNT_,
                                 & SL_RF_OOB_SZ_,
                                 & SL_RF_IBG_SZ_);
  stackTNQuantitativeForestObjectsPtrOnly(mode);
  uint bnpSize;
  bnpSize = getVimpRecoverySeedDimension(mode, SL_RF_opt);
  ran1A = &randomChainParallel;
  ran1B = &randomUChainParallel;
  ran1D = &randomChainParallelVimp;
  randomSetChain = &randomSetChainParallel;
  randomSetUChain = &randomSetUChainParallel;
  randomSetChainVimp = &randomSetChainParallelVimp;
  randomGetChain = &randomGetChainParallel;
  randomGetUChain = &randomGetUChainParallel;
  randomGetChainVimp = &randomGetChainParallelVimp;
  randomStack(SL_RF_ntree, bnpSize);
  if (mode == SL_RF_GROW) {
    seedValueLC = abs(seedValue);
    lcgenerator(&seedValueLC, TRUE);
    for (b = 1; b <= SL_RF_ntree; b++) {
      lcgenerator(&seedValueLC, FALSE);
      lcgenerator(&seedValueLC, FALSE);
      while(seedValueLC == 0) {
        lcgenerator(&seedValueLC, FALSE);
      }
      randomSetChain(b, -seedValueLC);
    }
    for (b = 1; b <= SL_RF_ntree; b++) {
      lcgenerator(&seedValueLC, FALSE);
      lcgenerator(&seedValueLC, FALSE);
      while(seedValueLC == 0) {
        lcgenerator(&seedValueLC, FALSE);
      }
      randomSetUChain(b, -seedValueLC);
    }
    if (bnpSize > 0) {
      for (p = 1; p <= bnpSize; p++) {
        lcgenerator(&seedValueLC, FALSE);
        lcgenerator(&seedValueLC, FALSE);
        while(seedValueLC == 0) {
          lcgenerator(&seedValueLC, FALSE);
        }
        randomSetChainVimp(p, -seedValueLC);
      }
    }
  }  
  else {
    for (b = 1; b <= SL_RF_ntree; b++) {
      randomSetChain(b , SL_RF_seed_[b]);
    }
    seedValueLC = abs(seedValue);
    lcgenerator(&seedValueLC, TRUE);
    for (b = 1; b <= SL_RF_ntree; b++) {
      lcgenerator(&seedValueLC, FALSE);
      lcgenerator(&seedValueLC, FALSE);
      while(seedValueLC == 0) {
        lcgenerator(&seedValueLC, FALSE);
      }
      randomSetUChain(b, -seedValueLC);
    }
    if (SL_RF_opt & OPT_VIMP) {
      if ( (SL_RF_opt & (OPT_VIMP | OPT_VIMP_JOIN | OPT_VIMP_TYP1 | OPT_VIMP_TYP2)) ==
           (SL_RF_optLoGrow & (OPT_VIMP | OPT_VIMP_JOIN | OPT_VIMP_TYP1 | OPT_VIMP_TYP2)) )  {
        for (p = 1; p <= bnpSize; p++) {
          randomSetChainVimp(p , SL_RF_seedVimp_[p]);
        }
      }
      else {
        for (p = 1; p <= bnpSize; p++) {
          lcgenerator(&seedValueLC, FALSE);
          lcgenerator(&seedValueLC, FALSE);
          while(seedValueLC == 0) {
            lcgenerator(&seedValueLC, FALSE);
          }
          randomSetChainVimp(p, -seedValueLC);
        }
      }
    }
    else {
    }
  }  
#ifdef _OPENMP
  stackLocksOpenMP(mode);
#endif
  for (r = 1; r <= SL_RF_nImpute; r++) {
    if (getUserTraceFlag()) {
      if (SL_RF_nImpute == 1) {
      }
      else {
        SL_RF_nativePrint("\nImpute Iteration:  %6d", r);
      }
    }
    if (r == SL_RF_nImpute) {
      if (mode == SL_RF_GROW) {
        if (SL_RF_opt & OPT_SEED) {
          for (b = 1; b <= SL_RF_ntree; b++) {
            if (r > 1) {
              lcgenerator(&seedValueLC, FALSE);
              lcgenerator(&seedValueLC, FALSE);
              while(seedValueLC == 0) {
                lcgenerator(&seedValueLC, FALSE);
              }
              randomSetChain(b, -seedValueLC);
            }
            SL_RF_seed_[b] = randomGetChain(b);
          }
          if (bnpSize > 0) {
            for (p = 1; p <= bnpSize; p++) {  
              if (r > 1) {
                lcgenerator(&seedValueLC, FALSE);
                lcgenerator(&seedValueLC, FALSE);
                while(seedValueLC == 0) {
                  lcgenerator(&seedValueLC, FALSE);
                }
                randomSetChainVimp(p, -seedValueLC);
              }
              SL_RF_seedVimp_[p] = randomGetChainVimp(p);
            }
          }
        }
      }
    }  
    for(b = 1; b <= SL_RF_ntree; b++) {
      SL_RF_serialTreeIndex[b] = 0;
    }
    SL_RF_serialTreeID = 0;
    SL_RF_ensbUpdtCount = 0;
    SL_RF_serialBlockID = 0;
    if (getUserTraceFlag()) {
      SL_RF_userTimeStart = time(NULL);
      SL_RF_userTimeSplit = SL_RF_userTreeID = 0;
      SL_RF_nativePrint("\n");
    }
#ifdef _OPENMP
#pragma omp parallel for num_threads(SL_RF_numThreads)
#endif
    for (b = 1; b <= SL_RF_getTreeCount; b++) {
      acquireTree(mode, r, SL_RF_getTreeIndex[SL_RF_getTreeCount - b + 1]);
    }
    if (r == SL_RF_nImpute) {
      SL_RF_rejectedTreeCount = SL_RF_validTreeCount = SL_RF_stumpedTreeCount = 0;
      SL_RF_totalTerminalCount = 0;
      for (b = 1; b <= SL_RF_ntree; b++) {
        SL_RF_totalTerminalCount += (ulong) SL_RF_tLeafCount[b];
        if (SL_RF_tLeafCount[b] == 0) {
          SL_RF_rejectedTreeCount ++;
        }
        else {
          SL_RF_validTreeCount ++;
          if (SL_RF_tLeafCount[b] == 1) {
            SL_RF_stumpedTreeCount ++;
          }
        }
      }
      if (SL_RF_opt & OPT_PROX) {
        if ((mode == SL_RF_PRED) && (SL_RF_opt & OPT_PROX_IBG) && (SL_RF_opt & OPT_PROX_OOB)) {
          finalizeProximityHybrid(mode);
        }
        else {
          finalizeProximity(mode);
        }
      }
      if (SL_RF_optHigh & OPT_DIST) {
        if ((mode == SL_RF_PRED) && (SL_RF_optHigh & OPT_DIST_IBG) && (SL_RF_optHigh & OPT_DIST_OOB)) {
          finalizeDistanceHybrid(mode);
        }
        else {
          finalizeDistance(mode);
        }
      }
      if (SL_RF_optHigh & OPT_WGHT) {
        finalizeWeight(mode);
      }  
      stackForestObjectsOutput(mode);
      writeForestObjectsOutput(mode);
      stackTNQuantitativeForestObjectsOutput(mode);
      writeTNQuantitativeForestObjectsOutput(mode);
      stackTNQualitativeObjectsUnknown(mode,
                                       & SL_RF_TN_RCNT_,
                                       & SL_RF_TN_ACNT_,
                                       & SL_RF_TN_OCNT_,
                                       & SL_RF_TN_ICNT_);
      if (SL_RF_optHigh & OPT_MEMB_OUTG) {
        for (b = 1; b <= SL_RF_ntree; b++) {
          SL_RF_OOB_SZ_[b] = SL_RF_oobSize[b];
          SL_RF_IBG_SZ_[b] = SL_RF_ibgSize[b];
        }
      }
      stackTNQualitativeObjectsUnknownMembership(mode, & SL_RF_OMBR_ID_, & SL_RF_IMBR_ID_);
    }  
    if (SL_RF_opt & OPT_MISS_OUT) {
      switch (mode) {
      case SL_RF_PRED:
        imputeSummary(SL_RF_PRED, ACTIVE);
        break;
      default:
        if (r == 1) {
          if ( (!(SL_RF_opt & OPT_BOOT_TYP1) && !(SL_RF_opt & OPT_BOOT_TYP2)) ||
               ( (SL_RF_opt & OPT_BOOT_TYP1) &&  (SL_RF_opt & OPT_BOOT_TYP2)) ) {
            imputeSummary(SL_RF_GROW, FALSE);
            if (SL_RF_timeIndex > 0) {
              if (SL_RF_mTimeFlag == TRUE) {
                imputeMultipleTime(FALSE);
              }
            }
          }
          else {
            imputeSummary(SL_RF_GROW, ACTIVE);
            if (SL_RF_timeIndex > 0) {
              if (SL_RF_mTimeFlag == TRUE) {
                imputeMultipleTime(ACTIVE);
              }
            }
          }
        }  
        else {
          if (r < SL_RF_nImpute) {
            if ( (!(SL_RF_opt & OPT_BOOT_TYP1) && !(SL_RF_opt & OPT_BOOT_TYP2)) ||
                 ( (SL_RF_opt & OPT_BOOT_TYP1) &&  (SL_RF_opt & OPT_BOOT_TYP2)) ) {
              imputeSummary(SL_RF_GROW, FALSE);
              if (SL_RF_timeIndex > 0) {
                if (SL_RF_mTimeFlag == TRUE) {
                  imputeMultipleTime(FALSE);
                }
              }
            }
            else {
              imputeSummary(SL_RF_GROW, ACTIVE);
              if (SL_RF_timeIndex > 0) {
                if (SL_RF_mTimeFlag == TRUE) {
                  imputeMultipleTime(ACTIVE);
                }
              }
            }
          }
          else {
            if (SL_RF_opt & OPT_IMPU_ONLY) {
              if ( (!(SL_RF_opt & OPT_BOOT_TYP1) && !(SL_RF_opt & OPT_BOOT_TYP2)) ||
                   ( (SL_RF_opt & OPT_BOOT_TYP1) &&  (SL_RF_opt & OPT_BOOT_TYP2)) ) {
                imputeSummary(SL_RF_GROW, TRUE);
                if (SL_RF_timeIndex > 0) {
                  if (SL_RF_mTimeFlag == TRUE) {
                    imputeMultipleTime(TRUE);
                  }
                }
              }
              else {
                imputeSummary(SL_RF_GROW, ACTIVE);
                if (SL_RF_timeIndex > 0) {
                  if (SL_RF_mTimeFlag == TRUE) {
                    imputeMultipleTime(ACTIVE);
                  }
                }
              }
            }
            else {
            }
          }
        }
        break;
      }
    }  
    if (r < SL_RF_nImpute) {
#ifdef _OPENMP
#pragma omp parallel for num_threads(SL_RF_numThreads)
#endif
      for (uint bb = 1; bb <= SL_RF_getTreeCount; bb++) {
        freeLeafLinkedObjList(SL_RF_leafLinkedObjHead[SL_RF_getTreeIndex[bb]]);
        if (SL_RF_tLeafCount[SL_RF_getTreeIndex[bb]] > 0) {
          free_new_vvector(SL_RF_tTermList[SL_RF_getTreeIndex[bb]], 1, SL_RF_tLeafCount[SL_RF_getTreeIndex[bb]], NRUTIL_TPTR);
        }
        free_new_vvector(SL_RF_tTermMembership[SL_RF_getTreeIndex[bb]], 1, SL_RF_observationSize, NRUTIL_TPTR);
        if (mode == SL_RF_PRED) {
          free_new_vvector(SL_RF_ftTermMembership[SL_RF_getTreeIndex[bb]], 1, SL_RF_fobservationSize, NRUTIL_TPTR);
        }
      }
    }
    if (getUserTraceFlag()) {
      SL_RF_nativePrint("\n\n");
    }
  }  
  if (SL_RF_validTreeCount > 0) {
    if (SL_RF_opt & OPT_VIMP) {
      finalizeVimpPerformance(mode);
    }
    if (SL_RF_optHigh & OPT_PART_PLOT) {
#ifdef _OPENMP
#pragma omp parallel for num_threads(SL_RF_numThreads)
#endif
      for (p = 1; p <= SL_RF_partialLength; p++) {
        summarizePartialCalculations(0, p);
      }
    }
    normalizeEnsembleEstimates(mode, TRUE);
#ifdef _OPENMP
#pragma omp parallel for num_threads(SL_RF_numThreads)
#endif
    for (uint bb = 1; bb <= SL_RF_getTreeCount; bb++) {
      freeLeafLinkedObjList(SL_RF_leafLinkedObjHead[SL_RF_getTreeIndex[bb]]);
      if (SL_RF_tLeafCount[SL_RF_getTreeIndex[bb]] > 0) {
        free_new_vvector(SL_RF_tTermList[SL_RF_getTreeIndex[bb]], 1, SL_RF_tLeafCount[SL_RF_getTreeIndex[bb]], NRUTIL_TPTR);
      }
      free_new_vvector(SL_RF_tTermMembership[SL_RF_getTreeIndex[bb]], 1, SL_RF_observationSize, NRUTIL_TPTR);
      if (mode == SL_RF_PRED) {
        free_new_vvector(SL_RF_ftTermMembership[SL_RF_getTreeIndex[bb]], 1, SL_RF_fobservationSize, NRUTIL_TPTR);
      }
    }
    if (SL_RF_opt & (OPT_VARUSED_F | OPT_VARUSED_T)) {
      if (SL_RF_opt & OPT_VARUSED_F) {
#ifdef _OPENMP
#pragma omp parallel for num_threads(SL_RF_numThreads)
#endif
        for (uint jj = 1; jj <= SL_RF_xSize; jj++) {
          SL_RF_varUsed_[jj] = 0;
          for (uint bb = 1; bb <= SL_RF_ntree; bb++) {
            SL_RF_varUsed_[jj] += SL_RF_varUsedPtr[bb][jj];
          }
        }
      }
      else {
      }
    }
    if (SL_RF_opt & (OPT_SPLDPTH_1 | OPT_SPLDPTH_2)) {
      if (SL_RF_opt & OPT_SPLDPTH_1) {
#ifdef _OPENMP
#pragma omp parallel for num_threads(SL_RF_numThreads)
#endif
        for (uint jj = 1; jj <= SL_RF_xSize; jj++) {
          for (uint ii = 1; ii <= SL_RF_observationSize; ii++) {
            SL_RF_splitDepthPtr[1][jj][ii] = SL_RF_splitDepthPtr[1][jj][ii] / (SL_RF_validTreeCount);
          }
        }
      }
      else {
      }
    }
  }  
  else {
#ifdef _OPENMP
#pragma omp parallel for num_threads(SL_RF_numThreads)
#endif
    for (uint bb = 1; bb <= SL_RF_getTreeCount; bb++) {
      freeLeafLinkedObjList(SL_RF_leafLinkedObjHead[SL_RF_getTreeIndex[bb]]);
      free_new_vvector(SL_RF_tTermMembership[SL_RF_getTreeIndex[bb]], 1, SL_RF_observationSize, NRUTIL_TPTR);
      if (mode == SL_RF_PRED) {
        free_new_vvector(SL_RF_ftTermMembership[SL_RF_getTreeIndex[bb]], 1, SL_RF_fobservationSize, NRUTIL_TPTR);
      }
    }
    SL_RF_nativePrint("\nRF-SRC:  *** WARNING *** ");
    SL_RF_nativePrint("\nRF-SRC:  Insufficient trees for analysis.  \n");
  }
  switch (mode) {
  case SL_RF_GROW:
    freeSplitRuleObj(SL_RF_splitRuleObj);
    unstackForestObjectsPtrOnly(mode);
    unstackTNQuantitativeForestObjectsPtrOnly(mode);
    unstackForestObjectsAuxOnly(mode);
    break;
  default:
    break;
  }
  unstackAuxiliaryInfoAndList((mode == SL_RF_GROW) ? FALSE : TRUE, SL_RF_snpAuxiliaryInfoList, SL_RF_stackCount);
  if ((SL_RF_optHigh & OPT_MEMB_INCG) || (SL_RF_optHigh & OPT_TERM_INCG)) {
    unstackAuxiliaryInfoAndList(FALSE, SL_RF_incAuxiliaryInfoList, 8);
  }
  unstackDefinedOutputObjects(mode);
  if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
    unstackCompetingArrays(mode);
  }
  if (SL_RF_rFactorCount > 0) {
    unstackClassificationArrays(mode);
  }
  unstackMissingArrays(mode);
  if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
    unstackTimeAndSubjectArrays(mode);
  }
  switch (mode) {
  case SL_RF_PRED:
    unstackPreDefinedPredictArrays();
    break;
  case SL_RF_REST:
    unstackPreDefinedRestoreArrays();
    break;
  default:
    unstackPreDefinedGrowthArrays();
    break;
  }
  unstackPreDefinedCommonArrays(mode,
                                SL_RF_nodeMembership,
                                SL_RF_tTermMembership,
                                SL_RF_tTermList,
                                SL_RF_root);
  unstackIncomingArrays(mode);
  randomUnstack(SL_RF_ntree, bnpSize);
#ifdef _OPENMP
  unstackLocksOpenMP(mode);
#endif
  unstackFactorArrays(mode);
}
