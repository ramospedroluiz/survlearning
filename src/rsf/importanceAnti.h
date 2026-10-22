/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */
#ifndef SL_RF_IMPORTANCE_ANTI_H
#define SL_RF_IMPORTANCE_ANTI_H
#include "terminal.h"
#include "node.h"
void getAntiMembership(char       mode,
                       uint       treeID,
                       Terminal **vimpMembership,
                       uint       p);
Node *antiMembershipGeneric(uint     treeID,
                            Node    *parent,
                            uint     individual,
                            uint     vimpX,
                            double **xArray);
Node *antiMembershipJIT(uint     treeID,
                        Node    *parent,
                        uint     individual,
                        uint     vimpX,
                        double **xArray);
#endif
