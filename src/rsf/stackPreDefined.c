/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "stackPreDefined.h"
#include "impute.h"
#include "nrutil.h"
#include "error.h"
void stackIncomingResponseArrays(char mode) {
  uint i, j;
  SL_RF_timeIndex = SL_RF_statusIndex = 0;
  SL_RF_masterTime = NULL;
  SL_RF_masterTimeIndexIn = NULL;
  if (SL_RF_ySize > 0) {
    SL_RF_yIndex = uivector(1, SL_RF_ySize);
    SL_RF_yIndexZero = uivector(1, SL_RF_ySize);
    j = 0;
    for (i = 1; i <= SL_RF_ySize; i++) {
      if ((SL_RF_rType[i] != 'B') &&
          (SL_RF_rType[i] != 'R') &&
          (SL_RF_rType[i] != 'I') &&
          (SL_RF_rType[i] != 'C') &&
          (SL_RF_rType[i] != 't') &&
          (SL_RF_rType[i] != 'T') &&
          (SL_RF_rType[i] != 'S')) {
        SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
        SL_RF_nativeError("\nRF-SRC:  Invalid type:  [%10d] = %2c", i, SL_RF_rType[i]);
        SL_RF_nativeError("\nRF-SRC:  Variables must be [B], [R], [I], [C], [t], [T], [S].");
        SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
        SL_RF_nativeExit();
      }
      SL_RF_yIndex[i] = SL_RF_yIndexZero[i] = 0;
      if (SL_RF_rType[i] == 'T') {
        SL_RF_timeIndex = i;
      }
      else if (SL_RF_rType[i] == 'S') {
        SL_RF_statusIndex = i;
      }
      else {
        SL_RF_yIndex[++j] = i;
      }
    }
    if (mode == SL_RF_PRED) {
      if (SL_RF_frSize > 0) {
        if (SL_RF_ySize != SL_RF_frSize) {
          SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
          SL_RF_nativeError("\nRF-SRC:  train and test outcome/response matrices must be of the same dimension.  ");
          SL_RF_nativeError("\nRF-SRC:  train vs test:  %10d vs %10d  ", SL_RF_ySize, SL_RF_frSize);
          SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
          SL_RF_nativeExit();
        }
      }
      else {
        if ((SL_RF_opt & OPT_PERF) | (SL_RF_opt & OPT_VIMP)) {
          SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
          SL_RF_nativeError("\nRF-SRC:  test outcome/response matrix must be present when PERF or VIMP is requested.  ");
          SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
          SL_RF_nativeExit();
        }
      }
    }
    if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
      SL_RF_ptnCount = 0;
    }
    SL_RF_ySizeProxy = SL_RF_ySize - ((SL_RF_timeIndex == 0) ? 0:1) - ((SL_RF_statusIndex == 0) ? 0:1);
    SL_RF_yIndexZeroSize = 0;
  }
  else {
    SL_RF_rType      = NULL;
    SL_RF_responseIn = NULL;
    SL_RF_ySizeProxy = 0;
    SL_RF_yIndexZeroSize = 0;
  }
  if (SL_RF_opt & OPT_ANON) {
    if (mode != SL_RF_PRED) {
      SL_RF_opt = SL_RF_opt & (~OPT_PERF);
      SL_RF_opt = SL_RF_opt & (~OPT_VIMP);
    }
  }
}
void unstackIncomingResponseArrays(char mode) {
  if (SL_RF_ySize > 0) {
    free_uivector(SL_RF_yIndex, 1, SL_RF_ySize);
    free_uivector(SL_RF_yIndexZero, 1, SL_RF_ySize);
  }
}
void stackIncomingCovariateArrays(char mode) {
  uint i;
  for (i = 1; i <= SL_RF_xSize; i++) {
    if ((SL_RF_xType[i] != 'B') &&
        (SL_RF_xType[i] != 'R') &&
        (SL_RF_xType[i] != 'I') &&
        (SL_RF_xType[i] != 'C')) {
      SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
      SL_RF_nativeError("\nRF-SRC:  Invalid type:  [%10d] = %2c", i, SL_RF_xType[i]);
      SL_RF_nativeError("\nRF-SRC:  Variables must be [B], [R], [I] or [C].");
      SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
      SL_RF_nativeExit();
    }
  }
}
void unstackIncomingCovariateArrays(char mode) {
}
void stackIncomingArrays(char mode) {
  stackIncomingResponseArrays(mode);
  stackIncomingCovariateArrays(mode);
  if (mode == SL_RF_GROW) {
    if (SL_RF_nodeSize < 1) {
      SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
      SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
      SL_RF_nativeError("\nRF-SRC:  Minimum node size must be greater than zero:  %10d \n", SL_RF_nodeSize);
      SL_RF_nativeExit();
    }
    if (SL_RF_bootstrapSize < 1) {
      SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
      SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
      SL_RF_nativeError("\nRF-SRC:  Bootstrap size must be greater than zero:  %12d \n", SL_RF_bootstrapSize);
      SL_RF_nativeExit();
    }
    if ( SL_RF_splitRule > MAXM_SPLIT) {
      SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
      SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
      SL_RF_nativeError("\nRF-SRC:  Invalid split rule:  %10d \n", SL_RF_splitRule);
      SL_RF_nativeExit();
    }
    if ((SL_RF_splitRule == USPV_NRM) || (SL_RF_splitRule == USPV_WT_OFF) || (SL_RF_splitRule == USPV_WT_HVY)) {
      if ( SL_RF_xSize < 2) {
        SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
        SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
        SL_RF_nativeError("\nRF-SRC:  Number of covariates must be greater than or equal to two (2) with specified split rule:  %10d \n", SL_RF_xSize);
        SL_RF_nativeExit();
      }
      if ( ((int) (SL_RF_xSize - SL_RF_ytry) < 1) || (SL_RF_mtry > SL_RF_xSize) ) {
        SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
        SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
        SL_RF_nativeError("\nRF-SRC:  ytry and mtry must be within range:  %10d %10d \n", SL_RF_ytry,  SL_RF_mtry);
        SL_RF_nativeExit();
      }
    }
    else {
      if (SL_RF_ySize == 0) {
        SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
        SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
        SL_RF_nativeError("\nRF-SRC:  Number of response variables must be greater than zero:  %10d \n", SL_RF_ySize);
        SL_RF_nativeExit();
      }
      if ( ((SL_RF_mtry < 1) || (SL_RF_mtry > SL_RF_xSize)) ) {
        SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
        SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
        SL_RF_nativeError("\nRF-SRC:  Number of random covariate parameters must be greater");
        SL_RF_nativeError("\nRF-SRC:  than zero and less than or equal to the total number of covariates:  %10d \n", SL_RF_mtry);
        SL_RF_nativeExit();
      }
    }
    if ((SL_RF_splitRule != USPV_NRM) && (SL_RF_splitRule != USPV_WT_OFF) && (SL_RF_splitRule != USPV_WT_HVY) && (SL_RF_splitRule != RAND_SPLIT)) {
      if ((SL_RF_timeIndex != 0) && (SL_RF_statusIndex != 0)) {
      }
      else {
        if (SL_RF_ySizeProxy == 0) {
          SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
          SL_RF_nativeError("\nRF-SRC:  No non-[S] and non-[C] responses found.");
          SL_RF_nativeExit();
        }
        if (SL_RF_ytry > SL_RF_ySizeProxy) {
          SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
          SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
          SL_RF_nativeError("\nRF-SRC:  ytry must be within range:  %10d \n", SL_RF_ytry);
          SL_RF_nativeExit();
        }
      }
    }
    for (uint i = 1; i <= SL_RF_xSize; i++) {
      if(SL_RF_xWeightStat[i] < 0) {
        SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
        SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
        SL_RF_nativeError("\nRF-SRC:  Split statistical weight elements must be greater than or equal to zero:  %12.4f \n", SL_RF_xWeightStat[i]);
        SL_RF_nativeExit();
      }
    }
    if(SL_RF_ySize > 0) {
      for (uint i = 1; i <= SL_RF_ySize; i++) {
        if(SL_RF_yWeight[i] < 0) {
          SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
          SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
          SL_RF_nativeError("\nRF-SRC:  Y-weight elements must be greater than or equal to zero:  %12.4f \n", SL_RF_yWeight[i]);
          SL_RF_nativeExit();
        }
      }
    }
    for (uint i = 1; i <= SL_RF_xSize; i++) {
      if(SL_RF_xWeight[i] < 0) {
        SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
        SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
        SL_RF_nativeError("\nRF-SRC:  X-weight elements must be greater than or equal to zero:  %12.4f \n", SL_RF_xWeight[i]);
        SL_RF_nativeExit();
      }
    }
    if ((SL_RF_timeIndex == 0) && (SL_RF_statusIndex == 0)) {
      if ((SL_RF_splitRule != RAND_SPLIT)  &&
          (SL_RF_splitRule != REGR_NRM)    &&
          (SL_RF_splitRule != REGR_WT_OFF) &&
          (SL_RF_splitRule != REGR_WT_HVY) &&
          (SL_RF_splitRule != REGR_SGS)    &&
          (SL_RF_splitRule != REGR_QUANT)  &&
          (SL_RF_splitRule != LARG_QUANT)  &&
          (SL_RF_splitRule != CLAS_NRM)    &&
          (SL_RF_splitRule != CLAS_WT_OFF) &&
          (SL_RF_splitRule != CLAS_WT_HVY) &&
          (SL_RF_splitRule != CLAS_SGS)    &&
          (SL_RF_splitRule != CLAS_AU_ROC) &&
          (SL_RF_splitRule != CLAS_ENTROP) &&
          (SL_RF_splitRule != MV_NRM)      &&
          (SL_RF_splitRule != MV_WT_OFF)   &&
          (SL_RF_splitRule != MV_WT_HVY)   &&
          (SL_RF_splitRule != USPV_NRM)    &&
          (SL_RF_splitRule != USPV_WT_OFF) &&
          (SL_RF_splitRule != USPV_WT_HVY) &&
          (SL_RF_splitRule != CUST_SPLIT)  &&
          (SL_RF_splitRule != MAHALANOBIS)) {
        SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
        SL_RF_nativeError("\nRF-SRC:  !SURV data and split rule specified are incompatible.");
        SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
        SL_RF_nativeExit();
      }
      if  ((SL_RF_splitRule == REGR_QUANT) || (SL_RF_splitRule == LARG_QUANT)) {
        if (SL_RF_quantileSize > 0) {
        }
        else {
          SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
          SL_RF_nativeError("\nRF-SRC:  Quantile regression split rules require the presence of a probability vector.");
          SL_RF_nativeExit();
        }
      }
      if  (SL_RF_splitRule == MAHALANOBIS) {
      }
    }
    else if ((SL_RF_timeIndex != 0) && (SL_RF_statusIndex != 0)) {
      if ((SL_RF_splitRule != SURV_LGRNK)  &&
          (SL_RF_splitRule != SURV_LRSCR)  &&
          (SL_RF_splitRule != SURV_BSG1)   &&
          (SL_RF_splitRule != SURV_CR_LAU) &&
          (SL_RF_splitRule != SURV_CR_GEN) &&
          (SL_RF_splitRule != RAND_SPLIT)  &&
          (SL_RF_splitRule != CUST_SPLIT)) {
        SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
        SL_RF_nativeError("\nRF-SRC:  SURV data and split rule specified are incompatible.");
        SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
        SL_RF_nativeExit();
      }
    }
    else {
      SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
      SL_RF_nativeError("\nRF-SRC:  Data set contains mixed outcomes with no comatible split rule.");
      SL_RF_nativeError("\nRF-SRC:  Please Contact Technical Support.");
      SL_RF_nativeExit();
    }
  }
  if (SL_RF_quantileSize > 0) {
    for (uint i = 1; i <= SL_RF_quantileSize; i++) {
      if ((0 < SL_RF_quantile[i]) && (SL_RF_quantile[i] <= 1.0)) {
      }
      else {
        SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
        SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
        SL_RF_nativeError("\nRF-SRC:  Quantile value is out of range (0, 1):  %.10e ", SL_RF_quantile[i]);
        SL_RF_nativeExit();
      }
    }
  }
}
void unstackIncomingArrays(char mode) {
  unstackIncomingResponseArrays(mode);
  unstackIncomingCovariateArrays(mode);
}
void stackPreDefinedCommonArrays(char          mode,
                                 Node      ****nodeMembership,
                                 Terminal  ****tTermMembership,
                                 Terminal  ****tTermList,
                                 Node       ***root) {
  uint i, j, k;
  *nodeMembership = (Node ***)     new_vvector(1, SL_RF_ntree, NRUTIL_NPTR2);
  *tTermMembership = (Terminal ***) new_vvector(1, SL_RF_ntree, NRUTIL_TPTR2);
  *tTermList = (Terminal ***) new_vvector(1, SL_RF_ntree, NRUTIL_NPTR2);
  SL_RF_nodeCount = uivector(1, SL_RF_ntree);
  for (i = 1; i <= SL_RF_ntree; i++) {
    SL_RF_nodeCount[i] = 0;    
  }
  SL_RF_leafLinkedObjHead = (LeafLinkedObj **) new_vvector(1, SL_RF_ntree, NRUTIL_LEAFPTR);
  SL_RF_leafLinkedObjTail = (LeafLinkedObj **) new_vvector(1, SL_RF_ntree, NRUTIL_LEAFPTR);
  SL_RF_bootMembershipIndex = (uint **) new_vvector(1, SL_RF_ntree, NRUTIL_UPTR);
  if ( (SL_RF_opt & OPT_BOOT_TYP1) && (SL_RF_opt & OPT_BOOT_TYP2) ) {
    for (i = 1; i <= SL_RF_ntree; i++) {
      k = 0;
      for (j = 1; j <= SL_RF_subjSize; j++) {
        k += SL_RF_bootstrapIn[i][j];
      }
      if(k != SL_RF_bootstrapSize) {
        SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
        SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
        SL_RF_nativeError("\nRF-SRC:  Bootstrap size implied by samp matrix inconsistent:  %12d found vs. %12d specified \n", k, SL_RF_bootstrapSize);
        SL_RF_nativeExit();
      }
    }
  }
  SL_RF_bootMembershipFlag = (char **) new_vvector(1, SL_RF_ntree, NRUTIL_CPTR);
  SL_RF_bootMembershipCount = (uint **) new_vvector(1, SL_RF_ntree, NRUTIL_UPTR);
  SL_RF_oobMembershipFlag = (char **) new_vvector(1, SL_RF_ntree, NRUTIL_CPTR);
  SL_RF_ibgMembershipIndex = (uint **) new_vvector(1, SL_RF_ntree, NRUTIL_UPTR);
  SL_RF_oobMembershipIndex = (uint **) new_vvector(1, SL_RF_ntree, NRUTIL_UPTR);
  SL_RF_oobSize = uivector(1, SL_RF_ntree);
  SL_RF_ibgSize = uivector(1, SL_RF_ntree);
  SL_RF_maxDepth = uivector(1, SL_RF_ntree);
  SL_RF_orderedTreeIndex = uivector(1, SL_RF_ntree);
  for (i = 1; i <= SL_RF_ntree; i++) {
    SL_RF_orderedTreeIndex[i] = i;
  }
  SL_RF_serialTreeIndex = uivector(1, SL_RF_ntree);
  *root = (Node **) new_vvector(1, SL_RF_ntree, NRUTIL_NPTR);
  for (i = 1; i <= SL_RF_ntree; i++) {
    (*root)[i] = NULL;
  }
  if (SL_RF_ptnCount > 0) {
    SL_RF_pNodeMembership = (Node ***)     new_vvector(1, SL_RF_ntree, NRUTIL_NPTR2);
    SL_RF_pTermMembership = (Terminal ***) new_vvector(1, SL_RF_ntree, NRUTIL_NPTR2);
    SL_RF_pNodeList = (Node ***)     new_vvector(1, SL_RF_ntree, NRUTIL_NPTR2);
    SL_RF_pTermList = (Terminal ***) new_vvector(1, SL_RF_ntree, NRUTIL_NPTR2);
    SL_RF_pLeafCount = uivector(1, SL_RF_ntree);
  }
  if ( ( (SL_RF_opt & OPT_BOOT_TYP1) && !(SL_RF_opt & OPT_BOOT_TYP2)) ||
       (!(SL_RF_opt & OPT_BOOT_TYP1) && !(SL_RF_opt & OPT_BOOT_TYP2)) ) {
    for (i = 1; i <= SL_RF_subjSize; i++) {
      if(SL_RF_subjWeight[i] < 0) {
        SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
        SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
        SL_RF_nativeError("\nRF-SRC:  Subject-weight elements must be greater than or equal to zero:  %12.4f \n", SL_RF_subjWeight[i]);
        SL_RF_nativeExit();
      }
    }
    stackWeights(SL_RF_subjWeight,
                 SL_RF_subjSize,
                 &SL_RF_subjWeightType,
                 &SL_RF_subjWeightSorted,
                 &SL_RF_subjWeightDensitySize); 
  }
  SL_RF_getTreeIndex = uivector(1, SL_RF_ntree);
  if (mode == SL_RF_GROW) {
    for (i = 1; i <= SL_RF_ntree; i++) {
      SL_RF_getTreeIndex[i] = i;
    }
    SL_RF_getTreeCount = SL_RF_ntree;
  }
  else {
    SL_RF_getTreeCount = 0;    
    for (i = 1; i <= SL_RF_ntree; i++) {
      if (SL_RF_getTree[i] != 0) {
        SL_RF_getTreeIndex[++SL_RF_getTreeCount] = i;
      }
    }
  }
}
void unstackPreDefinedCommonArrays(char         mode,
                                   Node      ***nodeMembership,
                                   Terminal  ***tTermMembership,
                                   Terminal  ***tTermList,
                                   Node       **root) {
  free_new_vvector(nodeMembership, 1, SL_RF_ntree, NRUTIL_NPTR2);
  free_new_vvector(tTermMembership, 1, SL_RF_ntree, NRUTIL_TPTR2);
  free_new_vvector(tTermList, 1, SL_RF_ntree, NRUTIL_TPTR2);
  free_new_vvector(SL_RF_leafLinkedObjHead, 1, SL_RF_ntree, NRUTIL_LEAFPTR);
  free_new_vvector(SL_RF_leafLinkedObjTail, 1, SL_RF_ntree, NRUTIL_LEAFPTR);
  free_uivector(SL_RF_nodeCount, 1, SL_RF_ntree);
  free_new_vvector(SL_RF_bootMembershipIndex, 1, SL_RF_ntree, NRUTIL_UPTR);
  if ( (SL_RF_opt & OPT_BOOT_TYP1) && (SL_RF_opt & OPT_BOOT_TYP2) ) {
  }
  free_new_vvector(SL_RF_bootMembershipFlag, 1, SL_RF_ntree, NRUTIL_CPTR);
  free_new_vvector(SL_RF_bootMembershipCount, 1, SL_RF_ntree, NRUTIL_UPTR);
  free_new_vvector(SL_RF_oobMembershipFlag, 1, SL_RF_ntree, NRUTIL_CPTR);
  free_new_vvector(SL_RF_ibgMembershipIndex, 1, SL_RF_ntree, NRUTIL_UPTR);
  free_new_vvector(SL_RF_oobMembershipIndex, 1, SL_RF_ntree, NRUTIL_UPTR);
  free_uivector(SL_RF_oobSize, 1, SL_RF_ntree);
  free_uivector(SL_RF_ibgSize, 1, SL_RF_ntree);
  free_uivector(SL_RF_maxDepth, 1, SL_RF_ntree);
  free_uivector(SL_RF_orderedTreeIndex, 1, SL_RF_ntree);
  free_uivector(SL_RF_serialTreeIndex, 1, SL_RF_ntree);
  free_new_vvector(root, 1, SL_RF_ntree, NRUTIL_NPTR);
  if (SL_RF_ptnCount > 0) {
    free_new_vvector(SL_RF_pNodeMembership, 1, SL_RF_ntree, NRUTIL_NPTR2);
    free_new_vvector(SL_RF_pTermMembership, 1, SL_RF_ntree, NRUTIL_NPTR2);
    free_new_vvector(SL_RF_pNodeList, 1, SL_RF_ntree, NRUTIL_NPTR2);
    free_new_vvector(SL_RF_pTermList, 1, SL_RF_ntree, NRUTIL_NPTR2);
    free_uivector(SL_RF_pLeafCount, 1, SL_RF_ntree);
  }
  if ( (!(SL_RF_opt & OPT_BOOT_TYP1) && !(SL_RF_opt & OPT_BOOT_TYP2)) ||
       ( (SL_RF_opt & OPT_BOOT_TYP1) &&  (SL_RF_opt & OPT_BOOT_TYP2)) ) {
    unstackWeights(SL_RF_subjWeightType, SL_RF_subjSize, SL_RF_subjWeightSorted); 
  }
  free_uivector(SL_RF_getTreeIndex, 1, SL_RF_ntree);
}
void stackPreDefinedGrowthArrays(void) {
  uint i;
  if (SL_RF_opt & OPT_VIMP) {
    SL_RF_intrPredictorSize = SL_RF_xSize;
    SL_RF_intrPredictor = uivector(1, SL_RF_intrPredictorSize);
    for (i = 1; i <= SL_RF_intrPredictorSize; i++) {
      SL_RF_intrPredictor[i] = i;
    }
    SL_RF_importanceFlag = cvector(1, SL_RF_xSize);
    for (i = 1; i <= SL_RF_xSize; i++) {
      SL_RF_importanceFlag[i] = TRUE;
    }
  }
  else {
    SL_RF_intrPredictorSize = 0;    
  }
  stackWeights(SL_RF_xWeight,
               SL_RF_xSize,
               &SL_RF_xWeightType,
               &SL_RF_xWeightSorted,
               &SL_RF_xWeightDensitySize); 
  if(SL_RF_ySize > 0) {
    stackWeights(SL_RF_yWeight,
                 SL_RF_ySize,
                 &SL_RF_yWeightType,
                 &SL_RF_yWeightSorted,
                 &SL_RF_yWeightDensitySize); 
    SL_RF_yIndexZeroSize = 0;
    for (i = 1; i <= SL_RF_ySizeProxy; i++) {
      if (SL_RF_yWeight[SL_RF_yIndex[i]] == 0) {
        SL_RF_yIndexZero[++SL_RF_yIndexZeroSize] = SL_RF_yIndex[i];
      }
    }
  }
}
void unstackPreDefinedGrowthArrays(void) {
  if (SL_RF_opt & OPT_VIMP) {
    free_uivector(SL_RF_intrPredictor, 1, SL_RF_intrPredictorSize);
    free_cvector(SL_RF_importanceFlag, 1, SL_RF_xSize);
  }
  unstackWeights(SL_RF_xWeightType,
                 SL_RF_xSize,
                 SL_RF_xWeightSorted); 
  if(SL_RF_ySize > 0) {
    unstackWeights(SL_RF_yWeightType,
                   SL_RF_ySize,
                   SL_RF_yWeightSorted); 
  }
}
void stackPreDefinedRestoreArrays(void) {
  uint i;
  if (SL_RF_opt & OPT_VIMP) {
    checkInteraction();
    SL_RF_importanceFlag = cvector(1, SL_RF_xSize);
    for (i = 1; i <= SL_RF_xSize; i++) {
      SL_RF_importanceFlag[i] = FALSE;
    }
    for (i = 1; i <= SL_RF_intrPredictorSize; i++) {
      SL_RF_importanceFlag[SL_RF_intrPredictor[i]] = TRUE;
    }
  }
}
void unstackPreDefinedRestoreArrays(void) {
  if (SL_RF_opt & OPT_VIMP) {
    free_cvector(SL_RF_importanceFlag, 1, SL_RF_xSize);
  }
}
void stackPreDefinedPredictArrays(void) {
  uint i;
  SL_RF_fnodeMembership = (Node ***)     new_vvector(1, SL_RF_ntree, NRUTIL_NPTR2);
  SL_RF_ftTermMembership = (Terminal ***) new_vvector(1, SL_RF_ntree, NRUTIL_TPTR2);
  SL_RF_fidentityMembershipIndex = uivector(1, SL_RF_fobservationSize);
  for (i = 1; i <= SL_RF_fobservationSize; i++) {
    SL_RF_fidentityMembershipIndex[i] = i;
  }
  SL_RF_testMembershipFlag = cvector(1, SL_RF_fobservationSize);
  for (i = 1; i <= SL_RF_fobservationSize; i++) {
    SL_RF_testMembershipFlag[i] = ACTIVE;
  }
  if (SL_RF_opt & OPT_VIMP) {
    checkInteraction();
    SL_RF_importanceFlag = cvector(1, SL_RF_xSize);
    for (i = 1; i <= SL_RF_xSize; i++) {
      SL_RF_importanceFlag[i] = FALSE;
    }
    for (i = 1; i <= SL_RF_intrPredictorSize; i++) {
      SL_RF_importanceFlag[SL_RF_intrPredictor[i]] = TRUE;
    }
  }
}
void unstackPreDefinedPredictArrays(void) {
  free_new_vvector(SL_RF_fnodeMembership, 1, SL_RF_ntree, NRUTIL_NPTR2);
  free_new_vvector(SL_RF_ftTermMembership, 1, SL_RF_ntree, NRUTIL_TPTR2);
  free_uivector(SL_RF_fidentityMembershipIndex, 1, SL_RF_fobservationSize);
  free_cvector(SL_RF_testMembershipFlag, 1, SL_RF_fobservationSize);
  if (SL_RF_opt & OPT_VIMP) {
    free_cvector(SL_RF_importanceFlag, 1, SL_RF_xSize);
  }
}
void checkInteraction(void) {
  uint leadingIndex, i;
  if((SL_RF_intrPredictorSize <= 0) || (SL_RF_intrPredictorSize > SL_RF_xSize)) {
    SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
    SL_RF_nativeError("\nRF-SRC:  Number of predictors to be perturbed must be greater than zero and less than or equal to %10d:  %10d \n", SL_RF_xSize, SL_RF_intrPredictorSize);
    SL_RF_nativeExit();
  }
  uint *intrPredictorCopy = uivector(1, SL_RF_intrPredictorSize);
  for (i=1; i <= SL_RF_intrPredictorSize; i++) {
    intrPredictorCopy[i] = SL_RF_intrPredictor[i];
  }
  hpsortui(intrPredictorCopy, SL_RF_intrPredictorSize);
  leadingIndex = 1;
  for (i=2; i <= SL_RF_intrPredictorSize; i++) {
    if (intrPredictorCopy[i] > intrPredictorCopy[leadingIndex]) {
      leadingIndex++;
    }
  }
  if (SL_RF_intrPredictorSize != leadingIndex) {
    SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
    SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
    SL_RF_nativeError("\nRF-SRC:  Interaction terms are not unique.");
    SL_RF_nativeError("\nRF-SRC:  Only %10d of %10d are unique.", leadingIndex, SL_RF_intrPredictorSize);
    SL_RF_nativeExit();
  }
  free_uivector(intrPredictorCopy, 1, SL_RF_intrPredictorSize);
  for (i=1; i <= SL_RF_intrPredictorSize; i++) {
    if (SL_RF_intrPredictor[i] > SL_RF_xSize) {
      SL_RF_nativeError("\nRF-SRC:  *** ERROR *** ");
      SL_RF_nativeError("\nRF-SRC:  Parameter verification failed.");
      SL_RF_nativeError("\nRF-SRC:  Interaction terms are not coherent.");
      SL_RF_nativeError("\nRF-SRC:  Predictor encountered is %10d, maximum allowable is %10d.", SL_RF_intrPredictor[i], SL_RF_xSize);
      SL_RF_nativeExit();
    }
  }
}
void stackWeights(double *weight,
                  uint    size,
                  uint   *weightType,
                  uint  **weightSorted,
                  uint   *weightDensitySize) {
  char uniformFlag, integerFlag;
  double meanWeight;
  uint i;
  *weightSorted      = NULL;
  *weightDensitySize = 0;
  meanWeight = getMeanValue(weight, size);
  uniformFlag = TRUE;
  i = 0;
  while (uniformFlag && (i < size)) {
    ++i;
    if (fabs(weight[i] - meanWeight) > 0) {
      uniformFlag = FALSE;
    }
  }
  if (uniformFlag) {
    *weightType = SL_RF_WGHT_UNIFORM;
  } 
  else {
    integerFlag = TRUE;
    i = 0;
    while (integerFlag && (i < size)) {
      i++;
      if (fabs(round(weight[i]) - weight[i]) > 0.0) {
        integerFlag = FALSE;
      }
    }
    if(integerFlag) {
      *weightType = SL_RF_WGHT_INTEGER;
    }
    else {
      *weightType = SL_RF_WGHT_GENERIC;
    }
  }
  switch (*weightType) {
  case SL_RF_WGHT_UNIFORM:
    break;
  case SL_RF_WGHT_INTEGER:
    *weightSorted = uivector(1, size);
    indexx(size, weight, *weightSorted);
    *weightDensitySize = 0;
    for (i = 1; i <= size; i++) {
      *weightDensitySize += (uint) weight[i];
    }
    break;
  case SL_RF_WGHT_GENERIC:
    *weightSorted = uivector(1, size);
    indexx(size, weight, *weightSorted);
    break;
  }
}
void unstackWeights(uint    weightType,
                    uint    size,
                    uint   *weightSorted) {
  switch (weightType) {
  case SL_RF_WGHT_UNIFORM:
    break;
  case SL_RF_WGHT_INTEGER:
    free_uivector(weightSorted, 1, size);
    break;
  case SL_RF_WGHT_GENERIC:
    free_uivector(weightSorted, 1, size);
    break;
  }
}
