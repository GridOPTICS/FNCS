#ifndef CINTEGRATOR_H_
#define CINTEGRATOR_H_

#include "csimtime.h"

#ifdef __cplusplus
extern "C"{
#endif
  /**
   * Initializes the framework with Mpi network interface
   */
  void initIntegratorGracePeriod(enum time_metric simTimeStep, 
				TIME packetLostPeriod, TIME initialTime);
  void initIntegratorSpeculative(enum time_metric simTimeStep, 
				TIME packetLostPeriod, TIME initialTime, TIME specTime);
  void initIntegratorNetworkDelay(enum time_metric simTimeStep, 
				TIME packetLostPeriod, TIME initialTime, TIME specTime);
  void setOffset(TIME initialTime);
  void finalizeRegistrations();
  void setregistercallback(TIME (*callback)());
  TIME getNextTime(TIME currentTime, TIME nextTime);
  void stopIntegrator();
  uint8_t isFinished();
  void InitMPI(int *arc,char ***argv);
  void finalizeMPI();

#ifdef __cplusplus
}
#endif

#endif