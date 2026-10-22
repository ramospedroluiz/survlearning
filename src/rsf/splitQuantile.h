/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */
#ifndef SL_RF_SPLIT_QUANTILE_H
#define SL_RF_SPLIT_QUANTILE_H
#include "node.h"
#include "splitInfo.h"
char locallyAdaptiveQuantileRegrSplit (uint       treeID,
                                       Node      *parent,
                                       SplitInfoMax *splitInfoMax,
                                       GreedyObj    *greedyMembr,
                                       char       multImpFlag);
char quantileRegrSplit (uint       treeID,
                        Node      *parent,
                        SplitInfoMax *splitInfoMax,
                        GreedyObj    *greedyMembr,
                        char       multImpFlag);
double quantile7 (double *r, uint s, double p);
#endif
