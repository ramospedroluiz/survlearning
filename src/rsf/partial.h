/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */
#ifndef SL_RF_PARTIAL_H
#define SL_RF_PARTIAL_H
#include "node.h"
#include "terminal.h"
void getAndUpdatePartialMembership(uint treeID, Node *root);
void partialMembershipGeneric(uint       treeID,
                              Node      *parent,
                              uint       partialIndex,
                              uint      *allMembrIndx,
                              uint       allMembrSize,
                              double   **xArray,
                              Terminal **membership);
void partialMembershipJIT(uint       treeID,
                          Node      *root,
                          uint       partialIndex,
                          uint      *nullMembrIndx,
                          uint       individual,
                          double   **xArray,
                          Terminal **membership);
void updatePartialCalculations (uint       treeID,
                                uint       pVarIdx,
                                Terminal **partialMembership);
void summarizePartialCalculations(uint       treeID,
                                  uint       pVarIdx);
#endif
