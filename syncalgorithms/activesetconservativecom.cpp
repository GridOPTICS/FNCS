/*
 * activesetconvervativecom.cpp
 *
 *  Created on: Mar 5, 2014
 *      Author: ciraci
 */

#include "config.h"

#include "activesetconservativecom.h"
#include "abscommmanager.h"

namespace sim_comm {

	FNCS_SYNCALGO(ActiveSetConservativeCom);

	ActiveSetConservativeCom::ActiveSetConservativeCom(
			AbsCommManager *interface) :
			AbsSyncAlgorithm(interface) {
		// TODO Auto-generated constructor stub

	}

	ActiveSetConservativeCom::~ActiveSetConservativeCom() {
		// TODO Auto-generated destructor stub
	}

	TIME ActiveSetConservativeCom::GetNextTime(TIME currentTime,
			TIME nextTime) {
		TIME nextEstTime;
		if (nextTime < grantedTime)
			return grantedTime;

		//bool busywait = false;
		bool needToRespond = false;
		bool reIterate = false;

		do {

			uint64_t diff = interface->reduceTotalSendReceive();
			if (diff > 0) {
				this->interface->packetLostCalculator(currentTime);
			}
			//network unstable, we need to wait!

			TIME minnetworkdelay = interface->reduceNetworkDelay();
			TIME myminNextTime = Infinity;
			TIME minNextTime = (TIME) interface->reduceMinTimeWithSleep(
					myminNextTime, false);

			if (minNextTime == 0) { //a sim signal endded
#if DEBUG
					CERR << "End Signaled!" << endl;
#endif
				this->finished = true;
				return 0;
			}

			if (minNextTime == grantedTime) {
				currentTime = convertToMyTime(Integrator::getCurSimMetric(),
						minNextTime);
				currentTime = convertToFrameworkTime(
						Integrator::getCurSimMetric(), currentTime);

				reIterate = true;
			}
		} while (reIterate);

		this->grantedTime = nextEstTime;
#ifdef PROFILE
		writeTime(currentTime);
#endif
		return nextEstTime;

	}

	AbsSyncAlgorithm* ActiveSetConservativeCom::Create(const Json::Value& val,
			AbsCommManager* manager) {
		return new ActiveSetConservativeCom(manager);
	}

} /* namespace sim_comm */
