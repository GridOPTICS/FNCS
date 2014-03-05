/*
 * activesetconservative.h
 *
 *  Created on: Mar 5, 2014
 *      Author: ciraci
 */

#ifndef ACTIVESETCONSERVATIVE_H_
#define ACTIVESETCONSERVATIVE_H_

#include "abssyncalgorithm.h"
#include "fncsconfig.h"
#include "factorydatabase.h"
#include "message.h"

namespace sim_comm {

	class ActiveSetConservativeTick : public AbsSyncAlgorithm {
		private:
			bool mightSleep,sentMessage;
			bool nodeSentMessage(Message* msg);

		public:
			ActiveSetConservativeTick(AbsCommManager* currentInterface);
			virtual ~ActiveSetConservativeTick();
			virtual TIME GetNextTime(TIME currentTime,TIME nextTime);
			virtual bool forkedNewChild() { return false; }
			static AbsSyncAlgorithm* Create(const Json::Value& val, AbsCommManager* manager);
	};

} /* namespace sim_comm */

#endif /* ACTIVESETCONSERVATIVE_H_ */
