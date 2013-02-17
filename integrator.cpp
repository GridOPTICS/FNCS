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

#include "abscomminterface.h"
#include "integrator.h"
#include "objectcomminterface.h"
#include "graceperiodpesimisticsyncalgo.h"

namespace sim_comm {

Integrator* Integrator::instance=NULL;

Integrator::Integrator(
        AbsCommInterface *currentInterface,
        AbsSyncAlgorithm *algo,
        time_metric simTimeStep,
        TIME gracePeriod) {
    this->currentInterface=currentInterface;
    this->simTimeMetric=simTimeStep;
    this->gracePreiod=gracePeriod;
    this->allowRegistrations = true;
    this->syncAlgo=algo;
}

Integrator::~Integrator(){
	this->currentInterface->stopReceiver();
	delete currentInterface;
	delete syncAlgo;
	Integrator::instance=NULL;

}

void Integrator::stopIntegrator(){
	//we need a way signal simulators that this sim has finished.
	delete Integrator::instance;
}

void Integrator::initIntegratorGracePeriod(AbsCommInterface *currentInterface, time_metric simTimeStep, TIME gracePeriod) {
    AbsSyncAlgorithm *algo=new GracePeriodSyncAlgo(currentInterface);
	instance=new Integrator(currentInterface,algo,simTimeStep,gracePeriod);
}


TIME Integrator::getGracePreiod() {
    return instance->gracePreiod;
}

void Integrator::setTimeCallBack(CallBack<TIME,empty,empty,empty>* t) {
    instance->getTimeCallBack=t;
}


TIME Integrator::getAdjustedGracePeriod() {
    return convertToMyTime(instance->simTimeMetric,instance->gracePreiod);
}

TIME Integrator::getCurSimTime() {
    TIME t=(*(instance->getTimeCallBack))();

    return convertToFrameworkTime(instance->simTimeMetric,t);
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
        throw ObjectInterfaceRegistrationException();
    }

    return toReturn;
}

bool Integrator::doDispatchNextEvent(TIME currentTime, TIME nextTime) {
    instance->currentInterface->sendAll();
    return instance->syncAlgo->doDispatchNextEvent(currentTime,nextTime);
}

TIME Integrator::getNextTime(TIME currentTime, TIME nextTime) {
    instance->currentInterface->sendAll();
    return instance->syncAlgo->GetNextTime(currentTime,nextTime);
}

void Integrator::finalizeRegistrations()
{
  instance->currentInterface->finalizeRegistrations();
  instance->allowRegistrations=false;
}


}
