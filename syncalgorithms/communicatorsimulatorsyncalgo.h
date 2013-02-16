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

	class CommunicatorSimulatorSyncalgo: public sim_comm::AbsSyncAlgorithm {
		private:
			TIME currentState;
		public:
			CommunicatorSimulatorSyncalgo(AbsCommInterface* currentInterface);
			virtual ~CommunicatorSimulatorSyncalgo();
			virtual TIME GetNextTime(TIME currentTime,TIME nextTime);
			virtual bool doDispatchNextEvent(TIME currentTime,TIME nextTime);
	};

} /* namespace sim_comm */
#endif /* COMMUNICATORSIMULATORSYNCALGO_H_ */
