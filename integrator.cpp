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
#include "config.h"

#include "abscommmanager.h"
#include "absnetworkinterface.h"
#include "communicationcommanager.h"
#include "graceperiodcommmanager.h"
#include "zmqnetworkinterface.h"
#include "conservativesleepingcommalgo.h"
#include "conservativesleepingtickalgo.h"
#include "integrator.h"
#include "objectcomminterface.h"
#include "optimisticcommsyncalgo.h"
#include "optimisticticksyncalgo.h"
#include "syncalgorithms/communicatorsimulatorsyncalgo.h"
#include "syncalgorithms/graceperiodpesimisticsyncalgo.h"
#include "syncalgorithms/graceperiodspeculativesyncalgo.h"
#include "optimisticlowoverheadcommsyncalgo.h"
#include "optimisticlowoverheadticksyncalgo.h"
#include "json/json.h"

Echo* Debug::instance=NULL;

Echo* Debug::getEcho()
{
  if(instance==NULL)
    throw "Debug system initialization error!!";
  return instance;
}

void Debug::setEcho(string& prefix)
{
  ostringstream os;
  os << prefix << "_" << getpid() << ".txt";
  instance=new Echo(os.str());
}

void Debug::setEcho(const char* prefix)
{
  ostringstream os;
  os << prefix << "_" << getpid() << ".txt";
  instance=new Echo(os.str());
}

void Debug::closeEcho()
{
  instance->close();
  delete instance;
}



namespace sim_comm {


Integrator* Integrator::instance=NULL;


Integrator::Integrator(
        AbsCommManager *currentInterface,
        AbsSyncAlgorithm *algo,
        time_metric simTimeStep,
	TIME packetLostPeriod,
	TIME onetimestep) {

#if DEBUG
    CERR << "Integrator::Integrator("
        << "AbsCommInterface*,"
        << "AbsSyncAlgorithm*,"
        << "simTimeStep=" << simTimeStep << ","
        << "packetlost=" << packetLostPeriod << "," <<
        << "onetimestep=" << onetimestep << ")" << endl;
#endif
    this->currentInterface=currentInterface;
    this->simTimeMetric=simTimeStep;
    this->allowRegistrations = true;
    this->syncAlgo=algo;
    this->packetLostPeriod=packetLostPeriod;
    this->onetimestep=convertToFrameworkTime(simTimeMetric,onetimestep);
   
}


Integrator::~Integrator(){
	this->currentInterface->stopReceiver();
	delete currentInterface;
	delete syncAlgo;
	instance=NULL;
}
// 

void Integrator::stopIntegrator(){
#if DEBUG
    CERR << "Integrator::stopIntegrator()" << endl;
#endif
	if(!isFinished()) {
#if DEBUG
	    CERR << "Signaling finish to other sims" << endl;
#endif
#if DEBUG_TO_FILE
	 Debug::closeEcho();
#endif
	    //instance->syncAlgo->GetNextTime(instance->getCurSimTime(),Infinity);
        instance->currentInterface->sendFinishedSignal();
	delete instance;
	instance=nullptr;
    }
	
}

TIME Integrator::getOneTimeStep()
{
  return instance->onetimestep;
}


void Integrator::terminate()
{
#if DEBUG
    CERR << "Integrator::terminate()" << endl;
#endif
#if DEBUG_TO_FILE
    Debug::closeEcho();
#endif
    if(instance!=nullptr){
      delete instance;
      instance = nullptr;
    }
}


TIME Integrator::getPacketLostPeriod()
{
  return instance->packetLostPeriod;
}

void Integrator::parseConfig(string jsonFile, TIME initialTime)
{

    TIME plp;
    TIME specTime;
    TIME onetimestep;
    enum time_metric tm;
    AbsNetworkInterface *comm;

   
    Json::Value root;
    std::ifstream file(jsonFile.c_str());
    file >> root;
    

    const Json::Value interface = root["interface"];
    const Json::Value simulator_type = root["simulator_type"];
    const Json::Value one_timestep = root["one_timestep"];
    const Json::Value synchronization_algorithm = root["synchronization_algorithm"];
    const Json::Value simulator_time_metric = root["simulator_time_metric"];
    const Json::Value packet_loss_period = root["packet_loss_period"];
    
    if (interface.isNull())
    {
        cout << "Error: No interface (either mpi or zmq)  in Json file" << endl;
        exit (1);
    }
    else
    {
        if (interface.asString() == "zmq") 
        {
	  
	    const Json::Value brokerAddress=root["broker"];
#if HAVE_ZMQ
            if (simulator_type.asString() == "power_grid") { 
		if(!brokerAddress.isNull())
		  comm = new ZmqNetworkInterface(brokerAddress.asString(),false);
		else
                comm = new ZmqNetworkInterface(false);
              
            }
            else if (simulator_type.asString() == "communication_network") {
	      if(!brokerAddress.isNull())
		comm = new ZmqNetworkInterface(brokerAddress.asString(),true);
	      else
                comm = new ZmqNetworkInterface(true);
                
            }
#else
            cout << "FNCS not built with zeromq" << endl;
            exit(EXIT_FAILURE);
#endif
        }
        else if (interface.asString() == "mpi") {
#if HAVE_MPI
            MpiNetworkInterface *comm = new MpiNetworkInterface(MPI_COMM_WORLD, true);
            cout << "interface is MPI" << endl;
#else
            cout << "FNCS not built with MPI" << endl;
            exit(EXIT_FAILURE);
#endif
        }
    }

    if (packet_loss_period.isNull()) 
        plp = 5000000000;
    else
        plp = packet_loss_period.asInt64();

#if DEBUG
    CERR << "packet loss period = " << plp << endl;
#endif
    if(one_timestep.isNull()){
      onetimestep=1;
    }else{
      onetimestep=one_timestep.asInt64();
    }

#if DEBUG
    CERR << "onetimestep=" << onetimestep <<endl;
#endif
    if(simulator_type.isNull())
    {
        cout << "Error: No simulator type in Json file" << endl; 
        exit (1);
    }

    if(synchronization_algorithm.isNull())
    {
        cout << "Error: No synchronization algorithm provided" << endl;
        exit (1);
    }

    if(simulator_time_metric.isNull())
    {
        cout << "Error: No simulator time metric provided" << endl;
        exit(1);
    }
    else
    {
        if (simulator_time_metric.asString() == "seconds")
            tm = SECONDS;

        else if ( simulator_time_metric.asString() == "milliseconds")
            tm = MILLISECONDS;

        else if (simulator_time_metric.asString() == "nanoseconds")
            tm = NANOSECONDS; 

    }

    if (simulator_type.asString().compare("power_grid") == 0)
    {
        if (synchronization_algorithm.isString())
        {
            if (synchronization_algorithm.asString().compare("conservative") == 0) 
            {
                Integrator::initIntegratorGracePeriod(comm, tm, plp, initialTime,onetimestep);
              
            } 
            else if(synchronization_algorithm.asString().compare("active_set_conservative") == 0)  
            {
   		
		if(synchronization_algorithm["conservative"]["number_of_power_simulators"].isNull()){
		  cout << "Number of power grid simulators is not defined!" << endl;
		  exit(1);
		}
		
		 int num=synchronization_algorithm["conservative"]["number_of_power_simulators"].asInt();
                Integrator::initIntegratorConservativeSleepingTick(comm,tm,plp,initialTime,num,onetimestep);

            }
        }
        else
        {
            if (!synchronization_algorithm["optimistic"].isNull())
            {
                if (synchronization_algorithm["optimistic"]["spec_time"].isNull())
                    specTime = 30000000000;
                else
                    specTime = synchronization_algorithm["optimistic"]["spec_time"].asInt64(); 

#if DEBUG
                CERR << "specTime = " << specTime << endl;
#endif

                if(synchronization_algorithm["optimistic"]["speculation_calculation_stragegy"].asString() == "constant") 
                {

                    ConstantSpeculationTimeStrategy *st = new ConstantSpeculationTimeStrategy(tm, specTime);
                    Integrator::initIntegratorOptimistic(comm, tm, plp,initialTime,specTime,st,onetimestep);
                   

                }
                else if (synchronization_algorithm["optimistic"]["speculation_calculation_stragegy"].asString() == "dynamic_increasing")
                {
                    IncreasingSpeculationTimeStrategy *st=new IncreasingSpeculationTimeStrategy(tm, specTime);
                    Integrator::initIntegratorOptimistic(comm, tm, plp, initialTime, specTime, st,onetimestep);
                 

                }  
            }
            else
            {
                cout << "Error: No appropriate algorithm provided" << endl;
                exit(1);
            }

        }
    } //end of "power_grid" type 

    else if (simulator_type.asString().compare("communication_network") == 0)
    {
        if (synchronization_algorithm.isString())
        {
            if (synchronization_algorithm.asString().compare("conservative") == 0) 
            {
                Integrator::initIntegratorCommunicationSim(comm, tm, plp, initialTime,onetimestep);
                
            }
            else if(synchronization_algorithm.asString().compare("active_set_conservative") == 0)
            {

                Integrator::initIntegratorConservativeSleepingComm(comm, tm, plp, initialTime,onetimestep);
                

            }
        }
        else
        {
            if (!synchronization_algorithm["optimistic"].isNull())
            { 

                if (synchronization_algorithm["optimistic"]["spec_time"].isNull())
                    specTime = 30000000000;
                else
                    specTime = (unsigned long)synchronization_algorithm["optimistic"]["spec_time"].asDouble();

                if(synchronization_algorithm["optimistic"]["speculation_calculation_stragegy"].asString() == "constant")
                {

                    ConstantSpeculationTimeStrategy *st = new ConstantSpeculationTimeStrategy(tm,specTime);
                    Integrator::initIntegratorOptimisticComm(comm, tm, plp, initialTime, specTime, st,onetimestep);
                   
                }
                else if (synchronization_algorithm["optimistic"]["speculation_calculation_stragegy"].asString() == "dynamic_increasing")
                {
                    IncreasingSpeculationTimeStrategy *st=new IncreasingSpeculationTimeStrategy(tm,specTime);
                    Integrator::initIntegratorOptimisticComm(comm, tm, plp, initialTime, specTime, st,onetimestep);
                  

                }
            }//end of if algorithm is optimistic
        }//end of else

    }//end of simulator type is comm network

}

void Integrator::initIntegrator(const char* configFile, TIME initialTime)
{
  //parseConfig(string(configFile),initialTime);
  FncsConfig config(configFile);
  config.createIntegrator(initialTime);
}


void Integrator::initIntegratorConservativeSleepingTick(
  AbsNetworkInterface* currentInterface, 
  time_metric simTimeStep, 
  TIME packetLostPeriod, 
  TIME initialTime,
  int numofpowersims,
  TIME onetimestep)
{
#if DEBUG
    CERR << "Integrator::initIntegratorConservativeSleepingTick("
        << "AbsCommInterface*,"
        << "simTimeStep=" << simTimeStep << ","
        << "packetlost=" << packetLostPeriod << ","
        << "initialTime=" << initialTime << ","
	<< "onetimestep=" << onetimestep << ")" << endl;
#endif
    AbsCommManager *command=new GracePeriodCommManager(currentInterface);
    AbsSyncAlgorithm *algo=new ConservativeSleepingTickAlgo(command,numofpowersims);
    instance=new Integrator(command,algo,simTimeStep,packetLostPeriod,onetimestep);
    instance->offset=convertToFrameworkTime(instance->simTimeMetric,initialTime);
}

void Integrator::initIntegratorConservativeSleepingComm(
	  AbsNetworkInterface* currentInterface,
	  time_metric simTimeStep,
	  TIME packetLostPeriod,
	  TIME initialTime,
	  TIME onetimestep)
{
#if DEBUG
    CERR << "Integrator::initIntegratorConservativeSleepingComm("
        << "AbsCommInterface*,"
        << "simTimeStep=" << simTimeStep << ","
        << "packetlost=" << packetLostPeriod << ","
        << "initialTime=" << initialTime << ","
	<< "onetimestep=" << onetimestep << ")" << endl;
#endif
    AbsCommManager *command=new CommunicationComManager(currentInterface);
    AbsSyncAlgorithm *algo=new ConservativeSleepingCommAlgo(command);
    instance=new Integrator(command,algo,simTimeStep,packetLostPeriod,onetimestep);
    instance->offset=convertToFrameworkTime(instance->simTimeMetric,initialTime);
}


void Integrator::initIntegratorGracePeriod(
  AbsNetworkInterface* currentInterface,
  time_metric simTimeStep, 
  TIME packetLostPeriod, 
  TIME initialTime,
  TIME onetimestep)
{
#if DEBUG
    CERR << "Integrator::initIntegratorGracePeriod("
        << "AbsCommInterface*,"
        << "simTimeStep=" << simTimeStep << ","
        << "packetlost=" << packetLostPeriod << ","
        << "initialTime=" << initialTime << 
        << "onetimestep=" << onetimestep <<")" << endl;
#endif
    AbsCommManager *command=new GracePeriodCommManager(currentInterface);
    AbsSyncAlgorithm *algo=new GracePeriodSyncAlgo(command);
    instance=new Integrator(command,algo,simTimeStep,packetLostPeriod,onetimestep);
    instance->offset=convertToFrameworkTime(instance->simTimeMetric,initialTime);
}

void Integrator::initIntegratorOptimistic(
	AbsNetworkInterface* currentInterface,
	time_metric simTimeStep,
	TIME packetLostPeriod,
	TIME initialTime, 
	TIME specDifference,
	SpeculationTimeCalculationStrategy *strategy,
	TIME onetimestep){

#if DEBUG
    CERR << "Integrator::initIntegratorOptimistic("
        << "AbsCommInterface*,"
        << "simTimeStep=" << simTimeStep << ","
        << "packetlost=" << packetLostPeriod << ","
        << "initialTime=" << initialTime << "," 
	<< "specDifference=" << specDifference << ","
	<< "strategy=" << strategy << ","
	<< "onetimestep=" << onetimestep << ")" << endl;
#endif
    AbsCommManager *command=new GracePeriodCommManager(currentInterface);
    TIME specDifferentFramework=convertToFrameworkTime(simTimeStep,specDifference);
    AbsSyncAlgorithm *algo=new OptimisticTickSyncAlgo(command,strategy);
    instance=new Integrator(command,algo,simTimeStep,packetLostPeriod,onetimestep);
    instance->offset=convertToFrameworkTime(instance->simTimeMetric,initialTime);
}

void Integrator::initIntegratorOptimisticLowOverhead(
	AbsNetworkInterface* currentInterface,
	time_metric simTimeStep,
	TIME packetLostPeriod,
	TIME initialTime, 
	TIME specDifference,
	SpeculationTimeCalculationStrategy *strategy,
	TIME onetimestep){

#if DEBUG
    CERR << "Integrator::initIntegratorOptimisticLowOverhead("
        << "AbsCommInterface*,"
        << "simTimeStep=" << simTimeStep << ","
        << "packetlost=" << packetLostPeriod << ","
        << "initialTime=" << initialTime << ","
	<< "specDifference=" << specDifference << "," <<
	<< "strategy=" << strategy << ","
	<< "onetimestep=" << onetimestep << ")" << endl;
#endif
    AbsCommManager *command=new GracePeriodCommManager(currentInterface);
    TIME specDifferentFramework=convertToFrameworkTime(simTimeStep,specDifference);
    AbsSyncAlgorithm *algo=new OptimisticLowOverheadTickSyncAlgo(command,specDifferentFramework,strategy);
    instance=new Integrator(command,algo,simTimeStep,packetLostPeriod,onetimestep);
    instance->offset=convertToFrameworkTime(instance->simTimeMetric,initialTime);
}

void Integrator::initIntegratorOptimisticComm(
	    AbsNetworkInterface* currentInterface, 
	    time_metric simTimeStep, 
	    TIME packetLostPeriod, 
	    TIME initialTime, 
	    TIME specDifference,
	    SpeculationTimeCalculationStrategy *strategy,
	    TIME onetimestep){
#if DEBUG
    CERR << "Integrator::initIntegratorOptimisticComm("
        << "AbsCommInterface*,"
        << "simTimeStep=" << simTimeStep << ","
        << "packetlost=" << packetLostPeriod << ","
        << "initialTime=" << initialTime << ","
	<< "specDifference=" << specDifference << "," <<
	<< "strategy=" << strategy << ","
	<< "onetimestep=" << onetimestep << ")" << endl;
#endif
    AbsCommManager *command=new CommunicationComManager(currentInterface);
    TIME specDifferentFramework=convertToFrameworkTime(simTimeStep,specDifference);
    AbsSyncAlgorithm *algo=new OptimisticCommSyncAlgo(command,strategy);
    instance=new Integrator(command,algo,simTimeStep,packetLostPeriod,onetimestep);
    instance->offset=convertToFrameworkTime(instance->simTimeMetric,initialTime);
}


void Integrator::initIntegratorOptimisticCommLowOverhead(
	    AbsNetworkInterface* currentInterface, 
	    time_metric simTimeStep, 
	    TIME packetLostPeriod, 
	    TIME initialTime, 
	    TIME specDifference,
	    SpeculationTimeCalculationStrategy *strategy,
	    TIME onetimestep){
#if DEBUG
    CERR << "Integrator::initIntegratorOptimisticCommLowOverhead("
        << "AbsCommInterface*,"
        << "simTimeStep=" << simTimeStep << ","
        << "packetlost=" << packetLostPeriod << ","
        << "initialTime=" << initialTime << ","
	<< "specDifference=" << specDifference << "," <<
	<< "strategy=" << strategy << ","
	<< "onetimestep=" << onetimestep << ")" << endl;
#endif
    AbsCommManager *command=new CommunicationComManager(currentInterface);
    TIME specDifferentFramework=convertToFrameworkTime(simTimeStep,specDifference);
    AbsSyncAlgorithm *algo=new OptimisticLowOverheadCommSyncAlgo(command,specDifferentFramework,strategy);
    instance=new Integrator(command,algo,simTimeStep,packetLostPeriod,onetimestep);
    instance->offset=convertToFrameworkTime(instance->simTimeMetric,initialTime);
}


void Integrator::initIntegratorCommunicationSim(
        AbsNetworkInterface *currentInterface,
        time_metric simTimeStep,
	TIME packetLostPeriod,
	TIME initialTime,
	TIME onetimestep)
{
#if DEBUG
    CERR << "Integrator::initIntegratorCommunicationSim("
        << "AbsCommInterface*,"
        << "simTimeStep=" << simTimeStep << ","
        << "packetLost=" << packetLostPeriod << ","
        << "initialTime=" << initialTime << ","
        << "onetimestep=" << onetimestep << ")" << endl;
#endif
    AbsCommManager *command=new CommunicationComManager(currentInterface);
    AbsSyncAlgorithm *algo=new CommunicatorSimulatorSyncalgo(command);
    instance=new Integrator(command,algo,simTimeStep,packetLostPeriod,onetimestep);
    instance->offset=convertToFrameworkTime(instance->simTimeMetric,initialTime);
}


void Integrator::setCommManager(AbsCommManager* given)
{
    instance->currentInterface=given;
}



void Integrator::setTimeCallBack(CallBack<TIME,empty,empty,empty,empty>* t) {
#if DEBUG
    CERR << "Integrator::setTimeCallBack(t*)" << endl;
#endif
    instance->getTimeCallBack=t;
}


void Integrator::grantTimeCompleted(TIME currentTime)
{
   TIME curTimeInFramework=convertToFrameworkTime(
            instance->simTimeMetric,currentTime) - instance->offset;
    instance->syncAlgo->timeStepStart(curTimeInFramework);
}

void Integrator::timeStepStart(TIME currentTime)
{
     TIME curTimeInFramework=convertToFrameworkTime(
            instance->simTimeMetric,currentTime) - instance->offset;
    instance->syncAlgo->timeStepStart(curTimeInFramework);
}




TIME Integrator::getCurSimTime() {

    TIME t=(*(instance->getTimeCallBack))();

    TIME curTime=convertToFrameworkTime(instance->simTimeMetric,t);
    
    return curTime-instance->offset;
}


bool Integrator::isFinished() {
    return instance->syncAlgo->isFinished();
}


time_metric Integrator::getCurSimMetric() {

    return instance->simTimeMetric;
}


ObjectCommInterface* Integrator::getCommInterface(string objectName) {

    ObjectCommInterface *toReturn = nullptr;

    if (instance->allowRegistrations) {
        toReturn = new ObjectCommInterface(objectName);
        instance->currentInterface->addObjectInterface(objectName,toReturn);
    }
    else {
        toReturn = instance->currentInterface->getObjectInterface(objectName);
    }

    return toReturn;
}

ObjectCommInterface* Integrator::getCommInterface(string objectName, BufferStrategy* st)
{
    ObjectCommInterface *toReturn = nullptr;

    if (instance->allowRegistrations) {
        toReturn = new ObjectCommInterface(objectName,st);
        instance->currentInterface->addObjectInterface(objectName,toReturn);
    }
    else {
        toReturn = instance->currentInterface->getObjectInterface(objectName);
    }

    return toReturn;
}

ObjectCommInterface* Integrator::getCommInterface(const char* objectName, BufferStrategy* st)
{
  return Integrator::getCommInterface(string(objectName),st);
}

ObjectCommInterface* Integrator::getCommInterface(string objectName, BufferStrategyType buffType)
{
  switch(buffType){
    case BUFFER_ALL:{
      return getCommInterface(objectName);
    }
    case BUFFER_FIRST:{
      BufferStrategy *st=new KeepFirstStrategy();
      return getCommInterface(objectName,st);
    }
    case BUFFER_LAST:{
      BufferStrategy *st=new KeepLastStrategy();
      return getCommInterface(objectName,st);
    }
    default:
      throw FNCSException("Unknown buffer strategy type");  
  }
}

ObjectCommInterface* Integrator::getCommInterface(char* objectName, BufferStrategyType buffType)
{
  return getCommInterface(string(objectName),buffType);
}



ObjectCommInterface* Integrator::getCommInterface(const char* objectName) {

    return Integrator::getCommInterface(string(objectName));
}



TIME Integrator::getNextTime(TIME currentTime, TIME nextTime) {
    
    TIME curTimeInFramework=convertToFrameworkTime(
            instance->simTimeMetric,currentTime) - instance->offset;
    TIME nextframeTime = convertToFrameworkTime(
            instance->simTimeMetric,nextTime) - instance->offset;
    instance->currentInterface->sendAll();

    TIME toReturn=instance->syncAlgo->GetNextTime(curTimeInFramework,nextframeTime);
    return convertToMyTime(instance->simTimeMetric,toReturn+instance->offset);
}


void Integrator::finalizeRegistrations() {

    instance->currentInterface->finalizeRegistrations();
    instance->allowRegistrations=false;
    instance->currentInterface->startReceiver();
}

void Integrator::setOffset(TIME otime)
{
 instance->offset=convertToFrameworkTime(instance->simTimeMetric,otime);
}

TIME Integrator::getOffset()
{
  return instance->offset;
}

TIME Integrator::getAdjustedOffset()
{
  return convertToMyTime(instance->simTimeMetric,instance->offset);
}

bool Integrator::isChild()
{
  return instance->syncAlgo->forkedNewChild();
}

bool Integrator::canFork()
{
  return instance->currentInterface->supportsFork() && instance->syncAlgo->usesFork();
}



} /* end namespace sim_comm */
