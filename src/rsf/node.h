/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */
#ifndef SL_RF_NODE_H
#define SL_RF_NODE_H
typedef struct splitInfo SplitInfo;
typedef struct node Node;
struct node {
  unsigned int nodeID;
  unsigned int bnodeID;
  unsigned int blnodeID;
  unsigned int brnodeID;
  unsigned int fsrecID;
  struct node *parent;
  struct node *left;
  struct node *right;
  struct terminal *mate;
  unsigned int xSize;
  char *permissible;
  uint *permissibleIndx;
  uint  permissibleIndxSize;
  char  permissibleReIndxFlag;
  char  permissibleOwnershipFlag;
  char splitFlag;
  double splitStatistic;
  double mean;
  double variance;
  unsigned int depth;
  unsigned int *splitDepth;
  char pseudoTerminal;
  unsigned int mpIndexSize;
  unsigned int fmpIndexSize;
  int *mpSign;
  int *fmpSign;
  char imputed;
  unsigned int *lmpIndex;
  unsigned int  lmpIndexAllocSize, lmpIndexActualSize;
  double *lmpValue;
  unsigned int *flmpIndex;
  unsigned int  flmpIndexAllocSize, flmpIndexActualSize;
  double *flmpValue;
  struct splitInfo *splitInfo;
  unsigned int *repMembrIndx;
  unsigned int *allMembrIndx;
  unsigned int  repMembrSizeAlloc;
  unsigned int  allMembrSizeAlloc;
  unsigned int  repMembrSize;
  unsigned int  allMembrSize;
  unsigned int  oobMembrSizeAlloc;
  unsigned int  oobMembrSize;
  unsigned int *oobMembrIndx;
  unsigned int *nonMissMembrIndxStatic;
  unsigned int  nonMissMembrSizeStatic;
  unsigned int *nonMissMembrIndx;
  unsigned int  nonMissMembrSize;
  double sumRght;
};
#endif
