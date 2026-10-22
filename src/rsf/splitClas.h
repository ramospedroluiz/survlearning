/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */
#ifndef SL_RF_SPLIT_CLAS_H
#define SL_RF_SPLIT_CLAS_H
char classificationXwghtSplitCur (uint treeID, Node *parent, SplitInfoMax *splitInfoMax, GreedyObj *greedyMembr, char multImpFlag);
char classificationAreaUnderROCSplit (uint treeID, Node *parent, SplitInfoMax *splitInfoMax, GreedyObj *greedyMembr, char multImpFlag);
char classificationEntropySplit      (uint treeID, Node *parent, SplitInfoMax *splitInfoMax, GreedyObj *greedyMembr, char multImpFlag);
#endif
