/*
 * activesetconservative.cpp
 *
 *  Created on: Mar 5, 2014
 *      Author: ciraci
 */

#include "config.h"
#include "activesetconservativetick.h"
#include "callback.h"
namespace sim_comm {

	FNCS_SYNCALGO(ActiveSetConservativeTick);

	ActiveSetConservativeTick::ActiveSetConservativeTick(
			AbsCommManager* currentInterface) :
			AbsSyncAlgorithm(currentInterface) {
		mightSleep = false;
		sentMessage = false;

		CallBack<bool, Message*, empty, empty, empty> *syncAlgoCallBackSend =
				CreateObjCallback<ActiveSetConservativeTick*,
						bool (ActiveSetConservativeTick::*)(Message *), bool,
						Message*>(this,
						&ActiveSetConservativeTick::nodeSentMessage);

		currentInterface->setSyncAlgoCallBacks(syncAlgoCallBackSend,nullptr);
	}

	ActiveSetConservativeTick::~ActiveSetConservativeTick() {
		// TODO Auto-generated destructor stub
	}

	TIME ActiveSetConservativeTick::GetNextTime(TIME currentTime, TIME nextTime) {

		TIME nextEstTime;
		if (nextTime < grantedTime)
			return grantedTime;

		//bool busywait = false;
		bool needToRespond = false;
		bool reIterate = false;

		do {

			uint64_t diff = interface->reduceTotalSendReceive();
			//network unstable, we need to wait!
			nextEstTime = currentTime + Integrator::getOneTimeStep();
			TIME minnetworkdelay = interface->reduceNetworkDelay();
			if (diff == 0 && !needToRespond) { //network stable grant next time
				nextEstTime = nextTime;
			} else {
				needToRespond = true; //set this condition so that when the simulator wakes up from busy wait it responds to messages
			}

			//Calculate next min time step
			TIME myminNextTime = nextEstTime;
			TIME minNextTime = (TIME) interface->reduceMinTimeWithSleep(
					myminNextTime, sentMessage);
			sentMessage = false;

			if (minNextTime == 0) { //a sim signal endded
#if DEBUG
					CERR << "End Signaled!" << endl;
#endif
				this->finished = true;
				return 0;
			}

			if (minNextTime < myminNextTime) {
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

	bool ActiveSetConservativeTick::nodeSentMessage(Message* msg) {
		sentMessage = true;
		return true;
	}

	AbsSyncAlgorithm* ActiveSetConservativeTick::Create(const Json::Value& val,
			AbsCommManager* manager) {

		return new ActiveSetConservativeTick(manager);
	}

} /* namespace sim_comm */
