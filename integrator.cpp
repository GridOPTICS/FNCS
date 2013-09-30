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
#include <cstdlib>

#include "config.h"

#include "abscommmanager.h"
#include "absnetworkinterface.h"
#include "communicationcommanager.h"
#include "graceperiodcommmanager.h"
#include "conservativesleepingcommalgo.h"
#include "conservativesleepingtickalgo.h"
#include "integrator.h"
#include "objectcomminterface.h"
#include "optimisticcommsyncalgo.h"
#include "optimisticticksyncalgo.h"
#include "syncalgorithms/communicatorsimulatorsyncalgo.h"
#include "syncalgorithms/graceperiodpesimisticsyncalgo.h"
#include "syncalgorithms/graceperiodspeculativesyncalgo.h"

//Chaomei added

#include "callback.h"
#include "simtime.h"
#include "zmqnetworkinterface.h"
//#include "mpinetworkinterface.h"
#include "speculationtimecalculationstrategy.h"
#include "include/json.h"

using namespace std;

#ifdef DEBUG
Echo echo;
#endif

namespace sim_comm {


Integrator* Integrator::instance=NULL;


Integrator::Integrator(
        AbsCommManager *currentInterface,
        AbsSyncAlgorithm *algo,
        time_metric simTimeStep,
	TIME packetLostPeriod) {
#if DEBUG
    CERR << "Integrator::Integrator("
        << "AbsCommInterface*,"
        << "AbsSyncAlgorithm*,"
        << "simTimeStep=" << simTimeStep << ","
        << "packetlost=" << packetLostPeriod << ")" << endl;
#endif
    this->currentInterface=currentInterface;
    this->simTimeMetric=simTimeStep;
    this->allowRegistrations = true;
    this->syncAlgo=algo;
    this->packetLostPeriod=packetLostPeriod;
   
}


Integrator::~Integrator(){
	this->currentInterface->stopReceiver();
	delete currentInterface;
	delete syncAlgo;
	instance=NULL;
}

TIME Integrator::getMinNetworkDelay()
{
  return instance->currentInterface->getMinNetworkDelay();
}


void Integrator::stopIntegrator(){
#if DEBUG
    CERR << "Integrator::stopIntegrator()" << endl;
#endif
	if(!isFinished()) {
#if DEBUG
	    CERR << "Signaling finish to other sims" << endl;
#endif
	    //instance->syncAlgo->GetNextTime(instance->getCurSimTime(),Infinity);
        instance->currentInterface->sendFinishedSignal();
	delete instance;
	instance=NULL;
    }
	
}

void Integrator::terminate()
{
#if DEBUG
    CERR << "Integrator::terminate()" << endl;
#endif
    if(instance!=NULL){
      if(!isFinished())
	instance->currentInterface->sendFinishedSignal();
      delete instance;
    }
}


TIME Integrator::getPacketLostPeriod()
{
  return instance->packetLostPeriod;
}

/**
 * Chaomei added
 *
 */
void Integrator::parseConfig(
  string jsonFile, TIME currentTime)
{

  TIME plp;
  TIME specTime;
  enum time_metric tm;
  ZmqNetworkInterface *comm;
 
  std::cout << "Json fle name : " << jsonFile << endl;
  Json::Value root;
  std::ifstream file(jsonFile.c_str());
  file >> root;
  std::cout << root;

  const Json::Value interface_ = root["interface"];
  const Json::Value simulator_type = root["simulator_type"];
  const Json::Value synchronization_algorithm = root["synchronization_algorithm"];
  const Json::Value simulator_time_metric = root["simulator_time_metric"];
  const Json::Value packet_loss_period = root["packet_loss_period"];
    
  if (interface_.isNull())
  {
    cout << "Error: No interface (either mpi or zmq)  in Json file" << endl;
    exit (1);
  }
  else
  {
    if (interface_.asString() == "zmq") 
    {
      if (simulator_type.asString() == "power_grid") { 
        comm = new ZmqNetworkInterface(false);
        cout << "interface is ZMQ with a power grid simulator" << endl;
      }
      else if (simulator_type.asString() == "communication_network") { 
        comm = new ZmqNetworkInterface(true);
        cout << "interface is ZMQ, with a network simulator" << endl;
      }
    }
    else if (interface_.asString() == "mpi") 
      //MpiNetworkInterface *comm = new MpiNetworkInterface(MPI_COMM_WORLD, true);
      cout << "interface is MPI" << endl;
  }

  if (packet_loss_period.isNull()) 
    plp = 5000000000;
  else
    plp = (TIME) packet_loss_period.asDouble();

  #if DEBUG
    CERR << "packet loss period = " << plp << endl;
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
        Integrator::initIntegratorGracePeriod(comm, tm, plp, currentTime);
        cout << "*** call initIntegratorGracePeriod" << endl;
      } 
      else if(synchronization_algorithm.asString().compare("active_set_conservative") == 0)  
      {
        time_metric others[1];
        others[0] = SECONDS;

        Integrator::initIntegratorConservativeSleepingTick(comm, tm, plp, currentTime, others, 1);  
        cout << "*** call initIntegratorConservativeSleepingTick" << endl;
    
      }
    }
    else
    {
      if (!synchronization_algorithm["optimistic"].isNull())
      {
        if (synchronization_algorithm["optimistic"]["spec_time"].isNull())
          specTime = 30000000000;
        else
          specTime = (TIME) synchronization_algorithm["optimistic"]["spec_time"].asDouble(); 

        #if DEBUG
          CERR << "specTime = " << specTime << endl;
        #endif

        if(synchronization_algorithm["optimistic"]["speculation_calculation_stragegy"].asString() == "constant") 
        {
        
          ConstantSpeculationTimeStrategy *st = new ConstantSpeculationTimeStrategy(tm, specTime);
          Integrator::initIntegratorOptimistic(comm, tm, plp,currentTime,specTime,st);
          cout << "*** call constant speculartion time stregy" << endl;  

        }
        else if (synchronization_algorithm["optimistic"]["speculation_calculation_stragegy"].asString() == "dynamic_increasing")
        {
          IncreasingSpeculationTimeStrategy *st=new IncreasingSpeculationTimeStrategy(tm, specTime);
          Integrator::initIntegratorOptimistic(comm, tm, plp, currentTime, specTime, st);
          cout << "*** call dynamical increasing and initIntegratorOptimistic" << endl;

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
        Integrator::initIntegratorCommunicationSim(comm, tm, plp, currentTime);
        cout << "*** call initIntegratorCommunicationSim" << endl;
      }
      else if(synchronization_algorithm.asString().compare("active_set_conservative") == 0)
      {

        Integrator::initIntegratorConservativeSleepingComm(comm, tm, plp, currentTime);
        cout << "*** call initIntegratorConservativeSleepingComm" << endl; 
   
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
          Integrator::initIntegratorOptimisticComm(comm, tm, plp, currentTime, specTime, st);
          cout << "*** call constant increasing and initIntegratorOptimisticComm " << endl;

        }
        else if (synchronization_algorithm["optimistic"]["speculation_calculation_stragegy"].asString() == "dynamic_increasing")
        {
          IncreasingSpeculationTimeStrategy *st=new IncreasingSpeculationTimeStrategy(tm,specTime);
          Integrator::initIntegratorOptimisticComm(comm, tm, plp, currentTime, specTime, st);
          cout << "*** call dynamic increasing and initIntegratorOptimisticComm" << endl;

        }
      }//end of if algorithm is optimistic
    }//end of else

  }//end of simulator type is comm network

}

void Integrator::initIntegrator(
  string jsonFile,
  TIME currentTime)
{
   parseConfig(jsonFile, currentTime);
}

void Integrator::initIntegratorConservativeSleepingTick(
  AbsNetworkInterface* currentInterface, 
  time_metric simTimeStep, 
  TIME packetLostPeriod, 
  TIME initialTime,
  time_metric connectedSimsMetric[],
  int metricsSize)
{

#if DEBUG
    CERR << "Integrator::initIntegratorNetworkDelaySupport("
        << "AbsCommInterface*,"
        << "simTimeStep=" << simTimeStep << ","
        << "packetlost=" << packetLostPeriod << ","
        << "initialTime=" << initialTime << ")" << endl;
#endif
    AbsCommManager *command=new GracePeriodCommManager(currentInterface);
    AbsSyncAlgorithm *algo=new ConservativeSleepingTickAlgo(command,connectedSimsMetric,metricsSize);
    instance=new Integrator(command,algo,simTimeStep,packetLostPeriod);
    instance->offset=convertToFrameworkTime(instance->simTimeMetric,initialTime);
}

void Integrator::initIntegratorConservativeSleepingComm(
	  AbsNetworkInterface* currentInterface,
	  time_metric simTimeStep,
	  TIME packetLostPeriod,
	  TIME initialTime)
{
#if DEBUG
    CERR << "Integrator::initIntegratorNetworkDelaySupport("
        << "AbsCommInterface*,"
        << "simTimeStep=" << simTimeStep << ","
        << "packetlost=" << packetLostPeriod << ","
        << "initialTime=" << initialTime << ")" << endl;
#endif
    AbsCommManager *command=new CommunicationComManager(currentInterface);
    AbsSyncAlgorithm *algo=new ConservativeSleepingCommAlgo(command);
    instance=new Integrator(command,algo,simTimeStep,packetLostPeriod);
    instance->offset=convertToFrameworkTime(instance->simTimeMetric,initialTime);
}


void Integrator::initIntegratorGracePeriod(
  AbsNetworkInterface* currentInterface,
  time_metric simTimeStep, 
  TIME packetLostPeriod, 
  TIME initialTime)
{

#if DEBUG
    CERR << "Integrator::initIntegratorGracePeriod("
        << "AbsCommInterface*,"
        << "simTimeStep=" << simTimeStep << ","
        << "packetlost=" << packetLostPeriod << ","
        << "initialTime=" << initialTime << ")" << endl;
#endif
    AbsCommManager *command=new GracePeriodCommManager(currentInterface);
    AbsSyncAlgorithm *algo=new GracePeriodSyncAlgo(command);
    instance=new Integrator(command,algo,simTimeStep,packetLostPeriod);
    instance->offset=convertToFrameworkTime(instance->simTimeMetric,initialTime);
}

void Integrator::initIntegratorOptimistic(
	AbsNetworkInterface* currentInterface,
	time_metric simTimeStep,
	TIME packetLostPeriod,
	TIME initialTime, 
	TIME specDifference,
	SpeculationTimeCalculationStrategy *strategy){
#if DEBUG
    CERR << "Integrator::initIntegratorOptimistic("
        << "AbsCommInterface*,"
        << "simTimeStep=" << simTimeStep << ","
        << "packetlost=" << packetLostPeriod << ","
        << "initialTime=" << specDifference << ")" << endl;
#endif
    AbsCommManager *command=new GracePeriodCommManager(currentInterface);
    TIME specDifferentFramework=convertToFrameworkTime(simTimeStep,specDifference);
//Chaomei
#ifndef _WIN32
	AbsSyncAlgorithm *algo=new OptimisticTickSyncAlgo(command,specDifferentFramework,strategy);
    instance=new Integrator(command,algo,simTimeStep,packetLostPeriod);
#endif

    instance->offset=convertToFrameworkTime(instance->simTimeMetric,initialTime);
}

void Integrator::initIntegratorOptimisticComm(
	    AbsNetworkInterface* currentInterface, 
	    time_metric simTimeStep, 
	    TIME packetLostPeriod, 
	    TIME initialTime, 
	    TIME specDifference,
	    SpeculationTimeCalculationStrategy *strategy){
#if DEBUG
    CERR << "Integrator::initIntegratorOptimisticComm("
        << "AbsCommInterface*,"
        << "simTimeStep=" << simTimeStep << ","
        << "packetlost=" << packetLostPeriod << ","
        << "initialTime=" << specDifference << ")" << endl;
#endif
    AbsCommManager *command=new CommunicationComManager(currentInterface);
    TIME specDifferentFramework=convertToFrameworkTime(simTimeStep,specDifference);
	//Chaomei
#ifndef _WIN32
    AbsSyncAlgorithm *algo=new OptimisticCommSyncAlgo(command,specDifferentFramework,strategy);
    instance=new Integrator(command,algo,simTimeStep,packetLostPeriod);
#endif
    instance->offset=convertToFrameworkTime(instance->simTimeMetric,initialTime);
}


void Integrator::initIntegratorCommunicationSim(
        AbsNetworkInterface *currentInterface,
        time_metric simTimeStep,
	TIME packetLostPeriod,
	TIME initialTime)
{
#if DEBUG
    CERR << "Integrator::initIntegratorCommunicationSim("
        << "AbsCommInterface*,"
        << "simTimeStep=" << simTimeStep << ","
        << "packetLost=" << packetLostPeriod << ","
        << "initialTime=" << initialTime << ")" << endl;
#endif
    AbsCommManager *command=new CommunicationComManager(currentInterface);
    AbsSyncAlgorithm *algo=new CommunicatorSimulatorSyncalgo(command);
    instance=new Integrator(command,algo,simTimeStep,packetLostPeriod);
    instance->offset=convertToFrameworkTime(instance->simTimeMetric,initialTime);
}


void Integrator::setCommManager(AbsCommManager* given)
{
    instance->currentInterface=given;
}



void Integrator::setTimeCallBack(CallBack<TIME,empty,empty,empty>* t) {
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


ObjectCommInterface* Integrator::getCommInterface(const char* objectName) {

    return Integrator::getCommInterface(string(objectName));
}


bool Integrator::doDispatchNextEvent(TIME currentTime, TIME nextTime) {

    TIME curTimeInFramework=convertToFrameworkTime(
            instance->simTimeMetric,currentTime) - instance->offset;
    TIME nextframeTime = convertToFrameworkTime(
            instance->simTimeMetric,nextTime) - instance->offset;
    instance->currentInterface->sendAll();
    return instance->syncAlgo->doDispatchNextEvent(
            curTimeInFramework,nextframeTime);
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

void Integrator::childDied(TIME dieTime)
{

  instance->syncAlgo->childDied(dieTime);
}

bool Integrator::canFork()
{
  return instance->currentInterface->supportsFork() && instance->syncAlgo->usesFork();
}


} /* end namespace sim_comm */
