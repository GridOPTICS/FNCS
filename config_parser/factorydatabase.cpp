/*
 * FactoryDataBase.cpp
 *
 *  Created on: Feb 20, 2014
 *      Author: ciraci
 */
#include "config.h"

#include "factorydatabase.h"

namespace sim_comm {

	FactoryDataBase* FactoryDataBase::instance = nullptr;

	FactoryDataBase::FactoryDataBase() {

	}

	FactoryDataBase* FactoryDataBase::getInstance() {
		if (FactoryDataBase::instance == nullptr)
			FactoryDataBase::instance = new FactoryDataBase();
		return FactoryDataBase::instance;
	}

	void FactoryDataBase::registerSyncAlgoFactory(string name,
			CallBack<AbsSyncAlgorithm*, const Json::Value&, AbsCommManager*, empty, empty>* factory) {
		this->syncAlgoFactories.insert(
				pair<string,
						CallBack<AbsSyncAlgorithm*, const Json::Value&, AbsCommManager*, empty,
								empty>*>(name, factory));
	}

	void FactoryDataBase::registerNetworkInterfaceFactory(string name,
			CallBack<AbsNetworkInterface*, const Json::Value&, bool, empty, empty>* factory) {
		this->networkInterfaceFactories.insert(
				pair<string,
						CallBack<AbsNetworkInterface*, const Json::Value&, bool,
								empty, empty>*>(name, factory));
	}

	void FactoryDataBase::registerCommManagerFactory(string name,
			CallBack<AbsCommManager*, AbsNetworkInterface*, bool, empty, empty>* factory) {
		this->commManagerFactories.insert(
				pair<string,
						CallBack<AbsCommManager*, AbsNetworkInterface*, bool,
								empty, empty>*>(name, factory));
	}

	CallBack<AbsCommManager*, AbsNetworkInterface*, bool, empty, empty>* FactoryDataBase::getCommManagerFactory(
			string name) {

		map<string,
				CallBack<AbsCommManager*, AbsNetworkInterface*, bool, empty,
						empty>*>::iterator it = commManagerFactories.find(name);
		if (it == commManagerFactories.end())
			return nullptr;
		return it->second;
	}

	CallBack<AbsSyncAlgorithm*, const Json::Value&, AbsCommManager*, empty, empty>* FactoryDataBase::getSyncAlgoFactory(
			string name) {
		map<string,
				CallBack<AbsSyncAlgorithm*, const Json::Value&, AbsCommManager*, empty, empty>*>::iterator it =
				syncAlgoFactories.find(name);
		if (it == syncAlgoFactories.end())
			return nullptr;
		return it->second;
	}

	CallBack<AbsNetworkInterface*, const Json::Value&, bool, empty, empty>* FactoryDataBase::getNetworkInterfaceFactory(
			string name) {
		map<string,
		CallBack<AbsNetworkInterface*, const Json::Value&, bool, empty, empty>*>::iterator it =
				networkInterfaceFactories.find(name);
		if (it == networkInterfaceFactories.end())
			return nullptr;
		return it->second;
	}

	FactoryDataBase::~FactoryDataBase() {

	}

} /* namespace sim_comm */
