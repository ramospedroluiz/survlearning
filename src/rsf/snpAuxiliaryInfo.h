/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */
#ifndef SL_RF_SNP_AUXILIARY_INFO_H
#define SL_RF_SNP_AUXILIARY_INFO_H
typedef struct snpAuxiliaryInfo SNPAuxiliaryInfo;
struct snpAuxiliaryInfo {
  char type;
  char *identity;
  uint slot;
  ulong linearSize;
  void *snpPtr;
  void *auxiliaryArrayPtr;
  uint dimSize;
  int *dim;
};
#endif
