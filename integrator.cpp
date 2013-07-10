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
#include "conservativesleepingcommalgo.h"
#include "conservativesleepingtickalgo.h"
#include "integrator.h"
#include "objectcomminterface.h"
#include "optimisticcommsyncalgo.h"
#include "optimisticticksyncalgo.h"
#include "syncalgorithms/communicatorsimulatorsyncalgo.h"
#include "syncalgorithms/graceperiodpesimisticsyncalgo.h"
#include "syncalgorithms/graceperiodspeculativesyncalgo.h"

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
    }
	delete instance;
}

void Integrator::terminate()
{
#if DEBUG
    CERR << "Integrator::terminate()" << endl;
#endif
    delete instance;
}


TIME Integrator::getPacketLostPeriod()
{
  return instance->packetLostPeriod;
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

void Integrator::initIntegratorSpeculative(
        AbsNetworkInterface *currentInterface,
        time_metric simTimeStep,
	TIME packetLostPeriod,
        TIME initialTime,
	TIME specDifference) {
 #if DEBUG
    CERR << "Integrator::initIntegratorSpeculative("
        << "AbsCommInterface*,"
        << "simTimeStep=" << simTimeStep << ","
        << "packetlost=" << packetLostPeriod << ","
        << "initialTime=" << specDifference << ")" << endl;
#endif
    AbsCommManager *command=new GracePeriodCommManager(currentInterface);
    TIME specDifferentFramework=convertToFrameworkTime(simTimeStep,specDifference);
    AbsSyncAlgorithm *algo=new GracePeriodSpeculativeSyncAlgo(command,specDifferentFramework);
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
    AbsSyncAlgorithm *algo=new OptimisticTickSyncAlgo(command,specDifferentFramework,strategy);
    instance=new Integrator(command,algo,simTimeStep,packetLostPeriod);
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
    AbsSyncAlgorithm *algo=new OptimisticCommSyncAlgo(command,specDifferentFramework,strategy);
    instance=new Integrator(command,algo,simTimeStep,packetLostPeriod);
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
#if DEBUG
    CERR << "Integrator::grantTimeCompleted(" << currentTime << ")" << endl;
#endif
   TIME curTimeInFramework=convertToFrameworkTime(
            instance->simTimeMetric,currentTime) - instance->offset;
    instance->syncAlgo->timeStepStart(curTimeInFramework);
}

void Integrator::timeStepStart(TIME currentTime)
{
#if DEBUG
    CERR << "Integrator::timeStepStart(" << currentTime << ")" << endl;
#endif
     TIME curTimeInFramework=convertToFrameworkTime(
            instance->simTimeMetric,currentTime) - instance->offset;
    instance->syncAlgo->timeStepStart(curTimeInFramework);
}




TIME Integrator::getCurSimTime() {
#if DEBUG
    CERR << "Integrator::getCurSimTime()" << endl;
#endif
    TIME t=(*(instance->getTimeCallBack))();

    TIME curTime=convertToFrameworkTime(instance->simTimeMetric,t);
    
    return curTime-instance->offset;
}


bool Integrator::isFinished() {
#if DEBUG
    CERR << "Integrator::isFinished()" << endl;
#endif
    return instance->syncAlgo->isFinished();
}


time_metric Integrator::getCurSimMetric() {
#if DEBUG
    CERR << "Integrator::getCurSimMetric()" << endl;
#endif
    return instance->simTimeMetric;
}


ObjectCommInterface* Integrator::getCommInterface(string objectName) {
#if DEBUG
    CERR << "Integrator::getCommInterface("
        << objectName << ")" << endl;
#endif
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
#if DEBUG
    CERR << "Integrator::getCommInterface("
        << objectName << ")" << endl;
#endif
    return Integrator::getCommInterface(string(objectName));
}


bool Integrator::doDispatchNextEvent(TIME currentTime, TIME nextTime) {
#if DEBUG
    CERR << "Integrator::doDispatchNextEvent("
        << "currentTime=" << currentTime << ","
        << "nextTime=" << nextTime << ")" << endl;
#endif
    TIME curTimeInFramework=convertToFrameworkTime(
            instance->simTimeMetric,currentTime) - instance->offset;
    TIME nextframeTime = convertToFrameworkTime(
            instance->simTimeMetric,nextTime) - instance->offset;
    instance->currentInterface->sendAll();
    return instance->syncAlgo->doDispatchNextEvent(
            curTimeInFramework,nextframeTime);
}


TIME Integrator::getNextTime(TIME currentTime, TIME nextTime) {
#if DEBUG
    CERR << "Integrator::getNextTime("
        << "currentTime=" << currentTime << ","
        << "nextTime=" << nextTime << ")" << endl;
#endif
    
    TIME curTimeInFramework=convertToFrameworkTime(
            instance->simTimeMetric,currentTime) - instance->offset;
    TIME nextframeTime = convertToFrameworkTime(
            instance->simTimeMetric,nextTime) - instance->offset;
    instance->currentInterface->sendAll();
#if DEBUG
    CERR << "Integrator::getNextTime("
        << "currentTime=" << curTimeInFramework << ","
        << "nextTime=" << nextframeTime << ")" << endl;
#endif
    TIME toReturn=instance->syncAlgo->GetNextTime(curTimeInFramework,nextframeTime);
    return convertToMyTime(instance->simTimeMetric,toReturn+instance->offset);
}


void Integrator::finalizeRegistrations() {
#if DEBUG
    CERR << "Integrator::finalizeRegistrations()" << endl;
#endif
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




} /* end namespace sim_comm */
