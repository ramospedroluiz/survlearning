/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "entryGeneric.h"
void processDefaultGrow(void) {
  SL_RF_optHigh = SL_RF_optHigh & (~OPT_MEMB_PRUN);
  SL_RF_ptnCount             = 0;
  SL_RF_optHigh = SL_RF_optHigh & (~OPT_PART_PLOT);
  SL_RF_partialLength = 0;
  SL_RF_opt = SL_RF_opt & (~OPT_VIMP_JOIN);
  SL_RF_opt = SL_RF_opt & (~OPT_OUTC_TYPE);
  SL_RF_opt = SL_RF_opt & (~OPT_COMP_RISK);
  SL_RF_optHigh = SL_RF_optHigh & (~OPT_TERM_INCG);
  SL_RF_optHigh = SL_RF_optHigh & (~OPT_MEMB_INCG);
  SL_RF_frSize = SL_RF_fobservationSize = 0;
  SL_RF_xMarginalSize = 0;
  if (SL_RF_opt & OPT_IMPU_ONLY) {
    SL_RF_opt                  = SL_RF_opt & (OPT_IMPU_ONLY | OPT_BOOT_TYP1 | OPT_BOOT_TYP2);
    SL_RF_optHigh              = SL_RF_optHigh & (OPT_MISS_SKIP | OPT_BOOT_SWOR);
  }
  else {
  }
  SL_RF_opt = SL_RF_opt | OPT_MISS_OUT;
  SL_RF_opt = SL_RF_opt | OPT_LEAF;
  if ( !(SL_RF_opt & OPT_BOOT_TYP1) && (SL_RF_opt & OPT_BOOT_TYP2)) {
    SL_RF_optHigh = SL_RF_optHigh & (~OPT_BOOT_SWOR);
    SL_RF_opt                  = SL_RF_opt & (~OPT_OENS);
    SL_RF_opt                  = SL_RF_opt & (~OPT_PERF);
    SL_RF_opt                  = SL_RF_opt & (~OPT_VIMP);
    SL_RF_optHigh              = SL_RF_optHigh & (~OPT_CSE);
    SL_RF_optHigh              = SL_RF_optHigh & (~OPT_CSV);
    if (SL_RF_opt & OPT_PROX) {
      SL_RF_opt = SL_RF_opt | OPT_PROX_IBG;
      SL_RF_opt = SL_RF_opt | OPT_PROX_OOB;
    }
    if (SL_RF_optHigh & OPT_DIST) {
      SL_RF_optHigh = SL_RF_optHigh | OPT_DIST_IBG;
      SL_RF_optHigh = SL_RF_optHigh | OPT_DIST_OOB;
    }
    if (SL_RF_optHigh & OPT_WGHT) {
      SL_RF_optHigh = SL_RF_optHigh | OPT_WGHT_IBG;
      SL_RF_optHigh = SL_RF_optHigh | OPT_WGHT_OOB;
    }
  }
  else {
  }
  if ((SL_RF_splitRule == USPV_NRM) || (SL_RF_splitRule == USPV_WT_OFF) || (SL_RF_splitRule == USPV_WT_HVY)) {
    SL_RF_opt                  = SL_RF_opt & (~OPT_PERF);
    SL_RF_opt                  = SL_RF_opt & (~OPT_VIMP);
    SL_RF_opt                  = SL_RF_opt & (~OPT_OENS);
    SL_RF_opt                  = SL_RF_opt & (~OPT_FENS);
    SL_RF_optHigh              = SL_RF_optHigh & (~OPT_CSE);
    SL_RF_optHigh              = SL_RF_optHigh & (~OPT_CSV);
    SL_RF_ySize = 0;
  }
  if (SL_RF_opt & OPT_PERF) {
  }
  else {
    SL_RF_opt = SL_RF_opt & (~OPT_VIMP);
    SL_RF_optHigh              = SL_RF_optHigh & (~OPT_CSE);
    SL_RF_optHigh              = SL_RF_optHigh & (~OPT_CSV);
  }
  if (SL_RF_opt & OPT_TREE) {
    SL_RF_opt = SL_RF_opt | OPT_SEED;
  }
  else {
    SL_RF_opt = SL_RF_opt & (~OPT_SEED);
  }
  SL_RF_opt = SL_RF_opt & (~OPT_EMPR_RISK);
  if ((SL_RF_opt & OPT_OENS) || (SL_RF_opt & OPT_FENS)) {
  }
  else {
    SL_RF_optHigh = SL_RF_optHigh & (~OPT_TERM_OUTG);
  }
  if (!(SL_RF_opt & OPT_OENS)) {
    SL_RF_opt = SL_RF_opt & (~OPT_PERF);
    SL_RF_optHigh              = SL_RF_optHigh & (~OPT_CSE);
    SL_RF_optHigh              = SL_RF_optHigh & (~OPT_CSV);
  }
  if (SL_RF_vtry > 0) {
    SL_RF_opt = SL_RF_opt & (~OPT_VIMP);
    SL_RF_nImpute = 1;
  }
}
void processDefaultPredict(void) {
  char mode;
  SL_RF_opt = SL_RF_opt & (~OPT_IMPU_ONLY);
  SL_RF_opt = SL_RF_opt & (~OPT_TREE);
  SL_RF_opt = SL_RF_opt & (~OPT_SEED);
  SL_RF_nImpute = 1;
  SL_RF_opt = SL_RF_opt | OPT_LEAF;
  SL_RF_opt  = SL_RF_opt | OPT_MISS_OUT;
  SL_RF_optHigh = SL_RF_optHigh & (~OPT_MEMB_OUTG);
  SL_RF_optHigh = SL_RF_optHigh & (~OPT_TERM_OUTG);
  SL_RF_vtry                 = 0;
  SL_RF_vtryArray            = NULL;
  if(SL_RF_fobservationSize > 0) {
    mode = SL_RF_PRED;
  }
  else {
    mode = SL_RF_REST;
  }
  switch (mode) {
  case SL_RF_PRED:
    SL_RF_opt = SL_RF_opt & (~OPT_OUTC_TYPE);
    SL_RF_optHigh = SL_RF_optHigh & (~OPT_PART_PLOT);
    SL_RF_partialLength = SL_RF_partialLength2 = 0;
    SL_RF_opt = SL_RF_opt & (~OPT_OENS);
    if (SL_RF_ySize == 0) {
      SL_RF_opt = SL_RF_opt & (~OPT_PERF);
      SL_RF_opt = SL_RF_opt & (~OPT_VIMP);
      SL_RF_optHigh              = SL_RF_optHigh & (~OPT_CSE);
      SL_RF_optHigh              = SL_RF_optHigh & (~OPT_CSV);
      SL_RF_opt = SL_RF_opt & (~OPT_FENS);
    }
    else {
      if (SL_RF_frSize == 0) {
        SL_RF_opt                  = SL_RF_opt & (~OPT_PERF);
        SL_RF_opt                  = SL_RF_opt & (~OPT_VIMP);
        SL_RF_optHigh              = SL_RF_optHigh & (~OPT_CSE);
        SL_RF_optHigh              = SL_RF_optHigh & (~OPT_CSV);
      }
    }
    if (SL_RF_opt & OPT_PROX) {
      SL_RF_optHigh = SL_RF_optHigh & (~OPT_JIT_TOP);
    }
    if (SL_RF_optHigh & OPT_DIST) {
      SL_RF_optHigh = SL_RF_optHigh & (~OPT_JIT_TOP);
    }
    if (SL_RF_optHigh & OPT_WGHT) {
      SL_RF_optHigh = SL_RF_optHigh | OPT_WGHT_IBG;
      SL_RF_optHigh = SL_RF_optHigh | OPT_WGHT_OOB;
    }
    if (!(SL_RF_opt & OPT_FENS)) {
      SL_RF_opt                  = SL_RF_opt & (~OPT_PERF);
      SL_RF_optHigh              = SL_RF_optHigh & (~OPT_CSE);
      SL_RF_optHigh              = SL_RF_optHigh & (~OPT_CSV);
    }
    SL_RF_opt = SL_RF_opt & (~OPT_SPLDPTH_1) & (~OPT_SPLDPTH_2);
    SL_RF_opt = SL_RF_opt & (~OPT_EMPR_RISK);
    if (SL_RF_opt & OPT_FENS) {
    }
    else {
    }
    break;
  case SL_RF_REST:
    SL_RF_frSize = SL_RF_fobservationSize = 0;
    if (SL_RF_opt & OPT_OUTC_TYPE) {
      SL_RF_optHigh = SL_RF_optHigh & (~OPT_PART_PLOT);
      SL_RF_partialLength = SL_RF_partialLength2 = 0;
      SL_RF_optHigh = SL_RF_optHigh & (~OPT_MEMB_INCG);
      SL_RF_optHigh = SL_RF_optHigh & (~OPT_TERM_INCG);
    }
    else if (SL_RF_optHigh & OPT_PART_PLOT) {
      SL_RF_opt = SL_RF_opt & (~OPT_OUTC_TYPE);
      SL_RF_opt = SL_RF_opt & (~OPT_PERF);
      SL_RF_optHigh              = SL_RF_optHigh & (~OPT_CSE);
      SL_RF_optHigh              = SL_RF_optHigh & (~OPT_CSV);
    }
    else {
    }
    if(SL_RF_ySize == 0) {
      SL_RF_opt = SL_RF_opt & (~OPT_PERF);
      SL_RF_opt = SL_RF_opt & (~OPT_VIMP);
      SL_RF_optHigh              = SL_RF_optHigh & (~OPT_CSE);
      SL_RF_optHigh              = SL_RF_optHigh & (~OPT_CSV);
      SL_RF_opt = SL_RF_opt & (~OPT_OENS);
      SL_RF_opt = SL_RF_opt & (~OPT_FENS);
    }
    if ( !(SL_RF_opt & OPT_BOOT_TYP1) && (SL_RF_opt & OPT_BOOT_TYP2)) { 
      SL_RF_opt                  = SL_RF_opt & (~OPT_PERF);
      SL_RF_opt                  = SL_RF_opt & (~OPT_VIMP);
      SL_RF_optHigh              = SL_RF_optHigh & (~OPT_CSE);
      SL_RF_optHigh              = SL_RF_optHigh & (~OPT_CSV);
      SL_RF_opt                  = SL_RF_opt & (~OPT_OENS);
      if (SL_RF_opt & OPT_PROX) {
        SL_RF_opt = SL_RF_opt | OPT_PROX_IBG;
        SL_RF_opt = SL_RF_opt | OPT_PROX_OOB;
      }
      if (SL_RF_optHigh & OPT_DIST) {
        SL_RF_optHigh = SL_RF_optHigh | OPT_DIST_IBG;
        SL_RF_optHigh = SL_RF_optHigh | OPT_DIST_OOB;
      }
      if (SL_RF_optHigh & OPT_WGHT) {
        SL_RF_optHigh = SL_RF_optHigh | OPT_WGHT_IBG;
        SL_RF_optHigh = SL_RF_optHigh | OPT_WGHT_OOB;
      }
    }
    SL_RF_opt = SL_RF_opt & (~OPT_EMPR_RISK);
    if ((SL_RF_opt & OPT_OENS) || (SL_RF_opt & OPT_FENS)) {
    }
    else {
    }
    if (!(SL_RF_opt & OPT_OENS)) {
      SL_RF_opt                  = SL_RF_opt & (~OPT_PERF);
      SL_RF_optHigh              = SL_RF_optHigh & (~OPT_CSE);
      SL_RF_optHigh              = SL_RF_optHigh & (~OPT_CSV);
    }
    break;
  }
  if ( !(SL_RF_opt & OPT_BOOT_TYP1) && (SL_RF_opt & OPT_BOOT_TYP2) ) {
    SL_RF_optHigh = SL_RF_optHigh & (~OPT_BOOT_SWOR);
  }
  if (SL_RF_ptnCount > 0) {
      SL_RF_optHigh = SL_RF_optHigh | OPT_MEMB_PRUN;
      SL_RF_opt     = SL_RF_opt & (~OPT_PERF);
      SL_RF_opt     = SL_RF_opt & (~OPT_VIMP);
      SL_RF_optHigh              = SL_RF_optHigh & (~OPT_CSE);
      SL_RF_optHigh              = SL_RF_optHigh & (~OPT_CSV);
      SL_RF_opt     = SL_RF_opt & (~OPT_PROX);
      SL_RF_opt     = SL_RF_opt & (~OPT_OENS);
      SL_RF_opt     = SL_RF_opt & (~OPT_FENS);
      SL_RF_optHigh = SL_RF_optHigh & (~OPT_DIST);
      SL_RF_optHigh = SL_RF_optHigh & (~OPT_WGHT);
  }
  else {
    SL_RF_optHigh = SL_RF_optHigh & (~OPT_MEMB_PRUN);
  }
  if (SL_RF_xMarginalSize > 0) {
    SL_RF_opt = SL_RF_opt & (~OPT_PERF);
    SL_RF_opt = SL_RF_opt & (~OPT_VIMP);
    SL_RF_optHigh              = SL_RF_optHigh & (~OPT_CSE);
    SL_RF_optHigh              = SL_RF_optHigh & (~OPT_CSV);
    SL_RF_opt = SL_RF_opt & (~OPT_OENS);
    SL_RF_opt = SL_RF_opt & (~OPT_FENS);
  }
  if (SL_RF_opt & OPT_PERF) {
  }
  else {
    SL_RF_opt                  = SL_RF_opt & (~OPT_VIMP);
    SL_RF_optHigh              = SL_RF_optHigh & (~OPT_CSE);
    SL_RF_optHigh              = SL_RF_optHigh & (~OPT_CSV);
  }
}  
