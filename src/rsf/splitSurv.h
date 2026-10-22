/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */
#ifndef SL_RF_SPLIT_SURV_H
#define SL_RF_SPLIT_SURV_H
#include "splitInfo.h"
#include "node.h"
char logRankNCR(uint       treeID,
                Node      *parent,
                SplitInfoMax *splitInfoMax,
                GreedyObj    *greedyMembr,
                char       multImpFlag);
char logRankCR(uint       treeID,
               Node      *parent,
               SplitInfoMax *splitInfoMax,
               GreedyObj    *greedyMembr,
               char       multImpFlag);
char wiBrierScore (uint       treeID,
                   Node      *parent,
                   SplitInfoMax *splitInfoMax,
                   GreedyObj    *greedyMembr,
                   char       multImpFlag);
char brierScoreGradient1 (uint       treeID,
                          Node      *parent,
                          SplitInfoMax *splitInfoMax,
                          GreedyObj    *greedyMembr,
                          char       multImpFlag);
#endif
