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

TIME currentTime, endTime;
//Selim : HOw is thsi returning simulation time ?
TIME getCurTime(){
    return currentTime;
}

int main(int argc,char* argv[]){

  int ierr = 0;
  int comm_rank = 0;
  int comm_size = 0;
  ostringstream ost;

  currentTime=0; //2000000000;
  endTime = INFINITY; // ask Selim 
  ZmqNetworkInterface *comm=new ZmqNetworkInterface(false);
  CallBack<TIME,empty,empty,empty>* cb=CreateCallback(getCurTime);
  // Ask Selim : does this do optimistic ?
      Integrator::initIntegratorGracePeriod(comm,MILLISECONDS,2300000000,currentTime);
  //Integrator::initIntegratorNetworkDelaySupport(comm,MILLISECONDS,2300000000,currentTime);
  //Integrator::initIntegratorSpeculative(comm,MILLISECONDS,2300000000,currentTime,1000);
  //IncreasingSpeculationTimeStrategy *st=new IncreasingSpeculationTimeStrategy(MILLISECONDS,60000);
  //Integrator::initIntegratorOptimistic(comm,MILLISECONDS,2300000000,currentTime,60000,st);
  //time_metric others[1];
  //others[0]=SECONDS;
  //Integrator::initIntegratorConservativeSleepingTick(comm,MILLISECONDS,2000000000,currentTime,others,1);
  Integrator::setTimeCallBack(cb);
  
  //ost << getpid();
  //ObjectCommInterface* myInterface=Integrator::getCommInterface(ost.str());
  
  Integrator::finalizeRegistrations();
  
  // All simulations start at time 0
  // // ask Sliem : how to create/read json file
  int hearbeat = getform_jsonfile();
  time currentTime = 0;
  /** Query real time clock **/
  time s1 = getWallclockTime();
  do{
    
    Integrator::timeStepStart(currentTime);
     //execute calculations that will solve all our problems
     //usleep(rand()%20);
     nextTime=currentTime+heartbeat;
     nextTime=Integrator::getNextTime(currentTime,nextTime);
    
     if(nextTime < currentTime + heartbeat)
	continue;
     time s2 = getWallclockTime();


     if(heartbeat < s2-s1){
	 FAIL();
     }
     else{
	 bool flag = true;
	 while(flag){  
	     ct = getWallClockTime();
	     if(ct < s1+ nextTime){
	     }else { flag = False;}
	 }
     }

    }
    s1 = getWallclockTime();
    currentTime=nextTime; 
    //assert(grantedTime==currentTime+1);
  }while(!Integrator::isFinished());

  return 0;
}
