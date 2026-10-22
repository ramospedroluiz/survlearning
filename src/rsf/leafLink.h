/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */
#ifndef SL_RF_LEAF_LINK_H
#define SL_RF_LEAF_LINK_H
#include "terminal.h"
#include "node.h"
typedef struct leafLinkedObj LeafLinkedObj;
struct leafLinkedObj {
  struct leafLinkedObj *fwdLink;
  struct leafLinkedObj *bakLink;
  struct node     *nodePtr;
  struct terminal *termPtr;
  struct terminal *termPtrAux;
  uint nodeID;
  uint ibgMembrCount;
  uint allMembrCount;
  uint oobMembrCount;
};
typedef struct leafLinkedObjSimple LeafLinkedObjSimple;
struct leafLinkedObjSimple {
  struct leafLinkedObjSimple *fwdLink;
  struct leafLinkedObjSimple *bakLink;
  struct node     *nodePtr;
};
LeafLinkedObj *makeLeafLinkedObj(void);
LeafLinkedObjSimple *makeLeafLinkedObjSimple(void);
LeafLinkedObj *makeAndSpliceLeafLinkedObj(LeafLinkedObj *tail,
                                          Node *nodePtr,
                                          uint ibgCount,
                                          uint allCount);
LeafLinkedObjSimple *makeAndSpliceLeafLinkedObjSimple(LeafLinkedObjSimple *tail,
                                                      Node *nodePtr);
void freeLeafLinkedObj(LeafLinkedObj *obj);
void freeLeafLinkedObjSimple(LeafLinkedObjSimple *obj);
void freeLeafLinkedObjList(LeafLinkedObj *obj);
void freeLeafLinkedObjListRev(LeafLinkedObj *obj);
#endif
