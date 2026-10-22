/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "bootstrap.h"
#include "random.h"
#include "nrutil.h"
#include "sampling.h"
#include "nodeOps.h"
#include "error.h"
char bootstrap (char     mode,
                uint     treeID,
                Node    *nodePtr,
                uint    *subsetIndex,
                uint     subsetSize,
                uint    *index,  
                uint     indexSize) {  
  char *permissible;
  char result;
  uint i, j, k;
  result = TRUE;
  if (!(SL_RF_opt & OPT_BOOT_TYP1) && (SL_RF_opt & OPT_BOOT_TYP2)) {
    for (i=1; i <= subsetSize; i++) {
      index[i] = subsetIndex[i];
    }
  }
  else {
    if ( (SL_RF_opt & OPT_BOOT_TYP1) && (SL_RF_opt & OPT_BOOT_TYP2) ) {
      i = 0;
      for (k = 1; k <= SL_RF_subjSize; k++) {
        for (j = 1; j <= SL_RF_bootstrapIn[treeID][k]; j++) {
          index[++i] = k;
        }
      }
    }
    else {
      if (SL_RF_subjWeightType == SL_RF_WGHT_UNIFORM) {
        if (SL_RF_optHigh & OPT_BOOT_SWOR) {
          uint *sworVector = uivector(1, subsetSize);
          uint sworVectorSize = subsetSize;
          uint sworIndex;
          for (j = 1; j <= sworVectorSize; j++) {
            sworVector[j] = subsetIndex[j];
          }
          for (j = 1; j <= indexSize; j++) {
            sworIndex = (uint) ceil(ran1A(treeID) * (sworVectorSize * 1.0));
            index[j] = sworVector[sworIndex];
            sworVector[sworIndex] = sworVector[sworVectorSize];
            sworVectorSize --;
          }
          free_uivector (sworVector, 1, subsetSize);
        }
        else {    
          for (i = 1; i <= indexSize; i++) {
            k = (uint) ceil(ran1A(treeID)*(subsetSize * 1.0));
            index[i] = subsetIndex[k];
          }
        }
      }
      else {
        if (SL_RF_subjWeightType != SL_RF_WGHT_UNIFORM) {
          permissible = cvector(1, SL_RF_subjSize);
          for (i = 1; i <= SL_RF_subjSize; i++) {
            permissible[i] = FALSE;
          }
          for (i = 1; i <= subsetSize; i++) {
            permissible[subsetIndex[i]] = TRUE;
          }
        }
        else {
          permissible = NULL;
        }
        DistributionObj *obj = makeDistributionObjRaw();
        obj -> permissibleIndex = (SL_RF_subjWeightType == SL_RF_WGHT_UNIFORM) ? subsetIndex : NULL;
        obj -> permissible       = (SL_RF_subjWeightType == SL_RF_WGHT_UNIFORM) ? NULL : permissible;
        obj -> permissibleSize   = (SL_RF_subjWeightType == SL_RF_WGHT_UNIFORM) ? subsetSize : SL_RF_subjSize;
        obj -> augmentationSize = NULL;
        obj -> weightType = SL_RF_subjWeightType;
        obj -> weight = SL_RF_subjWeight;
        obj -> weightSorted = SL_RF_subjWeightSorted;
        obj -> densityAllocSize = SL_RF_subjWeightDensitySize;
        initializeCDFNew(treeID, obj);
        for (i = 1; i <= indexSize; i++) {
          index[i] = sampleFromCDFNew(ran1A, treeID, obj);
          if (SL_RF_optHigh & OPT_BOOT_SWOR) {
            if (index[i] != 0) {
              updateCDFNew(treeID, obj);
            }
            else {
              SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
              SL_RF_nativeError("\nRF-SRC:  No cases left to select for bootstrap SWOR of size:  %10d", indexSize);
              SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
              SL_RF_nativeExit();
            }
          }
        }
        discardCDFNew(treeID, obj);
        freeDistributionObjRaw(obj);
        if (SL_RF_subjWeightType != SL_RF_WGHT_UNIFORM) {
          free_cvector(permissible, 1, SL_RF_subjSize);
        }
      }
    }
  }
  uint iter;
  for (i = 1; i <= SL_RF_observationSize; i++) {
    SL_RF_bootMembershipFlag[treeID][i]  = FALSE;
    SL_RF_oobMembershipFlag[treeID][i]   = TRUE;
    SL_RF_bootMembershipCount[treeID][i] = 0;
  }
  iter = 0;
  for (i = 1; i <= indexSize; i++) {
    SL_RF_bootMembershipIndex[treeID][++iter] = index[i];
    SL_RF_bootMembershipFlag[treeID][index[i]] = TRUE;
    SL_RF_oobMembershipFlag[treeID][index[i]]  = FALSE;
    SL_RF_bootMembershipCount[treeID][index[i]] ++;
    if (SL_RF_optHigh & OPT_MEMB_USER) {
      SL_RF_BOOT_CT_ptr[treeID][index[i]] ++;
    }
  }
  SL_RF_oobSize[treeID] = 0;
  SL_RF_ibgSize[treeID] = 0;
  for (i = 1; i <= SL_RF_observationSize; i++) {
    if (SL_RF_bootMembershipFlag[treeID][i] == FALSE) {
      SL_RF_oobMembershipIndex[treeID][++SL_RF_oobSize[treeID]] = i;
    }
    else {
      SL_RF_ibgMembershipIndex[treeID][++SL_RF_ibgSize[treeID]] = i;
    }
  }
  if (result) {
    result = getNodeSign(mode, treeID, nodePtr, index, indexSize);
    if (!result) {
    }
  }
  else {
  }
  if (result) {
    if (mode == SL_RF_PRED) {
    }
  } 
  else {
  }
  return result;
}
char getNodeSign (char mode,
                  uint treeID,
                  Node *nodePtr,
                  uint *bmIndex,
                  uint repMembrSize) {
  int   *mvNSptr;
  int   *fmvNSptr;
  char result;
  uint i,p,q,m;
  result = TRUE;
  switch (mode) {
  case SL_RF_PRED:
    if (SL_RF_mRecordSize > 0) {
      stackMPSign(nodePtr, SL_RF_mpIndexSize);
      mvNSptr = nodePtr -> mpSign;
    }
    else {
      mvNSptr = NULL;
    }
    if (SL_RF_fmRecordSize > 0) {
      stackFMPSign(nodePtr, SL_RF_fmpIndexSize);
      fmvNSptr = nodePtr -> fmpSign;
    }
    else {
      fmvNSptr = NULL;
    }
    break;
  default:
    if (SL_RF_mRecordSize > 0) {
      stackMPSign(nodePtr, SL_RF_mpIndexSize);
      mvNSptr = nodePtr -> mpSign;
    }
    else {
      mvNSptr = NULL;
    }
    fmvNSptr = NULL;
    break;
  }  
  if (mvNSptr != NULL) {
    int **mvBootstrapSign = imatrix(1, SL_RF_mpIndexSize, 1, repMembrSize);
    for (p = 1; p <= SL_RF_mpIndexSize; p++) {
      for (i = 1; i <= repMembrSize; i++) {
        mvBootstrapSign[p][i] = 0;
      }
    }
    for (p = 1; p <= SL_RF_mpIndexSize; p++) {
      mvNSptr[p] = 0;
    }
    for (i = 1; i <= repMembrSize; i++) {
      m = bmIndex[i];
      if (SL_RF_mRecordMap[m] != 0) {
        for (p = 1; p <= SL_RF_mpIndexSize; p++) {
          if (SL_RF_mpIndex[p] < 0) {
            mvBootstrapSign[p][i] = SL_RF_mpSign[(uint) abs(SL_RF_mpIndex[p])][SL_RF_mRecordMap[m]];
          }
          else {
            mvBootstrapSign[p][i] = SL_RF_mpSign[SL_RF_ySize + (uint) SL_RF_mpIndex[p]][SL_RF_mRecordMap[m]];
          }
        }
      }
      else {
        for (p = 1; p <= SL_RF_mpIndexSize; p++) {
          mvBootstrapSign[p][i] = 0;
        }
      }
      for (p = 1; p <= SL_RF_mpIndexSize; p++) {
        mvNSptr[p] = mvNSptr[p] + mvBootstrapSign[p][i];
      }
    }
    m = 0;
    for (p = 1; p <= SL_RF_mpIndexSize; p++) {
      if (mvNSptr[p] > 0) {
        if ((uint) mvNSptr[p] == repMembrSize) {
          mvNSptr[p] = -1;
        }
        else {
          mvNSptr[p] = 1;
        }
      }
      if(SL_RF_mpIndex[p] < 0) {
        if (mvNSptr[p] == -1) result = FALSE;
      }
      else {
        if (mvNSptr[p] == -1) m ++;
      }
    }  
    if (m == SL_RF_mpIndexSize) {
      result = FALSE;
    }
    free_imatrix(mvBootstrapSign, 1, SL_RF_mpIndexSize, 1, repMembrSize);
  }
  if (fmvNSptr != NULL) {
    for (p = 1; p <= SL_RF_fmpIndexSize; p++) {
      fmvNSptr[p] = 1;
    }
    if (SL_RF_mRecordSize > 0) {
      p = q = 1;
      while ((p <= SL_RF_mpIndexSize) && (q <= SL_RF_fmpIndexSize)) {
        if (SL_RF_mpIndex[p] == SL_RF_fmpIndex[q]) {
          if (mvNSptr[p] == -1) {
            fmvNSptr[q] = -1;
          }
          p++;
          q++;
        }
        else if (SL_RF_fmpIndex[q] < 0) {
          if (SL_RF_mpIndex[p] > 0) {
            q++;
          }
          else {
            if (abs(SL_RF_fmpIndex[q]) < abs(SL_RF_mpIndex[p])) {
              q++;
            }
            else {
              p++;
            }
          }
        }
        else {
          if (SL_RF_fmpIndex[q] < SL_RF_mpIndex[p]) {
            q++;
          }
          else {
            p++;
          }
        }
      }  
    }  
  }  
  return result;
}
char bootstrapSubject (char     mode,
                       uint     treeID,
                       Node    *nodePtr,
                       uint   **index,
                       uint    *indexSize) {
  char   *permissible;
  uint   *permissibleIndex;
  uint   *subjIndex;
  uint   *iterativeIndex;
  char result;
  uint i, j, k;
  subjIndex = NULL;  
  result = TRUE;
  iterativeIndex = uivector(1, SL_RF_bootstrapSize);
  if (!(SL_RF_opt & OPT_BOOT_TYP1) && (SL_RF_opt & OPT_BOOT_TYP2)) {
    for (i = 1; i <= SL_RF_bootstrapSize; i++) {
      iterativeIndex[i] = i;
    }
  }
  else {
    if ( (SL_RF_opt & OPT_BOOT_TYP1) && (SL_RF_opt & OPT_BOOT_TYP2) ) {
      i = 0;
      for (k = 1; k <= SL_RF_subjSize; k++) {
        for (j = 1; j <= SL_RF_bootstrapIn[treeID][k]; j++) {
          subjIndex[++i] = k;
        }
      }
    }
    else {
      if ((SL_RF_subjWeightType == SL_RF_WGHT_UNIFORM) && !(SL_RF_optHigh & OPT_BOOT_SWOR)) {
        for (i = 1; i <= SL_RF_bootstrapSize; i++) {
          k = (uint) ceil(ran1A(treeID)*(SL_RF_subjSize * 1.0));
          iterativeIndex[i] = k;
        }
      }
      else {
        if (SL_RF_subjWeightType != SL_RF_WGHT_UNIFORM) {
          permissible = cvector(1, SL_RF_subjSize);
          for (i = 1; i <= SL_RF_subjSize; i++) {
            permissible[i] = TRUE;
          }
          permissibleIndex = NULL;
        }
        else {
          permissibleIndex = uivector(1, SL_RF_subjSize);
          for (i = 1; i <= SL_RF_subjSize; i++) {
            permissibleIndex[i] = i;
          }
          permissible = NULL;
        }
        DistributionObj *obj = makeDistributionObjRaw();
        obj -> permissibleIndex = (SL_RF_subjWeightType == SL_RF_WGHT_UNIFORM) ? permissibleIndex : NULL;
        obj -> permissible       = (SL_RF_subjWeightType == SL_RF_WGHT_UNIFORM) ? NULL : permissible;
        obj -> permissibleSize   = (SL_RF_subjWeightType == SL_RF_WGHT_UNIFORM) ? SL_RF_subjSize : SL_RF_subjSize;
        obj -> augmentationSize = NULL;
        obj -> weightType = SL_RF_subjWeightType;
        obj -> weight = SL_RF_subjWeight;
        obj -> weightSorted = SL_RF_subjWeightSorted;
        obj -> densityAllocSize = SL_RF_subjWeightDensitySize;
        initializeCDFNew(treeID, obj);
        for (i = 1; i <= SL_RF_bootstrapSize; i++) {
          iterativeIndex[i] = sampleFromCDFNew(ran1A, treeID, obj);
          if (SL_RF_optHigh & OPT_BOOT_SWOR) {
            if (iterativeIndex[i] != 0) {
              updateCDFNew(treeID, obj);
            }
            else {
              SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
              SL_RF_nativeError("\nRF-SRC:  No cases left to select for bootstrap SWOR of size:  %10d", SL_RF_bootstrapSize);
              SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
              SL_RF_nativeExit();
            }
          }
        }
        discardCDFNew(treeID, obj);
        freeDistributionObjRaw(obj);
        if (SL_RF_subjWeightType != SL_RF_WGHT_UNIFORM) {
          free_cvector(permissible, 1, SL_RF_subjSize);
        }
        else {
          free_uivector(permissibleIndex, 1, SL_RF_subjSize);
        }
      }
    }
  }
  *indexSize = 0;
  for (i = 1; i <= SL_RF_bootstrapSize; i++) {
    (*indexSize) += SL_RF_subjSlotCount[iterativeIndex[i]];
  }
  *index = uivector(1, *indexSize);
  k = 0;
  for (i = 1; i <= SL_RF_bootstrapSize; i++) {
    for (j = 1; j <= SL_RF_subjSlotCount[iterativeIndex[i]]; j++) {    
      (*index)[++k] = SL_RF_subjList[iterativeIndex[i]][j];
    }
  }
  free_uivector(iterativeIndex, 1, SL_RF_bootstrapSize);
  result = getNodeSign(mode, treeID, nodePtr, *index, *indexSize);
  if (result == FALSE) {
  }
  if (result == TRUE) {
    if (mode == SL_RF_PRED) {
    }
  } 
  else {
  }
  return result;
}
