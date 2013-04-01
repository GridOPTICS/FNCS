#include <cassert>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>

/* C 3rd party headers */
#include <mpi.h>

/* our headers */
#include "abscommmanager.h"
#include "integrator.h"
#include "mpinetworkinterface.h"
#include "objectcomminterface.h"
#include "callback.h"
#include "simtime.h"



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

  ierr = MPI_Init(&argc, &argv);
  assert(MPI_SUCCESS == ierr);

  ierr = MPI_Comm_rank(MPI_COMM_WORLD, &comm_rank);
  assert(MPI_SUCCESS == ierr);

  ierr = MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  assert(MPI_SUCCESS == ierr);
    
  currentTime=0;
  MpiNetworkInterface *comm = new MpiNetworkInterface(MPI_COMM_WORLD, true);
  CallBack<TIME,empty,empty,empty>* cb=CreateCallback(getCurTime);
  Integrator::initIntegratorGracePeriod(comm,MILLISECONDS,2000000,0);
  Integrator::setTimeCallBack(cb);
  
  Integrator::finalizeRegistrations();
  
  TIME grantedTime;
  do{
     //execute calculations that will solve all our problems
     usleep(rand()%2000);
     //start the time sync
     grantedTime=Integrator::getNextTime(currentTime,currentTime+1);
     assert(grantedTime==currentTime+1);
     currentTime=grantedTime;
  }while(Integrator::isFinished());

  return 0;
}