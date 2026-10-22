/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */
#ifndef SL_RF_PROCESS_ENSEMBLE_H
#define SL_RF_PROCESS_ENSEMBLE_H
void stackFactorInSitu(uint treeID);
void unstackFactorInSitu(uint treeID);
void processEnsembleInSitu(char mode, char multImpFlag, uint b);
void processEnsemblePost(char mode);
void processEnsembleHoldout(uint xVarIdx, uint b);
void processEnsembleHoldoutPost(uint bb);
#endif
