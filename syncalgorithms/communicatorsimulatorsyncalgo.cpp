/*
 * communicatorsimulatorsyncalgo.cpp
 *
 *  Created on: Feb 16, 2013
 *      Author: ciraci
 */
#include "config.h"

#include "abscomminterface.h"
#include "communicatorsimulatorsyncalgo.h"
#include "integrator.h"


namespace sim_comm {

	CommunicatorSimulatorSyncalgo::CommunicatorSimulatorSyncalgo(AbsCommInterface* currentInterface): AbsSyncAlgorithm(currentInterface) {
		this->currentState=0;
		this->interface->setNoCounterIncrement(this,true);

	}

	CommunicatorSimulatorSyncalgo::~CommunicatorSimulatorSyncalgo() {
		
	}
	
	
	TIME CommunicatorSimulatorSyncalgo::GetNextTime(TIME currentTime,TIME nextTime){
	  	
		   


		    uint8_t diff=interface->realReduceTotalSendReceive();
		    //assume network stable
		   
		    if(diff>0)
		    { //network unstable 
			if(this->currentState>Integrator::getGracePeriod()*2){ //packet lost!
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

		    //If min time is infinity then there is something with the comm!
		    if(minNextTime==myminNextTime)
			      throw SyncAlgoException(minNextTime);
		  
		     if(minNextTime==0){ //a sim signal endded
			  this->finished=true;
			  return 0;
		     }
		   
		    return minNextTime;

	     
	}
	
	bool CommunicatorSimulatorSyncalgo::doDispatchNextEvent(TIME currentTime,TIME nextTime){
		TIME networkTime=this->GetNextTime(currentTime,nextTime);
		
		return networkTime>nextTime;
	}

} /* namespace sim_comm */
