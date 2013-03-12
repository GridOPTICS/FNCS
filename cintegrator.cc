#include "cintegrator.h"

#include "mpinetworkinterface.h"
#include "integrator.h"
#include "callback.h"

using namespace sim_comm;


void InitMPI(int *arc,char ***argv){

  MPI_Init(arc,argv);
}

void finalizeMPI(){
  MPI_Finalize();
  
}

uint8_t isFinished()
{
  return (uint8_t)Integrator::isFinished();
}

 void initIntegratorGracePeriod(enum time_metric simTimeStep, 
				TIME gracePeriod, TIME initialTime){
   MpiNetworkInterface *comm = new MpiNetworkInterface(MPI_COMM_WORLD, false);
   Integrator::initIntegratorGracePeriod(comm,SECONDS,5,initialTime);
 }
 
void finalizeRegistrations()
{
  Integrator::finalizeRegistrations();
}

void setregistercallback(TIME (*callback)())
{
  CallBack<TIME,empty,empty,empty> *cb=CreateCallback(callback);
  Integrator::setTimeCallBack(cb);
}

void stopIntegrator()
{
  Integrator::stopIntegrator();
}

TIME getNextTime(TIME currentTime, TIME nextTime)
{
  return Integrator::getNextTime(currentTime,nextTime);
}

