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


#include "tickbasedsimulatorsyncalgo.h"

#include "integrator.h"
#include "abscomminterface.h"

namespace sim_comm
{
  TickBasedSimulatorSyncAlgo::TickBasedSimulatorSyncAlgo(AbsCommInterface* interface ) : AbsSyncAlgorithm(interface)
  {

  }

  TickBasedSimulatorSyncAlgo::~TickBasedSimulatorSyncAlgo()
  {

  }

  bool TickBasedSimulatorSyncAlgo::doDispatchNextEvent(TIME currentTime, TIME nextTime)
  {
    return false;
  }

  TIME TickBasedSimulatorSyncAlgo::GetNextTime(TIME currentTime, TIME nextTime)
  {
	TIME nextEstTime;



    bool busywait=false;

    do
    {
    	uint8_t diff=interface->realReduceTotalSendReceive();
    		//network unstable, we need to wait!
    	nextEstTime=currentTime+1;
    	if(diff==0)
    	{ //network stable grant next time
    			nextEstTime=nextTime;
    	}

    	//Calculate next min time step
    	TIME myminNextTime=convertToFrameworkTime(Integrator::getCurSimMetric(),nextEstTime);
    	TIME minNextTime=(TIME)interface->realReduceMinTime(myminNextTime);

    	//min time is the estimated next time, so grant nextEstimated time
    	if(minNextTime==myminNextTime)
    		busywait=false;

    	if(minNextTime < myminNextTime){
    		//at this stage simulator can only go currentTime+1
    		nextEstTime=currentTime+1;
    		myminNextTime=convertToFrameworkTime(Integrator::getCurSimMetric(),nextEstTime);
    		if(minNextTime+Integrator::getGracePreiod()<myminNextTime) //we have to busy wait until other sims come to this time
    			busywait=true;
    		else //TODO the code above will cause re-iterations in GLD
    			busywait=false;
    	}


    }while(busywait);
      
    return nextEstTime;
  }

}
