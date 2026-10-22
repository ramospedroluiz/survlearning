/* Derived from randomForestSRC 3.9.0, Ishwaran and Kogalur.
 * GPL (>= 3); see inst/COPYRIGHTS. Numerical core retained. */

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***
#include           "global.h"
#include           "external.h"

// *** THIS HEADER IS AUTO GENERATED. DO NOT EDIT IT ***

      
    

#include "trace.h"
#include "error.h"
unsigned int    SL_RF_traceFlagDiagLevel;
unsigned int    SL_RF_traceFlagIterValue;
size_t          SL_RF_memor_maxMemoryAllocation;
size_t          SL_RF_memor_minMemoryAllocation;
size_t          SL_RF_memor_probeMemoryAllocation;
void setTraceFlag(unsigned int traceFlag, unsigned int tree) {
  SL_RF_traceFlagDiagLevel = traceFlag;
  SL_RF_traceFlagIterValue = tree;
}
unsigned int getTraceFlag(unsigned int tree) {
  unsigned int result;
  result = FALSE;
  if (SL_RF_traceFlagIterValue == tree) {
    result = SL_RF_traceFlagDiagLevel;
  }
  else {
    if (SL_RF_traceFlagIterValue == 0) {
      result = SL_RF_traceFlagDiagLevel;
    }
  }
  return result;
}
unsigned int updateTimeStamp(unsigned int before) {
  unsigned int stamp;
  double cpuTimeUsed;
  stamp = clock();
  cpuTimeUsed = ((double) (stamp - before)) / CLOCKS_PER_SEC;
  SL_RF_nativePrint("\nRF-SRC:  CPU process time:  %20.3f \n", cpuTimeUsed);
  return stamp;
}
void memoryCheck(void) {
}
void setMaxMemoryAllocation(size_t value) {
  SL_RF_memor_maxMemoryAllocation = value;
}
void setMinMemoryAllocation(size_t value) {
  SL_RF_memor_minMemoryAllocation = value;
}
void setProbeMemoryAllocation(size_t value) {
  SL_RF_memor_probeMemoryAllocation = value;
}
size_t getMaxMemoryAllocation(void) {
  return (SL_RF_memor_maxMemoryAllocation);
}
size_t getMinMemoryAllocation(void) {
  return (SL_RF_memor_minMemoryAllocation);
}
size_t getProbeMemoryAllocation(void) {
  return (SL_RF_memor_probeMemoryAllocation);
}
void increaseMemoryAllocation(size_t amount) {
  SL_RF_memor_minMemoryAllocation += amount;
  if (SL_RF_memor_minMemoryAllocation > SL_RF_memor_maxMemoryAllocation) {
    SL_RF_memor_maxMemoryAllocation = SL_RF_memor_minMemoryAllocation;
  }
}
void increaseProbeMemoryAllocation(size_t amount) {
  SL_RF_memor_probeMemoryAllocation += amount;
}
void decreaseMemoryAllocation(size_t amount) {
    SL_RF_memor_minMemoryAllocation -= amount;
}
void decreaseProbeMemoryAllocation(size_t amount) {
    SL_RF_memor_probeMemoryAllocation -= amount;
}
