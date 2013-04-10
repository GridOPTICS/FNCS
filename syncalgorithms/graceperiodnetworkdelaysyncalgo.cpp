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

#include "graceperiodnetworkdelaysyncalgo.h"
#include <abscommmanager.h>

namespace sim_comm{
  
  GracePeriodNetworkDelaySyncAlgo::GracePeriodNetworkDelaySyncAlgo(AbsCommManager *interface) : AbsSyncAlgorithm(interface)
  {
      this->threadopen=false;
      //this->busywaiting=false;
      
  }

  GracePeriodNetworkDelaySyncAlgo::~GracePeriodNetworkDelaySyncAlgo()
  {
    if(threadopen)
      pthread_cancel(this->thread);
  }

  TIME GracePeriodNetworkDelaySyncAlgo::GetNextTime(TIME currentTime, TIME nextTime)
  {
      if(threadopen){
	#if DEBUG
	      CERR << "Waiting for busy wait thread!" << endl;
	#endif
        TIME *retValue;
	int error=pthread_join(this->thread,(void**)&retValue);
	if(error!=0)
	    throw SyncStateException(string("Pthread join operation failed!"));
	if(retValue==0)
	  return 0;
	threadopen=false;
	#if DEBUG
	      CERR << "Busy wait thread is finished!" << endl;
	#endif
      }
      
      TIME nextEstTime;
      TIME minnetworkdelay;
      uint64_t bwaitCount=0;
      if(nextTime < grantedTime)
	return nextTime;

      bool busywait=false;
      bool needToRespond=false;
      //send all messages

      do
      {
	  if(busywait)
	    this->interface->waitforAll();
          uint64_t diff=interface->reduceTotalSendReceive();
          //network unstable, we need to wait!
          nextEstTime=currentTime+convertToFrameworkTime(Integrator::getCurSimMetric(),1); 
	  minnetworkdelay=interface->reduceNetworkDelay();
	    if(diff==0 && !needToRespond)
	    { //network stable grant next time
		nextEstTime=nextTime;
		if(minnetworkdelay!=Infinity && nextEstTime < currentTime + minnetworkdelay)
		  nextEstTime = currentTime + minnetworkdelay;
	    }
	    else{
	      needToRespond=true; //set this condition so that when the simulator wakes up from busy wait it responds to messages
	    }

	 
	  //this->maxBusyWaitTime=0;
          //Calculate next min time step
          TIME myminNextTime=nextEstTime;
          TIME minNextTime=(TIME)interface->reduceMinTime(myminNextTime);

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
		pthread_create(&this->thread,NULL,&GracePeriodNetworkDelaySyncAlgo::startThreadBusyWait,(void*)this);
		busywait=false;
	      }
	      else{*/
		busywait=true;
	      //}
	  }


      }while(busywait);
     
      
      this->grantedTime=nextEstTime;
      return nextEstTime;
  }

  bool GracePeriodNetworkDelaySyncAlgo::doDispatchNextEvent(TIME currentTime, TIME nextTime)
  {
    TIME syncedTime=this->GetNextTime(currentTime,nextTime);

    return syncedTime==nextTime;
  }
  
  TIME GracePeriodNetworkDelaySyncAlgo::threadBusyWait(TIME currentTime, TIME nextTime)
  { 
      TIME nextEstTime;

      if(nextTime < grantedTime)
	return nextTime;

      bool busywait=false;
      bool needToRespond=false;
      //send all messages

      do
      {
          uint8_t diff=interface->reduceTotalSendReceive();
          //network unstable, we need to wait!
          nextEstTime=currentTime+convertToFrameworkTime(Integrator::getCurSimMetric(),1); 
          if(diff==0 && !needToRespond)
          { //network stable grant next time
              nextEstTime=nextTime;
          }
          else{
	    needToRespond=true; //set this condition so that when the simulator wakes up from busy wait it responds to messages
	  }

          //Calculate next min time step
          TIME myminNextTime=nextEstTime;
          TIME minNextTime=(TIME)interface->reduceMinTime(myminNextTime);

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
		busywait=true;
	  }


      }while(busywait);
     
      
      //this->grantedTime=nextEstTime;
      return nextEstTime;
  }


  void* GracePeriodNetworkDelaySyncAlgo::startThreadBusyWait(void* args)
  {
      GracePeriodNetworkDelaySyncAlgo *dana=(GracePeriodNetworkDelaySyncAlgo*)args;
      TIME returnVal=dana->threadBusyWait(dana->threadOpenTime,dana->threadEndTime);
      return &returnVal;
  }

}

