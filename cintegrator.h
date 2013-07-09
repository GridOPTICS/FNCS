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
  void initIntegratorOptimisticConstant(enum time_metric simTimeStep, 
				TIME packetLostPeriod, TIME initialTime, TIME specTime);
  void initIntegratorOptimisticIncreasing(enum time_metric simTimeStep, 
				TIME packetLostPeriod, TIME initialTime, TIME specTime);
  void initIntegratorConservativeSleepingTick(enum time_metric simTimeStep, 
				TIME packetLostPeriod, TIME initialTime,enum time_metric metrics[],int metricsSize);
  void setOffset(TIME initialTime);
  void finalizeRegistrations();
  void setregistercallback(TIME (*callback)());
  TIME getNextTime(TIME currentTime, TIME nextTime);
  void stopIntegrator();
  uint8_t isFinished();
  void fenix_initialize(int *arc,char ***argv);
  void fenix_finalize();
  void InitMPI(int *arc,char ***argv);
  void finalizeMPI();
  void timeStepStart(TIME currenTime);
  void registerObject(char *name);
  void sendMesg(char *from,char *to,char *msg,int size,int networked);
  void receive(char *from,char **buff,int *size);

#ifdef __cplusplus
}
#endif

#endif

