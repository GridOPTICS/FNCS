/*
 * communicatorsimulatorsyncalgo.h
 *
 *  Created on: Feb 16, 2013
 *      Author: ciraci
 */

#ifndef COMMUNICATORSIMULATORSYNCALGO_H_
#define COMMUNICATORSIMULATORSYNCALGO_H_

#include "abssyncalgorithm.h"

namespace sim_comm {

 
	class CommSyncAlgoException: exception{
	
	  public:
	    virtual const char* what() const throw() {
		return "Incorrect CommManager instance, it should be an instance of CommSimulatorCommManage";
	    }
	    
	};
	
	/**
	 *Conservative sync algorithm for comminucator simulator
	 *(or any discrete event simulator).
	 */ 
	class CommunicatorSimulatorSyncalgo: public sim_comm::AbsSyncAlgorithm {
		private:
			TIME currentState;
			bool updated;
		public:
			CommunicatorSimulatorSyncalgo(AbsCommManager* currentInterface);
			virtual ~CommunicatorSimulatorSyncalgo();
			/** @copydoc AbsSyncAlgorithm::GetNextTime(TIME currentTime, TIME nextTime) */
			virtual TIME GetNextTime(TIME currentTime,TIME nextTime);
			/** @copydoc AbsSyncAlgorithm::doDispatchNextEvent(TIME currentTime, TIME nextTime) */
			virtual bool doDispatchNextEvent(TIME currentTime,TIME nextTime);
			
	};

} /* namespace sim_comm */
#endif /* COMMUNICATORSIMULATORSYNCALGO_H_ */
