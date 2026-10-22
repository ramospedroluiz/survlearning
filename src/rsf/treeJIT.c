/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "treeJIT.h"
#include "nodeOps.h"
#include "nrutil.h"
#include "splitGreedy.h"
#include "polarity.h"
#include "treeUtil.h"
#include "tree.h"
#include "processEnsemble.h"
#include "bootstrap.h"
#include "rfsrcUtil.h"
void acquireTreeJIT(char mode, uint r, uint treeID) {
  uint *bootMembrIndx;
  uint bootMembrSize;
  uint *membershipIndex;
  uint membershipSize;
  char result;
  uint i, ii;
#ifdef _OPENMP
#endif
  SL_RF_root[treeID] = makeNode(0);
  SL_RF_root[treeID] -> parent = NULL;
  SL_RF_root[treeID] -> nodeID = 0;
  result = (SL_RF_tLeafCount[treeID] > 0) ? TRUE : FALSE;
  if (result) {
    stackAuxiliary(mode, treeID);
    SL_RF_tTermList[treeID] = (Terminal **) new_vvector(1, SL_RF_tLeafCount[treeID], NRUTIL_TPTR);
    for (uint b = 1; b <= SL_RF_tLeafCount[treeID]; b++) {
      SL_RF_tTermList[treeID][b] = NULL;
    }
    SL_RF_tTermMembership[treeID] = (Terminal **) new_vvector(1, SL_RF_observationSize, NRUTIL_TPTR);
    SL_RF_leafLinkedObjHead[treeID] = SL_RF_leafLinkedObjTail[treeID] = makeLeafLinkedObj();
    if (mode == SL_RF_PRED) {
      SL_RF_ftTermMembership[treeID] = (Terminal **) new_vvector(1, SL_RF_fobservationSize, NRUTIL_TPTR);
    }
    if (mode == SL_RF_REST) {
      for (i = 1; i <= SL_RF_observationSize; i++) {
        SL_RF_nodeMembership[treeID][i] = SL_RF_root[treeID];
      }
      if ( (!(SL_RF_opt & OPT_BOOT_TYP1) && !(SL_RF_opt & OPT_BOOT_TYP2)) ||
           ( (SL_RF_opt & OPT_BOOT_TYP1) &&  (SL_RF_opt & OPT_BOOT_TYP2)) ) {
        bootMembrIndx  = uivector(1, SL_RF_bootstrapSize);
        bootMembrSize = SL_RF_bootstrapSize;
      }
      else {
        bootMembrIndx  = uivector(1, SL_RF_observationSize);
        bootMembrSize = SL_RF_observationSize;
      }
      bootstrap (mode,
                 treeID,
                 SL_RF_root[treeID],
                 SL_RF_identityMembershipIndex,
                 SL_RF_observationSize,
                 bootMembrIndx,
                 bootMembrSize);
      if (SL_RF_optHigh & OPT_WGHT) {
        membershipSize  = SL_RF_observationSize;
        membershipIndex = SL_RF_identityMembershipIndex;
      }
      else if ((SL_RF_opt & OPT_FENS) ||
               ((SL_RF_opt & OPT_PROX) && (SL_RF_opt & OPT_PROX_IBG) && (SL_RF_opt & OPT_PROX_OOB)) ||
               ((SL_RF_optHigh & OPT_DIST) && (SL_RF_optHigh & OPT_DIST_IBG) && (SL_RF_optHigh & OPT_DIST_OOB))) {
        membershipSize  = SL_RF_observationSize;
        membershipIndex = SL_RF_identityMembershipIndex;
      }
      else if ((SL_RF_opt & OPT_OENS) ||
               ((SL_RF_opt & OPT_PROX) && !(SL_RF_opt & OPT_PROX_IBG) && (SL_RF_opt & OPT_PROX_OOB)) ||
               ((SL_RF_optHigh & OPT_DIST) && !(SL_RF_optHigh & OPT_DIST_IBG) && (SL_RF_optHigh & OPT_DIST_OOB))) {
        membershipSize  = SL_RF_oobSize[treeID];
        membershipIndex = SL_RF_oobMembershipIndex[treeID];
      }
      else if (((SL_RF_opt & OPT_PROX) && (SL_RF_opt & OPT_PROX_IBG) && !(SL_RF_opt & OPT_PROX_OOB)) ||
               ((SL_RF_optHigh & OPT_DIST) && (SL_RF_optHigh & OPT_DIST_IBG) && !(SL_RF_optHigh & OPT_DIST_OOB))) {
        membershipSize  = SL_RF_ibgSize[treeID];
        membershipIndex = SL_RF_ibgMembershipIndex[treeID];
      }
      else {
        membershipSize  = SL_RF_observationSize;
        membershipIndex = SL_RF_identityMembershipIndex;
      }
      for (i = 1; i <= membershipSize; i++) {
        ii = membershipIndex[i];
        restoreTerminalNodeJIT(treeID, SL_RF_root[treeID], ii, SL_RF_observation[treeID], SL_RF_tTermMembership[treeID]);
      }
      if ( (!(SL_RF_opt & OPT_BOOT_TYP1) && !(SL_RF_opt & OPT_BOOT_TYP2)) ||
           ( (SL_RF_opt & OPT_BOOT_TYP1) &&  (SL_RF_opt & OPT_BOOT_TYP2)) ) {
        free_uivector(bootMembrIndx, 1, SL_RF_bootstrapSize);
      }
      else {
        free_uivector(bootMembrIndx, 1, SL_RF_observationSize);
      }
    }
    else {
      for (i = 1; i <= SL_RF_fobservationSize; i++) {
        restoreTerminalNodeJIT(treeID, SL_RF_root[treeID], i, SL_RF_fobservation[treeID], SL_RF_ftTermMembership[treeID]);
      }
      if (SL_RF_optHigh & OPT_WGHT) {
        for (i = 1; i <= SL_RF_observationSize; i++) {
          SL_RF_nodeMembership[treeID][i] = SL_RF_root[treeID];
        }
        if ( (!(SL_RF_opt & OPT_BOOT_TYP1) && !(SL_RF_opt & OPT_BOOT_TYP2)) ||
             ( (SL_RF_opt & OPT_BOOT_TYP1) &&  (SL_RF_opt & OPT_BOOT_TYP2)) ) {
          bootMembrIndx  = uivector(1, SL_RF_bootstrapSize);
          bootMembrSize = SL_RF_bootstrapSize;
        }
        else {
          bootMembrIndx  = uivector(1, SL_RF_observationSize);
          bootMembrSize = SL_RF_observationSize;
        }
        bootstrap (mode,
                   treeID,
                   SL_RF_root[treeID],
                   SL_RF_identityMembershipIndex,
                   SL_RF_observationSize,
                   bootMembrIndx,
                   bootMembrSize);
        for (i = 1; i <= SL_RF_ibgSize[treeID]; i++) {
          ii = SL_RF_ibgMembershipIndex[treeID][i];
          restoreTerminalNodeJIT(treeID, SL_RF_root[treeID], ii, SL_RF_observation[treeID], SL_RF_tTermMembership[treeID]);
        }
        if ( (!(SL_RF_opt & OPT_BOOT_TYP1) && !(SL_RF_opt & OPT_BOOT_TYP2)) ||
             ( (SL_RF_opt & OPT_BOOT_TYP1) &&  (SL_RF_opt & OPT_BOOT_TYP2)) ) {
          free_uivector(bootMembrIndx, 1, SL_RF_bootstrapSize);
        }
        else {
          free_uivector(bootMembrIndx, 1, SL_RF_observationSize);
        }
      }
    }
    processEnsembleInSitu(mode, FALSE, treeID);
    unstackAuxiliary(mode, treeID);
  }
  freeTree(treeID, SL_RF_root[treeID]);
  SL_RF_root[treeID] = NULL;
}
void restoreTerminalNodeJIT(uint treeID,
                            Node *root,
                            uint indv,
                            double **xArray,
                            Terminal **termMembership) {
  ulong rootIndex, nodeAbsIndex;
  uint  rmbrAbsOffset, ambrAbsOffset;
  uint  rmbrDummyIter, ambrDummyIter;
  ulong *rootMWCPoffset, *nodeAbsMWCPoffset;
  SplitInfo *info;
  char parseFlag;
  uint offset;
  char daughterFlag;
  char (*getDaughterPolarityGeneric) (uint       treeID,
                                      SplitInfo *info,
                                      uint       indv,
                                      void      *value,
                                      ...);
  void *gobsLocal;
  uint repMembrSize, leftRepMembrSize;
  uint adj, i, k;
  rmbrDummyIter = ambrDummyIter = 0;
  nodeAbsIndex = rootIndex = SL_RF_restoreTreeOffset[treeID];
  rmbrAbsOffset = ambrAbsOffset = 0;
    rootMWCPoffset    = ulvector(1, 1);
    nodeAbsMWCPoffset = ulvector(1, 1);
    rootMWCPoffset[1] = SL_RF_restoreMWCPoffset[1][treeID];
  parseFlag = TRUE;
  Node *parent = root;
  while (parseFlag) {
    if (parent -> nodeID == 0) {
      parent -> nodeID = SL_RF_nodeID_[nodeAbsIndex];
      parent -> repMembrSize = SL_RF_nodeSZ_[nodeAbsIndex];
      if (parent -> parent != NULL) {
        parent -> depth = (parent -> parent) -> depth + 1;
      }
      parent -> bnodeID = nodeAbsIndex - rootIndex + 1;
      if (SL_RF_parmID_[1][nodeAbsIndex] != 0) {
        parent -> blnodeID = parent -> bnodeID + 1;
        parent -> brnodeID = SL_RF_brnodeID_[nodeAbsIndex];
        info = parent -> splitInfo = makeSplitInfo(0);
        adj = 1;
        info -> mwcpSizeAbs = uivector(1, adj);
        info -> randomVar   = ivector(1, adj);
        info -> randomPts   = new_vvector(1, adj, NRUTIL_VPTR);
        for (k = 1; k <= adj; k++) {
          info -> randomVar[k] = SL_RF_parmID_[k][nodeAbsIndex];
          info -> mwcpSizeAbs[k] = SL_RF_mwcpSZ_[k][nodeAbsIndex];
          if (SL_RF_mwcpSZ_[k][nodeAbsIndex] > 0) {
            nodeAbsMWCPoffset[k] = rootMWCPoffset[k] + SL_RF_fsrecID_[k][nodeAbsIndex];
            info -> randomPts[k] = uivector(1, SL_RF_mwcpSZ_[k][nodeAbsIndex]);
            for (i = 1; i <= SL_RF_mwcpSZ_[k][nodeAbsIndex]; i++) {
              ((uint *) info -> randomPts[k])[i] = SL_RF_mwcpPT_[k][nodeAbsMWCPoffset[k]];
              nodeAbsMWCPoffset[k] ++;
            }
          }
          else {
            info -> randomPts[k] = dvector(1, 1);
            ((double *) info -> randomPts[k])[1] =  SL_RF_contPT_[k][nodeAbsIndex];
          }
        }
      }
      else {
        info = parent -> splitInfo = NULL;
      }
    }  
    else {
      info = parent -> splitInfo;
    }
    daughterFlag = NEITHER;
    if (info != NULL) {
      gobsLocal = (double **) xArray;
        gobsLocal = (double *) ((double **) gobsLocal)[info -> randomVar[1]];
        if (info -> mwcpSizeAbs[1] > 0) {
          getDaughterPolarityGeneric = &getDaughterPolaritySimpleFactor;
        }
        else {
          getDaughterPolarityGeneric = &getDaughterPolaritySimpleNonFactor;
        }
        if (SL_RF_fmRecordSize > 0) {
          if (SL_RF_fmRecordMap[indv] > 0) {
            offset = SL_RF_ySize + (info -> randomVar[1]);
            if (SL_RF_fmpSign[offset][SL_RF_fmRecordMap[indv]] == 1) {
              repMembrSize = parent -> repMembrSize;
              if (parent -> left != NULL) {
                leftRepMembrSize = (parent -> left) -> repMembrSize;
              }
              else {
                leftRepMembrSize = SL_RF_nodeSZ_[nodeAbsIndex + 1];
              }
              if (ran1B(treeID) <= (double) leftRepMembrSize / repMembrSize) {
                daughterFlag = LEFT;
              }
              else {
                daughterFlag = RIGHT;
              }
            }
          }
        }
      if (daughterFlag == NEITHER) {
        daughterFlag = getDaughterPolarityGeneric(treeID,
                                                  info,
                                                  indv,
                                                  gobsLocal,
                                                  parent,
                                                  SL_RF_PRED);
      }
      if (daughterFlag == LEFT) {
        nodeAbsIndex = nodeAbsIndex + 1;
        if (parent -> left == NULL) {
          parent -> left  = makeNode(0);
        }
        setParent(parent -> left, parent);
        parent = parent -> left;
      }
      else {
        nodeAbsIndex = rootIndex + (parent -> brnodeID) - 1;
        i = parent -> nodeID;
        while(i < SL_RF_nodeID_[nodeAbsIndex]) {
          rmbrAbsOffset = rmbrAbsOffset + SL_RF_TN_RCNT_ptr[treeID][i];
          ambrAbsOffset = ambrAbsOffset + SL_RF_TN_ACNT_ptr[treeID][i];
          i++;
        }
        if (parent -> right == NULL) {
          parent -> right = makeNode(0);
        }
        setParent(parent -> right, parent);
        parent = parent -> right;
      }
    }  
    else {
      if (SL_RF_tTermList[treeID][parent -> nodeID] != NULL) {
      }
      else {
        SL_RF_leafLinkedObjTail[treeID] = makeAndSpliceLeafLinkedObj(SL_RF_leafLinkedObjTail[treeID],
                                                                  parent,
                                                                  SL_RF_TN_RCNT_ptr[treeID][parent -> nodeID],
                                                                  SL_RF_TN_ACNT_ptr[treeID][parent -> nodeID]);
        SL_RF_tTermList[treeID][parent -> nodeID] = SL_RF_leafLinkedObjTail[treeID] -> termPtr;
        updateTerminalNodeOutcomes(SL_RF_PRED,
                                   treeID,
                                   parent -> mate,
                                   & SL_RF_RMBR_ID_ptr[treeID][rmbrAbsOffset],
                                   SL_RF_TN_RCNT_ptr[treeID][parent -> nodeID],
                                   & SL_RF_AMBR_ID_ptr[treeID][ambrAbsOffset],
                                   SL_RF_TN_ACNT_ptr[treeID][parent -> nodeID],
                                   & rmbrDummyIter,
                                   & ambrDummyIter);
      }
      termMembership[indv] = parent -> mate;
      if (SL_RF_optHigh & OPT_MEMB_USER) {
        SL_RF_MEMB_ID_ptr[treeID][indv] = parent -> nodeID;
      }
      parseFlag = FALSE;
    }
  }  
  free_ulvector(rootMWCPoffset, 1, 1);
  free_ulvector(nodeAbsMWCPoffset, 1, 1);
}
