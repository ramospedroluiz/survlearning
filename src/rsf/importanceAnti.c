/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "importanceAnti.h"
#include "polarity.h"
#include "splitInfo.h"
#include "splitGreedy.h"
#include "rfsrcUtil.h"
#include "nodeOps.h"
#include "nrutil.h"
void getAntiMembership (char       mode,
                        uint       treeID,
                        Terminal **vimpMembership,
                        uint       p) {
  Node    *rootPtr;
  uint    *membershipIndex;
  uint     membershipSize;
  double **xArray;
  rootPtr = SL_RF_root[treeID];
  switch (mode) {
  case SL_RF_PRED:
    membershipSize = SL_RF_fobservationSize;
    membershipIndex = SL_RF_fidentityMembershipIndex;
    xArray = SL_RF_fobservation[treeID];
    break;
  default:
    membershipSize  = SL_RF_oobSize[treeID];
    membershipIndex = SL_RF_oobMembershipIndex[treeID];
    xArray = SL_RF_observation[treeID];
    break;
  }
  for (uint i = 1; i <= membershipSize; i++) {
    uint ii;
    ii = membershipIndex[i];
    vimpMembership[ii] = antiMembership(treeID, rootPtr, ii, p, xArray) -> mate;
  }
}
Node *antiMembershipGeneric(uint     treeID,
                            Node    *parent,
                            uint     individual,                            
                            uint     vimpX,
                            double **xArray) {
  char daughterFlag;
  char antiSplitFlag;
  Node *result;
  SplitInfo *info;
  double alpha;
  result = parent;
  if (((parent -> left) != NULL) && ((parent -> right) != NULL)) {
    info = parent -> splitInfo;
    antiSplitFlag = FALSE;
    if (vimpX > 0) {
      if ((uint) info -> randomVar[1] == vimpX) {
        antiSplitFlag = TRUE;
      }
    }
    else {
      if(SL_RF_importanceFlag[info -> randomVar[1]] == TRUE) {
        antiSplitFlag = TRUE;
      }
    }
    daughterFlag = getDaughterPolarity(0, info, individual, xArray);
    if(antiSplitFlag == TRUE) {
      alpha = ran1D(treeID);
      if (alpha <= SL_RF_vimpThreshold) {
        if (daughterFlag == LEFT) {
          daughterFlag = RIGHT;
        }
        else {
          daughterFlag = LEFT;
        }
      }
      else {
      }
    }  
    else {
    }  
    if (daughterFlag == LEFT) {
      result = antiMembershipGeneric(treeID, parent ->  left, individual, vimpX, xArray);
    }
    else {
      result = antiMembershipGeneric(treeID, parent -> right, individual, vimpX, xArray);
    }
  }
  return result;
}
Node *antiMembershipJIT(uint     treeID,
                        Node    *root,
                        uint     individual,
                        uint     vimpX,
                        double **xArray) {
  ulong rootIndex, nodeAbsIndex;
  uint  rmbrAbsOffset, ambrAbsOffset;
  uint rmbrDummyIter, ambrDummyIter;
  ulong *rootMWCPoffset, *nodeAbsMWCPoffset;
  SplitInfo *info;
  char parseFlag;
  char daughterFlag;
  char antiSplitFlag;
  double alpha;
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
    if (info != NULL) {
      antiSplitFlag = FALSE;
      if (vimpX > 0) {
        if ((uint) info -> randomVar[1] == vimpX) {
          antiSplitFlag = TRUE;
        }
      }
      else {
        if(SL_RF_importanceFlag[info -> randomVar[1]] == TRUE) {
          antiSplitFlag = TRUE;
        }
      }
      daughterFlag = getDaughterPolarity(0, info, individual, xArray);
      if(antiSplitFlag == TRUE) {
        alpha = ran1D(treeID);
        if (alpha <= SL_RF_vimpThreshold) {
          if (daughterFlag == LEFT) {
            daughterFlag = RIGHT;
          }
          else {
            daughterFlag = LEFT;
          }
        }
        else {
        }
      }  
      else {
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
      parseFlag = FALSE;
    }
  }  
  free_ulvector(rootMWCPoffset, 1, 1);
  free_ulvector(nodeAbsMWCPoffset, 1, 1);
  return parent;
}
