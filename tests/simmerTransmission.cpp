/* autoconf header */
#include "config.h"

#include <cassert>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>

/* C 3rd party headers */
//#include <mpi.h>

/* our headers */
#include "abscommmanager.h"
#include "integrator.h"
//#include "mpinetworkinterface.h"
#include "objectcomminterface.h"
#include "callback.h"
#include "simtime.h"
#include "zmqnetworkinterface.h"
#include "speculationtimecalculationstrategy.h"

using namespace std;
using namespace sim_comm;

TIME currentTime;

TIME getCurTime(){
    return currentTime;
}

int main(int argc,char* argv[]){

  int ierr = 0;
  int comm_rank = 0;
  int comm_size = 0;

  /*ierr = MPI_Init(&argc, &argv);
  assert(MPI_SUCCESS == ierr);

  ierr = MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);
  assert(MPI_SUCCESS == ierr);

  ierr = MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  assert(MPI_SUCCESS == ierr);*/
    
  currentTime=2000000000;
  //MpiNetworkInterface *comm = new MpiNetworkInterface(MPI_COMM_WORLD, false);
  ZmqNetworkInterface *comm=new ZmqNetworkInterface(false);
  CallBack<TIME,empty,empty,empty>* cb=CreateCallback(getCurTime);
  //Integrator::initIntegratorGracePeriod(comm,MILLISECONDS,2300000000,currentTime);
  //Integrator::initIntegratorNetworkDelaySupport(comm,MILLISECONDS,2300000000,currentTime);
  //Integrator::initIntegratorSpeculative(comm,MILLISECONDS,2300000000,currentTime,1000);
  IncreasingSpeculationTimeStrategy *st=new IncreasingSpeculationTimeStrategy(MILLISECONDS,60000);
  Integrator::initIntegratorOptimistic(comm,MILLISECONDS,2300000000,currentTime,60000,st);
  //Integrator::initIntegratorNetworkDelaySupport(comm,MILLISECONDS,2000000000,0);
  Integrator::setTimeCallBack(cb);
  
  ObjectCommInterface* myInterface=Integrator::getCommInterface(string("1"));
  
  Integrator::finalizeRegistrations();
  
  TIME grantedTime;
  do{
    Integrator::timeStepStart(currentTime);
     //execute calculations that will solve all our problems
     usleep(rand()%20);
     if(myInterface->hasMoreMessages()){
     
       Message *msg=myInterface->getNextInboxMessage();
       const uint8_t *data=msg->getData();
       double re,im;
       memcpy(&re,data,sizeof(double));
       memcpy(&im,&data[sizeof(double)],sizeof(double));
       cout << "Received from gld " << re << " " << im << endl;
    }
     grantedTime=Integrator::getNextTime(currentTime,currentTime+1);
     //assert(grantedTime==currentTime+1);
     currentTime=currentTime+1;
  }while(!Integrator::isFinished());

  return 0;
}
