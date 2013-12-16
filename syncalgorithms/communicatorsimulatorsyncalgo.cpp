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
		updated=false;
		
	}

	CommunicatorSimulatorSyncalgo::~CommunicatorSimulatorSyncalgo() {
		
	}

	TIME CommunicatorSimulatorSyncalgo::GetNextTime(TIME currentTime,TIME nextTime){
	  	
		   if(nextTime < grantedTime)
		     return grantedTime;
		    

		    uint64_t diff=interface->reduceTotalSendReceive();
		    if(diff > 0){
		      this->interface->packetLostCalculator(currentTime);
		    }
		    TIME minnetworkdelay=interface->reduceNetworkDelay();
		    //We never wait for comm sim, instead we wait for oter sims
		    TIME myminNextTime=Infinity;
		    TIME minNextTime=(TIME)interface->reduceMinTime(myminNextTime);
		    
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
	

} /* namespace sim_comm */
