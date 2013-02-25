/*
 * communicatorsimulatorsyncalgo.cpp
 *
 *  Created on: Feb 16, 2013
 *      Author: ciraci
 */
#include "config.h"

#include "abscommmanager.h"
#include "communicationcommanager.h"
#include "communicatorsimulatorsyncalgo.h"
#include "integrator.h"


namespace sim_comm {

	CommunicatorSimulatorSyncalgo::CommunicatorSimulatorSyncalgo(AbsCommManager* currentInterface): AbsSyncAlgorithm(currentInterface) {
		this->currentState=0;
		CommunicationComManager *given=dynamic_cast<CommunicationComManager*>(currentInterface);
		if(given==nullptr)
		  throw CommSyncAlgoException();

	}

	CommunicatorSimulatorSyncalgo::~CommunicatorSimulatorSyncalgo() {
		
	}
	
	
	//TODO: packet loss needs a counter!
	TIME CommunicatorSimulatorSyncalgo::GetNextTime(TIME currentTime,TIME nextTime){
	  	
		   


		    uint8_t diff=interface->reduceTotalSendReceive();
		    //assume network stable
		   
		    if(diff>0)
		    { //network unstable 
			if(this->currentState>Integrator::getGracePeriod()*2){ //packet lost!
                /* TODO packetLost doesn't take parameters?? */
			    //this->interface->packetLost(this);
			    this->interface->packetLost();
			    //rest currentState;
			    this->currentState=0;
			}
		    }
		    else{
		      this->currentState=0;
		    }

		    //We never wait for comm sim, instead we wait for oter sims
		    TIME myminNextTime=Infinity;
		    TIME minNextTime=(TIME)interface->reduceMinTime(myminNextTime);
		    
		    //If min time is infinity then there is something with the comm!
		    if(minNextTime==myminNextTime)
			      throw SyncAlgoException(minNextTime);
		  
		     if(minNextTime==0){ //a sim signal endded
			  this->finished=true;
			  return 0;
		     }
		     this->currentState=minNextTime;
		   
		    return minNextTime;

	     
	}
	
	bool CommunicatorSimulatorSyncalgo::doDispatchNextEvent(TIME currentTime,TIME nextTime){
		TIME networkTime=this->GetNextTime(currentTime,nextTime);
		
		return networkTime>nextTime;
	}

} /* namespace sim_comm */
