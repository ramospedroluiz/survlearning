/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */
#ifndef SL_RF_STACK_PARALLEL_H
#define SL_RF_STACK_PARALLEL_H
void stackLocksOpenMP(char mode);
void unstackLocksOpenMP(char mode);
#endif
