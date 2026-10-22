/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <time.h>
#ifdef _OPENMP
#include <omp.h>
#endif

#include <R_ext/Print.h>
#include <Rdefines.h>
#define SL_RF_nativePrint printR
#define SL_RF_nativeError printR
#define SL_RF_nativeExit  exit2R
#define SL_RF_nativeNaN NA_REAL
#define SL_RF_nativeIsNaN ISNA

/*
#include <jni.h>
#define SL_RF_nativePrint printJ
#define SL_RF_nativeError errorJ
#define SL_RF_nativeExit  exit2J
#define SL_RF_nativeNaN nan("")
#define SL_RF_nativeIsNaN isnan
*/
/*
#define SL_RF_nativePrint printP
#define SL_RF_nativeError printP
#define SL_RF_nativeExit  exit2P
#define SL_RF_nativeNaN NAN
#define SL_RF_nativeIsNaN isnan
*/
#ifndef NULL
#define NULL 0
#endif
#ifndef TRUE
#define TRUE      0x01
#endif
#ifndef FALSE
#define FALSE     0x00
#endif
#ifndef uint
typedef unsigned int  uint;
#endif
#ifndef ulong
typedef unsigned long ulong;
#endif
#define SL_RF_OUTP_ID   0  
#define SL_RF_STRG_ID   1  
#define SL_RF_ASRG_ID   2  
#define SL_RF_OSRG_ID   3  
#define SL_RF_ACIF_ID   4  
#define SL_RF_OCIF_ID   5  
#define SL_RF_ASRV_ID   6  
#define SL_RF_OSRV_ID   7  
#define SL_RF_AMRT_ID   8  
#define SL_RF_OMRT_ID   9  
#define SL_RF_ACLS_ID  10  
#define SL_RF_OCLS_ID  11  
#define SL_RF_ARGR_ID  12  
#define SL_RF_ORGR_ID  13  
#define SL_RF_ER_SURV  14  
#define SL_RF_ER_CLAS  15  
#define SL_RF_ER_REGR  16  
#define SL_RF_PROX_ID  17  
#define SL_RF_LEAF_CT  18  
#define SL_RF_SEED_ID  19  
#define SL_RF_SEED_VM  20  
#define SL_RF_AQNT_ID  21  
#define SL_RF_OQNT_ID  22  
#define SL_RF_BLK_SRG  23  
#define SL_RF_BLK_CLS  24  
#define SL_RF_BLK_RGR  25  
#define SL_RF_VMP_SRG  26  
#define SL_RF_VMP_CLS  27  
#define SL_RF_VMP_RGR  28  
#define SL_RF_MISS_ID  29  
#define SL_RF_XXXX_30  30  
#define SL_RF_VUSE_ID  31  
#define SL_RF_DPTH_ID  32  
#define SL_RF_MEMB_ID  33  
#define SL_RF_PRUN_ID  34  
#define SL_RF_BOOT_CT  35  
#define SL_RF_RMBR_ID  36  
#define SL_RF_AMBR_ID  37  
#define SL_RF_TN_RCNT  38  
#define SL_RF_TN_ACNT  39  
#define SL_RF_SPLT_ST  40  
#define SL_RF_DPTH_ST  41  
#define SL_RF_WGHT_ID  42  
#define SL_RF_TN_SURV  43  
#define SL_RF_TN_MORT  44  
#define SL_RF_TN_NLSN  45  
#define SL_RF_TN_CSHZ  46  
#define SL_RF_TN_CIFN  47  
#define SL_RF_TN_REGR  48  
#define SL_RF_TN_CLAS  49  
#define SL_RF_50       50  
#define SL_RF_CSE_RGR  51  
#define SL_RF_CSV_RGR  52  
#define SL_RF_CSE_CLS  53  
#define SL_RF_CSV_CLS  54  
#define SL_RF_XXXX_55  55  
#define SL_RF_PART_SR   56  
#define SL_RF_PART_CL   57  
#define SL_RF_PART_RG   58  
#define SL_RF_DIST_ID   59  
#define SL_RF_TREE_ID   60  
#define SL_RF_NODE_ID   61  
#define SL_RF_PARM_ID   62  
#define SL_RF_CONT_PT   63  
#define SL_RF_MWCP_SZ   64  
#define SL_RF_MWCP_PT   65  
#define SL_RF_MWCP_CT   66  
#define SL_RF_HC_DIM    67  
#define SL_RF_CONT_PTR  68  
#define SL_RF_69        69  
#define SL_RF_70        70  
#define SL_RF_71        71  
#define SL_RF_72          72  
#define SL_RF_EMP_RSK     73  
#define SL_RF_OEMP_RSK    74  
#define SL_RF_STAT_LOT    75  
#define SL_RF_HLDOUT_BLK  76  
#define SL_RF_HLDOUT_SRG  77  
#define SL_RF_HLDOUT_CLS  78  
#define SL_RF_HLDOUT_RGR  79  
#define SL_RF_80            80  
#define SL_RF_81            81  
#define SL_RF_82            82  
#define SL_RF_83            83  
#define SL_RF_84            84  
#define SL_RF_85            85  
#define SL_RF_86            86  
#define SL_RF_87            87  
#define SL_RF_88            88  
#define SL_RF_89            89  
#define SL_RF_90            90  
#define SL_RF_91            91  
#define SL_RF_92            92  
#define SL_RF_93            93  
#define SL_RF_CSE_DEN       94  
#define SL_RF_CSV_DEN       95  
#define SL_RF_BL_NODE_ID     96  
#define SL_RF_BR_NODE_ID     97  
#define SL_RF_FS_REC_ID      98  
#define SL_RF_NODE_SZ        99  
#define SL_RF_CASE_DEPTH    100  
#define SL_RF_OPT_LO_GROW   101  
#define SL_RF_OPT_HI_GROW   102  
#define SL_RF_CPU_TIME      103  
#define SL_RF_TN_OCNT       104  
#define SL_RF_TN_ICNT       105  
#define SL_RF_OMBR_ID       106 
#define SL_RF_IMBR_ID       107 
#define SL_RF_OOB_SZ        108 
#define SL_RF_IBG_SZ        109 
#define SL_RF_SEXP_CNT      110  
#define SL_RF_SEXP_ASCII_SIZE 16
#define OPT_FENS      0x00000001 
#define OPT_OENS      0x00000002 
#define OPT_PERF      0x00000004 
#define OPT_PERF_CALB 0x00000008 
#define OPT_LEAF      0x00000010 
#define OPT_TREE      0x00000020 
#define OPT_SEED      0x00000040 
#define OPT_MISS_OUT  0x00000080 
#define OPT_VIMP_TYP1 0x00000100 
#define OPT_VIMP_TYP2 0x00000200 
#define OPT_VIMP_JOIN 0x00000400 
#define OPT_CASE_DPTH 0x00000800 
#define OPT_VARUSED_F 0x00001000 
#define OPT_VARUSED_T 0x00002000 
#define OPT_PERF_GMN2 0x00004000 
#define OPT_CLAS_RFQ  0x00008000 
#define OPT_IMPU_ONLY 0x00010000 
#define OPT_OUTC_TYPE 0x00020000 
#define OPT_EMPR_RISK 0x00040000 
#define OPT_BOOT_TYP1 0x00080000 
#define OPT_BOOT_TYP2 0x00100000 
#define OPT_COMP_RISK 0x00200000 
#define OPT_SPLDPTH_1 0x00400000 
#define OPT_SPLDPTH_2 0x00800000 
#define OPT_QUANTLE   0x01000000 
#define OPT_VIMP      0x02000000 
#define OPT_ANON      0x04000000 
#define OPT_NODE_STAT 0x08000000 
#define OPT_PROX      0x10000000 
#define OPT_PROX_IBG  0x20000000 
#define OPT_PROX_OOB  0x40000000 
#define OPT_PROX_FUL  0x60000000 
#define OPT_WGHT      0x00000001 
#define OPT_WGHT_IBG  0x00000002 
#define OPT_WGHT_OOB  0x00000004 
#define OPT_WGHT_FUL  0x00000006 
#define OPT_MISS_SKIP 0x00000010 
#define OPT_MEMB_PRUN 0x00000020 
#define OPT_MEMB_USER 0x00000040 
#define OPT_SPLT_CUST 0x00000F00 
#define OPT_BOOT_SWOR 0x00001000 
#define OPT_TREE_ERR  0x00002000 
#define OPT_PART_PLOT 0x00004000 
#define OPT_DATA_PASG 0x00008000 
#define OPT_MEMB_OUTG 0x00010000 
#define OPT_MEMB_INCG 0x00020000 
#define OPT_TERM_OUTG 0x00040000 
#define OPT_TERM_INCG 0x00080000 
#define OPT_DIST      0x00100000 
#define OPT_DIST_IBG  0x00200000 
#define OPT_DIST_OOB  0x00400000 
#define OPT_DIST_FUL  0x00600000 
#define OPT_JIT_TOP   0x00800000 
#define SL_RF_HI_24      0x01000000 
#define SL_RF_HI_25      0x02000000 
#define SL_RF_HI_26      0x04000000 
#define OPT_DATA_PASP 0x08000000 
#define OPT_CSE       0x10000000 
#define OPT_CSV       0x20000000 
#define OPT_EXPERMNT1 0x00000001 
#define OPT_EXPERMNT2 0x00000002 
#define OPT_EXPERMNT3 0x00000004 
#define SL_RF_PART_MORT 1
#define SL_RF_PART_NLSN 2
#define SL_RF_PART_SURV 3
#define SL_RF_PART_YRLS 1
#define SL_RF_PART_CIFN 2 
#define SL_RF_PART_CHFN 3
#define ACTIVE    0x02
#define LEFT      0x01
#define RIGHT     0x02
#define NEITHER   0x00
#define BOTH      0x03
#define EPSILON 1.0e-9
#define SVD_EPS 1.0e-9
#define SL_RF_GROW   0x01
#define SL_RF_PRED   0x02
#define SL_RF_REST   0x04
#define SL_RF_PART   0x08
#define SURV_LGRNK   1
#define SURV_LRSCR   2
#define SURV_CR_LAU  3
#define RAND_SPLIT   4
#define REGR_NRM     5 
#define REGR_WT_OFF  6 
#define REGR_WT_HVY  7 
#define CLAS_NRM     8 
#define CLAS_WT_OFF  9 
#define CLAS_WT_HVY 10 
#define USPV_NRM    11 
#define USPV_WT_OFF 12 
#define USPV_WT_HVY 13 
#define MV_NRM      14 
#define MV_WT_OFF   15 
#define MV_WT_HVY   16 
#define CUST_SPLIT  17
#define REGR_QUANT  18
#define LARG_QUANT  19
#define SURV_BSG1   20
#define CLAS_AU_ROC 21
#define CLAS_ENTROP 22
#define REGR_SGS    23
#define CLAS_SGS    24
#define SURV_SGS    25
#define MAHALANOBIS 26
#define SURV_CR_GEN 27
#define MAXM_SPLIT  28 
#define AUGT_INTR_NONE      0
#define AUGT_INTR_MULT      1
#define AUGT_INTR_DIVS      2
#define AUGT_INTR_ADDT      3
#define AUGT_INTR_SUBT      4
#define AUGT_INTR      1
#define AUGT_SYTH      2
#define SL_RF_VTRY_NULL 0
#define SL_RF_VTRY_HOLD 2
#define CLAS_FAM     0
#define REGR_FAM     1
#define SURV_FAM     2
#define CRSK_FAM     3
#define APROX 0
#define EXACT 1
#define MAX_EXACT_LEVEL sizeof(uint) * 8
#define SAFE_FACTOR_SIZE 16
#define MAX_HYPER_SIZE 15
#define MARGINAL_SIZE 8
#define SL_RF_WGHT_UNIFORM 1
#define SL_RF_WGHT_INTEGER 2
#define SL_RF_WGHT_GENERIC 3
#define SL_RF_DISTANCE_EUCLIDEAN 1
#define NATIVE_TYPE_CHARACTER 0
#define NATIVE_TYPE_INTEGER   1
#define NATIVE_TYPE_NUMERIC   2
#define NATIVE_TYPE_LIST      3
