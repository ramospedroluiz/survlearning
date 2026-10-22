/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */
#ifndef SL_RF_TREE_JIT_H
#define SL_RF_TREE_JIT_H
#include "node.h"
#include "terminal.h"
void acquireTreeJIT(char mode, uint r, uint treeID);
void restoreTerminalNodeJIT(uint treeID, Node *root, uint indv, double **xArray, Terminal **termMembership);
#endif
