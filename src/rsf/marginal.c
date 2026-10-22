/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "marginal.h"
#include "polarity.h"
#include "nrutil.h"
void getMarginalMembership(char mode, uint treeID) {
  double **observationPtr;
  uint    *gallMembrIndx;
  uint     obsSize;
  uint i;
  switch (mode) {
  case SL_RF_PRED:
    obsSize = SL_RF_fobservationSize;
    observationPtr = SL_RF_fobservation[treeID];
    gallMembrIndx  = SL_RF_fidentityMembershipIndex;
    break;
  default:
    obsSize = SL_RF_observationSize;
    observationPtr = SL_RF_observation[treeID];
    gallMembrIndx  = SL_RF_identityMembershipIndex;
    break;
  }
  SL_RF_utTermMembership[treeID] =       (uint **) new_vvector(1, obsSize, NRUTIL_UPTR);
  SL_RF_utTermMembershipCount[treeID] =   (uint *) uivector(1, obsSize);
  SL_RF_utTermMembershipAlloc[treeID] =   (uint *) uivector(1, obsSize);
  for (i = 1; i <= obsSize; i++) {
    SL_RF_utTermMembership[treeID][i] = uivector(1, MARGINAL_SIZE);
    SL_RF_utTermMembershipCount[treeID][i] = 0;
    SL_RF_utTermMembershipAlloc[treeID][i] = 1;            
  }
  marginalMembership(treeID,
                     SL_RF_root[treeID],
                     gallMembrIndx,
                     obsSize,
                     obsSize,
                     observationPtr);
}
void releaseMarginalMembership(char mode, uint treeID) {
  uint     obsSize;
  switch (mode) {
  case SL_RF_PRED:
    obsSize = SL_RF_fobservationSize;
    break;
  default:
    obsSize = SL_RF_observationSize;
    break;
  }
  free_uivector(SL_RF_utTermMembershipCount[treeID], 1, obsSize);
  for (uint i = 1; i <= obsSize; i++) {
    free_uivector(SL_RF_utTermMembership[treeID][i], 1, MARGINAL_SIZE * SL_RF_utTermMembershipAlloc[treeID][i]);
  }
  free_new_vvector(SL_RF_utTermMembership[treeID], 1, obsSize, NRUTIL_UPTR);
  free_uivector(SL_RF_utTermMembershipAlloc[treeID], 1, obsSize);
}
void marginalMembership(uint     treeID,
                        Node    *parent,
                        uint    *genAllMembrIndx,
                        uint     genAllMembrSize,
                        uint     obsSize,
                        double **xArray) {
  char terminalFlag;
  uint *leftAllMembrIndx;
  uint *rghtAllMembrIndx;
  uint leftAllMembrSize;
  uint rghtAllMembrSize;
  uint jLeft;
  uint jRght;
  char daughterFlag;
  SplitInfo *info;
  void *obsLocal;
  char (*getDaughterPolarityGeneric) (uint       treeID,
                                      SplitInfo *info,
                                      uint       index,
                                      void      *value,
                                      ...);
  uint i, j;
  getDaughterPolarityGeneric = NULL;  
  terminalFlag = TRUE;
  if (((parent -> left) != NULL) && ((parent -> right) != NULL)) {
    info = parent -> splitInfo;
    terminalFlag = FALSE;
    leftAllMembrIndx = rghtAllMembrIndx = NULL;
    leftAllMembrSize = rghtAllMembrSize = 0;
    uint *indicator = uivector(1, obsSize);
    leftAllMembrSize = rghtAllMembrSize = 0;
    obsLocal = xArray[info -> randomVar[1]];
    if (info -> mwcpSizeAbs[1] > 0) {
      getDaughterPolarityGeneric = &getDaughterPolaritySimpleFactor;
    }
    else {
      getDaughterPolarityGeneric = &getDaughterPolaritySimpleNonFactor;
    }
    daughterFlag = RIGHT;
    if (SL_RF_xMarginalFlag[info -> randomVar[1]] ==  TRUE) { 
      daughterFlag = NEITHER;
      for (i = 1; i <= genAllMembrSize; i++) {
        indicator[genAllMembrIndx[i]] = daughterFlag;
        leftAllMembrSize ++;
        rghtAllMembrSize ++;
      }        
    }
    if (daughterFlag != NEITHER) {
      for (i = 1; i <= genAllMembrSize; i++) {
        daughterFlag = getDaughterPolarityGeneric(treeID,
                                                  info,
                                                  genAllMembrIndx[i],
                                                  obsLocal);
        indicator[genAllMembrIndx[i]] = daughterFlag;
        if (daughterFlag == LEFT) {
          leftAllMembrSize ++;
        }
        else {
          rghtAllMembrSize ++;
        }
      }  
    }  
    leftAllMembrIndx  = uivector(1, leftAllMembrSize + 1);
    rghtAllMembrIndx  = uivector(1, rghtAllMembrSize + 1);
    jLeft = jRght = 0;
    if (daughterFlag == NEITHER) {
      for (i = 1; i <= genAllMembrSize; i++) {
        leftAllMembrIndx[++jLeft] = genAllMembrIndx[i];
        rghtAllMembrIndx[++jRght] = genAllMembrIndx[i];
      }
    }
    else {
      for (i = 1; i <= genAllMembrSize; i++) {
        if (indicator[genAllMembrIndx[i]] == LEFT) {
          leftAllMembrIndx[++jLeft] = genAllMembrIndx[i];
        }
        else {
          rghtAllMembrIndx[++jRght] = genAllMembrIndx[i];
        }
      }
    }
    free_uivector(indicator, 1, obsSize);
    marginalMembership(treeID,
                              parent -> left,
                              leftAllMembrIndx,
                              leftAllMembrSize,
                              obsSize,
                              xArray);
    marginalMembership(treeID,
                              parent -> right,
                              rghtAllMembrIndx,
                              rghtAllMembrSize,
                              obsSize,
                              xArray);
    free_uivector(leftAllMembrIndx, 1, leftAllMembrSize + 1);
    free_uivector(rghtAllMembrIndx, 1, rghtAllMembrSize + 1);
  }  
  else {
  }
  if (terminalFlag) {
    for (i = 1; i <= genAllMembrSize; i++) {
      SL_RF_utTermMembership[treeID][genAllMembrIndx[i]][++ SL_RF_utTermMembershipCount[treeID][genAllMembrIndx[i]]] = (parent -> nodeID);
      if ((SL_RF_utTermMembershipCount[treeID][genAllMembrIndx[i]]) == (SL_RF_utTermMembershipAlloc[treeID][genAllMembrIndx[i]] * MARGINAL_SIZE)) {
        SL_RF_utTermMembershipAlloc[treeID][genAllMembrIndx[i]] ++;
        uint *utTermMembershipNew = uivector(1, SL_RF_utTermMembershipAlloc[treeID][genAllMembrIndx[i]] * MARGINAL_SIZE);
        for (j = 1; j <= SL_RF_utTermMembershipCount[treeID][genAllMembrIndx[i]]; j++) {
          utTermMembershipNew[j] = SL_RF_utTermMembership[treeID][genAllMembrIndx[i]][j];
        }
        free_uivector(SL_RF_utTermMembership[treeID][genAllMembrIndx[i]], 1, (SL_RF_utTermMembershipAlloc[treeID][genAllMembrIndx[i]] - 1) * MARGINAL_SIZE);
        SL_RF_utTermMembership[treeID][genAllMembrIndx[i]] = utTermMembershipNew;
      }
    }
  }  
}
