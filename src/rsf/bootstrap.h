/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */
#ifndef  SL_RF_BOOTSTRAP_H
#define  SL_RF_BOOTSTRAP_H
#include "node.h"
char bootstrap (char     mode,
                uint     treeID,
                Node    *nodePtr,
                uint    *subIndex,
                uint     subsetSize,
                uint    *index,
                uint     indexSize);
char getNodeSign (char mode, uint treeID, Node *nodePtr, uint *bmIndex, uint repMembrSize);
char bootstrapSubject (char     mode,
                       uint     treeID,
                       Node    *nodePtr,
                       uint   **index,
                       uint    *indexSize);
#endif
