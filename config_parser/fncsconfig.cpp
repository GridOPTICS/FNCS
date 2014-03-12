/*
 * FncsConfig.cpp
 *
 *  Created on: Feb 20, 2014
 *      Author: ciraci
 */
#include "config.h"

#include "fncsconfig.h"

#include "zmqnetworkinterface.h"
#include "graceperiodcommmanager.h"
#include "communicationcommanager.h"

#include <json/json.h>

namespace sim_comm {

	struct ConfigNodes{
			Value root;
			Value broker;
			Value networkInterface;
			Value syncAlgorithm;
			Value syncAlgoParams;
	};

	FncsConfig::FncsConfig(const char *configFile) {

		this->nodes = new ConfigNodes();
		std::ifstream file(configFile);
		file >> nodes->root;


	}

	FncsConfig::~FncsConfig() {
		delete nodes;
	}

	time_metric FncsConfig::jsonToTimeMetric(const Value &given) {
		if (given.isNull())
			throw ConfigException("Value is not a time_metric specification");
		string timeMetric = given.asString();
		if (timeMetric.compare("seconds") == 0) {
			return SECONDS;
		} else if (timeMetric.compare("milliseconds") == 0) {
			return MILLISECONDS;
		} else if (timeMetric.compare("nanoseconds") == 0) {
			return NANOSECONDS;
		} else
			throw ConfigException(
					string("Simulator time metric is not correctly defined!"));

	}

	void FncsConfig::extractParams() {
		nodes->networkInterface = (nodes->root)["interface"];
		nodes->broker = (nodes->root)["broker"];

		if ((nodes->root)["simulator_type"].isNull()) {
			throw ConfigException(string("simulator type is not defined!"));
		}
		string simType = (nodes->root)["simulator_type"].asString();
		if (simType.compare("power_grid") == 0) {
			this->simType = true;
		} else {
			if (simType.compare("communication_network") ==0)
				this->simType = false;
			else
				throw ConfigException(string("unknown simulator type"));
		}

		nodes->syncAlgorithm = (nodes->root)["synchronization_algorithm"];
		nodes->syncAlgoParams = (nodes->root)["sync_params"];

		if ((nodes->root)["simulator_time_metric"].isNull()) {
			throw ConfigException(
					string("Simulator time metric is not defined!"));
		}
		this->simMetric=jsonToTimeMetric((nodes->root)["simulator_time_metric"]);


		if ((nodes->root)["one_time_step"].isNull()) {
			this->oneTimeStep = 1;
		} else {
			this->oneTimeStep = (TIME) (nodes->root)["one_time_step"].asUInt64();
		}

		if ((nodes->root)["packet_lost_period"].isNull()) {
			throw ConfigException(string("Packet lost period is not defined!"));
		} else
			this->packetLostP = (TIME) (nodes->root)["packet_lost_period"].asUInt64();
	}

	void FncsConfig::createIntegrator(TIME initialTime) {
		extractParams();
		CallBack<AbsNetworkInterface*, const Json::Value&, bool, empty, empty>* networkFactory =
				FactoryDataBase::getInstance()->getNetworkInterfaceFactory(
						nodes->networkInterface.asString());
		if (networkFactory == nullptr)
			throw ConfigException(
					"Cannot locate network interface in the registery!");
		AbsNetworkInterface *interface = (*networkFactory)(nodes->broker,
				this->simType);
		CallBack<AbsCommManager*, AbsNetworkInterface*, bool, empty, empty> *commManagerFactory;
		if (this->simType) { //powergrid sim
			commManagerFactory =
					FactoryDataBase::getInstance()->getCommManagerFactory(
							"GracePeriodCommManager");
		} else { //network simulator
			commManagerFactory =
					FactoryDataBase::getInstance()->getCommManagerFactory(
							"CommunicationComManager");
		}
		AbsCommManager *commManager = (*commManagerFactory)(interface,
				this->simType);

		CallBack<AbsSyncAlgorithm*, const Json::Value&, AbsCommManager *, empty,
				empty>* syncFactory =
				FactoryDataBase::getInstance()->getSyncAlgoFactory(
						nodes->syncAlgorithm.asString());
		if (syncFactory == nullptr)
			throw ConfigException("Cannot locate syncalgorithm!");

		AbsSyncAlgorithm *algo = (*syncFactory)(nodes->syncAlgoParams, commManager);

		Integrator::instance = new Integrator(commManager, algo,
				this->simMetric, this->packetLostP, this->oneTimeStep,!simType);
		Integrator::instance->offset = convertToFrameworkTime(simMetric,
				initialTime);
	}

} /* namespace sim_comm */
