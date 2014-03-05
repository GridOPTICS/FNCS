/*
 * activesetconvervativecom.h
 *
 *  Created on: Mar 5, 2014
 *      Author: ciraci
 */

#ifndef ACTIVESETCONVERVATIVECOM_H_
#define ACTIVESETCONVERVATIVECOM_H_

#include "abssyncalgorithm.h"
#include "factorydatabase.h"
#include "fncsconfig.h"

namespace sim_comm {

	class ActiveSetConservativeCom : public AbsSyncAlgorithm {
		public:
			ActiveSetConservativeCom(AbsCommManager *interface);
			virtual ~ActiveSetConservativeCom();
			virtual TIME GetNextTime(TIME currentTime,TIME nextTime);
			virtual bool forkedNewChild() { return false; }
			static AbsSyncAlgorithm* Create(const Json::Value& val, AbsCommManager* manager);
	};

} /* namespace sim_comm */

#endif /* ACTIVESETCONVERVATIVECOM_H_ */
