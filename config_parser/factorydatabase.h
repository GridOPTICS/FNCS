/*
 * FactoryDataBase.h
 *
 *  Created on: Feb 20, 2014
 *      Author: ciraci
 */

#ifndef FACTORYDATABASE_H_
#define FACTORYDATABASE_H_

#include <map>
#include "callback.h"
#include "json/json.h"

#include "abscommmanager.h"
#include "absnetworkinterface.h"
#include "abssyncalgorithm.h"

#define FNCS_SYNCALGO(type)       \
  static struct x ## type ## Reg      \
  {                                             \
    x ## type ## Reg () {             \
	  sim_comm::CallBack<sim_comm::AbsSyncAlgorithm*,const Json::Value&,sim_comm::AbsCommManager*,sim_comm::empty,sim_comm::empty> *c=\
	  sim_comm::CreateCallback(type::Create);\
	  sim_comm::FactoryDataBase::getInstance()->registerSyncAlgoFactory(string(#type),c);\
    }                                           \
  } x_ ## type ## Var

#define FNCS_NETWORKINTERFACE(type)       \
  static struct x ## type ## Reg      \
  {                                             \
    x ## type ## Reg () {             \
	  sim_comm::CallBack<sim_comm::AbsNetworkInterface*,const Json::Value&,bool,sim_comm::empty,sim_comm::empty> *c=\
	  sim_comm::CreateCallback(type::Create);\
	  sim_comm::FactoryDataBase::getInstance()->registerNetworkInterfaceFactory(string(#type),c);\
    }                                           \
  } x_ ## type ## Var

#define FNCS_COMMMANAGER(type)       \
  static struct x ## type ## Reg      \
  {                                             \
    x ## type ## Reg () {             \
	  sim_comm::CallBack<sim_comm::AbsCommManager*,AbsNetworkInterface*,bool,sim_comm::empty,sim_comm::empty> *c=\
	  sim_comm::CreateCallback(type::Create);\
	  sim_comm::FactoryDataBase::getInstance()->registerCommManagerFactory(string(#type),c);\
    }                                           \
  } x_ ## type ## Var

using namespace std;

namespace sim_comm {

	class FactoryDataBase {

		private:
			map<string, CallBack<AbsSyncAlgorithm*,const Json::Value&,AbsCommManager *,empty,empty>* > syncAlgoFactories;
			map<string, CallBack<AbsNetworkInterface*,const Json::Value&,bool,empty,empty>* > networkInterfaceFactories;
			map<string, CallBack<AbsCommManager*, AbsNetworkInterface*,bool,empty,empty>* > commManagerFactories;
			static FactoryDataBase *instance;
			FactoryDataBase();
		public:
			static FactoryDataBase* getInstance();
			void registerSyncAlgoFactory(string name,CallBack<AbsSyncAlgorithm*,const Json::Value&,AbsCommManager*,empty,empty>* factory);
			void registerNetworkInterfaceFactory(string name,CallBack<AbsNetworkInterface*,const Json::Value&,bool,empty,empty>* factory);
			void registerCommManagerFactory(string name,CallBack<AbsCommManager*, AbsNetworkInterface*,bool,empty,empty>* factory);
			CallBack<AbsCommManager*, AbsNetworkInterface*,bool,empty,empty>* getCommManagerFactory(string name);
			CallBack<AbsSyncAlgorithm*,const Json::Value&,AbsCommManager*,empty,empty>* getSyncAlgoFactory(string name);
			CallBack<AbsNetworkInterface*,const Json::Value&,bool,empty,empty>* getNetworkInterfaceFactory(string name);
			~FactoryDataBase();
	};

} /* namespace sim_comm */

#endif /* FACTORYDATABASE_H_ */
