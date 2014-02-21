/*
 Copyright (c) 2013, <copyright holder> <email>
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the <organization> nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY <copyright holder> <email> ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL <copyright holder> <email> BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "optimisticcommsyncalgo.h"

namespace sim_comm {

	FNCS_SYNCALGO(OptimisticCommSyncAlgo);

	OptimisticCommSyncAlgo::OptimisticCommSyncAlgo(AbsCommManager* interface,
			SpeculationTimeCalculationStrategy *strategy) :
			OptimisticTickSyncAlgo(interface, strategy) {
		//the base constructor does what we want!
	}

	OptimisticCommSyncAlgo::~OptimisticCommSyncAlgo() {
		//we add nothing to base class!
	}

	TIME OptimisticCommSyncAlgo::GetNextTime(TIME currentTime, TIME nextTime) {
		if (nextTime < grantedTime)
			return grantedTime;
#if DEBUG
		CERR << "Start sync " << currentTime << " " << nextTime << endl;
#endif
		if (currentTime > this->specFailTime) { //we passed beyond spec. failure time. We can speculatie again.
			this->specFailTime = Infinity;
		}

		/*if(this->isChild) //if a child comes here, algorithm is not working!
		 throw SyncStateException("Child wants to sync but parent is still alive cannot happen!");*/

		uint64_t diff = interface->reduceTotalSendReceive();

		if (diff > 0)
			this->interface->packetLostCalculator(currentTime);

		TIME minnetworkdelay = interface->reduceNetworkDelay();
		//We never wait for comm sim, instead we wait for oter sims
		TIME specNextTime = Infinity; //netsim follows what the power simulates do
		TIME minNextTime = Infinity;

#ifdef DEBUG
		CERR << "Consensus on message-diff " << diff << endl;
#endif
		if (diff == 0)
			if (!hasChild()) {
				interface->aggreateReduceMin(minNextTime, specNextTime);

			} else {
				globalAction = comm->action;
				interface->aggreateReduceMin(minNextTime, globalAction);
			}
		else { //diff!=0 only reduce min op
			minNextTime = (TIME) interface->reduceMinTime(Infinity);
			specNextTime = 0;
		}
#ifdef DEBUG
		CERR << "Consensus " << minNextTime << " spec: " << specNextTime << endl;
#endif
		//speculation stuff!!
		TIME specResult = testSpeculationState(specNextTime, currentTime);
		if (specResult > 0) //we are in child! We are granted up to specNextTime
			minNextTime = specNextTime;
		//normal convervative algorithm!!!
		//If min time is infinity then there is something with the comm!
		if (minNextTime == Infinity) {
#if DEBUG
			CERR << "End Signaled!" << endl;
#endif
			this->finished = true;
			return Infinity;
		}
		this->grantedTime = minNextTime;

		return minNextTime;
	}

	AbsSyncAlgorithm* OptimisticCommSyncAlgo::Create(Json::Value param,
			AbsCommManager *comm) {
		if (param["strategy"].isNull())
			throw ConfigException("Speculation strategy is not defined!");

		SpeculationTimeCalculationStrategy *st;
		string strategy = param["strategy"].asString();
		if (strategy.compare("constant") == 0) {
			time_metric met = FncsConfig::jsonToTimeMetric(param["metric"]);
			TIME specTime = (TIME) param["look_ahead_time"].asUInt64();
			st = new ConstantSpeculationTimeStrategy(met, specTime);
		} else if (strategy.compare("increasing") == 0) {
			time_metric met = FncsConfig::jsonToTimeMetric(param["metric"]);
			TIME specTime = (TIME) param["initial_look_ahead_time"].asUInt64();
			st = new IncreasingSpeculationTimeStrategy(met, specTime);
		} else if (strategy.compare("infinity") == 0) {
			st = new InfinitySpeculationTimeStrategy();
		} else {
			throw ConfigException("unknown speculation strategy!");
		}

		return new OptimisticCommSyncAlgo(comm, st);
	}
}

