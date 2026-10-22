/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "splitGreedy.h"
#include "splitUtil.h"
#include "treeUtil.h"
#include "polarity.h"
#include "factorOps.h"
#include "nodeOps.h"
#include "nrutil.h"
#include "error.h"
char summarizeSplitResultGreedy(SplitInfo *info) {
  char result;
  result = FALSE;
  if (info != NULL) {
    result = TRUE;
  }
  return result;
}
SplitInfo *makeSplitInfo(uint size) {
  SplitInfo *info = (SplitInfo*) gblock((size_t) sizeof(SplitInfo));
  info -> size = size;
  if (size > 0) {
   info -> indicator = cvector(1, size);
  }
  else {
    info -> indicator = NULL;
  }
  info -> mwcpSizeAbs    = NULL;
  info -> randomVar      = NULL;
  info -> randomPts      = NULL;
  return info;
}
void freeSplitInfo(SplitInfo *info) {
  uint adj;
  uint j;
  if (info -> size > 0) {
    if(info -> indicator != NULL) {
      free_cvector(info -> indicator, 1, info -> size);
    }
  }
  adj = 1;
  if (info -> mwcpSizeAbs != NULL) {
    for (j = 1; j <= adj; j++) {
      if (info -> mwcpSizeAbs[j] > 0) {
        free_uivector((uint *) ((info -> randomPts)[j]), 1, info -> mwcpSizeAbs[j]);
      }
      else {
        free_dvector((double *) ((info -> randomPts)[j]), 1, 1);
      }
    }
    free_uivector(info -> mwcpSizeAbs, 1, adj);
    free_ivector(info -> randomVar, 1, adj);
    free_new_vvector(info -> randomPts, 1, adj, NRUTIL_VPTR);
  }
  free_gblock(info, (size_t) sizeof(SplitInfo));
}
SplitInfoMax *makeSplitInfoMax(uint size) {
  SplitInfoMax *info = (SplitInfoMax*) gblock((size_t) sizeof(SplitInfoMax));
  info -> size = size;
  if (size > 0) {
   info -> indicator = cvector(1, size);
  }
  else {
    info -> indicator = NULL;
  }
  info -> deltaMax              = SL_RF_nativeNaN;
  info -> splitParameterMax     = 0;
  info -> splitValueMaxCont     = SL_RF_nativeNaN;
  info -> splitValueMaxFactSize = 0;
  info -> splitValueMaxFactPtr  = NULL;
  info -> splitStatistic        = SL_RF_nativeNaN;
  return info;
}
void freeSplitInfoMax(SplitInfoMax *info) {
  if (info -> size > 0) {
    if(info -> indicator != NULL) {
      free_cvector(info -> indicator, 1, info -> size);
    }
  }
  if (info -> splitValueMaxFactSize > 0) {
    free_uivector(info -> splitValueMaxFactPtr, 1, info -> splitValueMaxFactSize);
  }
  free_gblock(info, (size_t) sizeof(SplitInfoMax));
}
char forkAndUpdateGeneric(uint       treeID,
                          Node      *parent,
                          uint      *repMembrIndx,
                          uint       repMembrSize,
                          uint      *allMembrIndx,
                          uint       allMembrSize,
                          char       multImpFlag,
                          SplitInfo *info,
                          uint      *leafCount,
                          Node     **nodeMembership) {
  uint *leftAllMembrIndx, *rghtAllMembrIndx;
  uint *leftRepMembrIndx, *rghtRepMembrIndx;
  uint  leftRepMembrSize,  rghtRepMembrSize;
  char daughterFlag;
  char result;
  void *obsLocal;
  char (*getDaughterPolarityGeneric) (uint       treeID,
                                      SplitInfo *info,
                                      uint       index,
                                      void      *value,
                                      ...);
  uint i;
  getDaughterPolarityGeneric = NULL;  
  result = forkNode(parent, info);
  if (result == TRUE) {
    char *indicator = cvector(1, SL_RF_observationSize);
    (*leafCount) ++;
    ((parent -> left) -> nodeID) = (parent -> nodeID);
    ((parent -> right) -> nodeID) = *leafCount;
    ((parent -> left) -> depth) = parent -> depth + 1;
    ((parent -> right) -> depth) = parent -> depth + 1;
    if (info -> indicator != NULL) {
      for (i = 1; i <= repMembrSize; i++) {
        indicator[repMembrIndx[i]] = info -> indicator[i];
      }
    }
        obsLocal = SL_RF_observation[treeID][info -> randomVar[1]];
        if (info -> mwcpSizeAbs[1] > 0) {
          getDaughterPolarityGeneric = &getDaughterPolaritySimpleFactor;
        }
        else {
          getDaughterPolarityGeneric = &getDaughterPolaritySimpleNonFactor;
        }
    (parent -> left)  -> allMembrSizeAlloc = allMembrSize;
    (parent -> right) -> allMembrSizeAlloc = allMembrSize;
    (parent -> left)  -> allMembrIndx = leftAllMembrIndx  = uivector(1, allMembrSize);
    (parent -> right) -> allMembrIndx = rghtAllMembrIndx  = uivector(1, allMembrSize);
    uint leftSize, rghtSize;
    leftSize = rghtSize = 0;
    for (i = 1; i <= allMembrSize; i++) {
      daughterFlag = getDaughterPolarityGeneric(treeID,
                                                info,
                                                allMembrIndx[i],
                                                obsLocal,
                                                parent,
                                                SL_RF_GROW);
      indicator[allMembrIndx[i]] = daughterFlag;
      if (daughterFlag == LEFT) {
        leftAllMembrIndx[++leftSize] = allMembrIndx[i];
      }
      else if (daughterFlag == RIGHT) {
        rghtAllMembrIndx[++rghtSize] = allMembrIndx[i];
      }
      else {
        leftAllMembrIndx[++leftSize] = allMembrIndx[i];
        rghtAllMembrIndx[++rghtSize] = allMembrIndx[i];
      }
    } 
    (parent -> left)  -> allMembrSize = leftSize;
    (parent -> right) -> allMembrSize = rghtSize;
    (parent -> left)  -> repMembrSizeAlloc = repMembrSize;
    (parent -> right) -> repMembrSizeAlloc = repMembrSize;
    (parent -> left)  -> repMembrIndx = leftRepMembrIndx  = uivector(1, repMembrSize);
    (parent -> right) -> repMembrIndx = rghtRepMembrIndx  = uivector(1, repMembrSize);
    leftRepMembrSize = rghtRepMembrSize = 0;
    for (i = 1; i <= repMembrSize; i++) {
      if (indicator[repMembrIndx[i]] == LEFT) {
        leftRepMembrIndx[++leftRepMembrSize] = repMembrIndx[i];
      }
      else if (indicator[repMembrIndx[i]] == RIGHT) {
        rghtRepMembrIndx[++rghtRepMembrSize] = repMembrIndx[i];
      }
      else {
        leftRepMembrIndx[++leftRepMembrSize] = repMembrIndx[i];
        rghtRepMembrIndx[++rghtRepMembrSize] = repMembrIndx[i];
      }
    }
    (parent ->  left) -> repMembrSize = leftRepMembrSize;
    (parent -> right) -> repMembrSize = rghtRepMembrSize;
    if ((leftRepMembrSize == 0) || (rghtRepMembrSize == 0)) {
      SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
      SL_RF_nativeError("\nRF-SRC:  Left or Right Daughter of size zero:  (%10d, %10d)", leftRepMembrSize, rghtRepMembrSize);
      SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
      SL_RF_nativeExit();
    }
    free_cvector(indicator, 1, SL_RF_observationSize);
  }
  else {
    SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    SL_RF_nativeError("\nRF-SRC:  forkNode() failed.");
    SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
    SL_RF_nativeExit();
  }
  if (info -> size > 0) {
    if (info -> indicator != NULL) {
      free_cvector(info -> indicator, 1, info -> size);
      info -> indicator = NULL;
      info -> size = 0;
    }
  }
  return result;
}
char forkNode(Node      *parent,
              SplitInfo *info) {
  unsigned int i, j;
  if (parent == NULL) {
    SL_RF_nativePrint("\nRF-SRC:  *** WARNING *** ");
    SL_RF_nativePrint("\nRF-SRC:  Inconsistent call to forkNode().  ");
    SL_RF_nativePrint("\nRF-SRC:  The parent node is NULL.");
    return FALSE;
  }
  if (((parent -> left) != NULL) && ((parent -> right) != NULL)) {
    SL_RF_nativePrint("\nRF-SRC:  *** WARNING *** ");
    SL_RF_nativePrint("\nRF-SRC:  Inconsistent call to forkNode().  ");
    SL_RF_nativePrint("\nRF-SRC:  The daughter nodes are NON-NULL.");
    return FALSE;
  }
  if (parent -> splitFlag == FALSE) {
    SL_RF_nativePrint("\nRF-SRC:  *** WARNING *** ");
    SL_RF_nativePrint("\nRF-SRC:  Inconsistent call to forkNode().  ");
    SL_RF_nativePrint("\nRF-SRC:  The split flag is FALSE.");
    return FALSE;
  }
  Node *left  = makeNode(parent -> xSize);
  Node *right = makeNode(parent -> xSize);
  parent -> splitInfo = info;
  setParent(left, parent);
  setParent(right, parent);
  setLeftDaughter(left, parent);
  setRightDaughter(right, parent);
  if (parent -> xSize > 0) {
    for (i = 1; i <= parent -> xSize; i++) {
      left  -> permissible[i] = right -> permissible[i] = parent -> permissible[i];
    }
    if (parent -> permissibleReIndxFlag == FALSE) {
      for (i = 1; i <= parent -> permissibleIndxSize; i++) {
        left  -> permissibleIndx[i] = right -> permissibleIndx[i] = parent -> permissibleIndx[i];
      }
      left  -> permissibleIndxSize = right -> permissibleIndxSize = parent -> permissibleIndxSize;
      left  -> permissibleReIndxFlag = right -> permissibleReIndxFlag = FALSE;
    }
    else {
      j = 0;
      for (i = 1; i <= parent -> xSize; i++) {
        if ((parent -> permissible)[i] == TRUE) {
          ++j;
          left -> permissibleIndx[j] = right -> permissibleIndx[j] = i;
        }
      }
      left  -> permissibleIndxSize = right -> permissibleIndxSize = j;
      left  -> permissibleReIndxFlag = right -> permissibleReIndxFlag = FALSE;      
    }
    free_cvector(parent -> permissible, 1, parent -> xSize);
    free_uivector(parent -> permissibleIndx, 1, parent -> xSize);
    parent -> permissible = NULL;
    parent -> permissibleIndx = NULL;
    parent -> permissibleIndxSize = 0;
  }
  parent -> splitFlag = FALSE;
  return TRUE;
}
void saveTree(uint b, Node *parent, uint *offset) {
  uint adj;
  uint i, k;
  (*offset) ++;
  parent -> bnodeID = *offset;
  SL_RF_treeID_ptr[b][*offset] = b;
  SL_RF_nodeID_ptr[b][*offset] = parent -> nodeID;
  SL_RF_nodeSZ_ptr[b][*offset] = parent -> repMembrSize;
  if (parent -> splitInfo == NULL) {
    adj = 1;
    for (k = 1; k <= adj; k++) {
      SL_RF_parmID_ptr[b][k][*offset] = 0;
      SL_RF_contPT_ptr[b][k][*offset] = SL_RF_nativeNaN;
      SL_RF_mwcpSZ_ptr[b][k][*offset] = 0;
      SL_RF_fsrecID_ptr[b][k][*offset] = 0;
    }
  }
  else {
    adj = 1;
    for (k = 1; k <= adj; k++) {
      SL_RF_parmID_ptr[b][k][*offset] = ((parent -> splitInfo) -> randomVar)[k];
      SL_RF_mwcpSZ_ptr[b][k][*offset] = ((parent -> splitInfo) -> mwcpSizeAbs)[k];
      if (SL_RF_mwcpSZ_ptr[b][k][*offset] > 0) {
        SL_RF_fsrecID_ptr[b][k][*offset] = SL_RF_mwcpCT_ptr[b][k] + 1;
        for (i = 1; i <= SL_RF_mwcpSZ_ptr[b][k][*offset]; i++) {
          SL_RF_mwcpCT_ptr[b][k] ++;
          SL_RF_mwcpPT_ptr[b][k][SL_RF_mwcpCT_ptr[b][k]] = ((uint *) ((parent -> splitInfo) -> randomPts)[k])[i];
        }
        SL_RF_contPT_ptr[b][k][*offset] = SL_RF_nativeNaN;
      }
      else {
        SL_RF_fsrecID_ptr[b][k][*offset] = 0;
        SL_RF_contPT_ptr[b][k][*offset] = ((double *) ((parent -> splitInfo) -> randomPts)[k])[1];
      }
    }  
  }
  if (((parent -> left) != NULL) && ((parent -> right) != NULL)) {
    saveTree(b, parent ->  left, offset);
    SL_RF_blnodeID_ptr[b][parent -> bnodeID] = (parent -> left) -> bnodeID;
    saveTree(b, parent -> right, offset);
    SL_RF_brnodeID_ptr[b][parent -> bnodeID] = (parent -> right) -> bnodeID;
  }
  else {
    SL_RF_blnodeID_ptr[b][parent -> bnodeID] = 0;
    SL_RF_brnodeID_ptr[b][parent -> bnodeID] = 0;
  }
}
void restoreTree(char mode, uint b, Node *parent) {
  ulong *offset;
  SplitInfo *info;
  uint adj;
  uint i, k;
  offset = &SL_RF_restoreTreeOffset[b];
  if (b != SL_RF_treeID_[*offset]) {
    SL_RF_nativeError("\nRF-SRC:  Diagnostic Trace of Tree Record:  \n");
    SL_RF_nativeError("\nRF-SRC:      treeID     nodeID ");
    SL_RF_nativeError("\nRF-SRC:  %10d %10d \n", SL_RF_treeID_[*offset], SL_RF_nodeID_[*offset]);
    SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    SL_RF_nativeError("\nRF-SRC:  Invalid forest input record in tree:  %10d", b);
    SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
    SL_RF_nativeExit();
  }
  if (parent -> parent != NULL) {
    parent -> depth = (parent -> parent) -> depth + 1;
  }
  parent -> left  = NULL;
  parent -> right = NULL;
  parent -> splitFlag = FALSE;
  parent -> nodeID = SL_RF_nodeID_[*offset];
  parent -> repMembrSize = SL_RF_nodeSZ_[*offset];
  if (SL_RF_parmID_[1][*offset] != 0) {
    info = parent -> splitInfo = makeSplitInfo(0);
    adj = 1;
    info -> mwcpSizeAbs = uivector(1, adj);
    info -> randomVar   = ivector(1, adj);
    info -> randomPts   = new_vvector(1, adj, NRUTIL_VPTR);
    for (k = 1; k <= adj; k++) {
      info -> randomVar[k] = SL_RF_parmID_[k][*offset];
      info -> mwcpSizeAbs[k] = SL_RF_mwcpSZ_[k][*offset];
      if (SL_RF_mwcpSZ_[k][*offset] > 0) {
        info -> randomPts[k] = uivector(1, SL_RF_mwcpSZ_[k][*offset]);
        for (i = 1; i <= SL_RF_mwcpSZ_[k][*offset]; i++) {
          SL_RF_restoreMWCPoffset[k][b] ++;
          ((uint *) info -> randomPts[k])[i] = SL_RF_mwcpPT_[k][SL_RF_restoreMWCPoffset[k][b]];
        }
      }
      else {
        info -> randomPts[k] = dvector(1, 1);
        ((double *) info -> randomPts[k])[1] =  SL_RF_contPT_[k][*offset];
      }
    }
  }
  else {
    parent -> splitInfo = NULL;
  }
  (*offset) ++;
  if (parent -> splitInfo != NULL) {
    parent -> left  = makeNode(0);
    setParent(parent ->  left, parent);
    restoreTree(mode, b, parent -> left);
    parent -> right = makeNode(0);
    setParent(parent -> right, parent);
    restoreTree(mode, b, parent -> right);
  }
  else {
  }
}
void integerToHexString(uint n, char *s) {
    const char hex_lookup[] = "0123456789ABCDEF";
    int len = numHexDigits(n);
    if (len & 1) {
        *s++ = '0';
    }
    s[len] = '\0';
    for (--len; len >= 0; n >>= 4, --len) {
        s[len] = hex_lookup[n & 0xf];
    }
}
uint numHexDigits(unsigned n) {
  if (!n) return 1;
  uint ret = 0;
  for (; n; n >>= 4) {
        ++ret;
    }
    return ret;
}
double standardVector(uint       treeID,
                      char       standardFlag,
                      GreedyObj *greedyMembr,
                      double    *rawVector,
                      uint      *repMembrIndx,
                      uint       repMembrSize) {
  uint i;
  double mean;
  double stdDeviation;
  double result;
  mean         = SL_RF_nativeNaN;    
  stdDeviation = SL_RF_nativeNaN;    
  result       = SL_RF_nativeNaN;    
  if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
  }
  else {
    if (SL_RF_rNonFactorCount > 0) {
      greedyMembr -> standardResponse = dvector(1, SL_RF_observationSize);
      mean = 0.0;
      for (i = 1; i <= repMembrSize; i++) {
        mean += rawVector[ repMembrIndx[i] ];
      }
      mean = mean / repMembrSize;
      result = 0.0;
      for (i = 1; i <= repMembrSize; i++) {      
        result += pow (rawVector[ repMembrIndx[i] ] - mean, 2.0);
      }
      if (standardFlag) {
        stdDeviation = sqrt(result / repMembrSize);
        for (i = 1; i <= repMembrSize; i++) {
          greedyMembr -> standardResponse[repMembrIndx[i]] = ( rawVector[repMembrIndx[i] ] - mean) / stdDeviation;
        }
        result = 0.0;
        for (i = 1; i <= repMembrSize; i++) {      
          result += pow (greedyMembr -> standardResponse[repMembrIndx[i]], 2.0);
        }
      }
      else {
        for (i = 1; i <= repMembrSize; i++) {
          greedyMembr -> standardResponse[repMembrIndx[i]] = rawVector[repMembrIndx[i] ];
        }
      }
    }
  }
  return result;
}
double getL2Loss(uint    treeID,
                 double *response,
                 uint   *repMembrIndx,
                 uint    repMembrSize,
                 uint   *allMembrIndx,
                 uint    allMembrSize,
                 char   *membershipFlag,
                 char    selectFlag) {
  uint i;
  double localMean;
  double result;
  localMean = 0.0;
  for (i = 1; i <= repMembrSize; i++) {
    localMean += response[repMembrIndx[i]];
  }
  localMean = localMean / repMembrSize;
  result = 0.0;
  if (allMembrSize == 0) {
    for (i = 1; i <= repMembrSize; i++) {
      result += pow (response[repMembrIndx[i]] - localMean, 2.0);
    }
  }
  else {
    for (i = 1; i <= allMembrSize; i++) {
      if (membershipFlag[allMembrIndx[i]] == selectFlag) {
        result += pow (response[allMembrIndx[i]] - localMean, 2.0);
      }
    }
  }
  return result;
}
double getNegLogLikelihood(uint    treeID,
                           uint    maxLevel,
                           double *response,
                           uint   *repMembrIndx,
                           uint    repMembrSize,
                           uint   *allMembrIndx,
                           uint    allMembrSize,
                           char   *membershipFlag,
                           char    selectFlag) {
  uint i, k;
  double *piHat;
  double result;
  piHat = dvector(1, maxLevel);
  for (k = 1; k <= maxLevel; k++) {
    piHat[k] = 0.0;
  }
  for (i = 1; i <= repMembrSize; i++) {
    piHat[(uint) response[repMembrIndx[i]]] += 1.0; 
  }
  for (k = 1; k <= maxLevel; k++) {
    piHat[k] = piHat[k] / repMembrSize;
  }
  result = 0.0;
  if (allMembrSize == 0) {
    for (i = 1; i <= repMembrSize; i++) {
      if (piHat[(uint) response[repMembrIndx[i]]] > 0) {
        result -= log(piHat[(uint) response[repMembrIndx[i]]]);
      }
    }
  }
  else {
    for (i = 1; i <= allMembrSize; i++) {
      if (membershipFlag[allMembrIndx[i]] == selectFlag) {
        if (piHat[(uint) response[allMembrIndx[i]]] > 0) {
          result -= log(piHat[(uint) response[allMembrIndx[i]]]);
        }
      }
    }
  }
  free_dvector(piHat, 1, maxLevel);
  return result;
}
GreedyObj *makeGreedyObj(Node *parent, GreedyObj *head) {
  GreedyObj *greedyObj = (GreedyObj*) gblock((size_t) sizeof(GreedyObj));
  greedyObj -> parent = parent;
  greedyObj -> fwdLink = NULL;
  greedyObj -> bakLink = NULL;
  greedyObj -> head = head;
  greedyObj -> splitInfo = NULL;
  greedyObj -> G_nR_h_l  = SL_RF_nativeNaN;
  greedyObj -> G_nR_h_r  = SL_RF_nativeNaN;
  greedyObj -> sgStat    = SL_RF_nativeNaN;
  greedyObj -> inbagProxy = 0;
  greedyObj -> nodeID     = 0;
  greedyObj -> depth      = 0;
  greedyObj -> leafFlag = FALSE;
  greedyObj -> standardResponse = NULL;
  greedyObj -> membershipComplement = NULL;
  greedyObj -> eRisk = SL_RF_nativeNaN;
  greedyObj -> oobEmprRisk = SL_RF_nativeNaN;
  return greedyObj;
}
void freeGreedyObj(GreedyObj *gObj) {
  if (gObj -> splitInfo != NULL) {
    freeSplitInfo(gObj -> splitInfo);
  }
  if (gObj -> standardResponse != NULL) {
    free_dvector(gObj -> standardResponse, 1, SL_RF_observationSize);
  }
  free_gblock(gObj, (size_t) sizeof(GreedyObj));
}
void freeGreedyObjList(GreedyObj *gObj) {
  if (gObj -> fwdLink != NULL) {
    freeGreedyObjList(gObj -> fwdLink);
  }
  freeGreedyObj(gObj);
}
GreedyObj *findGreedyObj(GreedyObj *head, Node *parent) {
  GreedyObj *currentPtr = head;   
  char foundFlag = FALSE;
  while (!foundFlag) {
    if (currentPtr != NULL) {
      if(currentPtr -> parent == parent) {
        foundFlag = TRUE;
      }
      else {
        currentPtr = currentPtr -> fwdLink;
      }
    }
    else {
      foundFlag = TRUE;
    }
  }
  return currentPtr;
}
