/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "importance.h"
#include "importanceAnti.h"
#include "importancePerm.h"
#include "importanceRand.h"
#include "survivalE.h"
#include "rfsrcUtil.h"
#include "polarity.h"
#include "nrutil.h"
#include "error.h"
Node *identifyExtrapolatedMembership (Node     *parent,
                                      double **yShadow,
                                      double **xShadow) {
  void *obsLocal;
  char daughterFlag;
  Node *result = parent;
  SplitInfo *info;
  daughterFlag = NEITHER;  
  if (((parent -> left) != NULL) && ((parent -> right) != NULL)) {
    info = parent -> splitInfo;
    if (info -> randomVar[1] > 0) {
      obsLocal = xShadow;
    }
    else {
      obsLocal = yShadow;
    }
    daughterFlag = getDaughterPolarity(0, info, 1, obsLocal);    
    if (daughterFlag == LEFT) {
      result = identifyExtrapolatedMembership(parent ->  left, yShadow, xShadow);
    }
    else {
      result = identifyExtrapolatedMembership(parent -> right, yShadow, xShadow);
    }
  }
  return result;
}
void getVimpMembership (char       mode,
                        uint       treeID,
                        Terminal **vimpMembership,
                        uint p) {
  char result;
  if (!(SL_RF_opt & OPT_VIMP)) {
    SL_RF_nativePrint("\nRF-SRC:  *** ERROR *** ");
    SL_RF_nativePrint("\nRF-SRC:  Attempt to compute variable importance though not requested.");
    SL_RF_nativePrint("\nRF-SRC:  Please Contact Technical Support.");
    SL_RF_nativeExit();
  }
  result = FALSE;
  switch (mode) {
  case SL_RF_PRED:
    result = TRUE;
    break;
  default:
    if (SL_RF_oobSize[treeID] > 0) {
      result = TRUE;
    }
    break;
  }
  if (result == TRUE) {
    if (!(SL_RF_opt & OPT_VIMP_TYP1) && !(SL_RF_opt & OPT_VIMP_TYP2)) {
      getAntiMembership(mode, treeID, vimpMembership, p);
    }
    else if ((SL_RF_opt & OPT_VIMP_TYP1) && !(SL_RF_opt & OPT_VIMP_TYP2)) { 
      getPermuteMembership(mode, treeID, vimpMembership, p);
    }
    else if (!(SL_RF_opt & OPT_VIMP_TYP1) && (SL_RF_opt & OPT_VIMP_TYP2)) { 
      getRandomMembership(mode, treeID, vimpMembership, p);
    }
    else {
      SL_RF_nativePrint("\nRF-SRC:  *** ERROR *** ");
      SL_RF_nativePrint("\nRF-SRC:  Unknown VIMP type encountered:  %10d", SL_RF_opt);
      SL_RF_nativePrint("\nRF-SRC:  Please Contact Technical Support.");
      SL_RF_nativeExit();
    }
  }
  else {
  }
}
void updateEnsembleVimp (char       mode,
                         uint       treeID,
                         Terminal **noiseMembership,
                         uint       xVarIdx) {
  uint  *membershipIndex;
  uint   membershipSize;
  double *denomPtr;
  switch (mode) {
  case SL_RF_PRED:
    membershipSize = SL_RF_fobservationSize;
    membershipIndex = SL_RF_fidentityMembershipIndex;
    break;
  default:
    membershipSize  = SL_RF_oobSize[treeID];
    membershipIndex = SL_RF_oobMembershipIndex[treeID];
    break;
  }
  denomPtr = SL_RF_vimpEnsembleDen[xVarIdx];
  for (uint i = 1; i <= membershipSize; i++) {
    uint ii = membershipIndex[i];
    Terminal *terminalNode = noiseMembership[ii];
    if ((terminalNode -> membrCount) > 0) {
      rfsrc_omp_atomic_update(&denomPtr[ii], 1.0);
#ifdef _OPENMP
      omp_set_lock(&(SL_RF_lockVimp[xVarIdx][ii]));
#endif
      if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
        for (uint k = 1; k <= SL_RF_eventTypeSize; k++) {
          SL_RF_vimpMRTstd[xVarIdx][k][ii] += terminalNode -> mortality[k];
        }
      }
      else {
        if (SL_RF_rTargetFactorCount > 0) {
          for (uint j = 1; j <= SL_RF_rTargetFactorCount; j++) {
            for (uint k = 1; k <= SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[j]]]; k++) {
              SL_RF_vimpCLSstd[xVarIdx][j][k][ii] += (double) (terminalNode -> multiClassProb)[SL_RF_rFactorMap[SL_RF_rTargetFactor[j]]][k] / (double) (terminalNode -> membrCount);
            }
          }
          if (SL_RF_optHigh & OPT_CSV) {          
            for (uint j = 1; j <= SL_RF_rTargetFactorCount; j++) {
              SL_RF_csvNumCLSptr[xVarIdx][j][ii] += 
                ( (terminalNode -> maxClass)[SL_RF_rFactorMap[SL_RF_rTargetFactor[j]]] ==
                  (uint) SL_RF_response[treeID][SL_RF_rFactorMap[SL_RF_rTargetFactor[j]]][ii] ) ? 1 : 0;
            }
          }
        }
        if (SL_RF_rTargetNonFactorCount > 0) {
          for (uint j = 1; j <= SL_RF_rTargetNonFactorCount; j++) {
            SL_RF_vimpRGRstd[xVarIdx][j][ii] += (terminalNode -> meanResponse)[SL_RF_rNonFactorMap[SL_RF_rTargetNonFactor[j]]];
          }
          if (SL_RF_optHigh & OPT_CSV) {          
            for (uint j = 1; j <= SL_RF_rTargetNonFactorCount; j++) {
              SL_RF_csvNumRGRptr[xVarIdx][j][ii] += 
                pow((terminalNode -> meanResponse)[SL_RF_rNonFactorMap[SL_RF_rTargetNonFactor[j]]] -
                    SL_RF_response[treeID][SL_RF_rNonFactorMap[SL_RF_rTargetNonFactor[j]]][ii], 2.0);
            }
          }
        }
        if ((SL_RF_rTargetFactorCount > 0) || (SL_RF_rTargetNonFactorCount > 0)) {
          if (SL_RF_optHigh & OPT_CSV) {
            SL_RF_csvDENptr[xVarIdx][ii] ++;
          }
        }
      }
#ifdef _OPENMP
      omp_unset_lock(&(SL_RF_lockVimp[xVarIdx][ii]));
#endif
    }
    else {
      if (!(SL_RF_opt & OPT_OUTC_TYPE)) {
        SL_RF_nativePrint("\nRF-SRC:  *** ERROR *** ");
        SL_RF_nativePrint("\nRF-SRC:  NA encountered for VIMP outcome in terminal node:  %10d", terminalNode -> nodeID);
        SL_RF_nativePrint("\nRF-SRC:  Please Contact Technical Support.");
        SL_RF_nativeExit();
      }
    }
  }  
}
void summarizePerturbedPerformance(char       mode,
                                   uint       treeID,
                                   uint       bb,
                                   uint       p,
                                   double   **responsePtr) {
  uint      obsSize;
  double   *vimpDenom;
  double   **ensembleMRT;
  double  ***ensembleCLS;
  double   **ensembleRGR;
  double    *vimpMRTptr;
  double   **vimpCLSptr;
  double    *vimpRGRptr;
  uint i, j, k;
  obsSize = (mode == SL_RF_PRED) ?  SL_RF_fobservationSize : SL_RF_observationSize;  
  vimpDenom = SL_RF_vimpEnsembleDen[p];
  if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
    if (!(SL_RF_opt & OPT_COMP_RISK)) {
      getEnsembleMortality(mode, treeID, obsSize, SL_RF_vimpMRTstd[p], vimpDenom, SL_RF_vimpMRTstd[p][1]);
    }
    else {
      getEnsembleMortalityCR(mode, treeID, obsSize, SL_RF_vimpMRTstd[p], vimpDenom, SL_RF_vimpMRTstd[p]);
    }
  }  
  else {
    if (SL_RF_rTargetFactorCount > 0) {
      for (i = 1; i <= obsSize; i++) {
        if(vimpDenom[i] > 0) {
          for (j = 1; j <= SL_RF_rTargetFactorCount; j++) {
            for (k = 1; k <= SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[j]]]; k++) {
              SL_RF_vimpCLSstd[p][j][k][i] = SL_RF_vimpCLSstd[p][j][k][i] / vimpDenom[i];
            }
          }
        }
      }
    }
    if (SL_RF_rTargetNonFactorCount > 0) {
      for (i = 1; i <= obsSize; i++) {
        if(vimpDenom[i] > 0) {
          for (j = 1; j <= SL_RF_rTargetNonFactorCount; j++) {
            SL_RF_vimpRGRstd[p][j][i] = SL_RF_vimpRGRstd[p][j][i] / vimpDenom[i];
          }
        }
      }
    }
  }
  ensembleMRT = NULL;
  ensembleCLS = NULL;
  ensembleRGR = NULL;
  vimpMRTptr = NULL;
  vimpCLSptr = NULL;
  vimpRGRptr = NULL;
  if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
    ensembleMRT =  SL_RF_vimpMRTstd[p];
  }
  else {
    if (SL_RF_rTargetFactorCount > 0) {
      ensembleCLS = SL_RF_vimpCLSstd[p];
    }
    if (SL_RF_rTargetNonFactorCount > 0) {
      ensembleRGR = SL_RF_vimpRGRstd[p];
    }
  }
  if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
    vimpMRTptr = SL_RF_vimpMRTblk[bb][p];
  }
  else {
    if (SL_RF_rTargetFactorCount > 0) {
      vimpCLSptr = SL_RF_vimpCLSblk[bb][p];
    }
    if (SL_RF_rTargetNonFactorCount > 0) {
      vimpRGRptr = SL_RF_vimpRGRblk[bb][p];
    }    
  }
  getPerformance(treeID,            
                 mode,
                 obsSize,
                 responsePtr,
                 vimpDenom,
                 ensembleMRT,       
                 ensembleCLS,       
                 ensembleRGR,       
                 vimpMRTptr,        
                 vimpCLSptr,        
                 vimpRGRptr);       
}
void finalizeVimpPerformance(char       mode) {
  double  ***vimpMRTptr;
  double ****vimpCLSptr;
  double  ***vimpRGRptr;
  double   **perfMRTptr;
  double  ***perfCLSptr;
  double   **perfRGRptr;
  uint xVimpCount;
  double result;
  uint cumDenomCount;
  uint genericBlockCount;
  uint i, j, k, p;
  if (SL_RF_opt & OPT_VIMP_JOIN) {
    xVimpCount = 1;
  }
  else {
    xVimpCount = SL_RF_intrPredictorSize;
  }
    vimpMRTptr = SL_RF_vimpMRTblk;
    vimpCLSptr = SL_RF_vimpCLSblk;
    vimpRGRptr = SL_RF_vimpRGRblk;
    perfMRTptr = SL_RF_perfMRTblk;
    perfCLSptr = SL_RF_perfCLSblk;
    perfRGRptr = SL_RF_perfRGRblk;
    genericBlockCount = SL_RF_perfBlockCount;
    if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
      for(p = 1; p <= xVimpCount; p++) {
        for (k = 1; k <= SL_RF_eventTypeSize; k++) {
          cumDenomCount = 0;
          result = 0.0;
          for (i = 1; i <= genericBlockCount; i++) {
            if(!SL_RF_nativeIsNaN(vimpMRTptr[i][p][k])) {
              if(!SL_RF_nativeIsNaN(perfMRTptr[i][k])) {
                result += vimpMRTptr[i][p][k] - perfMRTptr[i][k];
                cumDenomCount ++;
              }
            }
          }
          if (cumDenomCount != 0) {
            SL_RF_vimpMRTptr[p][k] = result / (double) cumDenomCount;
          }
          else {
            SL_RF_vimpMRTptr[p][k] = SL_RF_nativeNaN;
          }
        }
      }
    }
    else {
      if (SL_RF_rTargetFactorCount > 0) {
        for(p = 1; p <= xVimpCount; p++) {
          for (j = 1; j <= SL_RF_rTargetFactorCount; j++) {
            for (k = 1; k <= 1 + SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[j]]]; k++) {
              cumDenomCount = 0;
              result = 0.0;
              for (i = 1; i <= genericBlockCount; i++) {
                if(!SL_RF_nativeIsNaN(vimpCLSptr[i][p][j][k])) {
                  if(!SL_RF_nativeIsNaN(perfCLSptr[i][j][k])) {
                    result += vimpCLSptr[i][p][j][k] - perfCLSptr[i][j][k];
                    cumDenomCount ++;
                  }
                }
              }
              if (cumDenomCount != 0) {
                SL_RF_vimpCLSptr[p][j][k] = result / (double) cumDenomCount;
              }
              else {
                SL_RF_vimpCLSptr[p][j][k] = SL_RF_nativeNaN;
              }
            }
          }
        }
      }
      if (SL_RF_rTargetNonFactorCount > 0) {
        for(p = 1; p <= xVimpCount; p++) {
          for (j = 1; j <= SL_RF_rTargetNonFactorCount; j++) {
            cumDenomCount = 0;
            result = 0.0;
            for (i = 1; i <= genericBlockCount; i++) {
              if(!SL_RF_nativeIsNaN(vimpRGRptr[i][p][j])) {
                if(!SL_RF_nativeIsNaN(perfRGRptr[i][j])) {
                  result += vimpRGRptr[i][p][j] - perfRGRptr[i][j];
                  cumDenomCount ++;
                }
              }
            }
            if (cumDenomCount != 0) {
              SL_RF_vimpRGRptr[p][j] = result / (double) cumDenomCount;
            }
            else {
              SL_RF_vimpRGRptr[p][j] = SL_RF_nativeNaN;
            }
          }
        }
      }    
    }
}
void stackVimpMembership(char mode, Terminal ***membership) {
  uint obsSize;
  (*membership) = NULL;
  if (SL_RF_opt & OPT_VIMP) {
    switch (mode) {
    case SL_RF_PRED:
      obsSize = SL_RF_fobservationSize;
      break;
    default:
      obsSize = SL_RF_observationSize;
      break;
    }
    *membership = (Terminal **) new_vvector(1, obsSize, NRUTIL_TPTR);
  }
}
void unstackVimpMembership(char mode, Terminal **membership) {
  uint obsSize;
  if (SL_RF_opt & OPT_VIMP) {
    switch (mode) {
    case SL_RF_PRED:
      obsSize = SL_RF_fobservationSize;
      break;
    default:
      obsSize = SL_RF_observationSize;
      break;
    }
    free_new_vvector(membership, 1, obsSize, NRUTIL_TPTR);
  }
}
void normalizeBlockedEnsembleEstimates(char      mode,
                                       double  **ensembleMRTptr,
                                       double ***ensembleCLSptr,
                                       double  **ensembleRGRptr,
                                       double   *ensembleDen) {
  uint      obsSize;
  uint i, j, k;
  obsSize = (mode == SL_RF_PRED) ? SL_RF_fobservationSize : SL_RF_observationSize;
    if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
      for (i = 1; i <= obsSize; i++) {
        if (ensembleDen[i] != 0) {
          if (!(SL_RF_opt & OPT_COMP_RISK)) {
            ensembleMRTptr[1][i] = ensembleMRTptr[1][i] / ensembleDen[i];
          }
          else {
            for(j = 1; j <= SL_RF_eventTypeSize; j ++) {
              ensembleMRTptr[j][i] = ensembleMRTptr[j][i] / ensembleDen[i];
            }
          }
        }
        else {
        }
      }
    }  
    else {
      if (SL_RF_rTargetFactorCount > 0) {
        if (ensembleCLSptr != NULL) {
        for (i = 1; i <= obsSize; i++) {
          if (ensembleDen[i] != 0) {
            for (j = 1; j <= SL_RF_rTargetFactorCount; j++) {
              for (k = 1; k <= SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[j]]]; k++) {
                ensembleCLSptr[j][k][i] = ensembleCLSptr[j][k][i] / ensembleDen[i];
              }
            }
          }
          else {
          }
        }
        }
      }
      if (SL_RF_rTargetNonFactorCount > 0) {      
        if (ensembleRGRptr != NULL) {
        for (i = 1; i <= obsSize; i++) {
          if (ensembleDen[i] != 0) {
            for (j = 1; j <= SL_RF_rTargetNonFactorCount; j++) {
              ensembleRGRptr[j][i] = ensembleRGRptr[j][i] / ensembleDen[i];
            }
          }
          else {
          }
        }
        }
      }
    }
}
void resetBlockedEnsembleEstimates(char mode) {
  uint      obsSize, xVimpCount;
  uint i, j, k, p;
  obsSize = (mode == SL_RF_PRED) ? SL_RF_fobservationSize : SL_RF_observationSize;
  if (SL_RF_opt & OPT_VIMP_JOIN) {
    xVimpCount = 1;
  }
  else {
    xVimpCount = SL_RF_intrPredictorSize;
  }
  for (i = 1; i <= obsSize; i++) {
    SL_RF_blkEnsembleDen[i]  = 0.0;
    for (p = 1; p <= xVimpCount; p++) {
      SL_RF_vimpEnsembleDen[p][i] = 0.0;
    }
  }
  if ((SL_RF_timeIndex > 0) && (SL_RF_statusIndex > 0)) {
    for(j = 1; j <= SL_RF_eventTypeSize; j ++) {
      for (i = 1; i <= obsSize; i++) {
        SL_RF_blkEnsembleMRTnum[j][i] = 0.0;
        for (p = 1; p <= xVimpCount; p++) {
          SL_RF_vimpMRTstd[p][j][i] = 0.0;
        }
      }
    }
  }  
  else {
    if (SL_RF_rTargetFactorCount > 0) {
      for (j = 1; j <= SL_RF_rTargetFactorCount; j++) {
        for (k = 1; k <= SL_RF_rFactorSize[SL_RF_rFactorMap[SL_RF_rTargetFactor[j]]]; k++) {
          for (i = 1; i <= obsSize; i++) {
            SL_RF_blkEnsembleCLSnum[j][k][i] = 0.0;
            for (p = 1; p <= xVimpCount; p++) {
              SL_RF_vimpCLSstd[p][j][k][i] = 0.0;
            }
          }
        }
      }
    }
    if (SL_RF_rTargetNonFactorCount > 0) {      
      for (j = 1; j <= SL_RF_rTargetNonFactorCount; j++) {
        for (i = 1; i <= obsSize; i++) {
          SL_RF_blkEnsembleRGRnum[j][i] = 0.0;
          for (p = 1; p <= xVimpCount; p++) {
            SL_RF_vimpRGRstd[p][j][i] = 0.0;
          }
        }
      }
    }
  }
}
#ifdef _OPENMP
void rfsrc_omp_atomic_update(double *addr, double incr) {
#pragma omp atomic update
  (*addr) += incr;
}
#else
void rfsrc_omp_atomic_update(double *addr, double incr) { 
  (*addr) += incr;
}
#endif
uint getVimpRecoverySeedDimension(char mode, uint opt) {
  uint bnpSize;
  bnpSize = 0;
  if (opt & OPT_VIMP) {
      bnpSize = SL_RF_ntree;
  }
  return bnpSize;
}
