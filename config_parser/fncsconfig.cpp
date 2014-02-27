/*
 * FncsConfig.cpp
 *
 *  Created on: Feb 20, 2014
 *      Author: ciraci
 */

#include "FncsConfig.h"

#include "zmqnetworkinterface.h"
#include "graceperiodcommmanager.h"
#include "communicationcommanager.h"

namespace sim_comm {

	FncsConfig::FncsConfig(const char *configFile) {

		Json::Value root;
		std::ifstream file(configFile);
		file >> root;
		extractParams(root);
	}

	FncsConfig::~FncsConfig() {

	}

	void FncsConfig::extractParams(const Value &root){
		this->networkInterface=root["interface"];
		this->broker=root["broker"];

		if(root["simulator_type"].isNull()){
			throw ConfigException(string("simulator type is not defined!"));
		}
		string simType=root["simulator_type"].asString();
		if(simType.compare("power_grid")==0){
			this->simType=true;
		}else{
			if(simType.compare("communication_network"))
				this->simType=false;
			else
				throw ConfigException(string("unknown simulator type"));
		}

		this->syncAlgorithm=root["synchronization_algorithm"];
		this->syncAlgoParams=root["sync_params"];

		if(root["simulator_time_metric"].isNull()){
			throw ConfigException(string("Simulator time metric is not defined!"));
		}
		string timeMetric=root["simulator_time_metric"].asString();
		if(timeMetric.compare("seconds")==0){
			this->simMetric=SECONDS;
		}else
		if(timeMetric.compare("milliseconds")==0){
			this->simMetric=MILLISECONDS;
		}else
		if(timeMetric.compare("nanoseconds")==0){
			this->simMetric=NANOSECONDS;
		}else
			throw ConfigException(string("Simulator time metric is not correctly defined!"));

		if(root["one_time_step"].isNull()){
			this->oneTimeStep=1;
		}else{
			this->oneTimeStep=(TIME)root["one_time_step"].asUInt64();
		}

		if(root["packet_lost_period"].isNull()){
			throw ConfigException(string("Packet lost period is not defined!"));
		}else
			this->packetLostP=(TIME)root["packet_lost_period"].asUInt64();
	}

	bool FncsConfig::isCommSimulator(){
		return !this->simType;
	}

	AbsNetworkInterface* FncsConfig::createNetworkInterface() {
		if(this->networkInterface.isNull())
			throw ConfigException(string("Network interface is not defined!"));

		string type=this->networkInterface.asString();
		if(type.compare("zmq")==0){
			if(this->broker.isNull()){
				throw ConfigException(string("Borker address is not defined!"));
			}
			string brokerAdd=broker.asString();
			return new ZmqNetworkInterface(brokerAdd,this->simType);
		}//add other network interfaces here!
		else{
			throw ConfigException(string("Unknown network interface type!"));
		}
	}

	AbsCommManager* FncsConfig::createCommManager() {
		AbsNetworkInterface *interface=createNetworkInterface();
		if(this->simType){
			return new GracePeriodCommManager(interface);
		}else{
			return new CommunicationComManager(interface);
		}
	}

	bool FncsConfig::isPowerGridSimulator(){
		return this->simType;
	}



} /* namespace sim_comm */
