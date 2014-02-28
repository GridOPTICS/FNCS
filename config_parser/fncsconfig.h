/*
 * FncsConfig.h
 *
 *  Created on: Feb 20, 2014
 *      Author: ciraci
 */

#ifndef FNCSCONFIG_H_
#define FNCSCONFIG_H_



#include "absnetworkinterface.h"
#include "abssyncalgorithm.h"

#include <fstream>
#include <exception>

namespace Json{
	class Value;
}

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

	struct ConfigNodes;

	class FncsConfig {
		private:
			ConfigNodes *nodes;
			bool simType;
			time_metric simMetric;
			TIME packetLostP;
			TIME oneTimeStep;
			void extractParams();


		public:
			FncsConfig(const char *configFileName);
			virtual ~FncsConfig();
			void createIntegrator(TIME initialTime);
			static time_metric jsonToTimeMetric(const Value &given);
	};

} /* namespace sim_comm */

#endif /* FNCSCONFIG_H_ */
