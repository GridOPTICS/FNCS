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


  ConservativeSleepingTickAlgo::ConservativeSleepingTickAlgo(AbsCommManager *interface, int &numberOfPowerSims) : AbsSyncAlgorithm(interface)
  {
     this->othersimsSize=numberOfPowerSims-1;
     this->powersimgrantedTime=new TIME[numberOfPowerSims-1];
     this->diff=0;
     this->mightSleep=false;
#ifdef DEBUG_WITH_PROFILE
     setElsapedTimer();
#endif
  }

  ConservativeSleepingTickAlgo::~ConservativeSleepingTickAlgo()
  {
  
  }
  
  void ConservativeSleepingTickAlgo::timeStepStart(TIME currentTime)
  {
#ifdef PROFILE
      syncStart();
#endif
      if(currentTime < grantedTime)
	return;

#if DEBUG_WITH_PROFILE
      CERR <<  "Start sync time step " << currentTime << " " << getCurTimeInMs() << endl;
#elif DEBUG
      CERR << "Start sync time step " << currentTime <<  endl;
#endif
     
      this->interface->waitforAll();
  }


  TIME ConservativeSleepingTickAlgo::GetNextTime(TIME currentTime, TIME nextTime)
  {

      TIME nextEstTime; //here this variable represents the sycnhronization time with the network simulator.
      TIME minnetworkdelay;
      
      if(nextTime < grantedTime)
	return nextTime;

      bool needToRespond=false;
      busywait=false;
      
      //send all messages
      if(currentTime < grantedTime){ //we still have some granted time we need to barier at granted Time
	    busywait=true;
	    currentTime=grantedTime;
      }
      //call sleep to wake up other sims
      if(mightSleep){
	  for(int i=0;i<this->othersimsSize;i++)
	    if(nextTime >= this->powersimgrantedTime[i]){ //might cause unnecessary sleeps
	      this->interface->sleep();
	      mightSleep=false;
	    }
	 
      }
      do
      {
	  if(busywait){
	    if(this->diff==0){
	      bool result=this->interface->sleep(); //sleep until woken up
	      mightSleep=false;
	      if(!needToRespond)
		needToRespond=result;
	    }
	    else{
	      this->interface->waitforAll();
	    }
	  }
          this->diff=interface->reduceTotalSendReceive();
          //network unstable, we need to wait!
          nextEstTime=currentTime+Integrator::getOneTimeStep(); 
	  //find next responseTime
	  minnetworkdelay=interface->reduceNetworkDelay();
	  if(diff==0)
	    this->interface->resetCounters();
	  if(diff==0 && !needToRespond)
	  { 
	    nextEstTime=nextTime;
	  }
	  else{
	      needToRespond=true; //set this condition so that when the simulator wakes up from busy wait it responds to message
	      if(mightSleep){ //there might a sleeping sim and there is new packet wake it up
		this->powersimgrantedTime[0]=0; //we need to wake up other sims
	      }
	  }
          //Calculate next min time step
          TIME myminNextTime=nextEstTime;
          TIME minNextTime=(TIME)interface->reduceMinTime(myminNextTime); //we do this operation for the netsim
	  if(!mightSleep && diff==0){ //this operation allows us to find how long the sims need to sleep
		uint32_t worldSize;
		TIME *nextTimes=this->interface->getNextTimes(myminNextTime,worldSize);
		for(uint32_t i=0,j=0;i<worldSize;i++){
		 if(nextTimes[i] == Infinity)
		   continue;
		 powersimgrantedTime[j++]=nextTimes[i];
		}
		delete[] nextTimes;
		mightSleep=true;
	  }
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
	      
	      busywait=true;
	  }


      }while(busywait);
     
      this->grantedTime=nextEstTime;
#ifdef DEBUG_WITH_PROFILE
      CERR << "Sync finished at time step " << getCurTimeInMs() << endl;
#endif
#ifdef PROFILE
      writeTime(currentTime);
#endif
      return nextEstTime;
  }
  
 

}

