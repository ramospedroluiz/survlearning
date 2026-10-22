/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "stackOutputQQ.h"
#include "stackOutput.h"
#include "nativeUtil.h"
#include "sexpOutgoing.h"
#include "nrutil.h"
void stackTNQualitativeObjectsKnown(char     mode,
                                    uint   **pRF_RMBR_ID_,
                                    uint   **pRF_AMBR_ID_,
                                    uint   **pRF_TN_RCNT_,
                                    uint   **pRF_TN_ACNT_,
                                    uint   **pRF_OOB_SZ_,
                                    uint   **pRF_IBG_SZ_) {
  ulong localSize;
  if (SL_RF_optHigh & OPT_MEMB_OUTG) {
    localSize = (ulong) SL_RF_ntree * SL_RF_bootstrapSize;
    *pRF_RMBR_ID_ = (uint*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_RMBR_ID, localSize, 0, SL_RF_sexpString[SL_RF_RMBR_ID], &SL_RF_RMBR_ID_ptr, 2, SL_RF_ntree, SL_RF_bootstrapSize);
    localSize = (ulong) SL_RF_ntree * SL_RF_observationSize;
    *pRF_AMBR_ID_ = (uint*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_AMBR_ID, localSize, 0, SL_RF_sexpString[SL_RF_AMBR_ID], &SL_RF_AMBR_ID_ptr, 2, SL_RF_ntree, SL_RF_observationSize);
    *pRF_OOB_SZ_ = (uint*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_OOB_SZ, SL_RF_ntree, 0, SL_RF_sexpString[SL_RF_OOB_SZ], NULL, 1, SL_RF_ntree);
    (*pRF_OOB_SZ_) --;
    *pRF_IBG_SZ_ = (uint*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_IBG_SZ, SL_RF_ntree, 0, SL_RF_sexpString[SL_RF_IBG_SZ], NULL, 1, SL_RF_ntree);
    (*pRF_IBG_SZ_) --;
  }
  else if (SL_RF_optHigh & OPT_MEMB_INCG) {
    int *dim = ivector(1, 2);
    dim[1] = SL_RF_ntree;
    dim[2] = SL_RF_bootstrapSize;
    allocateAuxiliaryInfo(FALSE,
                          NATIVE_TYPE_INTEGER,
                          SL_RF_sexpString[SL_RF_RMBR_ID],
                          SL_RF_incAuxiliaryInfoList,
                          SL_RF_incStackCount,
                          *pRF_RMBR_ID_,
                          &SL_RF_RMBR_ID_ptr,
                          2,
                          dim);
    SL_RF_incStackCount ++;
    dim[1] = SL_RF_ntree;
    dim[2] = SL_RF_observationSize;
    allocateAuxiliaryInfo(FALSE,
                          NATIVE_TYPE_INTEGER,
                          SL_RF_sexpString[SL_RF_AMBR_ID],
                          SL_RF_incAuxiliaryInfoList,
                          SL_RF_incStackCount,                          
                          *pRF_AMBR_ID_,
                          &SL_RF_AMBR_ID_ptr,
                          2,
                          dim);
    SL_RF_incStackCount ++;
    dim[1] = SL_RF_ntree;
    dim[2] = -2;
    allocateAuxiliaryInfo(FALSE,
                          NATIVE_TYPE_INTEGER,
                          SL_RF_sexpString[SL_RF_TN_RCNT],
                          SL_RF_incAuxiliaryInfoList,
                          SL_RF_incStackCount,
                          *pRF_TN_RCNT_,
                          &SL_RF_TN_RCNT_ptr,
                          2,
                          dim);
    SL_RF_incStackCount ++;
    allocateAuxiliaryInfo(FALSE,
                          NATIVE_TYPE_INTEGER,
                          SL_RF_sexpString[SL_RF_TN_ACNT],
                          SL_RF_incAuxiliaryInfoList,
                          SL_RF_incStackCount,
                          *pRF_TN_ACNT_,
                          &SL_RF_TN_ACNT_ptr,
                          2,
                          dim);
    SL_RF_incStackCount ++;
    free_ivector(dim, 1, 2);
  }
}
void stackTNQualitativeObjectsUnknown(char     mode,
                                      uint   **pRF_TN_RCNT_,
                                      uint   **pRF_TN_ACNT_,
                                      uint   **pRF_TN_OCNT_,
                                      uint   **pRF_TN_ICNT_) {
  LeafLinkedObj *leafLinkedPtr;
  ulong localSize;
  uint i;
  if (SL_RF_optHigh & OPT_MEMB_OUTG) {
    localSize = SL_RF_totalTerminalCount;
    *pRF_TN_RCNT_ = (uint*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_TN_RCNT, localSize, 0, SL_RF_sexpString[SL_RF_TN_RCNT], &SL_RF_TN_RCNT_ptr, 2, SL_RF_ntree, -2);
    *pRF_TN_ACNT_ = (uint*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_TN_ACNT, localSize, 0, SL_RF_sexpString[SL_RF_TN_ACNT], &SL_RF_TN_ACNT_ptr, 2, SL_RF_ntree, -2);
    *pRF_TN_OCNT_ = (uint*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_TN_OCNT, localSize, 0, SL_RF_sexpString[SL_RF_TN_OCNT], &SL_RF_TN_OCNT_ptr, 2, SL_RF_ntree, -2);
    *pRF_TN_ICNT_ = (uint*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_TN_ICNT, localSize, 0, SL_RF_sexpString[SL_RF_TN_ICNT], &SL_RF_TN_ICNT_ptr, 2, SL_RF_ntree, -2);
    for (i = 1; i <= SL_RF_ntree; i++) {
      leafLinkedPtr = SL_RF_leafLinkedObjHead[i] -> fwdLink;
      while (leafLinkedPtr != NULL) {
        SL_RF_TN_RCNT_ptr[i][(leafLinkedPtr -> termPtr) -> nodeID] = leafLinkedPtr -> ibgMembrCount;
        SL_RF_TN_ACNT_ptr[i][(leafLinkedPtr -> termPtr) -> nodeID] = leafLinkedPtr -> allMembrCount;
        SL_RF_TN_OCNT_ptr[i][(leafLinkedPtr -> termPtr) -> nodeID] = leafLinkedPtr -> oobMembrCount = leafLinkedPtr -> termPtr -> oobMembrSize;
        SL_RF_TN_ICNT_ptr[i][(leafLinkedPtr -> termPtr) -> nodeID] = leafLinkedPtr -> ibgMembrCount = leafLinkedPtr -> termPtr -> ibgMembrSize;        
        leafLinkedPtr = leafLinkedPtr -> fwdLink;
      }
    }
  }
}
void stackTNQuantitativeForestObjectsPtrOnly(char mode) {
  if (SL_RF_optHigh & OPT_TERM_OUTG) {
    SL_RF_totalTerminalCount = 0;
    if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
      SL_RF_TN_MORT_ptr = (double ***) new_vvector(1, SL_RF_ntree, NRUTIL_DPTR2);
      if (!(SL_RF_opt & OPT_COMP_RISK)) {
        SL_RF_TN_SURV_ptr = (double ***) new_vvector(1, SL_RF_ntree, NRUTIL_DPTR2);
        SL_RF_TN_NLSN_ptr = (double ***) new_vvector(1, SL_RF_ntree, NRUTIL_DPTR2);
      }
      else {
        SL_RF_TN_CSHZ_ptr = (double ****) new_vvector(1, SL_RF_ntree, NRUTIL_DPTR3);
        SL_RF_TN_CIFN_ptr = (double ****) new_vvector(1, SL_RF_ntree, NRUTIL_DPTR3);
      }
    }
    else {
      if (SL_RF_rNonFactorCount > 0) {
        SL_RF_TN_REGR_ptr = (double ***) new_vvector(1, SL_RF_ntree, NRUTIL_DPTR2);
      }
      if (SL_RF_rFactorCount > 0) {
        SL_RF_TN_CLAS_ptr = (uint ****) new_vvector(1, SL_RF_ntree, NRUTIL_UPTR3);
      }
    }
  }
  else if (SL_RF_optHigh & OPT_TERM_INCG) {
    int *dim = ivector(1, 4);
    if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
      dim[1] = SL_RF_ntree;
      dim[2] = -2;
      dim[3] = SL_RF_eventTypeSize;
      allocateAuxiliaryInfo(FALSE,
                            NATIVE_TYPE_NUMERIC,
                            SL_RF_sexpString[SL_RF_TN_MORT],
                            SL_RF_incAuxiliaryInfoList,
                            SL_RF_incStackCount,
                            SL_RF_TN_MORT_,
                            &SL_RF_TN_MORT_ptr,
                            3,
                            dim);
      SL_RF_incStackCount ++;
      if (!(SL_RF_opt & OPT_COMP_RISK)) {
        dim[1] = SL_RF_ntree;
        dim[2] = -2;
        dim[3] = SL_RF_sortedTimeInterestSize;
        allocateAuxiliaryInfo(FALSE,
                              NATIVE_TYPE_NUMERIC,
                              SL_RF_sexpString[SL_RF_TN_SURV],
                              SL_RF_incAuxiliaryInfoList,
                              SL_RF_incStackCount,
                              SL_RF_TN_SURV_,
                              &SL_RF_TN_SURV_ptr,
                              3,
                              dim);
        SL_RF_incStackCount ++;
        allocateAuxiliaryInfo(FALSE,
                              NATIVE_TYPE_NUMERIC,
                              SL_RF_sexpString[SL_RF_TN_NLSN],
                              SL_RF_incAuxiliaryInfoList,
                              SL_RF_incStackCount,
                              SL_RF_TN_NLSN_,
                              &SL_RF_TN_NLSN_ptr,
                              3,
                              dim);
        SL_RF_incStackCount ++;
      }
      else {
        dim[1] = SL_RF_ntree;
        dim[2] = -2;
        dim[3] = SL_RF_eventTypeSize;
        dim[4] = SL_RF_sortedTimeInterestSize;
        allocateAuxiliaryInfo(FALSE,
                              NATIVE_TYPE_NUMERIC,
                              SL_RF_sexpString[SL_RF_TN_CSHZ],
                              SL_RF_incAuxiliaryInfoList,
                              SL_RF_incStackCount,
                              SL_RF_TN_CSHZ_,
                              &SL_RF_TN_CSHZ_ptr,
                              4,
                              dim);
        SL_RF_incStackCount ++;
        allocateAuxiliaryInfo(FALSE,
                              NATIVE_TYPE_NUMERIC,
                              SL_RF_sexpString[SL_RF_TN_CIFN],
                              SL_RF_incAuxiliaryInfoList,
                              SL_RF_incStackCount,
                              SL_RF_TN_CIFN_,
                              &SL_RF_TN_CIFN_ptr,
                              4,
                              dim);
        SL_RF_incStackCount ++;
      }
    }
    else {
      if (SL_RF_rNonFactorCount > 0) {
        dim[1] = SL_RF_ntree;
        dim[2] = -2;
        dim[3] = SL_RF_rNonFactorCount;
        allocateAuxiliaryInfo(FALSE,
                              NATIVE_TYPE_NUMERIC,
                              SL_RF_sexpString[SL_RF_TN_REGR],
                              SL_RF_incAuxiliaryInfoList,
                              SL_RF_incStackCount,
                              SL_RF_TN_REGR_,
                              &SL_RF_TN_REGR_ptr,
                              3,
                              dim);
        SL_RF_incStackCount ++;
      }
      if (SL_RF_rFactorCount > 0) {
        dim[1] = SL_RF_ntree;
        dim[2] = -2;
        dim[3] = SL_RF_rFactorCount;
        dim[4] = 0;
        allocateAuxiliaryInfo(FALSE,
                              NATIVE_TYPE_INTEGER,
                              SL_RF_sexpString[SL_RF_TN_CLAS],
                              SL_RF_incAuxiliaryInfoList,
                              SL_RF_incStackCount,
                              SL_RF_TN_CLAS_,
                              &SL_RF_TN_CLAS_ptr,
                              4,
                              dim);
        SL_RF_incStackCount ++;
      }
    }
    free_ivector(dim, 1, 4);
  }
}
void unstackTNQuantitativeForestObjectsPtrOnly(char mode) {
  uint i;
  if (SL_RF_optHigh & OPT_TERM_OUTG) {
    if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
      for (i = 1; i <= SL_RF_ntree; i++) {
        unstackTNQuantitativeTreeObjectsPtrOnly(i);
      }
      free_new_vvector(SL_RF_TN_MORT_ptr, 1, SL_RF_ntree, NRUTIL_DPTR2);
      if (!(SL_RF_opt & OPT_COMP_RISK)) {
        free_new_vvector(SL_RF_TN_SURV_ptr, 1, SL_RF_ntree, NRUTIL_DPTR2);
        free_new_vvector(SL_RF_TN_NLSN_ptr, 1, SL_RF_ntree, NRUTIL_DPTR2);          
      }
      else {
        free_new_vvector(SL_RF_TN_CSHZ_ptr, 1, SL_RF_ntree, NRUTIL_DPTR3);
        free_new_vvector(SL_RF_TN_CIFN_ptr, 1, SL_RF_ntree, NRUTIL_DPTR3);
      }
    }
    else {
      if ((SL_RF_rNonFactorCount > 0) || (SL_RF_rFactorCount > 0)) {
        for (i = 1; i <= SL_RF_ntree; i++) {
          unstackTNQuantitativeTreeObjectsPtrOnly(i);
        }
        if (SL_RF_rNonFactorCount > 0) {
          free_new_vvector(SL_RF_TN_REGR_ptr, 1, SL_RF_ntree, NRUTIL_DPTR2);
        }
        if (SL_RF_rFactorCount > 0) {
          free_new_vvector(SL_RF_TN_CLAS_ptr, 1, SL_RF_ntree, NRUTIL_UPTR2);
        }
      }
    }
  }
  else if (SL_RF_optHigh & OPT_TERM_INCG) {
    if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
    }
    else {
      if (SL_RF_rNonFactorCount > 0) {
      }
      if (SL_RF_rFactorCount > 0) {
      }
    }
  }
}
void stackTNQuantitativeTreeObjectsPtrOnly(uint treeID) {
  uint i, j;
  if (SL_RF_optHigh & OPT_TERM_OUTG) {
    if (SL_RF_tLeafCount[treeID] > 0 ) {
      if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
        SL_RF_TN_MORT_ptr[treeID] = (double **) new_vvector(1, SL_RF_tLeafCount[treeID], NRUTIL_DPTR);
        for (i = 1; i <= SL_RF_tLeafCount[treeID]; i++) {
          SL_RF_TN_MORT_ptr[treeID][i] = dvector(1, SL_RF_eventTypeSize);
        }
        if (!(SL_RF_opt & OPT_COMP_RISK)) {
          SL_RF_TN_SURV_ptr[treeID] = (double **) new_vvector(1, SL_RF_tLeafCount[treeID], NRUTIL_DPTR);
          SL_RF_TN_NLSN_ptr[treeID] = (double **) new_vvector(1, SL_RF_tLeafCount[treeID], NRUTIL_DPTR);
          for (i = 1; i <= SL_RF_tLeafCount[treeID]; i++) {
            SL_RF_TN_SURV_ptr[treeID][i] = dvector(1, SL_RF_sortedTimeInterestSize);
            SL_RF_TN_NLSN_ptr[treeID][i] = dvector(1, SL_RF_sortedTimeInterestSize);
          }
        }
        else {
          SL_RF_TN_CSHZ_ptr[treeID] = (double ***) new_vvector(1, SL_RF_tLeafCount[treeID], NRUTIL_DPTR2);
          SL_RF_TN_CIFN_ptr[treeID] = (double ***) new_vvector(1, SL_RF_tLeafCount[treeID], NRUTIL_DPTR2);
          for (i = 1; i <= SL_RF_tLeafCount[treeID]; i++) {
            SL_RF_TN_CSHZ_ptr[treeID][i] = (double **) new_vvector(1, SL_RF_eventTypeSize, NRUTIL_DPTR);
            SL_RF_TN_CIFN_ptr[treeID][i] = (double **) new_vvector(1, SL_RF_eventTypeSize, NRUTIL_DPTR);
            for (j = 1; j <= SL_RF_eventTypeSize; j++) {
              SL_RF_TN_CSHZ_ptr[treeID][i][j] = dvector(1, SL_RF_sortedTimeInterestSize);
              SL_RF_TN_CIFN_ptr[treeID][i][j] = dvector(1, SL_RF_sortedTimeInterestSize);
            }
          }
        }
      }
      else {
        if (SL_RF_rNonFactorCount > 0) {
          SL_RF_TN_REGR_ptr[treeID] = (double **) new_vvector(1, SL_RF_tLeafCount[treeID], NRUTIL_DPTR);
          for (i = 1; i <= SL_RF_tLeafCount[treeID]; i++) {
            SL_RF_TN_REGR_ptr[treeID][i] = dvector(1, SL_RF_rNonFactorCount);
          }
        }
        if (SL_RF_rFactorCount > 0) {
          SL_RF_TN_CLAS_ptr[treeID] = (uint ***) new_vvector(1, SL_RF_tLeafCount[treeID], NRUTIL_UPTR2);
          for (i = 1; i <= SL_RF_tLeafCount[treeID]; i++) {
            SL_RF_TN_CLAS_ptr[treeID][i] = (uint **) new_vvector(1, SL_RF_rFactorCount, NRUTIL_UPTR);
            for (j = 1; j <= SL_RF_rFactorCount; j++) {
              SL_RF_TN_CLAS_ptr[treeID][i][j] = uivector(1, SL_RF_rFactorSize[j]);
            }
          }
        }
      }
    }
  }
}
void unstackTNQuantitativeTreeObjectsPtrOnly(uint treeID) {
  uint i, j;
  if (SL_RF_optHigh & OPT_TERM_OUTG) {
    if (SL_RF_tLeafCount[treeID] > 0 ) {
      if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
        for (i = 1; i <= SL_RF_tLeafCount[treeID]; i++) {
          free_dvector(SL_RF_TN_MORT_ptr[treeID][i], 1, SL_RF_eventTypeSize);
        }
        free_new_vvector(SL_RF_TN_MORT_ptr[treeID], 1, SL_RF_tLeafCount[treeID], NRUTIL_DPTR);
        if (!(SL_RF_opt & OPT_COMP_RISK)) {
          for (i = 1; i <= SL_RF_tLeafCount[treeID]; i++) {
            free_dvector(SL_RF_TN_SURV_ptr[treeID][i], 1, SL_RF_sortedTimeInterestSize);
            free_dvector(SL_RF_TN_NLSN_ptr[treeID][i], 1, SL_RF_sortedTimeInterestSize);
          }
          free_new_vvector(SL_RF_TN_SURV_ptr[treeID], 1, SL_RF_tLeafCount[treeID], NRUTIL_DPTR);
          free_new_vvector(SL_RF_TN_NLSN_ptr[treeID], 1, SL_RF_tLeafCount[treeID], NRUTIL_DPTR);
        }
        else {
          for (i = 1; i <= SL_RF_tLeafCount[treeID]; i++) {
            for (j = 1; j <= SL_RF_eventTypeSize; j++) {
              free_dvector(SL_RF_TN_CSHZ_ptr[treeID][i][j], 1, SL_RF_sortedTimeInterestSize);
              free_dvector(SL_RF_TN_CIFN_ptr[treeID][i][j], 1, SL_RF_sortedTimeInterestSize);
            }
            free_new_vvector(SL_RF_TN_CSHZ_ptr[treeID][i], 1, SL_RF_eventTypeSize, NRUTIL_DPTR);
            free_new_vvector(SL_RF_TN_CIFN_ptr[treeID][i], 1, SL_RF_eventTypeSize, NRUTIL_DPTR);
          }
          free_new_vvector(SL_RF_TN_CSHZ_ptr[treeID], 1, SL_RF_tLeafCount[treeID], NRUTIL_DPTR2);
          free_new_vvector(SL_RF_TN_CIFN_ptr[treeID], 1, SL_RF_tLeafCount[treeID], NRUTIL_DPTR2);
        }
      }
      else {
        if (SL_RF_rNonFactorCount > 0) {
          for (i = 1; i <= SL_RF_tLeafCount[treeID]; i++) {
            free_dvector(SL_RF_TN_REGR_ptr[treeID][i], 1, SL_RF_rNonFactorCount);
          }
          free_new_vvector(SL_RF_TN_REGR_ptr[treeID], 1, SL_RF_tLeafCount[treeID], NRUTIL_DPTR);
        }
        if (SL_RF_rFactorCount > 0) {
          for (i = 1; i <= SL_RF_tLeafCount[treeID]; i++) {
            for (j = 1; j <= SL_RF_rFactorCount; j++) {
              free_uivector(SL_RF_TN_CLAS_ptr[treeID][i][j], 1, SL_RF_rFactorSize[j]);
            }
            free_new_vvector(SL_RF_TN_CLAS_ptr[treeID][i], 1, SL_RF_rFactorCount, NRUTIL_UPTR);
          }
          free_new_vvector(SL_RF_TN_CLAS_ptr[treeID], 1, SL_RF_tLeafCount[treeID], NRUTIL_UPTR2);
        }
      }
    }
  }
}
void saveTNQuantitativeTreeObjects(uint treeID) {
  LeafLinkedObj *leafLinkedPtr;
  Terminal *parent;
  uint leaf;
  uint j, k;
  if (SL_RF_optHigh & OPT_TERM_OUTG) {
    if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
      leafLinkedPtr = SL_RF_leafLinkedObjHead[treeID] -> fwdLink;
      while (leafLinkedPtr != NULL) {
        parent = leafLinkedPtr -> termPtr;
        leaf = parent -> nodeID;
        for (j = 1; j <= SL_RF_eventTypeSize; j++) {
          SL_RF_TN_MORT_ptr[treeID][leaf][j] = parent -> mortality[j];
        }
        leafLinkedPtr = leafLinkedPtr -> fwdLink;
      }
      if (!(SL_RF_opt & OPT_COMP_RISK)) {
        leafLinkedPtr = SL_RF_leafLinkedObjHead[treeID] -> fwdLink;
        while (leafLinkedPtr != NULL) {
          parent = leafLinkedPtr -> termPtr;
          leaf = parent -> nodeID;
          for (k = 1; k <= SL_RF_sortedTimeInterestSize; k++) {
            SL_RF_TN_SURV_ptr[treeID][leaf][k] = parent -> survival[k];
            SL_RF_TN_NLSN_ptr[treeID][leaf][k] = parent -> nelsonAalen[k];
          }
          leafLinkedPtr = leafLinkedPtr -> fwdLink;
        }
      }
      else {
        leafLinkedPtr = SL_RF_leafLinkedObjHead[treeID] -> fwdLink;
        while (leafLinkedPtr != NULL) {
          parent = leafLinkedPtr -> termPtr;
          leaf = parent -> nodeID;
          for (j = 1; j <= SL_RF_eventTypeSize; j++) {
            for (k = 1; k <= SL_RF_sortedTimeInterestSize; k++) {
              SL_RF_TN_CSHZ_ptr[treeID][leaf][j][k] = parent -> CSH[j][k];
              SL_RF_TN_CIFN_ptr[treeID][leaf][j][k] = parent -> CIF[j][k];
            }
          }
          leafLinkedPtr = leafLinkedPtr -> fwdLink;
        }
      }
    }
    else {
      if (SL_RF_rNonFactorCount > 0) {
        leafLinkedPtr = SL_RF_leafLinkedObjHead[treeID] -> fwdLink;
        while (leafLinkedPtr != NULL) {
          parent = leafLinkedPtr -> termPtr;
          leaf = parent -> nodeID;
          for (j = 1; j <= SL_RF_rNonFactorCount; j++) {
            SL_RF_TN_REGR_ptr[treeID][leaf][j] = (parent -> meanResponse)[j];
          }
          leafLinkedPtr = leafLinkedPtr -> fwdLink;
        }
      }
      if (SL_RF_rFactorCount > 0) {
        leafLinkedPtr = SL_RF_leafLinkedObjHead[treeID] -> fwdLink;
        while (leafLinkedPtr != NULL) {
          parent = leafLinkedPtr -> termPtr;
          leaf = parent -> nodeID;
          for (j = 1; j <= SL_RF_rFactorCount; j++) {
            for (k = 1; k <= SL_RF_rFactorSize[j]; k++) {
              SL_RF_TN_CLAS_ptr[treeID][leaf][j][k] = (parent -> multiClassProb)[j][k];
            }
          }
          leafLinkedPtr = leafLinkedPtr -> fwdLink;
        }
      }
    }
  }
}
void stackTNQuantitativeForestObjectsOutput(char mode) {
  ulong localSize;
  uint tnDimOne, tnDimTwo;
  uint j;
  if (SL_RF_optHigh & OPT_TERM_OUTG) {
    tnDimOne = SL_RF_totalTerminalCount;
    if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
      localSize = (ulong) tnDimOne * SL_RF_eventTypeSize;
      SL_RF_TN_MORT_ = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_TN_MORT, localSize, SL_RF_nativeNaN, SL_RF_sexpString[SL_RF_TN_MORT], NULL, 1, localSize);
      SL_RF_TN_MORT_ --;
      if (!(SL_RF_opt & OPT_COMP_RISK)) {
        localSize = (ulong) tnDimOne * SL_RF_sortedTimeInterestSize;
        SL_RF_TN_SURV_ = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_TN_SURV, localSize, SL_RF_nativeNaN, SL_RF_sexpString[SL_RF_TN_SURV], NULL, 1, localSize);
        SL_RF_TN_NLSN_ = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_TN_NLSN, localSize, SL_RF_nativeNaN, SL_RF_sexpString[SL_RF_TN_NLSN], NULL, 1, localSize);
        SL_RF_TN_SURV_ --;
        SL_RF_TN_NLSN_ --;
      }
      else {
        localSize = (ulong) tnDimOne * SL_RF_eventTypeSize * SL_RF_sortedTimeInterestSize;
        SL_RF_TN_CSHZ_ = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_TN_CSHZ, localSize, SL_RF_nativeNaN, SL_RF_sexpString[SL_RF_TN_CSHZ], NULL, 1, localSize);
        SL_RF_TN_CIFN_ = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_TN_CIFN, localSize, SL_RF_nativeNaN, SL_RF_sexpString[SL_RF_TN_CIFN], NULL, 1, localSize);
        SL_RF_TN_CSHZ_ --;
        SL_RF_TN_CIFN_ --;
      }
    }
    else {
      if (SL_RF_rNonFactorCount > 0) {
        localSize = (ulong) tnDimOne * SL_RF_rNonFactorCount;
        SL_RF_TN_REGR_ = (double*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_NUMERIC, SL_RF_TN_REGR, localSize, SL_RF_nativeNaN, SL_RF_sexpString[SL_RF_TN_REGR], NULL, 1, localSize);
        SL_RF_TN_REGR_ --;
      }
      if (SL_RF_rFactorCount > 0) {
        tnDimTwo = 0;
        for (j = 1; j <= SL_RF_rFactorCount; j++) {
          tnDimTwo += SL_RF_rFactorSize[j];
        }
        localSize = (ulong) tnDimOne * tnDimTwo;
        SL_RF_TN_CLAS_ = (uint*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_TN_CLAS, localSize, SL_RF_nativeNaN, SL_RF_sexpString[SL_RF_TN_CLAS], NULL, 1, localSize);
        SL_RF_TN_CLAS_ --;
      }
    }
  }
}
void writeTNQuantitativeForestObjectsOutput(char mode) {
  uint i, j, k, m;
  ulong iter;
  if (SL_RF_optHigh & OPT_TERM_OUTG) {
    if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
      iter = 0;
      for (i = 1; i <= SL_RF_ntree; i++) {        
        for (j = 1; j <= SL_RF_tLeafCount[i]; j++) {        
          for (k = 1; k <= SL_RF_eventTypeSize; k++) {
            SL_RF_TN_MORT_[++iter] = SL_RF_TN_MORT_ptr[i][j][k];
          }
        }
      }
      if (!(SL_RF_opt & OPT_COMP_RISK)) {
        iter = 0;          
        for (i = 1; i <= SL_RF_ntree; i++) {
          for (j = 1; j <= SL_RF_tLeafCount[i]; j++) {
            for (k = 1; k <= SL_RF_sortedTimeInterestSize; k++) {
              SL_RF_TN_SURV_[++iter] = SL_RF_TN_SURV_ptr[i][j][k];
              SL_RF_TN_NLSN_[iter]   = SL_RF_TN_NLSN_ptr[i][j][k];
            }
          }
        }
      }
      else {
        iter = 0;          
        for (i = 1; i <= SL_RF_ntree; i++) {
          for (j = 1; j <= SL_RF_tLeafCount[i]; j++) {
            for (k = 1; k <= SL_RF_eventTypeSize; k++) {
              for (m = 1; m <= SL_RF_sortedTimeInterestSize; m++) {
                SL_RF_TN_CSHZ_[++iter] = SL_RF_TN_CSHZ_ptr[i][j][k][m];
                SL_RF_TN_CIFN_[iter]   = SL_RF_TN_CIFN_ptr[i][j][k][m];
              }
            }
          }
        }
      }
    }
    else {
      if (SL_RF_rNonFactorCount > 0) {
        iter = 0;
        for (i = 1; i <= SL_RF_ntree; i++) {
          for (j = 1; j <= SL_RF_tLeafCount[i]; j++) {
            for (k = 1; k <= SL_RF_rNonFactorCount; k++) {
              SL_RF_TN_REGR_[++iter] = SL_RF_TN_REGR_ptr[i][j][k];
            }
          }
        }
      }
      if (SL_RF_rFactorCount > 0) {
        iter = 0;
        for (i = 1; i <= SL_RF_ntree; i++) {
          for (j = 1; j <= SL_RF_tLeafCount[i]; j++) {
            for (k = 1; k <= SL_RF_rFactorCount; k++) {
              for (m = 1; m <= SL_RF_rFactorSize[k]; m++) {
                SL_RF_TN_CLAS_[++iter] = SL_RF_TN_CLAS_ptr[i][j][k][m];
              }
            }
          }
        }
      }
    }
  }
}
void stackTNQualitativeObjectsUnknownMembership(char   mode, uint **pRF_OMBR_ID_, uint **pRF_IMBR_ID_) {
  Terminal *termPtr;
  ulong localSize;
  uint  treeID;
  uint  i, j;
  uint iter;
  if (SL_RF_optHigh & OPT_MEMB_OUTG) {
    localSize = 0;
    for (uint treeID = 1; treeID <= SL_RF_ntree; treeID++) {
      localSize += SL_RF_oobSize[treeID];
    }
    if (localSize > 0) {
      SL_RF_OMBR_ID_ = (uint*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_OMBR_ID, localSize, 0, SL_RF_sexpString[SL_RF_OMBR_ID], NULL, 1, localSize);
      SL_RF_OMBR_ID_ --;
      iter = 0;
      for (treeID = 1; treeID <= SL_RF_ntree; treeID++) {
        for (i = 1; i <= SL_RF_tLeafCount[treeID]; i++) {
          termPtr = SL_RF_tTermList[treeID][i];
          for (j = 1; j <= termPtr -> oobMembrSize; j ++) {
            SL_RF_OMBR_ID_[++iter] = termPtr -> oobMembrIndx[j];
          }
        }
      }
    }
    else {
      localSize = 0;
      for (uint treeID = 1; treeID <= SL_RF_ntree; treeID++) {
        localSize += SL_RF_tLeafCount[treeID];
      }
      SL_RF_OMBR_ID_ = (uint*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_OMBR_ID, localSize, 0, SL_RF_sexpString[SL_RF_OMBR_ID], NULL, 1, localSize);      
    }
    localSize = 0;
    for (uint treeID = 1; treeID <= SL_RF_ntree; treeID++) {
      localSize += SL_RF_ibgSize[treeID];
    }
    SL_RF_IMBR_ID_ = (uint*) stackAndProtect(mode, &SL_RF_nativeIndex, NATIVE_TYPE_INTEGER, SL_RF_IMBR_ID, localSize, 0, SL_RF_sexpString[SL_RF_IMBR_ID], NULL, 1, localSize);
    SL_RF_IMBR_ID_ --;
    iter = 0;
    for (treeID = 1; treeID <= SL_RF_ntree; treeID++) {
      for (i = 1; i <= SL_RF_tLeafCount[treeID]; i++) {
        termPtr = SL_RF_tTermList[treeID][i];
        for (j = 1; j <= termPtr -> ibgMembrSize; j ++) {
          SL_RF_IMBR_ID_[++iter] = termPtr -> ibgMembrIndx[j];
        }
      }
    }
  }  
}
