/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */
// *** THIS FILE IS AUTO GENERATED. DO NOT EDIT IT ***
#ifndef SL_RF_EXTERNAL_H
#define SL_RF_EXTERNAL_H

#include "global.h"
/*
#include "nativeInfo.h"
extern NATXDInfo **SL_RF_natXDInfoList;
extern NativeEnsembleInfo **SL_RF_nativeEnsembleInfoList;
extern uint       SL_RF_natXDInfoListSize;
extern uint       SL_RF_nativeEnsembleInfoListSize;
extern PS_arrayXD *SL_RF_rLevelsPYTH;
extern PS_arrayXD *SL_RF_xLevelsPYTH;
*/
/*
extern JNIEnv    *SL_RF_java_env;
extern jobject    SL_RF_java_obj;
extern jclass     SL_RF_java_cls;
extern jclass     SL_RF_java_except_cls;
extern jclass     SL_RF_java_hshmap_cls;
extern jmethodID  SL_RF_java_hshmap_constr;
extern jmethodID  SL_RF_java_hshmap_put;
extern jobject    SL_RF_java_hshmap_obj;
extern jclass     SL_RF_java_ens_cls;
extern jmethodID  SL_RF_java_ens_mid;
extern jmethodID  SL_RF_java_mid_log;
extern jmethodID  SL_RF_java_mid_logError;
extern jmethodID  SL_RF_java_mid_logExit;
extern NAT1DInfo **SL_RF_nat1DInfoList;
extern NAT2DInfo **SL_RF_nat2DInfoList;
extern NativeEnsembleInfo **SL_RF_nativeEnsembleInfoList;
extern uint       SL_RF_nat1DInfoListSize;
extern uint       SL_RF_nat2DInfoListSize;
extern uint       SL_RF_nativeEnsembleInfoListSize;
extern jobject    SL_RF_rLevelsJNIE;
extern jobject    SL_RF_xLevelsJNIE;
*/

#include <Rinternals.h>
#include "nativeInfo.h"
extern SEXP SL_RF_sexpVector[2];
extern SEXP      SL_RF_rLevelsSEXP;
extern SEXP      SL_RF_xLevelsSEXP;

#include "factor.h"
#include "leafLink.h"
#include "node.h"
#include "terminal.h"
#include "snpAuxiliaryInfo.h"
#include "split.h"
#include "quantile.h"
#include "sortedLink.h"
#include "sampling.h"
extern SNPAuxiliaryInfo **SL_RF_snpAuxiliaryInfoList;
extern SNPAuxiliaryInfo **SL_RF_incAuxiliaryInfoList;
extern uint SL_RF_stackCount;
extern uint SL_RF_nativeIndex;
extern uint SL_RF_incStackCount;
extern double   *SL_RF_cpuTime_;
extern uint     *SL_RF_treeID_;
extern uint     *SL_RF_nodeID_;
extern uint     *SL_RF_nodeSZ_;
extern uint     *SL_RF_blnodeID_;
extern uint     *SL_RF_brnodeID_;
extern int      **SL_RF_parmID_;
extern double   **SL_RF_contPT_;
extern uint     **SL_RF_mwcpSZ_;
extern uint     **SL_RF_fsrecID_;
extern uint     **SL_RF_mwcpPT_;
extern uint   **SL_RF_mwcpCT_;
extern ulong     SL_RF_totalNodeCount;
extern ulong     SL_RF_totalTerminalCount;
extern ulong     SL_RF_totalMWCPCount;
extern ulong    *SL_RF_restoreTreeOffset;
extern ulong   **SL_RF_restoreMWCPoffset;
extern uint     *SL_RF_orderedTreeIndex;
extern uint     *SL_RF_serialTreeIndex;
extern uint      SL_RF_serialTreeID;
extern uint      SL_RF_userTreeID;
extern uint     *SL_RF_restoreTreeID;
extern uint      SL_RF_perfBlockCount;
extern uint      SL_RF_serialBlockID;
extern double   *SL_RF_TN_SURV_;
extern double   *SL_RF_TN_MORT_;
extern double   *SL_RF_TN_NLSN_;
extern double   *SL_RF_TN_CSHZ_;
extern double   *SL_RF_TN_CIFN_;
extern double   *SL_RF_TN_REGR_;
extern uint     *SL_RF_TN_CLAS_;
extern int      *SL_RF_seed_;
extern int      *SL_RF_seedVimp_;
extern uint     *SL_RF_optLoGrow_;
extern uint      SL_RF_optLoGrow;
extern uint     *SL_RF_tLeafCount_;
extern uint     *SL_RF_tLeafCount;
extern double   *SL_RF_imputation_;
extern uint     *SL_RF_varUsed_;
extern double   *SL_RF_splitDepth_;
extern uint     *SL_RF_MEMB_ID_;
extern uint     *SL_RF_BOOT_CT_;
extern uint     *SL_RF_PRUN_ID_;
extern uint     *SL_RF_CASE_DPTH_;
extern uint     *SL_RF_RMBR_ID_;
extern uint     *SL_RF_AMBR_ID_;
extern uint     *SL_RF_OMBR_ID_;
extern uint     *SL_RF_IMBR_ID_;
extern uint     *SL_RF_TN_RCNT_;
extern uint     *SL_RF_TN_ACNT_;
extern uint     *SL_RF_TN_OCNT_;
extern uint     *SL_RF_TN_ICNT_;
extern uint     *SL_RF_OOB_SZ_;
extern uint     *SL_RF_IBG_SZ_;
extern double   *SL_RF_perfMRT_;
extern double   *SL_RF_perfCLS_;
extern double   *SL_RF_perfRGR_;
extern double   *SL_RF_holdMRT_;
extern double   *SL_RF_holdCLS_;
extern double   *SL_RF_holdRGR_;
extern uint     *SL_RF_holdBLK_;
extern double   *SL_RF_splitStatLOT_;
extern double   *SL_RF_emprRSK_;
extern double   *SL_RF_oobEmprRSK_;
extern double   *SL_RF_vimpMRT_;
extern double   *SL_RF_vimpCLS_;
extern double   *SL_RF_vimpRGR_;
extern double   *SL_RF_perfBlockMRT_;
extern double   *SL_RF_perfBlockCLS_;
extern double   *SL_RF_perfBlockRGR_;
extern double   *SL_RF_partial_SURV_;
extern double   *SL_RF_partial_CRSK_;
extern double   *SL_RF_partial_CLAS_;
extern double   *SL_RF_partial_REGR_;
extern double   *SL_RF_oobEnsembleSRG_;
extern double   *SL_RF_fullEnsembleSRG_;
extern double   *SL_RF_oobEnsembleCIF_;
extern double   *SL_RF_fullEnsembleCIF_;
extern double   *SL_RF_oobEnsembleSRV_;
extern double   *SL_RF_fullEnsembleSRV_;
extern double   *SL_RF_oobEnsembleMRT_;
extern double   *SL_RF_fullEnsembleMRT_;
extern double   *SL_RF_oobEnsembleCLS_;
extern double   *SL_RF_fullEnsembleCLS_;
extern double   *SL_RF_cseNumCLS_;
extern double   *SL_RF_csvNumCLS_;
extern double   *SL_RF_oobEnsembleRGR_;
extern double   *SL_RF_fullEnsembleRGR_;
extern double   *SL_RF_cseNumRGR_;
extern double   *SL_RF_csvNumRGR_;
extern uint     *SL_RF_cseDen_;
extern uint     *SL_RF_csvDen_;
extern double   *SL_RF_oobEnsembleQNT_;
extern double   *SL_RF_fullEnsembleQNT_;
extern double     *SL_RF_proximity_;
extern double     *SL_RF_distance_;
extern double     *SL_RF_weight_;
extern uint      SL_RF_opt;
extern uint      SL_RF_optHigh;
extern uint      SL_RF_optSup;
extern uint      SL_RF_splitRule;
extern uint      SL_RF_splitCustomIdx;
extern uint      SL_RF_nsplit;
extern uint      SL_RF_nImpute;
extern uint      SL_RF_ntree;
extern uint      SL_RF_nodeSize;
extern int       SL_RF_nodeDepth;
extern uint      SL_RF_crWeightSize;
extern double   *SL_RF_crWeight;
extern uint      SL_RF_perfBlock;
extern uint      SL_RF_mtry;
extern uint      SL_RF_vtry;
extern uint    **SL_RF_vtryArray;
extern uint      SL_RF_vtryMode;
extern uint      SL_RF_vtryBlockSize;
extern uint      SL_RF_subjSize;
extern double   *SL_RF_subjWeight;
extern uint      SL_RF_bootstrapSize;
extern uint    **SL_RF_bootstrapIn;
extern double   *SL_RF_xWeightStat;
extern double   *SL_RF_yWeight;
extern uint      SL_RF_xMarginalSize;
extern uint     *SL_RF_xMarginal;
extern uint     *SL_RF_xMarginalFlag;
extern double   *SL_RF_xWeight;
extern uint      SL_RF_ptnCount;
extern int       SL_RF_numThreads;
extern uint      SL_RF_observationSize;
extern uint      SL_RF_ySize;
extern uint      SL_RF_rTargetCount;
extern uint      SL_RF_rTargetFactorCount;
extern uint      SL_RF_rTargetNonFactorCount;
extern uint     *SL_RF_rTarget;
extern uint     *SL_RF_rTargetFactor;
extern uint     *SL_RF_rTargetNonFactor;
extern double  **SL_RF_responseIn;
extern double  **SL_RF_observationIn;
extern uint     *SL_RF_subjIn;
extern uint      SL_RF_xSize;
extern char     *SL_RF_xType;
extern uint     *SL_RF_xLevelsMax;
extern uint     *SL_RF_xLevelsCnt;
extern uint    **SL_RF_xLevels;
extern uint     *SL_RF_xtType;
extern uint     *SL_RF_stType;
extern char     *SL_RF_rType;
extern uint     *SL_RF_rLevelsMax;
extern uint     *SL_RF_rLevelsCnt;
extern uint    **SL_RF_rLevels;
extern uint      SL_RF_ytry;
extern double    SL_RF_wibsTau;
extern double   **SL_RF_qStarPlus;
extern double    *SL_RF_unoWeight;
extern double    SL_RF_vimpThreshold;
extern uint      SL_RF_fobservationSize;
extern uint      SL_RF_frSize;
extern double  **SL_RF_fresponseIn;
extern double  **SL_RF_fobservationIn;
extern uint      SL_RF_timeIndex;
extern uint      SL_RF_statusIndex;
extern uint     *SL_RF_yIndex;
extern uint      SL_RF_ySizeProxy;
extern uint     *SL_RF_yIndexZero;
extern uint      SL_RF_yIndexZeroSize;
extern char     *SL_RF_testMembershipFlag;  
extern uint      SL_RF_intrPredictorSize;
extern uint     *SL_RF_intrPredictor;
extern char     *SL_RF_importanceFlag;   
extern uint      SL_RF_partialType;
extern uint      SL_RF_partialXvar;
extern uint      SL_RF_partialLength;
extern double   *SL_RF_partialValue;
extern uint      SL_RF_partialLength2;
extern uint     *SL_RF_partialXvar2;
extern double   *SL_RF_partialValue2;
extern uint      SL_RF_partialTimeLength;
extern double   *SL_RF_partialTime;
extern double   *SL_RF_quantile;
extern uint      SL_RF_quantileSize;
extern double    SL_RF_qEpsilon;
extern uint      SL_RF_inv_2qEpsilon;
extern uint     *SL_RF_getTree;
extern uint      SL_RF_xWeightType;
extern uint     *SL_RF_xWeightSorted;
extern uint      SL_RF_xWeightDensitySize;
extern uint      SL_RF_yWeightType;
extern uint     *SL_RF_yWeightSorted;
extern uint      SL_RF_yWeightDensitySize;
extern uint      SL_RF_subjWeightType;
extern uint     *SL_RF_subjWeightSorted;
extern uint      SL_RF_subjWeightDensitySize;
extern uint      SL_RF_eventTypeSize;
extern uint      SL_RF_feventTypeSize;
extern uint      SL_RF_mStatusSize;
extern uint     *SL_RF_eventType;
extern uint     *SL_RF_eventTypeIndex;
extern uint     *SL_RF_eIndividualSize;
extern uint    **SL_RF_eIndividualIn;
extern uint     *SL_RF_subjSlot;
extern uint     *SL_RF_subjSlotCount;
extern uint    **SL_RF_subjList;
extern uint     *SL_RF_caseMap;
extern uint      SL_RF_subjCount;
extern uint      *SL_RF_classLevelSize;
extern uint     **SL_RF_classLevel;
extern uint     **SL_RF_classLevelIndex;
extern uint    ***SL_RF_cIndividualIn;
extern double   *SL_RF_timeInterest;
extern uint      SL_RF_timeInterestSize;
extern uint      SL_RF_sortedTimeInterestSize;
extern double   *SL_RF_masterTime;
extern uint      SL_RF_masterTimeSize;
extern uint     *SL_RF_masterTimeIndexIn;
extern uint     *SL_RF_masterToInterestTimeMap;
extern double    SL_RF_wibsTauTime;
extern uint      SL_RF_wibsTauTimeIdx;
extern uint      SL_RF_rFactorCount;
extern uint     *SL_RF_rFactorMap;
extern uint     *SL_RF_rFactorIndex;
extern uint     *SL_RF_rFactorSize;
extern uint     *SL_RF_rFactorSizeTest;
extern uint      SL_RF_mrFactorSize;
extern uint      SL_RF_fmrFactorSize;
extern uint     *SL_RF_mrFactorIndex;
extern uint     *SL_RF_fmrFactorIndex;
extern uint      SL_RF_rNonFactorCount;
extern uint     *SL_RF_rNonFactorMap;
extern uint     *SL_RF_rNonFactorIndex;
extern uint      SL_RF_xFactorCount;
extern uint     *SL_RF_xFactorMap;
extern uint     *SL_RF_xFactorIndex;
extern uint     *SL_RF_xFactorSize;
extern uint      SL_RF_mxFactorSize;
extern uint      SL_RF_fmxFactorSize;
extern uint     *SL_RF_mxFactorIndex;
extern uint     *SL_RF_fmxFactorIndex;
extern uint      SL_RF_xNonFactorCount;
extern uint     *SL_RF_xNonFactorMap;
extern uint     *SL_RF_xNonFactorIndex;
extern uint      SL_RF_rMaxFactorLevel;
extern uint      SL_RF_xMaxFactorLevel;
extern uint      SL_RF_maxFactorLevel;
extern char      SL_RF_mStatusFlag;
extern char      SL_RF_mTimeFlag;
extern char      SL_RF_mResponseFlag;
extern char      SL_RF_mPredictorFlag;
extern char      SL_RF_fmStatusFlag;
extern char      SL_RF_fmTimeFlag;
extern char      SL_RF_fmResponseFlag;
extern char      SL_RF_fmPredictorFlag;
extern uint     *SL_RF_mRecordMap;
extern uint     *SL_RF_fmRecordMap;
extern uint      SL_RF_mRecordSize;
extern uint      SL_RF_fmRecordSize;
extern uint     *SL_RF_mRecordIndex;
extern uint     *SL_RF_fmRecordIndex;
extern uint      SL_RF_mpIndexSize;
extern uint      SL_RF_fmpIndexSize;
extern int     **SL_RF_mpSign;
extern int     **SL_RF_fmpSign;
extern int      *SL_RF_mpIndex;
extern int      *SL_RF_fmpIndex;
extern uint     *SL_RF_getTreeIndex;
extern uint      SL_RF_getTreeCount;
extern SplitRuleObj *SL_RF_splitRuleObj;
extern double   **SL_RF_importancePtr;
extern double **SL_RF_sImputeResponsePtr;
extern double **SL_RF_sImputePredictorPtr;
extern double **SL_RF_sImputeDataPtr;
extern uint  **SL_RF_MEMB_ID_ptr;
extern uint  **SL_RF_BOOT_CT_ptr;
extern uint  **SL_RF_PRUN_ID_ptr;
extern uint  **SL_RF_CASE_DPTH_ptr;
extern uint     **SL_RF_treeID_ptr;
extern uint     **SL_RF_nodeID_ptr;
extern uint     **SL_RF_nodeSZ_ptr;
extern uint     **SL_RF_blnodeID_ptr;
extern uint     **SL_RF_brnodeID_ptr;
extern int     ***SL_RF_parmID_ptr;
extern double  ***SL_RF_contPT_ptr;
extern uint    ***SL_RF_mwcpSZ_ptr;
extern uint    ***SL_RF_fsrecID_ptr;
extern uint    ***SL_RF_mwcpPT_ptr;
extern uint     **SL_RF_mwcpCT_ptr;
extern double   **SL_RF_spltST_ptr;
extern uint     **SL_RF_dpthST_ptr;
extern uint  **SL_RF_RMBR_ID_ptr;
extern uint  **SL_RF_AMBR_ID_ptr;
extern uint  **SL_RF_OMBR_ID_ptr;
extern uint  **SL_RF_IMBR_ID_ptr;
extern uint  **SL_RF_TN_RCNT_ptr;
extern uint  **SL_RF_TN_ACNT_ptr;
extern uint  **SL_RF_TN_OCNT_ptr;
extern uint  **SL_RF_TN_ICNT_ptr;
extern LeafLinkedObj **SL_RF_leafLinkedObjHead;
extern LeafLinkedObj **SL_RF_leafLinkedObjTail;
extern double **SL_RF_proximityPtr;
extern double **SL_RF_proximityDenPtr;
extern double  *SL_RF_proximityDen;
extern double **SL_RF_distancePtr;
extern double **SL_RF_distanceDenPtr;
extern double  *SL_RF_distanceDen;
extern uint    SL_RF_rejectedTreeCount;
extern uint    SL_RF_validTreeCount;
extern uint    SL_RF_stumpedTreeCount;
extern double  ***SL_RF_TN_SURV_ptr;
extern double  ***SL_RF_TN_MORT_ptr;
extern double  ***SL_RF_TN_NLSN_ptr;
extern double ****SL_RF_TN_CSHZ_ptr;
extern double ****SL_RF_TN_CIFN_ptr;
extern double  ***SL_RF_TN_REGR_ptr;
extern uint   ****SL_RF_TN_CLAS_ptr;
extern Terminal ****SL_RF_vimpMembership;
extern Terminal ***SL_RF_partMembership;
extern double  ***SL_RF_vimpMRTstd;
extern double ****SL_RF_vimpCLSstd;
extern double  ***SL_RF_vimpRGRstd;
extern double   **SL_RF_vimpEnsembleDen;
extern double   **SL_RF_perfMRTptr;
extern double  ***SL_RF_perfCLSptr;
extern double   **SL_RF_perfRGRptr;
extern double   **SL_RF_cseNumCLSptr;
extern double   **SL_RF_cseNumRGRptr;
extern uint      *SL_RF_cseDENptr;
extern double  ***SL_RF_csvNumCLSptr;
extern double  ***SL_RF_csvNumRGRptr;
extern uint     **SL_RF_csvDENptr;
extern double   **SL_RF_blkEnsembleMRTnum;
extern double  ***SL_RF_blkEnsembleCLSnum;
extern double   **SL_RF_blkEnsembleRGRnum;
extern double    *SL_RF_blkEnsembleDen;
extern double  ***SL_RF_vimpMRTblk;
extern double ****SL_RF_vimpCLSblk;
extern double  ***SL_RF_vimpRGRblk;
extern double   **SL_RF_perfMRTblk;
extern double  ***SL_RF_perfCLSblk;
extern double   **SL_RF_perfRGRblk;
extern double   **SL_RF_vimpMRTptr;
extern double  ***SL_RF_vimpCLSptr;
extern double   **SL_RF_vimpRGRptr;
extern double  ***SL_RF_holdMRTptr;
extern double ****SL_RF_holdCLSptr;
extern double  ***SL_RF_holdRGRptr;
extern double  ****SL_RF_holdMRTstd;
extern double *****SL_RF_holdCLSstd;
extern double  ****SL_RF_holdRGRstd;
extern double   ***SL_RF_holdEnsembleDen;
extern uint **SL_RF_holdoutMap;
extern uint *SL_RF_holdBLKptr;
extern uint **SL_RF_runningHoldoutCount;
extern uint ***SL_RF_blockSerialTreeIndex;
extern double   **SL_RF_splitStatLOTptr;
extern double   **SL_RF_emprRSKptr;
extern double   **SL_RF_oobEmprRSKptr;
extern double  ****SL_RF_partSURVptr;
extern double  ****SL_RF_partCLASptr;
extern double   ***SL_RF_partREGRptr;
extern double ***SL_RF_oobEnsembleSRGptr;
extern double ***SL_RF_fullEnsembleSRGptr;
extern double ***SL_RF_oobEnsembleCIFptr;
extern double ***SL_RF_fullEnsembleCIFptr;
extern double  **SL_RF_oobEnsembleSRVptr;
extern double  **SL_RF_fullEnsembleSRVptr;
extern double  **SL_RF_oobEnsembleMRTptr;
extern double  **SL_RF_fullEnsembleMRTptr;
extern double ***SL_RF_oobEnsembleCLSptr;
extern double ***SL_RF_fullEnsembleCLSptr;
extern double  **SL_RF_oobEnsembleRGRptr;
extern double  **SL_RF_fullEnsembleRGRptr;
extern double  ***SL_RF_oobEnsembleQNTptr;
extern double  ***SL_RF_fullEnsembleQNTptr;
extern uint    **SL_RF_oobQuantileStreamSize;
extern uint    **SL_RF_fullQuantileStreamSize;
extern LookUpInfo *** SL_RF_oobQuantileSearchTree;
extern LookUpInfo *** SL_RF_fullQuantileSearchTree;
extern QuantileObj ***SL_RF_oobQuantileHead;
extern QuantileObj ***SL_RF_oobQuantileTail;
extern uint         **SL_RF_oobQuantileLinkLength;
extern QuantileObj ***SL_RF_fullQuantileHead;
extern QuantileObj ***SL_RF_fullQuantileTail;
extern uint         **SL_RF_fullQuantileLinkLength;
extern double ***SL_RF_oobEnsembleSRGnum;
extern double ***SL_RF_fullEnsembleSRGnum;
extern double ***SL_RF_oobEnsembleCIFnum;
extern double ***SL_RF_fullEnsembleCIFnum;
extern double  **SL_RF_oobEnsembleSRVnum;
extern double  **SL_RF_fullEnsembleSRVnum;
extern double  **SL_RF_oobEnsembleMRTnum;
extern double  **SL_RF_fullEnsembleMRTnum;
extern double ***SL_RF_oobEnsembleCLSnum;
extern double ***SL_RF_fullEnsembleCLSnum;
extern double  **SL_RF_oobEnsembleRGRnum;
extern double  **SL_RF_fullEnsembleRGRnum;
extern double   *SL_RF_oobEnsembleDen;
extern double   *SL_RF_fullEnsembleDen;
extern double ***SL_RF_splitDepthPtr;
extern double **SL_RF_weightPtr;
extern uint    *SL_RF_weightDenom;
extern char    **SL_RF_dmRecordBootFlag;
extern uint   **SL_RF_varUsedPtr;
extern uint    *SL_RF_oobSize;
extern uint    *SL_RF_ibgSize;
extern uint    *SL_RF_nodeCount;
extern uint   **SL_RF_mwcpPtr;
extern uint    *SL_RF_pLeafCount;
extern uint    *SL_RF_maxDepth;
extern Node    **SL_RF_root;
extern Node   ***SL_RF_nodeMembership;  
extern Node   ***SL_RF_fnodeMembership;
extern Node   ***SL_RF_pNodeMembership;
extern Node   ***SL_RF_pNodeList;
extern Terminal   ***SL_RF_tTermMembership;
extern Terminal   ***SL_RF_ftTermMembership;
extern uint       ***SL_RF_utTermMembership;
extern uint        **SL_RF_utTermMembershipCount;
extern uint        **SL_RF_utTermMembershipAlloc;
extern Terminal   ***SL_RF_pTermMembership;
extern Terminal   ***SL_RF_tTermList;
extern Terminal   ***SL_RF_pTermList;
extern LeafLinkedObj ***SL_RF_hTermMembership;
extern uint    **SL_RF_bootMembershipIndex;
extern uint     *SL_RF_identityMembershipIndex;
extern uint      SL_RF_identityMembershipIndexSize;
extern uint     *SL_RF_fidentityMembershipIndex;
extern char    **SL_RF_bootMembershipFlag;
extern uint    **SL_RF_bootMembershipCount;
extern char    **SL_RF_oobMembershipFlag;
extern uint    **SL_RF_ibgMembershipIndex;
extern uint    **SL_RF_oobMembershipIndex;
extern double  **SL_RF_status;
extern double  **SL_RF_time;
extern double ***SL_RF_response;
extern double  **SL_RF_ftime;
extern double  **SL_RF_fstatus;
extern double ***SL_RF_fresponse;
extern double ***SL_RF_observation;
extern double ***SL_RF_fobservation;
extern uint    ***SL_RF_observationRank;
extern uint     **SL_RF_observationUniqueSize;
extern SortedLinkedObj **SL_RF_sortedLinkedHead;
extern SortedLinkedObj **SL_RF_sortedLinkedTail;
extern uint    **SL_RF_masterTimeIndex;
extern Factor ***SL_RF_factorList;
extern double   *SL_RF_rFactorThreshold;
extern uint     *SL_RF_rFactorMinority;
extern uint     *SL_RF_rFactorMajority;
extern char     *SL_RF_rFactorMinorityFlag;
extern void (*acquireTree) (char mode, uint r, uint b);
extern Node *(*antiMembership) (uint     treeID,
                         Node    *parent,
                         uint     individual,
                         uint     vimpX,
                         double **xArray);
extern Node *(*randomMembership) (uint     treeID,
                           Node    *parent,
                           uint     individual,
                           uint     vimpX,
                           double **xArray);
extern Node *(*getMembership) (uint     treeID,
                        Node    *parent,
                        uint     individual,
                        double **xArray);
extern void (*partialMembership) (uint       treeID,
                           Node      *parent,
                           uint       partialIndex,
                           uint      *allMembrIndx,
                           uint       obsSize,
                           double   **xArray,
                           Terminal **membership);
extern char (*getVariance) (uint, uint*, uint, uint*, double*, double*, double*);
extern char (*growTree) (uint, char, char, uint, Node*, uint*, uint*, uint*);
extern char (*getPreSplitResult) (uint, Node*, char, char);
extern DistributionObj *(*stackRandomCovariates) (uint, Node*);
extern void (*unstackRandomCovariates) (uint, DistributionObj*);
extern char (*selectRandomCovariates) (uint, Node*, DistributionObj*, char*, uint*, uint*);
extern DistributionObj *(*stackRandomResponses) (uint, Node*);
extern void (*unstackRandomResponses) (uint, DistributionObj*);
extern char (*selectRandomResponses) (uint, Node*, DistributionObj*, uint*, uint*);
extern uint (*virtuallySplitNode) (uint, Node*, char, uint, double*, uint*, void*, uint, char*, uint*, uint, uint*);
extern uint (*stackAndConstructSplitVector) (uint treeID, Node *parent, uint covariate);
extern char (*updateMaximumSplit) (uint    treeID,
                            Node   *parent,
                            double  delta,
                            uint    covariate,
                            uint    index,
                            char    factorFlag,
                            uint    mwcpSizeAbsolute,
                            uint    repMembrSize,
                            char  **polarity,
                            void   *splitVectorPtr,
                            SplitInfoMax *splitInfoMax);
extern char (*forkAndUpdate) (uint       treeID,
                       Node      *parent,
                       uint      *repMembrIndx,
                       uint       repMembrSize,
                       uint      *allMembrIndx,
                       uint       allMembrSize,
                       char       multImpFlag,
                       SplitInfo *info,
                       uint      *leafCount,
                       Node     **nodeMembership);
extern void (*freeNode) (Node *parent);
extern void (*unstackSplitVector) (uint   treeID,
                            Node  *parent,
                            uint   splitLength,
                            char   factorFlag,
                            uint   splitVectorSize,
                            uint   mwcpSizeAbsolute,
                            char   deterministicSplitFlag,
                            void  *splitVectorPtr,
                            char   multImpFlag,
                            uint  *indxx);
extern char (*randomSplit) (uint, Node*, SplitInfoMax*, GreedyObj*, char);
extern char (*regressionXwghtSplit) (uint, Node*, SplitInfoMax*, GreedyObj*, char);
extern char (*classificationXwghtSplit) (uint, Node*, SplitInfoMax*, GreedyObj*, char);
extern char (*multivariateSplit) (uint, Node*, SplitInfoMax*, GreedyObj*, char);
extern char (*unsupervisedSplit) (uint, Node*, SplitInfoMax*, GreedyObj*, char);
extern void (*getConditionalClassificationIndex) (uint,
                                           uint,
                                           double*,
                                           double**,
                                           double*,
                                           double*,
                                           double*);
extern double (*getGMeanIndex) (uint,
                         uint,
                         double*,
                         double*,
                         double*);
extern float (*ran1A) (uint);
extern void  (*randomSetChain) (uint, int);
extern int   (*randomGetChain) (uint);
extern float (*ran1B) (uint);
extern void  (*randomSetUChain) (uint, int);
extern int   (*randomGetUChain) (uint);
extern void  (*randomSetUChainVimp) (uint, int);
extern int   (*randomGetUChainVimp) (uint);
extern float (*ran1D) (uint);
extern void  (*randomSetChainVimp) (uint, int);
extern int   (*randomGetChainVimp) (uint);
extern uint SL_RF_forestChunkSize;
extern uint SL_RF_forestChunkCount;
extern uint *SL_RF_forestChunkSizeActual;
extern uint **SL_RF_forestChunk;
#ifdef _OPENMP
extern omp_lock_t   *SL_RF_lockPartial;
extern omp_lock_t  **SL_RF_lockWeight;
extern omp_lock_t   *SL_RF_lockWeightRow;
extern omp_lock_t  **SL_RF_lockVimp;
extern omp_lock_t   *SL_RF_lockVimpRow;
extern omp_lock_t   *SL_RF_lockVimpCol;
extern omp_lock_t  **SL_RF_lockVimpHoldout;
extern omp_lock_t  **SL_RF_lockMRToens;
extern omp_lock_t  **SL_RF_lockMRTfens;
extern omp_lock_t  **SL_RF_lockSRVoens;
extern omp_lock_t  **SL_RF_lockSRVfens;
extern omp_lock_t ***SL_RF_lockSRGoens;
extern omp_lock_t ***SL_RF_lockSRGfens;
extern omp_lock_t ***SL_RF_lockCIFoens;
extern omp_lock_t ***SL_RF_lockCIFfens;
extern omp_lock_t ***SL_RF_lockCLSoens;
extern omp_lock_t ***SL_RF_lockCLSfens;
extern omp_lock_t   *SL_RF_lockDENoens;
extern omp_lock_t   *SL_RF_lockDENfens;
extern omp_lock_t   *SL_RF_lockQNToens;
extern omp_lock_t   *SL_RF_lockQNTfens;
extern omp_lock_t      SL_RF_lockPerf;
extern omp_lock_t      SL_RF_lockEnsbUpdtCount;
#endif
extern uint SL_RF_ensbUpdtCount;
extern customFunction customFunctionArray[4][16];
extern uint   SL_RF_userTraceFlag;
extern time_t SL_RF_userTimeStart;
extern time_t SL_RF_userTimeSplit;  

#endif
