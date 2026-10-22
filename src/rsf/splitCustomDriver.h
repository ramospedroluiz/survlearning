/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */
#ifndef SL_RF_SPLIT_CUSTOM_DRIVER_H
#define SL_RF_SPLIT_CUSTOM_DRIVER_H
#include "splitInfo.h"
#include "node.h"
char customMultivariateSplit (uint       treeID,
                              Node      *parent,
                              SplitInfoMax *splitInfoMax,
                              GreedyObj    *greedyMembr,
                              char       multImpFlag);
char customSurvivalSplit (uint       treeID,
                          Node      *parent,
                          SplitInfoMax *splitInfoMax,
                          GreedyObj    *greedyMembr,
                          char       multImpFlag);
char customCompetingRiskSplit (uint       treeID,
                               Node      *parent,
                               SplitInfoMax *splitInfoMax,
                               GreedyObj    *greedyMembr,
                               char       multImpFlag);
#endif
