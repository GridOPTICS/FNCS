/* autoconf header */
#include "config.h"

#include <cassert>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>

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
  ostringstream ost;

  currentTime=2000000000;
  ZmqNetworkInterface *comm=new ZmqNetworkInterface(false);
  CallBack<TIME,empty,empty,empty,empty>* cb=CreateCallback(getCurTime);
  Integrator::initIntegratorGracePeriod(comm,MILLISECONDS,2300000000,currentTime);
  //Integrator::initIntegratorNetworkDelaySupport(comm,MILLISECONDS,2300000000,currentTime);
  //Integrator::initIntegratorSpeculative(comm,MILLISECONDS,2300000000,currentTime,1000);
  //IncreasingSpeculationTimeStrategy *st=new IncreasingSpeculationTimeStrategy(MILLISECONDS,60000);
  //Integrator::initIntegratorOptimistic(comm,MILLISECONDS,2300000000,currentTime,60000,st);
  //time_metric others[1];
  //others[0]=SECONDS;
  //Integrator::initIntegratorConservativeSleepingTick(comm,MILLISECONDS,2000000000,currentTime,others,1);
  Integrator::setTimeCallBack(cb);
  
  ost << getpid();
  ObjectCommInterface* myInterface=Integrator::getCommInterface(ost.str());
  
  Integrator::finalizeRegistrations();
  
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
     currentTime=Integrator::getNextTime(currentTime,currentTime+100);
     
     if((currentTime-2000000000)%1000==0){
      cout << "Current Time:" << (currentTime-2000000000)/1000 << endl;
       
    }
     //assert(grantedTime==currentTime+1);
  }while(!Integrator::isFinished());

  return 0;
}
