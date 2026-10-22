/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */
#ifndef SL_RF_FACTOR_OPS_H
#define SL_RF_FACTOR_OPS_H
#include "factor.h"
Factor *makeFactor(uint r, char bookFlag);
void freeFactor(Factor *f);
char bookFactor(Factor *f);
char unbookFactor(Factor *f);
void bookPair (uint    levelCount,
               uint    groupIndex,
               uint    levelIndex,
               uint   *row,
               uint   *level,
               Factor *f);
void nChooseK (uint n, uint r, char type, void *result);
char reduceFraction(uint *numerator, uint *denominator);
char splitOnFactor(uint level, uint *mwcp);
#endif
