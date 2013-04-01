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
		this->algotype=ALGO_COMM_SIM;
		
	}

	CommunicatorSimulatorSyncalgo::~CommunicatorSimulatorSyncalgo() {
		
	}
	
	
	//TODO: packet loss needs a counter!
	TIME CommunicatorSimulatorSyncalgo::GetNextTime(TIME currentTime,TIME nextTime){
	  	
		   if(nextTime < grantedTime)
		     return grantedTime;
		    

		    uint8_t diff=interface->reduceTotalSendReceive();
		    //assume network stable
		   
		    if(diff>0)
		    { //network unstable 
			TIME graceTime=Integrator::getCurSimTime()-Integrator::getPacketLostPeriod();
			if(graceTime>Integrator::getCurSimTime()) //overflowed
			    graceTime=0;
			if(updated){
			    if(this->currentState<graceTime){ //test if it has been graceperiod amount of time before we declare the packet as lost
		    /* TODO packetLost doesn't take parameters?? */
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
		    }

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
	
	bool CommunicatorSimulatorSyncalgo::doDispatchNextEvent(TIME currentTime,TIME nextTime){
		TIME networkTime=this->GetNextTime(currentTime,nextTime);
		
		return networkTime>nextTime;
	}

} /* namespace sim_comm */
