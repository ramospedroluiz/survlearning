/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "tree.h"
#include "impute.h"
#include "stackOutput.h"
#include "treeUtil.h"
#include "treeJIT.h"
#include "processEnsemble.h"
#include "splitGreedy.h"
#include "regression.h"
#include "importance.h"
#include "stackOutputQQ.h"
#include "nodeOps.h"
#include "node.h"
#include "nativeUtil.h"
#include "nrutil.h"
#include "error.h"
void acquireTreeGeneric(char mode, uint r, uint b) {
  char multImpFlag;
  uint *fallMembrIndx;
  uint bootMembrIndxIter;
  uint rmbrIterator;
  uint ambrIterator;
  char result;
  uint offset;
  uint i;
#ifdef _OPENMP
#endif
  SL_RF_root[b] = makeNode((mode == SL_RF_GROW) ? SL_RF_xSize : 0);
  for (i = 1; i <= SL_RF_root[b] -> xSize; i++) {
    SL_RF_root[b] -> permissible[i] = TRUE;
  }
  stackAuxiliary(mode, b);
  if (mode == SL_RF_PRED) {
    fallMembrIndx = uivector(1, SL_RF_fobservationSize);
  }
  else {
    fallMembrIndx = NULL;
  }
  SL_RF_root[b] -> allMembrSizeAlloc = SL_RF_root[b] -> allMembrSize = SL_RF_observationSize;
  SL_RF_root[b] -> allMembrIndx = uivector(1, SL_RF_root[b] -> allMembrSizeAlloc);
  stackShadow(mode, b);
  stackFactorInSitu(b);
  SL_RF_tTermMembership[b] = (Terminal **) new_vvector(1, SL_RF_observationSize, NRUTIL_TPTR);
  SL_RF_leafLinkedObjHead[b] = SL_RF_leafLinkedObjTail[b] = makeLeafLinkedObj();
  if (mode == SL_RF_PRED) {
    SL_RF_ftTermMembership[b] = (Terminal **) new_vvector(1, SL_RF_fobservationSize, NRUTIL_TPTR);
  }
  if (mode == SL_RF_GROW) {
    if (SL_RF_nImpute > 1) {
      if (r > 1) {
        if (SL_RF_mRecordSize > 0) {
          imputeUpdateShadow(SL_RF_GROW,
                             SL_RF_response[b],
                             SL_RF_observation[b]);
        }
        if (SL_RF_timeIndex > 0) {
          if (SL_RF_mTimeFlag == TRUE) {
            updateTimeIndexArray(0,
                                 NULL,
                                 SL_RF_observationSize,
                                 SL_RF_time[b],
                                 FALSE,
                                 FALSE,
                                 SL_RF_masterTimeIndex[b]);
          }
        }
      }  
    }  
  }  
  SL_RF_root[b] -> parent = NULL;
  SL_RF_root[b] -> nodeID = 1;
  SL_RF_maxDepth[b] = 0;
  bootMembrIndxIter = 0;
  for (i = 1; i <= SL_RF_observationSize; i++) {
    SL_RF_root[b] -> allMembrIndx[i] = i;
    SL_RF_nodeMembership[b][i] = SL_RF_root[b];
  }
  if (SL_RF_optHigh & OPT_MEMB_PRUN) {
    Node     ***gNodeMembership;
    uint        obsSize;
    switch (mode) {
    case SL_RF_PRED:
      obsSize = SL_RF_fobservationSize;
      gNodeMembership = SL_RF_fnodeMembership;
      break;
    default:
      obsSize = SL_RF_observationSize;
      gNodeMembership = SL_RF_nodeMembership;
      break;
    }
    for (i = 1; i <= obsSize; i++) {
      SL_RF_pNodeMembership[b][i] = gNodeMembership[b][i];
    }
  }
  multImpFlag = FALSE;
  if (mode == SL_RF_GROW) {
    if (r > 1) {
      multImpFlag = TRUE;
    }
  }
  if ((SL_RF_mRecordSize == 0) || multImpFlag || !(SL_RF_optHigh & OPT_MISS_SKIP)) {
    getVariance = & getVarianceClassicNoMiss;
  }
  else {
    getVariance = & getVarianceClassic;
  }
  switch (mode) {
  case SL_RF_GROW:
    SL_RF_tLeafCount[b] = 0;
    rmbrIterator = ambrIterator = 0;
    result = growTree (r,
                       TRUE,
                       multImpFlag,
                       b,
                       SL_RF_root[b],
                       & bootMembrIndxIter,
                       & rmbrIterator,
                       & ambrIterator);
    if (result) {
      SL_RF_nodeCount[b]  = (SL_RF_tLeafCount[b] << 1) - 1;
    }
    else {
      SL_RF_nodeCount[b] = 1;
    }
    break;
  default:
    if (mode == SL_RF_PRED) {
      for (i = 1; i <= SL_RF_fobservationSize; i++) {
        fallMembrIndx[i] = i;
        SL_RF_fnodeMembership[b][i] = SL_RF_root[b];
      }
    }
    restoreTree(mode, b, SL_RF_root[b]);
    rmbrIterator = ambrIterator = 0;
    result = restoreNodeMembership(mode,
                                   TRUE,
                                   b,
                                   SL_RF_root[b],
                                   NULL,
                                   0,
                                   SL_RF_root[b] -> allMembrIndx,
                                   SL_RF_root[b] -> allMembrSize,
                                   fallMembrIndx,
                                   SL_RF_fobservationSize,
                                   & bootMembrIndxIter,
                                   & rmbrIterator,
                                   & ambrIterator);
    break;
  }
  if (result) {
    SL_RF_tTermList[b] = (Terminal **) new_vvector(1, SL_RF_tLeafCount[b], NRUTIL_TPTR);
    LeafLinkedObj *leafLinkedPtr;
    leafLinkedPtr = SL_RF_leafLinkedObjHead[b] -> fwdLink;
    while (leafLinkedPtr != NULL) {
      SL_RF_tTermList[b][(leafLinkedPtr -> termPtr) -> nodeID] = leafLinkedPtr -> termPtr;
      leafLinkedPtr = leafLinkedPtr -> fwdLink;
    }
    if (mode != SL_RF_PRED) {
      if (SL_RF_mRecordSize > 0) {
        for (i = 1; i <= SL_RF_mRecordSize; i++) {
          if (SL_RF_bootMembershipFlag[b][SL_RF_mRecordIndex[i]] == TRUE) {
            SL_RF_dmRecordBootFlag[b][i] = TRUE;
          }
          else {
            SL_RF_dmRecordBootFlag[b][i] = FALSE;
          }
        }
      }  
    }  
    if (r == SL_RF_nImpute) {      
      if ( (SL_RF_vtry == 0) || ((SL_RF_vtry > 0) && (SL_RF_vtryMode == SL_RF_VTRY_NULL)) ) {
        if (SL_RF_ptnCount > 0) {
          updatePruning(mode, b);
        }
        processEnsembleInSitu(mode, multImpFlag, b);
      }
      else {
        for (uint p = 1; p <= SL_RF_xSize; p++) {
          processEnsembleHoldout(p, b);
        }
      }
    }
  }  
  if (r == SL_RF_nImpute) {
    if (mode == SL_RF_GROW) {
      if (SL_RF_opt & OPT_TREE) {
        offset = 0;
        stackTreeObjectsPtrOnly(mode, b);
        saveTree(b, SL_RF_root[b], & offset);
        if (SL_RF_tLeafCount[b] > 0) {
          stackTNQuantitativeTreeObjectsPtrOnly(b);
          saveTNQuantitativeTreeObjects(b);
        }          
      }  
    }  
  }  
  unstackShadow(mode, b);
  unstackFactorInSitu(b);
  if (mode == SL_RF_PRED) {
    free_uivector(fallMembrIndx, 1, SL_RF_fobservationSize);
  }
  unstackAuxiliary(mode, b);
  freeTree(b, SL_RF_root[b]);
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
        SL_RF_nativePrint("Trees Grown:  %6d,    Time Remaining (sec):  %6.0f \n", SL_RF_userTreeID, ceil(userTimeRemaining));
        SL_RF_userTimeSplit = current;
      }
    }  
  }
}
void updateWeight(char mode, uint b) {
  uint     **utTermMembership;
  uint      *utTermMembershipCount;
  Terminal **itTermMembership, **gtTermMembership;
  uint  *gMembershipIndex, *iMembershipIndex;
  uint   gMembershipSize,   iMembershipSize;
  uint  *bootMembershipCount;
  uint  mtnmFlag;
  gMembershipIndex = NULL;
  gMembershipSize  = 0;
  gtTermMembership = NULL;
  if((SL_RF_optHigh & OPT_WGHT_IBG) && (SL_RF_optHigh & OPT_WGHT_OOB)) {
    switch (mode) {
    case SL_RF_PRED:
      gMembershipSize = SL_RF_fobservationSize;
      gMembershipIndex = SL_RF_fidentityMembershipIndex;
      gtTermMembership = SL_RF_ftTermMembership[b];
      break;
    default:
      gMembershipSize = SL_RF_observationSize;
      gMembershipIndex = SL_RF_identityMembershipIndex;
      gtTermMembership = SL_RF_tTermMembership[b];
      break;
    }
  }
  else {
    if((SL_RF_optHigh & OPT_WGHT_IBG)  && !(SL_RF_optHigh & OPT_WGHT_OOB)) {
      gMembershipSize  = SL_RF_observationSize;
      gMembershipIndex = SL_RF_identityMembershipIndex;
      gtTermMembership = SL_RF_tTermMembership[b];
    }
    else if(!(SL_RF_optHigh & OPT_WGHT_IBG)  && (SL_RF_optHigh & OPT_WGHT_OOB)) {
      gMembershipIndex = SL_RF_oobMembershipIndex[b];
      gMembershipSize  = SL_RF_oobSize[b];
      gtTermMembership = SL_RF_tTermMembership[b];
    }
    else {
      SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
      SL_RF_nativeError("\nRF-SRC:  Illegal updateWeight() call.");
      SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
      SL_RF_nativeExit();
    }
  }
  iMembershipIndex = SL_RF_ibgMembershipIndex[b];
  iMembershipSize  = SL_RF_ibgSize[b];
  bootMembershipCount = SL_RF_bootMembershipCount[b];
  itTermMembership = SL_RF_tTermMembership[b];
  if (SL_RF_xMarginalSize > 0) {
    mtnmFlag = TRUE;
  }
  else {
    mtnmFlag = FALSE;
  }
  if (!mtnmFlag) {
    for (uint i = 1; i <= gMembershipSize; i++) {
      uint ii, jj;
      ii = gMembershipIndex[i];
#ifdef _OPENMP
      omp_set_lock(&(SL_RF_lockWeightRow[ii]));
#endif
      SL_RF_weightDenom[ii] ++;
      for (uint j = 1; j <= iMembershipSize; j++) {
        jj = iMembershipIndex[j];
        if ( gtTermMembership[ii] == itTermMembership[jj] ) {
          SL_RF_weightPtr[ii][jj] +=  (double) bootMembershipCount[jj] / (double) (gtTermMembership[ii] -> membrCount);
        }
      }
#ifdef _OPENMP
      omp_unset_lock(&(SL_RF_lockWeightRow[ii]));
#endif
    }
  }
  else {
    utTermMembershipCount = SL_RF_utTermMembershipCount[b];
    utTermMembership      = SL_RF_utTermMembership[b];
    for (uint i = 1; i <= gMembershipSize; i++) {
      uint rowNodeID, colNodeID;
      uint ii, jj;
      uint xMembrCount;
      ii = gMembershipIndex[i];
#ifdef _OPENMP
      omp_set_lock(&(SL_RF_lockWeightRow[ii]));
#endif
      SL_RF_weightDenom[ii] ++;
      for (uint j = 1; j <= iMembershipSize; j++) {
        jj = iMembershipIndex[j];
        colNodeID = itTermMembership[jj] -> nodeID;
        xMembrCount = 0;
        for (uint k = 1; k <= utTermMembershipCount[ii]; k++) {
          xMembrCount += SL_RF_tTermList[b][utTermMembership[ii][k]] -> membrCount;
          rowNodeID = utTermMembership[ii][k];
          if ( colNodeID == rowNodeID ) {
            for (uint kk = k+1; kk <= utTermMembershipCount[ii]; kk++) {
              xMembrCount += SL_RF_tTermList[b][utTermMembership[ii][kk]] -> membrCount;
            }
            SL_RF_weightPtr[ii][jj] +=  (double) bootMembershipCount[jj] / (double) xMembrCount;
            goto wghtMarginal;
          }
        }
      wghtMarginal:
        continue;
      }
#ifdef _OPENMP
      omp_unset_lock(&(SL_RF_lockWeightRow[ii]));
#endif
    }
  }
}
void finalizeWeight(char mode) {
  uint    obsSize;
  switch (mode) {
  case SL_RF_PRED:
    obsSize = SL_RF_fobservationSize;
    break;
  default:
    obsSize = SL_RF_observationSize;
    break;
  }
#ifdef _OPENMP
#pragma omp parallel for num_threads(SL_RF_numThreads)
#endif
  for (uint i = 1; i <= obsSize; i++) {
    if (SL_RF_weightDenom[i] > 0) {    
      for (uint j = 1; j <= SL_RF_observationSize; j++) {
        SL_RF_weightPtr[i][j] = SL_RF_weightPtr[i][j] /  SL_RF_weightDenom[i];
      }
    }
    else {        
      for (uint j = 1; j <= SL_RF_observationSize; j++) {
        SL_RF_weightPtr[i][j] = SL_RF_nativeNaN;
      }
    }
  }
}
void updateProximity(char mode, uint b) {
  uint     **utTermMembership;
  uint      *utTermMembershipCount;
  Terminal **tTermMembership;
  uint  *membershipIndex;
  uint   membershipSize;
  uint  mtnmFlag;
  if ((mode == SL_RF_PRED) && (SL_RF_opt & OPT_PROX_IBG) && (SL_RF_opt & OPT_PROX_OOB)) {
    SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    SL_RF_nativeError("\nRF-SRC:  Illegal updateProximity() call.");
    SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
    SL_RF_nativeExit();
  }
  membershipSize  = 0;  
  membershipIndex = NULL;  
  if((SL_RF_opt & OPT_PROX_IBG) && (SL_RF_opt & OPT_PROX_OOB)) {
    membershipSize = SL_RF_observationSize;
    membershipIndex = SL_RF_identityMembershipIndex;
    tTermMembership = SL_RF_tTermMembership[b];
  }
  else {
    if((SL_RF_opt & OPT_PROX_IBG)  && !(SL_RF_opt & OPT_PROX_OOB)) {
      membershipIndex = SL_RF_ibgMembershipIndex[b];
      membershipSize  = SL_RF_ibgSize[b];
      tTermMembership = SL_RF_tTermMembership[b];
    }
    else if(!(SL_RF_opt & OPT_PROX_IBG)  && (SL_RF_opt & OPT_PROX_OOB)) {
      switch (mode) {
      case SL_RF_PRED:
        membershipSize = SL_RF_fobservationSize;
        membershipIndex = SL_RF_fidentityMembershipIndex;
        tTermMembership = SL_RF_ftTermMembership[b];
        break;
      default:
        membershipSize  = SL_RF_oobSize[b];
        membershipIndex = SL_RF_oobMembershipIndex[b];
        tTermMembership = SL_RF_tTermMembership[b];
        break;
      }
    }
    else {
      SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
      SL_RF_nativeError("\nRF-SRC:  Illegal updateProximity() call.");
      SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
      SL_RF_nativeExit();
    }
  }
  if (SL_RF_xMarginalSize > 0) {
    mtnmFlag = TRUE;
  }
  else {
    mtnmFlag = FALSE;
  }
  if (!mtnmFlag) {
    for (uint i = 1; i <= membershipSize; i++) {
      uint ii, jj;
      ii = membershipIndex[i];
      for (uint j = 1; j <= i; j++) {
        jj = membershipIndex[j];
        rfsrc_omp_atomic_update(&SL_RF_proximityDenPtr[ii][jj], 1.0);
        if ( tTermMembership[ii] == tTermMembership[jj] ) {
          rfsrc_omp_atomic_update(&SL_RF_proximityPtr[ii][jj], 1.0);
        }
      }
    }
  }
  else {
    utTermMembership =  SL_RF_utTermMembership[b];
    utTermMembershipCount =  SL_RF_utTermMembershipCount[b];
    for (uint i = 1; i <= membershipSize; i++) {
      uint ii, jj;
      ii = membershipIndex[i];
      for (uint j = 1; j <= i; j++) {
        jj = membershipIndex[j];
        rfsrc_omp_atomic_update(&SL_RF_proximityDenPtr[ii][jj], 1.0);
        for (uint ki = 1; ki <= utTermMembershipCount[ii]; ki++) {
          for (uint kj = 1; kj <= utTermMembershipCount[jj]; kj++) {
            if ( utTermMembership[ii][ki] == utTermMembership[jj][kj] ) {
              rfsrc_omp_atomic_update(&SL_RF_proximityPtr[ii][jj], 1.0);
              goto proxMarginal;
            }
          }
        }
      proxMarginal:
        continue;
      }
    }
  }
}
void updateProximityHybrid(char mode, uint b) {
  Terminal **tTermMembership, **ftTermMembership;
  uint  *trainMembershipIndex, *testMembershipIndex;
  uint   trainMembershipSize,   testMembershipSize;
  if ( !((mode == SL_RF_PRED) && (SL_RF_opt & OPT_PROX_IBG) && (SL_RF_opt & OPT_PROX_OOB)) ) {
    SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    SL_RF_nativeError("\nRF-SRC:  Illegal updateProximityHybrid() call.");
    SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
    SL_RF_nativeExit();
  }
  trainMembershipSize = SL_RF_observationSize;
  testMembershipSize  = SL_RF_fobservationSize;
  trainMembershipIndex = SL_RF_identityMembershipIndex;
  testMembershipIndex  = SL_RF_fidentityMembershipIndex;
  tTermMembership  = SL_RF_tTermMembership[b];
  ftTermMembership = SL_RF_ftTermMembership[b];
  for (uint i = 1; i <= testMembershipSize; i++) {
    uint ii, jj;
    ii = testMembershipIndex[i];
    for (uint j = 1; j <= trainMembershipSize; j++) {
      jj = trainMembershipIndex[j];
      rfsrc_omp_atomic_update(&SL_RF_proximityDenPtr[ii][jj], 1.0);
      if ( ftTermMembership[ii] == tTermMembership[jj] ) {
        rfsrc_omp_atomic_update(&SL_RF_proximityPtr[ii][jj], 1.0);
      }
    }
  }
}
void finalizeProximity(char mode) {
  uint  obsSize;
  switch (mode) {
  case SL_RF_PRED:
    obsSize = SL_RF_fobservationSize;
    break;
  default:
    obsSize = SL_RF_observationSize;
    break;
  }
#ifdef _OPENMP
#pragma omp parallel for num_threads(SL_RF_numThreads)
#endif
  for (uint i = 1; i <= obsSize; i++) {
    for (uint j = 1; j <= i; j++) {
      if (SL_RF_proximityDenPtr[i][j] > 0) {
        SL_RF_proximityPtr[i][j] = SL_RF_proximityPtr[i][j] /  SL_RF_proximityDenPtr[i][j];
      }
      else {
        SL_RF_proximityPtr[i][j] = SL_RF_nativeNaN;
      }
    }
  }
}
void finalizeProximityHybrid(char mode) {
#ifdef _OPENMP
#pragma omp parallel for num_threads(SL_RF_numThreads)
#endif
  for (uint i = 1; i <= SL_RF_fobservationSize; i++) {
    for (uint j = 1; j <= SL_RF_observationSize; j++) {
      if (SL_RF_proximityDenPtr[i][j] > 0) {
        SL_RF_proximityPtr[i][j] = SL_RF_proximityPtr[i][j] /  SL_RF_proximityDenPtr[i][j];
      }
      else {
        SL_RF_proximityPtr[i][j] = SL_RF_nativeNaN;
      }
    }
  }
}
void updateDistance(char mode, uint b) {
  uint     **utTermMembership;
  uint      *utTermMembershipCount;
  Terminal **tTermMembership;
  uint  *membershipIndex;
  uint   membershipSize;
  uint  mtnmFlag;
  if ((mode == SL_RF_PRED) && (SL_RF_optHigh & OPT_DIST_IBG) && (SL_RF_optHigh & OPT_DIST_OOB)) {
    SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    SL_RF_nativeError("\nRF-SRC:  Illegal updateDistance() call.");
    SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
    SL_RF_nativeExit();
  }
  membershipSize  = 0;  
  membershipIndex = NULL;  
  if((SL_RF_optHigh & OPT_DIST_IBG) && (SL_RF_optHigh & OPT_DIST_OOB)) {
    membershipSize = SL_RF_observationSize;
    membershipIndex = SL_RF_identityMembershipIndex;
    tTermMembership = SL_RF_tTermMembership[b];
  }
  else {
    if((SL_RF_optHigh & OPT_DIST_IBG)  && !(SL_RF_optHigh & OPT_DIST_OOB)) {
      membershipIndex = SL_RF_ibgMembershipIndex[b];
      membershipSize  = SL_RF_ibgSize[b];
      tTermMembership = SL_RF_tTermMembership[b];
    }
    else if(!(SL_RF_optHigh & OPT_DIST_IBG)  && (SL_RF_optHigh & OPT_DIST_OOB)) {
      switch (mode) {
      case SL_RF_PRED:
        membershipSize = SL_RF_fobservationSize;
        membershipIndex = SL_RF_fidentityMembershipIndex;
        tTermMembership = SL_RF_ftTermMembership[b];
        break;
      default:
        membershipSize  = SL_RF_oobSize[b];
        membershipIndex = SL_RF_oobMembershipIndex[b];
        tTermMembership = SL_RF_tTermMembership[b];
        break;
      }
    }
    else {
      SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
      SL_RF_nativeError("\nRF-SRC:  Illegal updateDistance() call.");
      SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
      SL_RF_nativeExit();
    }
  }
  if (SL_RF_xMarginalSize > 0) {
    mtnmFlag = TRUE;
  }
  else {
    mtnmFlag = FALSE;
  }
  if (!mtnmFlag) {
    for (uint i = 1; i <= membershipSize; i++) {
      Node *iNodeMembership, *jNodeMembership;
      Node *deepNodeMembership, *shallowNodeMembership;
      uint  iEdgeCount, jEdgeCount;
      uint  iDepthCount, jDepthCount;
      uint  *deepEdgeCount;
      double realDistance;
      uint ii, jj;
      ii = membershipIndex[i];
      iNodeMembership = tTermMembership[ii] -> mate;
      iDepthCount = iNodeMembership -> depth;
      for (uint j = 1; j <= i; j++) {
        jj = membershipIndex[j];
        rfsrc_omp_atomic_update(&SL_RF_distanceDenPtr[ii][jj], 1.0);
        jNodeMembership = tTermMembership[jj] -> mate;
        jDepthCount = jNodeMembership -> depth;
        iEdgeCount = jEdgeCount = 0;
        if ((iNodeMembership -> depth) > (jNodeMembership -> depth)) {
          deepNodeMembership = iNodeMembership;
          shallowNodeMembership = jNodeMembership;
          deepEdgeCount = & iEdgeCount;
        }
        else {
          deepNodeMembership = jNodeMembership;
          shallowNodeMembership = iNodeMembership;
          deepEdgeCount = & jEdgeCount;
        }
        while ((deepNodeMembership -> depth) > (shallowNodeMembership -> depth)) {
          deepNodeMembership = deepNodeMembership -> parent;
          (*deepEdgeCount) ++;
        }
        while (deepNodeMembership != shallowNodeMembership) {
          deepNodeMembership = deepNodeMembership -> parent;
          shallowNodeMembership = shallowNodeMembership -> parent;
          iEdgeCount ++;
          jEdgeCount ++;
        }
        if (iDepthCount > 0) { 
          realDistance = (double) (iEdgeCount + jEdgeCount) / (iDepthCount + jDepthCount);
        }
        else {
          realDistance = 0;
        }
        rfsrc_omp_atomic_update(&SL_RF_distancePtr[ii][jj], realDistance);
      }
    }
  }
  else {
    utTermMembership =  SL_RF_utTermMembership[b];
    utTermMembershipCount =  SL_RF_utTermMembershipCount[b];
    for (uint i = 1; i <= membershipSize; i++) {
      Node *iNodeMembership, *jNodeMembership;
      Node *deepNodeMembership, *shallowNodeMembership;
      uint  iEdgeCount, jEdgeCount;
      uint  iDepthCount, jDepthCount;
      uint  *deepEdgeCount;
      double realDistance;
      double minDistance;
      uint ii, jj;
      ii = membershipIndex[i];
      for (uint j = 1; j <= i; j++) {
        jj = membershipIndex[j];
        rfsrc_omp_atomic_update(&SL_RF_distanceDenPtr[ii][jj], 1.0);
        minDistance = 1.0;
        for (uint ki = 1; ki <= utTermMembershipCount[ii]; ki++) {
          iNodeMembership = SL_RF_tTermList[b][utTermMembership[ii][ki]] -> mate;
          iDepthCount = iNodeMembership -> depth;
          for (uint kj = 1; kj <= utTermMembershipCount[jj]; kj++) {
            jNodeMembership = SL_RF_tTermList[b][utTermMembership[jj][kj]] -> mate;
            jDepthCount = jNodeMembership -> depth;
            iEdgeCount = jEdgeCount = 0;
            if ((iNodeMembership -> depth) > (jNodeMembership -> depth)) {
              deepNodeMembership = iNodeMembership;
              shallowNodeMembership = jNodeMembership;
              deepEdgeCount = & iEdgeCount;
            }
            else {
              deepNodeMembership = jNodeMembership;
              shallowNodeMembership = iNodeMembership;
              deepEdgeCount = & jEdgeCount;
            }
            while ((deepNodeMembership -> depth) > (shallowNodeMembership -> depth)) {
              deepNodeMembership = deepNodeMembership -> parent;
              (*deepEdgeCount) ++;
            }
            while (deepNodeMembership != shallowNodeMembership) {
              deepNodeMembership = deepNodeMembership -> parent;
              shallowNodeMembership = shallowNodeMembership -> parent;
              iEdgeCount ++;
              jEdgeCount ++;
            }
            if (iDepthCount > 0) { 
              realDistance = (double) (iEdgeCount + jEdgeCount) / (iDepthCount + jDepthCount);
            }
            else {
              realDistance = 0;
            }
            if (realDistance < minDistance) {
              minDistance = realDistance;
              if (minDistance == 0) {
                goto distMarginal;
              }
            }
          }
        }
      distMarginal:
        rfsrc_omp_atomic_update(&SL_RF_distancePtr[ii][jj], minDistance);
      }
    }
  }
}
void updateDistanceHybrid(char mode, uint b) {
  Terminal **tTermMembership, **ftTermMembership;
  uint  *trainMembershipIndex, *testMembershipIndex;
  uint   trainMembershipSize,   testMembershipSize;
  if ( !((mode == SL_RF_PRED) && (SL_RF_optHigh & OPT_DIST_IBG) && (SL_RF_optHigh & OPT_DIST_OOB)) ) {
    SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    SL_RF_nativeError("\nRF-SRC:  Illegal updateDistanceHybrid() call.");
    SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
    SL_RF_nativeExit();
  }
  trainMembershipSize = SL_RF_observationSize;
  testMembershipSize  = SL_RF_fobservationSize;
  trainMembershipIndex = SL_RF_identityMembershipIndex;
  testMembershipIndex  = SL_RF_fidentityMembershipIndex;
  tTermMembership  = SL_RF_tTermMembership[b];
  ftTermMembership = SL_RF_ftTermMembership[b];
  for (uint i = 1; i <= testMembershipSize; i++) {
    Node *iNodeMembership, *jNodeMembership;
    Node *deepNodeMembership, *shallowNodeMembership;
    uint  iEdgeCount, jEdgeCount;
    uint  iDepthCount, jDepthCount;
    uint  *deepEdgeCount;
    double realDistance;
    uint ii, jj;
    ii = testMembershipIndex[i];
    iNodeMembership = ftTermMembership[ii] -> mate;
    iDepthCount = iNodeMembership -> depth;
    for (uint j = 1; j <= trainMembershipSize; j++) {
      jj = trainMembershipIndex[j];
      rfsrc_omp_atomic_update(&SL_RF_distanceDenPtr[ii][jj], 1.0);
      jNodeMembership = tTermMembership[jj] -> mate;
      jDepthCount = jNodeMembership -> depth;
      iEdgeCount = jEdgeCount = 0;
      if ((iNodeMembership -> depth) > (jNodeMembership -> depth)) {
        deepNodeMembership = iNodeMembership;
        shallowNodeMembership = jNodeMembership;
        deepEdgeCount = & iEdgeCount;
      }
      else {
        deepNodeMembership = jNodeMembership;
        shallowNodeMembership = iNodeMembership;
        deepEdgeCount = & jEdgeCount;
      }
      while ((deepNodeMembership -> depth) > (shallowNodeMembership -> depth)) {
        deepNodeMembership = deepNodeMembership -> parent;
        (*deepEdgeCount) ++;
      }
      while (deepNodeMembership != shallowNodeMembership) {
        deepNodeMembership = deepNodeMembership -> parent;
        shallowNodeMembership = shallowNodeMembership -> parent;
        iEdgeCount ++;
        jEdgeCount ++;
      }
      if (iDepthCount > 0) { 
        realDistance = (double) (iEdgeCount + jEdgeCount) / (iDepthCount + jDepthCount);
      }
      else {
        realDistance = 0;
      }
      rfsrc_omp_atomic_update(&SL_RF_distancePtr[ii][jj], realDistance);
    }
  }
}
void finalizeDistance(char mode) {
  uint  obsSize;
  switch (mode) {
  case SL_RF_PRED:
    obsSize = SL_RF_fobservationSize;
    break;
  default:
    obsSize = SL_RF_observationSize;
    break;
  }
#ifdef _OPENMP
#pragma omp parallel for num_threads(SL_RF_numThreads)
#endif
  for (uint i = 1; i <= obsSize; i++) {
    for (uint j = 1; j <= i; j++) {
      if (SL_RF_distanceDenPtr[i][j] > 0) {
        SL_RF_distancePtr[i][j] = SL_RF_distancePtr[i][j] /  SL_RF_distanceDenPtr[i][j];
      }
      else {
        SL_RF_distancePtr[i][j] = SL_RF_nativeNaN;
      }
    }
  }
}
void finalizeDistanceHybrid(char mode) {
#ifdef _OPENMP
#pragma omp parallel for num_threads(SL_RF_numThreads)
#endif
  for (uint i = 1; i <= SL_RF_fobservationSize; i++) {
    for (uint j = 1; j <= SL_RF_observationSize; j++) {
      if (SL_RF_distanceDenPtr[i][j] > 0) {
        SL_RF_distancePtr[i][j] = SL_RF_distancePtr[i][j] /  SL_RF_distanceDenPtr[i][j];
      }
      else {
        SL_RF_distancePtr[i][j] = SL_RF_nativeNaN;
      }
    }
  }
}
void updateSplitDepth(uint treeID, Node *rootPtr, uint maxDepth) {
  Node  *parent;
  double *localSplitDepth;
  uint index;
  uint i, j, k;
  if (SL_RF_tLeafCount[treeID] > 0) {
    index = 0;  
    if (SL_RF_opt & (OPT_SPLDPTH_1 | OPT_SPLDPTH_2)) {
      if (SL_RF_opt & OPT_SPLDPTH_1) {
        index = 1;
      }
      else {
        index = treeID;
      }
    }
    else {
      SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
      SL_RF_nativeError("\nRF-SRC:  Illegal updateSplitDepth() call.");
      SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
      SL_RF_nativeExit();
    }
    localSplitDepth = dvector(1, SL_RF_xSize);
    for (i = 1; i <= SL_RF_observationSize; i++) {
      for (j = 1; j <= SL_RF_xSize; j++) {
        localSplitDepth[j] = SL_RF_nativeNaN;
      }
      parent = SL_RF_tTermList[treeID][SL_RF_tTermMembership[treeID][i] -> nodeID] -> mate;
      for (k = 1; k <= parent -> depth; k++) {
        if (SL_RF_nativeIsNaN(localSplitDepth[(parent -> splitDepth)[k]])) {
          localSplitDepth[(parent -> splitDepth)[k]] = (double) k;
        }
      }
      for (j = 1; j <= SL_RF_xSize; j++) {
        if (SL_RF_nativeIsNaN(localSplitDepth[j])) {
          localSplitDepth[j] = (double) maxDepth + 1;
        }
      }
      if (SL_RF_opt & OPT_SPLDPTH_1) {
#ifdef _OPENMP
#pragma omp critical (_update_splitdepth)
#endif
        {  
          for (j = 1; j <= SL_RF_xSize; j++) {
            SL_RF_splitDepthPtr[index][j][i] += localSplitDepth[j];
          }
        }
      }
      else {
        for (j = 1; j <= SL_RF_xSize; j++) {
          SL_RF_splitDepthPtr[index][j][i] += localSplitDepth[j];
        }
      }
    }
    free_dvector(localSplitDepth, 1, SL_RF_xSize);
    freeSplitPath(treeID);
  }
}
char pruneBranch(uint obsSize,
                 uint treeID,
                 Node **nodesAtDepth,
                 uint nadCount,
                 uint ptnTarget,
                 uint ptnCurrent) {
  char pruneFlag;
  uint i, j;
  pruneFlag = TRUE;
  double *varianceAtDepth =  dvector(1, nadCount);
  uint   *vadSortedIndex  = uivector(1, nadCount);
  for (i = 1; i <= nadCount; i++) {
    varianceAtDepth[i] = nodesAtDepth[i] -> variance;
  }
  indexx(nadCount, varianceAtDepth, vadSortedIndex);
  j = nadCount;
  while ((j >= 1) && pruneFlag) {
    nodesAtDepth[vadSortedIndex[j]] -> pseudoTerminal = TRUE;
    (nodesAtDepth[vadSortedIndex[j]] -> left)  -> pseudoTerminal = FALSE;
    (nodesAtDepth[vadSortedIndex[j]] -> right) -> pseudoTerminal = FALSE;
    for (i = 1; i <= obsSize; i++) {
      if ( (SL_RF_pNodeMembership[treeID][i] == nodesAtDepth[vadSortedIndex[j]] -> left) ||
           (SL_RF_pNodeMembership[treeID][i] == nodesAtDepth[vadSortedIndex[j]] -> right)) {
        SL_RF_pNodeMembership[treeID][i] = nodesAtDepth[vadSortedIndex[j]];
      }
    }
    j --;
    ptnCurrent --;
    if (ptnCurrent <= ptnTarget) {
      pruneFlag = FALSE;
    }
  }
  free_dvector(varianceAtDepth, 1, nadCount);
  free_uivector(vadSortedIndex, 1, nadCount);
  return pruneFlag;
}
uint pruneTree(uint obsSize, uint treeID, uint ptnTarget) {
  Node **nodesAtDepth;
  uint   ptnCurrent;
  uint   nadCount;
  uint   tagDepth;
  char   pruneFlag;
  uint   i;
  if (ptnTarget < 1) {
    SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    SL_RF_nativeError("\nRF-SRC:  Illegal target PTN count in pruneTree():  %10d", ptnTarget);
    SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
    SL_RF_nativeExit();
  }
  if (SL_RF_tLeafCount[treeID] == 0) {
    SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    SL_RF_nativeError("\nRF-SRC:  Illegal call to pruneTree() on a rejected tree:  %10d", treeID);
    SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
    SL_RF_nativeExit();
  }
  nodesAtDepth = (Node **) new_vvector(1, SL_RF_tLeafCount[treeID], NRUTIL_NPTR);
  ptnCurrent = SL_RF_tLeafCount[treeID];
  tagDepth = getMaximumDepth(SL_RF_root[treeID]) - 1;
  pruneFlag = (ptnCurrent > ptnTarget) && (tagDepth > 0);
  while (pruneFlag) {
    for (i = 1; i <= SL_RF_tLeafCount[treeID]; i++) {
      nodesAtDepth[i] = NULL;
    }
    nadCount = 0;
    getNodesAtDepth(SL_RF_root[treeID], tagDepth, nodesAtDepth, &nadCount);
    pruneFlag = pruneBranch(obsSize, treeID, nodesAtDepth, nadCount, ptnTarget, ptnCurrent);
    if(pruneFlag) {
      ptnCurrent -= nadCount;
      tagDepth --;
    }
    else {
      ptnCurrent = ptnTarget;
    }
  }
  free_new_vvector(nodesAtDepth, 1, SL_RF_tLeafCount[treeID], NRUTIL_NPTR);
  return ptnCurrent;
}
void stackAuxiliary(char mode, uint b) {
  uint obsSize;
  SL_RF_nodeMembership[b] = (Node **) new_vvector(1, SL_RF_observationSize, NRUTIL_NPTR);
  SL_RF_bootMembershipFlag[b] = cvector(1, SL_RF_observationSize);
  SL_RF_oobMembershipFlag[b] = cvector(1, SL_RF_observationSize);
  SL_RF_bootMembershipCount[b] = uivector(1, SL_RF_observationSize);
  SL_RF_ibgMembershipIndex[b] = uivector(1, SL_RF_observationSize);
  SL_RF_oobMembershipIndex[b] = uivector(1, SL_RF_observationSize);
  SL_RF_bootMembershipIndex[b] = uivector(1, SL_RF_identityMembershipIndexSize);
  if (mode == SL_RF_PRED) {
    SL_RF_fnodeMembership[b] = (Node **) new_vvector(1, SL_RF_fobservationSize, NRUTIL_NPTR);
  }
  switch (mode) {
  case SL_RF_PRED:
    obsSize = SL_RF_fobservationSize;
    break;
  default:
    obsSize = SL_RF_observationSize;
    break;
  }
  if (SL_RF_optHigh & OPT_MEMB_PRUN) {
    SL_RF_pNodeMembership[b] = (Node **) new_vvector(1, obsSize, NRUTIL_NPTR);
  }
}
void unstackAuxiliary(char mode, uint b) {
  uint obsSize;
  free_new_vvector(SL_RF_nodeMembership[b], 1, SL_RF_observationSize, NRUTIL_NPTR);
  free_cvector(SL_RF_bootMembershipFlag[b], 1, SL_RF_observationSize);
  free_cvector(SL_RF_oobMembershipFlag[b], 1, SL_RF_observationSize);
  free_uivector(SL_RF_bootMembershipCount[b], 1, SL_RF_observationSize);
  free_uivector(SL_RF_ibgMembershipIndex[b], 1, SL_RF_observationSize);
  free_uivector(SL_RF_oobMembershipIndex[b], 1, SL_RF_observationSize);
  free_uivector(SL_RF_bootMembershipIndex[b], 1, SL_RF_identityMembershipIndexSize);
  if (mode == SL_RF_PRED) {
    free_new_vvector(SL_RF_fnodeMembership[b],  1, SL_RF_fobservationSize, NRUTIL_NPTR);
  }
  switch (mode) {
  case SL_RF_PRED:
    obsSize = SL_RF_fobservationSize;
    break;
  default:
    obsSize = SL_RF_observationSize;
    break;
  }
  if (SL_RF_optHigh & OPT_MEMB_PRUN) {
    free_new_vvector(SL_RF_pNodeMembership[b], 1, obsSize, NRUTIL_NPTR);
  }
}
void printPseudoTNInfo(char mode, uint b) {
  uint i;
  SL_RF_pNodeList[b] = (Node **) new_vvector(1, SL_RF_pLeafCount[b] + 1, NRUTIL_NPTR);
  i = 0;
  getPTNodeList(SL_RF_root[b], SL_RF_pNodeList[b], &i);
  free_new_vvector(SL_RF_pNodeList[b], 1, SL_RF_pLeafCount[b] + 1, NRUTIL_NPTR);
}      
void getPTNodeList(Node    *parent,
                   Node   **list,
                   uint    *offset) {
  if (!(parent -> pseudoTerminal)) {
    getPTNodeList(parent ->  left, list, offset);
    getPTNodeList(parent -> right, list, offset);
  }
  else {
    (*offset) ++;
    list[*offset] = parent;
  }
}
void getSplitPath(uint treeID, Node *parent) {
  Node *reversePtr;
  uint i;
  if (!(SL_RF_opt & (OPT_SPLDPTH_1 | OPT_SPLDPTH_2))) {
    SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    SL_RF_nativeError("\nRF-SRC:  Call to calculate split depth without the option being active.");
    SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
    SL_RF_nativeExit();
  }
  if (parent -> depth > 0) {
    SL_RF_maxDepth[treeID] = (parent -> depth > SL_RF_maxDepth[treeID]) ? parent -> depth : SL_RF_maxDepth[treeID];
    stackSplitDepth(parent, parent -> depth);
    reversePtr = parent;
    for (i = 1; i <= parent -> depth; i++) {
      if ((reversePtr -> parent) == NULL) {
        SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
        SL_RF_nativeError("\nRF-SRC:  Reverse parsing of tree failed in restoreTree().");
        SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
        SL_RF_nativeExit();
      }
      (parent -> splitDepth)[(parent -> depth) - i + 1] = ((reversePtr -> parent) -> splitInfo) -> randomVar[1];
      reversePtr = reversePtr -> parent;
    }
  }
}
void freeSplitPath(uint treeID) {
  LeafLinkedObj *leafLinkedPtr;
  leafLinkedPtr = SL_RF_leafLinkedObjHead[treeID] -> fwdLink;
  while (leafLinkedPtr != NULL) {
    unstackSplitDepth(leafLinkedPtr -> nodePtr);
    leafLinkedPtr = leafLinkedPtr -> fwdLink;
  }
}
uint getMaximumDepth(Node *parent) {
  uint result, rLeft, rRight;
  result = parent -> depth;
  if (((parent -> left) != NULL) && ((parent -> right) != NULL)) {
    rLeft = getMaximumDepth(parent ->  left);
    rRight = getMaximumDepth(parent -> right);
    result = (rLeft > rRight) ? rLeft : rRight;
  }
  return result;
}
void getNodesAtDepth(Node *parent, uint tagDepth, Node **nodesAtDepth, uint *nadCount) {
  char recurseFlag;
  recurseFlag = TRUE;
  if (tagDepth == parent -> depth) {
    if (parent -> splitInfo != NULL) {
      (*nadCount) ++;
      nodesAtDepth[*nadCount] = parent;
    }
    recurseFlag = FALSE;
  }
  else {
    if (((parent -> left) == NULL) && ((parent -> right) == NULL)) {
      recurseFlag = FALSE;
    }
  }
  if (recurseFlag) {
    getNodesAtDepth(parent ->  left, tagDepth, nodesAtDepth, nadCount);
    getNodesAtDepth(parent -> right, tagDepth, nodesAtDepth, nadCount);
  }
}
