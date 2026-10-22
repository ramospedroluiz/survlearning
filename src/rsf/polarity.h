/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */
#ifndef SL_RF_POLARITY_H
#define SL_RF_POLARITY_H
#include "splitInfo.h"
char getDaughterPolaritySimpleFactor   (uint treeID, SplitInfo *info, uint index, void *value, ...);
char getDaughterPolaritySimpleNonFactor(uint treeID, SplitInfo *info, uint index, void *value, ...);
char getDaughterPolarity               (uint treeID, SplitInfo *info, uint index, void *value, ...);
char getDaughterPolaritySimpleFactorSingle(uint treeID, SplitInfo *info, uint index, void *value, ...);
char getDaughterPolaritySimpleNonFactorSingle(uint treeID, SplitInfo *info, uint index, void *value, ...);
#endif
