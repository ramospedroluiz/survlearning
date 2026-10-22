/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */
#ifndef SL_RF_SPLIT_MAHALANOBIS_H
#define SL_RF_SPLIT_MAHALANOBIS_H
#include "node.h"
#include "splitInfo.h"
char mahalanobis (uint       treeID,
                  Node      *parent,
                  SplitInfoMax *splitInfoMax,
                  GreedyObj    *greedyMembr,
                  char       multImpFlag);
#endif
