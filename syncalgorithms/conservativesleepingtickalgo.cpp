/*
    Copyright (c) 2013, <copyright holder> <email>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the <organization> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY <copyright holder> <email> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL <copyright holder> <email> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
/* autoconf header */
#include "config.h"

#include "conservativesleepingtickalgo.h"
#include <abscommmanager.h>

namespace sim_comm{


  ConservativeSleepingTickAlgo::ConservativeSleepingTickAlgo(AbsCommManager *interface, time_metric connectedSims[], int &connectedSimsSize) : AbsSyncAlgorithm(interface)
  {
     min=SECONDS;
     for(int i=0;i<connectedSimsSize;i++){
       if(connectedSims[i]>min)
	 min=connectedSims[i];
     }
     
     if(min==UNKNOWN)
       throw new SyncStateException(string("Connected Sims cannot have UNKNOWN time scale"));
     
     this->minResponseTime=convertToFrameworkTime(min,1);
  }

  ConservativeSleepingTickAlgo::~ConservativeSleepingTickAlgo()
  {
  
  }
  
  void ConservativeSleepingTickAlgo::timeStepStart(TIME currentTime)
  {
      if(currentTime < grantedTime)
	return;
      //call sleep to wake up other sims
      this->interface->sleep();
      this->interface->waitforAll();
  }


  TIME ConservativeSleepingTickAlgo::GetNextTime(TIME currentTime, TIME nextTime)
  {

      TIME nextEstTime; //here this variable represents the sycnhronization time with the network simulator.
      TIME minnetworkdelay;
      TIME powerSyncTime;
      
      if(nextTime < grantedTime)
	return nextTime;

      bool busywait=false;
      bool needToRespond=false;
      //send all messages
      if(currentTime < grantedTime){ //we still have some granted time we need to barier at granted Time
	    busywait=true;
	    currentTime=grantedTime;
      }
      
      do
      {
	  if(busywait){
	    this->interface->sleep(); //sleep until woken up
	    this->interface->waitforAll();
	  }
          uint64_t diff=interface->reduceTotalSendReceive();
          //network unstable, we need to wait!
          nextEstTime=currentTime+convertToFrameworkTime(Integrator::getCurSimMetric(),1); 
	  //find next responseTime
	  powerSyncTime=convertToMyTime(min,currentTime)+this->minResponseTime;
	  minnetworkdelay=interface->reduceNetworkDelay();
	    if(diff==0 && !needToRespond)
	    { 
	      //network stable grant min(next_time,responseTime) to power sims, and next_time to network simulator
	      if(powerSyncTime < nextTime)
		powerSyncTime = nextTime;
	      nextEstTime=nextTime;
	    }
	    else{
	      needToRespond=true; //set this condition so that when the simulator wakes up from busy wait it responds to messages
	    }

	 
	  //this->maxBusyWaitTime=0;
          //Calculate next min time step
          TIME myminNextTime=nextEstTime;
          TIME minNextTime=(TIME)interface->reduceMinTime(myminNextTime);
	  TIME minPowerSyncTime=(TIME)interface->reduceMinTime(powerSyncTime);
	  powerSyncTime=minPowerSyncTime; //we need to grant the simulator upto min power sync time;
          //min time is the estimated next time, so grant nextEstimated time
          if(minNextTime==myminNextTime)
              busywait=false;
	  
	  if(minNextTime==0){ //a sim signal endded
#if DEBUG
	      CERR << "End Signaled!" << endl;
#endif
	      this->finished=true;
	      return 0;
	  }
	  
          if(minNextTime < myminNextTime){
	      //update the current Time!
	      currentTime = convertToMyTime(Integrator::getCurSimMetric(),minNextTime);
	      currentTime = convertToFrameworkTime(Integrator::getCurSimMetric(),currentTime);
	     /* if(!threadopen && myminNextTime-interface->getMinNetworkDelay()<minNextTime){
		this->threadopen=true;
		this->threadOpenTime=currentTime;
		this->threadEndTime=myminNextTime;
		
#if DEBUG
	      CERR << "Starting busy wait thread! my:" << myminNextTime << " others:" << minNextTime <<  endl;
#endif
		pthread_create(&this->thread,NULL,&ConservativeSleepingTickAlgo::startThreadBusyWait,(void*)this);
		busywait=false;
	      }
	      else{*/
		busywait=true;
	      //}
	  }


      }while(busywait);
     
      
      this->grantedTime=powerSyncTime;
      return nextEstTime;
  }

  bool ConservativeSleepingTickAlgo::doDispatchNextEvent(TIME currentTime, TIME nextTime)
  {
    TIME syncedTime=this->GetNextTime(currentTime,nextTime);

    return syncedTime==nextTime;
  }
  
 

}
