/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "diagnostic.h"
#include "error.h"
#include "nrutil.h"
void getSplitObjectInfo(SplitInfo *info) {
  SL_RF_nativePrint("\nSplitInfo:  %20x \n", info);
  SL_RF_nativePrint("\n  info -> size        :    %20d", info -> size);
  SL_RF_nativePrint("\n  info -> indicator   : 0x %20x", info -> indicator);
  SL_RF_nativePrint("\n  info -> randomVar   : 0x %20x", info -> randomVar);
  SL_RF_nativePrint("\n  info -> mwcpSizeAbs : 0x %20x", info -> mwcpSizeAbs);
  SL_RF_nativePrint("\n  info -> randomPts   : 0x %20x", info -> randomPts);
  SL_RF_nativePrint("\n   x-variable:   %10d", info -> randomVar[1]);
  SL_RF_nativePrint("\n");
  int covariate = info -> randomVar[1];
  if (info -> mwcpSizeAbs[1] > 0) {
    SL_RF_nativePrint(" (cov = %10d, mwcpPT =", covariate);
    for (uint m = 1; m <= info -> mwcpSizeAbs[1]; m++) {
      SL_RF_nativePrint(" %10x", ((uint *) info -> randomPts[1])[m]);
    }
    SL_RF_nativePrint(")");
  }
  else {
    SL_RF_nativePrint(" (cov = %10d, spltPT = %12.4f) ", covariate, ((double *) info -> randomPts[1])[1]);
  }
  SL_RF_nativePrint("\n");
}
void getNodeInfo(Node *nodePtr) {
  SL_RF_nativePrint("\nNodeInfo:  (address, node) = (%20x, %10d)", nodePtr, nodePtr -> nodeID);
  if (nodePtr -> splitInfo != NULL) {
    getSplitObjectInfo(nodePtr -> splitInfo);
  }
  SL_RF_nativePrint("\nSplit Statistic \n");
  SL_RF_nativePrint(" %12.4f \n", nodePtr -> splitStatistic);
  SL_RF_nativePrint("\nNode Variance \n");
  SL_RF_nativePrint(" %12.4f \n", nodePtr -> variance);
  SL_RF_nativePrint("\nPermissible Flag Size:          %10d", nodePtr -> xSize);
  SL_RF_nativePrint("\n mpIndexSize   = %20d", nodePtr -> mpIndexSize);
  SL_RF_nativePrint("\n fmpIndexSize  = %20d", nodePtr -> fmpIndexSize);
  SL_RF_nativePrint("\n");
  SL_RF_nativePrint("\n mpSign       = %20x", nodePtr -> mpSign);
  SL_RF_nativePrint("\n fmpSign      = %20x", nodePtr -> fmpSign);
  SL_RF_nativePrint("\n");
  SL_RF_nativePrint("\n lmpIndexActualSize        = %20d", nodePtr -> lmpIndexActualSize);
  SL_RF_nativePrint("\n flmpIndexActualSize       = %20d", nodePtr -> flmpIndexActualSize);
  SL_RF_nativePrint("\n lmpIndexAllocSize         = %20d", nodePtr -> lmpIndexAllocSize);
  SL_RF_nativePrint("\n flmpIndexAllocSize        = %20d", nodePtr -> flmpIndexAllocSize);
  SL_RF_nativePrint("\n");
  SL_RF_nativePrint("\n lmpIndex            = %20x", nodePtr -> lmpIndex);
  SL_RF_nativePrint("\n flmpIndex           = %20x", nodePtr -> flmpIndex);
  SL_RF_nativePrint("\n");
}
void getTerminalInfo(Terminal *termPtr) {
  SL_RF_nativePrint("\nTerminalInfo:  %20x", termPtr);
  SL_RF_nativePrint("\n  nodeID: %10d", termPtr -> nodeID);
  SL_RF_nativePrint("\n");
  SL_RF_nativePrint("\n lmiIndex            = %20x", termPtr -> lmiIndex);
  SL_RF_nativePrint("\n lmiAllocSize        = %20d", termPtr -> lmiAllocSize);
  SL_RF_nativePrint("\n lmiSize             = %20d", termPtr -> lmiSize);
  SL_RF_nativePrint("\n lmiValue            = %20x", termPtr -> lmiValue);
  SL_RF_nativePrint("\n rnfCount            = %20d", termPtr -> rnfCount);
  SL_RF_nativePrint("\n meanResponse        = %20x", termPtr -> meanResponse);
  SL_RF_nativePrint("\n membrCount          = %20d", termPtr -> membrCount);
  SL_RF_nativePrint("\n membrStream         = %20x", termPtr -> membrStream);
}
Node *getTerminalNode(uint treeID, uint leaf) {
  uint i, j;
  Node *parent;
  parent = NULL;
  for (j = 1; j <= SL_RF_observationSize; j++) {
    if ((SL_RF_nodeMembership[treeID][j] -> nodeID) == leaf) {
      parent = SL_RF_nodeMembership[treeID][j];
      j = SL_RF_observationSize;
    }
  }
  if (parent == NULL) {
    SL_RF_nativePrint("\nDiagnostic Trace of (individual, boot, node, leaf) vectors in data set:  ");
    SL_RF_nativePrint("\n        index         boot         node         leaf \n");
    for (i = 1; i <= SL_RF_observationSize; i++) {
      SL_RF_nativePrint(" %12d %12d %12x %12d \n", i,
              SL_RF_bootMembershipFlag[treeID][i], SL_RF_nodeMembership[treeID][i],
              SL_RF_nodeMembership[treeID][i] -> nodeID);
    }
    SL_RF_nativePrint("\nDiagnostic State of TRAIN (SHADOW) data:  ");
    SL_RF_nativePrint("\n       index       status         time   observations -> \n");
    SL_RF_nativePrint("\n                                      ");
    for (i=1; i <= SL_RF_xSize; i++) {
      SL_RF_nativePrint(" %12d", i);
    }
    SL_RF_nativePrint("\n");
    for (j = 1; j <= SL_RF_observationSize; j++) {
      SL_RF_nativePrint("%12d %12.4f %12.4f", j, SL_RF_status[treeID][j], SL_RF_time[treeID][j]);
      for (i=1; i <= SL_RF_xSize; i++) {
        SL_RF_nativePrint(" %12.4f", (SL_RF_observation[treeID][i][j]));
      }
      SL_RF_nativePrint("\n");
    }
    SL_RF_nativePrint("\nDiagnostic State of TRAIN (INCOMING) data:  ");
    SL_RF_nativePrint("\n       index       status         time   observations -> \n");
    SL_RF_nativePrint("\n                                      ");
    for (i=1; i <= SL_RF_xSize; i++) {
      SL_RF_nativePrint(" %12d", i);
    }
    SL_RF_nativePrint("\n");
    for (j = 1; j <= SL_RF_observationSize; j++) {
      SL_RF_nativePrint("%12d %12.4f %12.4f", j, SL_RF_responseIn[SL_RF_statusIndex][j], SL_RF_responseIn[SL_RF_timeIndex][j]);
      for (i=1; i <= SL_RF_xSize; i++) {
        SL_RF_nativePrint(" %12.4f", (SL_RF_observationIn[i][j]));
      }
      SL_RF_nativePrint("\n");
    }
    SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    SL_RF_nativeError("\nRF-SRC:  Proxy member for (tree, node) = (%12d, %12d) not found.", treeID, leaf);
    SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
    SL_RF_nativeExit();
  }
  return parent;
}
void getRawNodeSize(uint  type,
                    uint  treeID,
                    Node *parent,
                    uint *repMembrIndx,
                    uint *repMembrSize,
                    uint *allMembrIndx,
                    uint *allMembrSize) {
  uint      obsSize;
  Node   ***nodeMembershipPtr;
  uint      bootMembrSize;
  uint i;
  obsSize           = 0;     
  nodeMembershipPtr = NULL;  
  switch (type) {
  case SL_RF_PRED:
    obsSize = SL_RF_fobservationSize;
    nodeMembershipPtr = SL_RF_fnodeMembership;
    break;
  default:
    obsSize = SL_RF_observationSize;
    nodeMembershipPtr = SL_RF_nodeMembership;
    break;
  }
  bootMembrSize = SL_RF_bootstrapSize;
  *repMembrSize = 0;
  for (i=1; i <= bootMembrSize; i++) {
    if (SL_RF_nodeMembership[treeID][SL_RF_bootMembershipIndex[treeID][i]] == parent) {
      repMembrIndx[++(*repMembrSize)] = SL_RF_bootMembershipIndex[treeID][i];
    }
  }
  *allMembrSize = 0;
  for (i=1; i <= obsSize; i++) {
    if (nodeMembershipPtr[treeID][i] == parent) {
      allMembrIndx[++(*allMembrSize)] = i;
    }
  }
}
void printTreeInfo(uint treeID, Node *parent) {
  if (((parent -> left) != NULL) && ((parent -> right) != NULL)) {
    printTreeInfo(treeID, parent ->  left);
    printTreeInfo(treeID, parent -> right);
  }
}
void printParameters(char mode) {
}
