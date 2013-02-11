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

#include "integrator.h"
#include "abscomminterface.h"

namespace sim_comm{

Integrator* Integrator::instance=NULL;
  
Integrator::Integrator(AbsCommInterface *currentInterface,time_metric simTimeStep,int numberOfCommNodes, TIME gracePeriod)
{
  this->currentInterface=currentInterface;
  this->simTimeMetric=simTimeStep;
  this->numberOfCommNodes=numberOfCommNodes;
  this->gracePreiod=gracePeriod;
}

void Integrator::initIntegrator(AbsCommInterface *currentInterface, time_metric simTimeStep, int numberOfCommNodes, TIME gracePeriod)
{
  instance=new Integrator(currentInterface,simTimeStep,numberOfCommNodes,gracePeriod);
}


TIME Integrator::getGracePreiod()
{
  return instance->gracePreiod;
}

void Integrator::setTimeCallBack(CallBack<TIME,empty,empty,empty>* t)
{
  instance->getTimeCallBack=t;
}


TIME Integrator::getAdjustedGracePeriod()
{
  return convertToMyTime(instance->simTimeMetric,instance->gracePreiod);
}

TIME Integrator::getCurSimeTime()
{
  TIME t=(*(instance->getTimeCallBack))();
  
  return convertToFrameworkTime(instance->simTimeMetric,t);
}

  
time_metric Integrator::getCurSimMetric()
{
  return instance->simTimeMetric;
}

ObjectCommInterface* Integrator::getCommInterface(string objectName)
{
  ObjectCommInterface *toReturn=new ObjectCommInterface(objectName);
  instance->currentInterface->addObjectInterface(objectName,toReturn);
  
  return toReturn;
}

int Integrator::getRank()
{
  return instance->currentInterface->getMyRank();
}

ObjectCommInterface* Integrator::getCommInterface(char* objectName)
{

}

int Integrator::getNumberOfCommNodes()
{
  return instance->numberOfCommNodes;
}

}
