/*
 * communicatorsimulatorsyncalgo.cpp
 *
 *  Created on: Feb 16, 2013
 *      Author: ciraci
 */

#include "communicatorsimulatorsyncalgo.h"

#include "integrator.h"
#include "abscomminterface.h"


namespace sim_comm {

	CommunicatorSimulatorSyncalgo::CommunicatorSimulatorSyncalgo(AbsCommInterface* currentInterface): AbsSyncAlgorithm(currentInterface) {
		this->currentState=0;

	}

	CommunicatorSimulatorSyncalgo::~CommunicatorSimulatorSyncalgo() {
		// TODO Auto-generated destructor stub
	}
	
	
	TIME CommunicatorSimulatorSyncalgo::GetNextTime(TIME currentTime,TIME nextTime){
	  	
		bool busywait=false;


		    uint8_t diff=interface->realReduceTotalSendReceive();
		    //assume network stable
		   
		    if(diff>0)
		    { //network unstable 
			if(this->currentState>Integrator::getGracePreiod()*2){ //packet lost!
			    this->interface->packetLost(this);
			    //rest currentState;
			    this->currentState=0;
			}
		    }
		    else{
		      this->currentState=0;
		    }

		    //We never wait for comm sim, instead we wait for oter sims
		    TIME myminNextTime=Infinity;
		    TIME minNextTime=(TIME)interface->realReduceMinTime(myminNextTime);

		    //min time is the estimated next time, so grant nextEstimated time
		    if(minNextTime==myminNextTime)
			      throw SyncAlgoException(minNextTime);

		   
		    return minNextTime;

	     
	}
	
	bool CommunicatorSimulatorSyncalgo::doDispatchNextEvent(TIME currentTime,TIME nextTime){
		TIME networkTime=this->GetNextTime(currentTime,nextTime);
		
		return networkTime==nextTime;
	}

} /* namespace sim_comm */
