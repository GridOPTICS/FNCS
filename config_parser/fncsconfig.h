/*
 * FncsConfig.h
 *
 *  Created on: Feb 20, 2014
 *      Author: ciraci
 */

#ifndef FNCSCONFIG_H_
#define FNCSCONFIG_H_

#include "json/json.h"
#include "absnetworkinterface.h"
#include "abssyncalgorithm.h"

#include <fstream>
#include <exception>

using namespace Json;
using namespace std;

namespace sim_comm {

	class ConfigException : public exception{
		private:
			string msg;
		public:
			ConfigException(string msg) throw() { this->msg=msg; }
			virtual const char* what() const throw() {
				stringstream toReturn;
				toReturn << "Error while parsing config: " << msg;
				return toReturn.str().c_str();
			}
			virtual ~ConfigException() throw() {}
	};

	class FncsConfig {
		private:
			Value syncAlgorithm;
			Value syncAlgoParams;
			Value networkInterface;
			Value broker;
			bool simType;
			time_metric simMetric;
			TIME packetLostP;
			TIME oneTimeStep;
			void extractParams(const Value &root);
			AbsNetworkInterface* createNetworkInterface();

		public:
			FncsConfig(const char *configFileName);
			virtual ~FncsConfig();
			AbsSyncAlgorithm* createSyncAlgorithm();
			AbsCommManager* createCommManager();
			bool isCommSimulator();
			bool isPowerGridSimulator();
			time_metric getSimTimeMetric();
			TIME getPacketLostPeriod();

	};

} /* namespace sim_comm */

#endif /* FNCSCONFIG_H_ */
