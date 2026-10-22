/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */
#ifndef SL_RF_IMPORTANCE_PERM_H
#define SL_RF_IMPORTANCE_PERM_H
#include "terminal.h"
#include "node.h"
void getPermuteMembership(char       mode,
                          uint       treeID,
                          Terminal **vimpMembership,
                          uint       p);
Node *permuteMembershipGeneric(uint     treeID,
                               Node    *parent,
                               uint     individual,
                               uint     vimpX,
                               double **xArray);
Node *permuteMembershipJIT(uint     treeID,
                           Node    *parent,
                           uint     individual,
                           uint     vimpX,
                           double **xArray);
Node *getMembershipGeneric(uint     treeID,
                           Node    *parent,
                           uint     individual,
                           double **xArray);
Node *getMembershipJIT(uint     treeID,
                       Node    *parent,
                       uint     individual,
                       double **xArray);
void permute(uint ranGenID, uint parallelID, uint n, uint *indx);
#endif
