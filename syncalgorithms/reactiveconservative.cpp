/*
 * reactiveconservative.cpp
 *
 *  Created on: Mar 17, 2014
 *      Author: ciraci
 */

#include "reactiveconservative.h"

namespace sim_comm {

	FNCS_SYNCALGO(ReactiveConservative);

	ReactiveConservative::ReactiveConservative(AbsCommManager *given) : AbsSyncAlgorithm(given) {
		CallBack<bool,Message*,empty,empty,empty> *sendlink=
				CreateObjCallback<ReactiveConservative*, bool (ReactiveConservative::*)(Message *),bool, Message*>(this,&ReactiveConservative::sentMessage);
		CallBack<bool,Message*,empty,empty,empty> *recvlink=
				CreateObjCallback<ReactiveConservative*, bool (ReactiveConservative::*)(Message *),bool, Message*>(this,&ReactiveConservative::receivedMessage);
		this->interface->setSyncAlgoCallBacks(sendlink,recvlink);
		respond=false;
	}

	ReactiveConservative::~ReactiveConservative() {
		// TODO Auto-generated destructor stub
	}

	bool ReactiveConservative::receivedMessage(Message* msg) {
		respond=true;
		return true;
	}

	TIME ReactiveConservative::GetNextTime(TIME currentTime, TIME nextTime) {
		TIME nextEstTime;

		if(nextTime <= grantedTime)
			return nextTime;


		bool needToRespond=false;
		busywait=false;
		//send all messages

		//nextEstTime=currentTime;
		do
		{
			if(busywait)
				this->interface->waitforAll();
			uint64_t diff=interface->reduceTotalSendReceive();

			//we need to grant one time step only if I need to respond.
			if(this->respond){
#if DEBUG
			CERR << "I recieved a message, so I'm responding" << endl;
#endif
				nextEstTime=currentTime+Integrator::getOneTimeStep();
				if(diff>0)
					needToRespond=true;
			}
			else
				nextEstTime=Infinity;
			TIME minnetworkdelay=interface->reduceNetworkDelay();


			//Calculate next min time step
			TIME myminNextTime=nextEstTime;
			TIME minNextTime=(TIME)interface->reduceMinTime(myminNextTime);

			//min time is the estimated next time, so grant nextEstimated time
			if(minNextTime==myminNextTime || myminNextTime==Infinity){
				busywait=false;
				nextEstTime=minNextTime;
			}
			else
			if(minNextTime < myminNextTime){
				currentTime = convertToMyTime(Integrator::getCurSimMetric(),minNextTime);
				currentTime = convertToFrameworkTime(Integrator::getCurSimMetric(),currentTime);
				/* if(minNextTime+Integrator::getGracePeriod()<myminNextTime) //we have to busy wait until other sims come to this time
		                  busywait=true;
		              else //TODO this will cause gld to re-iterate*/
				busywait=true;
			}


		}while(busywait);


		this->grantedTime=nextEstTime;
#ifdef PROFILE
		writeTime(currentTime);
#endif
		if(nextTime < nextEstTime){
			return nextTime;
		}
		return nextEstTime;
	}

	AbsSyncAlgorithm* ReactiveConservative::Create(const Json::Value& params,
			AbsCommManager* manager) {
		return new ReactiveConservative(manager);
	}

	bool ReactiveConservative::sentMessage(Message* msg) {
		respond=false;
		return true;
	}

} /* namespace sim_comm */
