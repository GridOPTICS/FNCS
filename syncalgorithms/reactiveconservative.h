/*
 * reactiveconservative.h
 *
 *  Created on: Mar 17, 2014
 *      Author: ciraci
 */

#ifndef REACTIVECONSERVATIVE_H_
#define REACTIVECONSERVATIVE_H_

#include "graceperiodpesimisticsyncalgo.h"
#include "message.h"
#include "callback.h"
#include "factorydatabase.h"
#include "fncsconfig.h"

namespace sim_comm {

	class ReactiveConservative: public sim_comm::AbsSyncAlgorithm {
		public:
			ReactiveConservative(AbsCommManager *given);
			virtual ~ReactiveConservative();
			virtual TIME GetNextTime(TIME currentTime,TIME nextTime);
			virtual bool forkedNewChild(){ return false;}
			static AbsSyncAlgorithm* Create(const Json::Value& params, AbsCommManager *manager);
		private:
			bool receivedMessage(Message *msg);
			bool sentMessage(Message *msg);
			bool respond;
	};

} /* namespace sim_comm */

#endif /* REACTIVECONSERVATIVE_H_ */
