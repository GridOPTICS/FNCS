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


#include "conservativesleepingcommalgo.h"
#include "integrator.h"
#include <abscommmanager.h>

namespace sim_comm{

	FNCS_SYNCALGO(ConservativeSleepingCommAlgo);

ConservativeSleepingCommAlgo::ConservativeSleepingCommAlgo(AbsCommManager* interface): AbsSyncAlgorithm(interface)
{
  this->powersimgrantedTime=0;
  this->diff=0;
}

ConservativeSleepingCommAlgo::~ConservativeSleepingCommAlgo()
{

}


TIME ConservativeSleepingCommAlgo::GetNextTime(TIME currentTime, TIME nextTime)
{
	  	
		   if(nextTime < grantedTime)
		     return grantedTime;
		    

		    diff=interface->reduceTotalSendReceive();
		    //assume network stable
		    if(diff==0)
		      this->interface->resetCounters();
		    /*if(diff>0)
		    { //network unstable 
			TIME graceTime=Integrator::getCurSimTime()-Integrator::getPacketLostPeriod();
			if(graceTime>Integrator::getCurSimTime()) //overflowed
			    graceTime=0;
			if(updated){
			    if(this->currentState<graceTime){ //test if it has been graceperiod amount of time before we declare the packet as lost
		    TODO packetLost doesn't take parameters?? 
				this->interface->packetLost();
				//rest currentState;
				this->currentState=Integrator::getCurSimTime(); //restart counter
				updated=false;
			    }
			}
			else{
			  this->currentState=Integrator::getCurSimTime(); //restart counter
			  updated=true;
			}
		    }
		    else{
		      this->currentState=Integrator::getCurSimTime();
		      updated=false;
		    }*/
		    if(diff > 0)
		      this->interface->packetLostCalculator(currentTime);
		    TIME minnetworkdelay=interface->reduceNetworkDelay();
		    //We never wait for comm sim, instead we wait for oter sims
		    TIME myminNextTime=Infinity;
		    TIME minNextTime=(TIME)interface->reduceMinTime(myminNextTime);
		    /*if(diff==0 && this->powersimgrantedTime <= minNextTime){
			uint32_t worldSize;
			TIME *nextTimes=this->interface->getNextTimes(myminNextTime,worldSize);
			this->powersimgrantedTime=Infinity;
			TIME temp=Infinity;
			for(uint32_t i=0;i<worldSize;i++){ //find the next second smallest time (we will use this for wake up)
			  if(nextTimes[i] < temp){
			    this->powersimgrantedTime=temp;
			    temp=nextTimes[i];
			  }
			}
			
			delete[] nextTimes;
		    }*/
		    //If min time is infinity then there is something with the comm!
		    if(minNextTime==Infinity){
#if DEBUG
			  CERR << "End Signaled!" << endl;
#endif
			  this->finished=true;
			  return Infinity;
		     }
		    this->grantedTime=minNextTime;
		    return minNextTime;
}

AbsSyncAlgorithm* ConservativeSleepingCommAlgo::Create(const Json::Value& params,AbsCommManager *comm){

	return new ConservativeSleepingCommAlgo(comm);
}

}
