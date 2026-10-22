/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "treeUtil.h"
#include "impute.h"
#include "splitGreedy.h"
#include "bootstrap.h"
#include "rfsrcUtil.h"
#include "tree.h"
#include "nrutil.h"
#include "error.h"
char growTreeRecursive (uint     r,
                        char     rootFlag,
                        char     multImpFlag,
                        uint     treeID,
                        Node    *parent,
                        uint    *bootMembrIndxIter,
                        uint    *rmbrIterator,
                        uint    *ambrIterator) {
  char  bootResult;
  char  splitResult;
  char  forkResult;
  char leftResult, rghtResult;
  char terminalFlag;
  uint *bootMembrIndx;
  uint bootMembrSize;
  SplitInfoMax *splitInfoMax;
  SplitInfo *splitInfo;
  uint *repMembrIndx, *allMembrIndx;
  uint  repMembrSize,  allMembrSize;
  uint i, p;
  bootResult = TRUE;
  terminalFlag = TRUE;
  splitInfo = NULL;
  allMembrIndx = parent -> allMembrIndx;
  allMembrSize = parent -> allMembrSize;
  repMembrIndx = parent -> repMembrIndx;
  repMembrSize = parent -> repMembrSize;
  if (rootFlag) {
    if ( (!(SL_RF_opt & OPT_BOOT_TYP1) && !(SL_RF_opt & OPT_BOOT_TYP2)) ||
         ( (SL_RF_opt & OPT_BOOT_TYP1) &&  (SL_RF_opt & OPT_BOOT_TYP2)) ) {
      bootMembrIndx  = uivector(1, SL_RF_bootstrapSize);
      bootMembrSize = SL_RF_bootstrapSize;
    }
    else {
      bootMembrIndx  = uivector(1, allMembrSize);
      bootMembrSize = allMembrSize;
    }
    bootResult = bootstrap (SL_RF_GROW,
                            treeID,
                            parent,
                            allMembrIndx,
                            allMembrSize,
                            bootMembrIndx,
                            bootMembrSize);
    repMembrIndx = parent -> repMembrIndx = bootMembrIndx;
    repMembrSize = parent -> repMembrSize = parent -> repMembrSizeAlloc = bootMembrSize;
    if (rootFlag & bootResult) {
      if (SL_RF_vtry > 0) {
        if (SL_RF_vtryMode == SL_RF_VTRY_NULL) {
          for (p = 1; p <= SL_RF_xSize; p++) {
            if (SL_RF_vtryArray[treeID][p] > 0) {
              (parent -> permissible)[p] = FALSE;
            }
          }
        }
        else {
          if (SL_RF_vtryMode == SL_RF_VTRY_HOLD) {
            for (p = 1; p <= SL_RF_xSize; p++) {
              if (SL_RF_vtryArray[treeID][p] > 0) {
                (parent -> permissible)[p] = FALSE;
              }
            }
          }
          else {
          }
        }
      }
      if (SL_RF_mRecordSize > 0) {
        for (p = 1; p <= SL_RF_mpIndexSize; p++) {
          if (SL_RF_mpIndex[p] > 0) {
            if (parent -> mpSign[p] == -1) {
              (parent -> permissible)[SL_RF_mpIndex[p]] = FALSE;
            }
          }
        }
      }  
      parent -> permissibleIndxSize = 0;
      for (p = 1; p <= SL_RF_xSize; p++) {
        if ((parent -> permissible)[p] == TRUE) {
          parent -> permissibleIndx[++ (parent -> permissibleIndxSize)] = p;
        }
      }
      parent -> permissibleReIndxFlag = FALSE;
      parent -> permissibleOwnershipFlag = TRUE;
    }  
  }  
  else {
    bootMembrIndx = repMembrIndx;
    bootMembrSize = repMembrSize;
    parent -> mpSign = (parent -> parent) -> mpSign;
  }
  if (bootResult) {
    if (multImpFlag == FALSE) {
      if (SL_RF_mRecordSize > 0) {
        imputeNode(SL_RF_GROW,
                   FALSE,  
                   TRUE,   
                   treeID,
                   parent,
                   repMembrIndx,
                   repMembrSize,
                   allMembrIndx,
                   allMembrSize);
        if (SL_RF_timeIndex > 0) {
          if (SL_RF_mTimeFlag == TRUE) {
            updateTimeIndexArray(treeID,
                                 allMembrIndx,
                                 allMembrSize,
                                 SL_RF_time[treeID],
                                 FALSE,
                                 FALSE,
                                 SL_RF_masterTimeIndex[treeID]);
          }
        }
      }
    }  
  }  
  if (bootResult) {
    if (rootFlag) {
      SL_RF_tLeafCount[treeID] = 1;
    }
    splitInfoMax = makeSplitInfoMax(0);
    splitResult = getBestSplit(treeID,
                               parent,
                               SL_RF_splitRule,
                               splitInfoMax,
                               multImpFlag);
    if (splitResult == TRUE) {
      splitInfo = makeSplitInfo(0);
      if (splitInfoMax -> size > 0) {
        if (splitInfoMax -> indicator != NULL) {
          splitInfo -> size = splitInfoMax -> size;
          splitInfo -> indicator = splitInfoMax -> indicator;
          splitInfoMax -> indicator = NULL;
          splitInfoMax -> size      = 0;
        }
      }
      else {
        splitInfo -> size = 0;
        splitInfo -> indicator = NULL;
      }
      splitInfo -> mwcpSizeAbs = uivector(1, 1);
      splitInfo -> randomVar   = ivector(1, 1);
      splitInfo -> randomPts   = new_vvector(1, 1, NRUTIL_VPTR);
      (splitInfo -> mwcpSizeAbs)[1] = splitInfoMax -> splitValueMaxFactSize;
      (splitInfo -> randomVar)[1] = splitInfoMax -> splitParameterMax;
      if ((splitInfo -> mwcpSizeAbs)[1] > 0) {
        (splitInfo -> randomPts)[1] = uivector(1, (splitInfo -> mwcpSizeAbs)[1]);
        for (i = 1; i <= (splitInfo -> mwcpSizeAbs)[1]; i++) {
          ((uint *) (splitInfo -> randomPts)[1])[i] = (splitInfoMax -> splitValueMaxFactPtr)[i];
        }
      }
      else {
        (splitInfo -> randomPts)[1] = dvector(1, 1);
        ((double *) (splitInfo -> randomPts)[1])[1] = splitInfoMax -> splitValueMaxCont;
      }
      freeSplitInfoMax(splitInfoMax);
      terminalFlag = FALSE;
      forkResult = forkAndUpdate(treeID,
                                 parent,
                                 repMembrIndx,
                                 repMembrSize,
                                 allMembrIndx,
                                 allMembrSize,
                                 multImpFlag,
                                 splitInfo,
                                 &SL_RF_tLeafCount[treeID],
                                 SL_RF_nodeMembership[treeID]);
      if (forkResult == TRUE) {
        leftResult = growTreeRecursive (r,
                               FALSE,
                               multImpFlag,
                               treeID,
                               parent -> left,
                               bootMembrIndxIter,
                               rmbrIterator,
                               ambrIterator);
        if(!leftResult) {
        }
        rghtResult = growTreeRecursive (r,
                               FALSE,
                               multImpFlag,
                               treeID,
                               parent -> right,
                               bootMembrIndxIter,
                               rmbrIterator,
                               ambrIterator);
        if(!rghtResult) {
        }
        free_uivector((parent -> left)  -> allMembrIndx, 1, (parent -> left)  -> allMembrSizeAlloc);
        free_uivector((parent -> right) -> allMembrIndx, 1, (parent -> right) -> allMembrSizeAlloc);
        (parent -> left) -> allMembrIndx = (parent -> right) -> allMembrIndx = NULL;
        (parent -> left) -> allMembrSize = (parent -> right) -> allMembrSize = 0;
        (parent -> left) -> allMembrSizeAlloc = (parent -> right) -> allMembrSizeAlloc = 0;
        free_uivector((parent -> left)  -> repMembrIndx, 1, (parent -> left)  -> repMembrSizeAlloc);
        free_uivector((parent -> right) -> repMembrIndx, 1, (parent -> right) -> repMembrSizeAlloc);
        (parent -> left) -> repMembrIndx = (parent -> right) -> repMembrIndx = NULL;
        (parent -> left) -> repMembrSizeAlloc = (parent -> right) -> repMembrSizeAlloc = 0;
      }
      else {
        SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
        SL_RF_nativeError("\nRF-SRC:  forkAndUpdate(%10d) failed.", treeID);
        SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
        SL_RF_nativeExit();
      }
    }  
    else {
      parent -> splitFlag = FALSE;
      free_cvector(parent -> permissible, 1, parent -> xSize);
      free_uivector(parent -> permissibleIndx, 1, parent -> xSize);
      parent -> permissible = NULL;
      parent -> permissibleIndx = NULL;
      parent -> permissibleIndxSize = 0;
      parent -> splitInfo = NULL;
      freeSplitInfoMax(splitInfoMax);
    }
  }  
  else {
    if (rootFlag) {
      if (!bootResult) {
        terminalFlag = FALSE;
      }
    }
  }  
  if (terminalFlag) {
    SL_RF_leafLinkedObjTail[treeID] = makeAndSpliceLeafLinkedObj(SL_RF_leafLinkedObjTail[treeID],
                                                              parent,
                                                              repMembrSize,
                                                              allMembrSize);
    parent -> pseudoTerminal = TRUE;
    if (SL_RF_opt & OPT_MISS_OUT) {
      imputeNodeAndSummarize(r,
                             SL_RF_GROW,
                             treeID,
                             parent,
                             repMembrIndx,
                             repMembrSize,
                             allMembrIndx,
                             allMembrSize,
                             NULL,
                             0);
    }
    if (r == SL_RF_nImpute) {
      if (SL_RF_optHigh & OPT_MEMB_USER) {
        for (i = 1; i <= allMembrSize; i++) {
          SL_RF_MEMB_ID_ptr[treeID][allMembrIndx[i]] = parent -> nodeID;
        }
      }
      if (SL_RF_optHigh & OPT_MEMB_OUTG) {
        Terminal *termPtr;
        uint iter;
        termPtr = SL_RF_leafLinkedObjTail[treeID] -> termPtr;
        termPtr -> oobMembrSizeAlloc = allMembrSize;
        termPtr -> oobMembrIndx = uivector(1, termPtr -> oobMembrSizeAlloc);      
        iter = 0;
        for (i = 1; i <= allMembrSize; i++) {
          if (SL_RF_bootMembershipFlag[treeID][allMembrIndx[i]] == FALSE) {
            termPtr -> oobMembrIndx[++iter] = allMembrIndx[i];
          }
        }
        termPtr -> oobMembrSize = iter;
        termPtr -> ibgMembrSizeAlloc = allMembrSize;
        termPtr -> ibgMembrIndx = uivector(1, termPtr -> ibgMembrSizeAlloc);      
        iter = 0;
        for (i = 1; i <= allMembrSize; i++) {
          if (SL_RF_bootMembershipFlag[treeID][allMembrIndx[i]] == TRUE) {
            termPtr -> ibgMembrIndx[++iter] = allMembrIndx[i];
          }
        }
        termPtr -> ibgMembrSize = iter;
      }
      updateTerminalNodeOutcomes(SL_RF_GROW,
                                 treeID,
                                 SL_RF_leafLinkedObjTail[treeID] -> termPtr,
                                 repMembrIndx,
                                 repMembrSize,
                                 allMembrIndx,
                                 allMembrSize,
                                 rmbrIterator,
                                 ambrIterator);
      if (SL_RF_opt & (OPT_SPLDPTH_1 | OPT_SPLDPTH_2)) {
        getSplitPath(treeID, parent);
      }
    }
    else {
      initTerminalNodeMembership(treeID,
                                 SL_RF_leafLinkedObjTail[treeID] -> termPtr,
                                 allMembrIndx,
                                 allMembrSize);
    }
  }  
  if (rootFlag) {
    if ( (!(SL_RF_opt & OPT_BOOT_TYP1) && !(SL_RF_opt & OPT_BOOT_TYP2)) ||
         ( (SL_RF_opt & OPT_BOOT_TYP1) &&  (SL_RF_opt & OPT_BOOT_TYP2)) ) {
      free_uivector(bootMembrIndx, 1, SL_RF_bootstrapSize);
    }
    else {
      free_uivector(bootMembrIndx, 1, allMembrSize);
    }
    parent -> repMembrIndx = NULL;
    parent -> repMembrSizeAlloc = 0;
  }
  return bootResult;
}
void freeTree(uint treeID, Node *parent) {
  if (parent != NULL) {
    if ((parent -> left) != NULL) {
      freeTree(treeID, parent -> left);
    }
    if ((parent -> right) != NULL) {
      freeTree(treeID, parent -> right);
    }
    freeNode(parent);
  }
}
void saveStatistics(char     mode,
                    uint     b,
                    Node    *parent,
                    uint    *offset,
                    double  *spltST,
                    uint    *dpthST) {
  spltST[++(*offset)] = parent -> splitStatistic;
  dpthST[(*offset)]   = parent -> depth;
  if (((parent -> left) != NULL) && ((parent -> right) != NULL)) {
    saveStatistics(mode, b, parent ->  left, offset, spltST, dpthST);
    saveStatistics(mode, b, parent -> right, offset, spltST, dpthST);
  }
}
void initTerminalNodeMembership(uint       treeID,
                                Terminal  *parent,
                                uint      *allMembrIndx,
                                uint       allMembrSize) {
  uint i;
  for (i = 1; i <= allMembrSize; i++) {
    SL_RF_tTermMembership[treeID][allMembrIndx[i]] = parent;
  }
}
void updatePruning(char mode, uint treeID) {
  Terminal ***gTermMembership;
  uint        obsSize;
  uint i;
  if (SL_RF_optHigh & OPT_MEMB_PRUN) {
    switch (mode) {
    case SL_RF_PRED:
      obsSize = SL_RF_fobservationSize;
      gTermMembership = SL_RF_ftTermMembership;
      break;
    default:
      obsSize = SL_RF_observationSize;
      gTermMembership = SL_RF_tTermMembership;
      break;
    }
    for (i = 1; i <= obsSize; i++) {
      SL_RF_pNodeMembership[treeID][i] = gTermMembership[treeID][i] -> mate;
    }
    SL_RF_pLeafCount[treeID] = pruneTree(obsSize, treeID, SL_RF_ptnCount);
    for (i=1; i <= obsSize; i++) {
      SL_RF_PRUN_ID_ptr[treeID][i] = SL_RF_pNodeMembership[treeID][i] -> nodeID;
    }
  }
}
void updateCaseDepth(char mode, uint treeID) {
  Terminal ***gTermMembership;
  uint        obsSize;
  uint i;
  if (SL_RF_opt & OPT_CASE_DPTH) {
    switch (mode) {
    case SL_RF_PRED:
      obsSize = SL_RF_fobservationSize;
      gTermMembership = SL_RF_ftTermMembership;
      break;
    default:
      obsSize = SL_RF_observationSize;
      gTermMembership = SL_RF_tTermMembership;
      break;
    }
    for (i = 1; i <= obsSize; i++) {
      SL_RF_CASE_DPTH_ptr[treeID][i] = gTermMembership[treeID][i] -> mate -> depth;
    }
  }
}
