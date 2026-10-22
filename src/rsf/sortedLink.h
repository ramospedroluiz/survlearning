/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */
#ifndef SL_RF_SORTED_LINK_H
#define SL_RF_SORTED_LINK_H
typedef struct sortedLinkedObj SortedLinkedObj;
struct sortedLinkedObj {
  struct sortedLinkedObj *fwdLink;
  struct sortedLinkedObj *bakLink;
  uint rank;
  uint indx;
};
SortedLinkedObj *makeSortedLinkedObj(void);
void makeAndSpliceSortedLinkedObj(uint treeID,
                                  SortedLinkedObj **headPtr,
                                  SortedLinkedObj **tailPtr,
                                  uint *listLength,
                                  uint rank, uint indx);
void freeSortedLinkedObjList(SortedLinkedObj *obj);
void freeSortedLinkedObj(SortedLinkedObj *obj);
#endif
