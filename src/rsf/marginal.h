/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */
#ifndef SL_RF_MARGINAL_H
#define SL_RF_MARGINAL_H
#include "node.h"
void getMarginalMembership(char mode, uint treeID);
void releaseMarginalMembership(char mode, uint treeID);
void marginalMembership(uint     treeID,
                        Node    *parent,
                        uint    *gAllMembrIndx,
                        uint     gAllMembrSize,
                        uint     obsSize,
                        double **xArray);
#endif
