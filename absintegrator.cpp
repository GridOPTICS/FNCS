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

#include "absintegrator.h"
#include "../../../llvm-3.2.src/test/TableGen/SuperSubclassSameName.td"

namespace sim_comm{

AbsIntegrator* AbsIntegrator::instance=NULL;
  
AbsIntegrator::AbsIntegrator(int rank, time_metric simTimeStep, int numberOfCommNodes, TIME gracePeriod)
{
  this->rank=rank;
  this->simTimeMetric=simTimeStep;
  this->numberOfCommNodes=numberOfCommNodes;
  this->gracePreiod=gracePeriod;
}

void AbsIntegrator::initIntegrator(AbsCommInterface *currentInterface, time_metric simTimeStep, int numberOfCommNodes, TIME gracePeriod)
{
  instance=new AbsIntegrator(currentInterface,simTimeStep,numberOfCommNodes,gracePeriod);
}


TIME AbsIntegrator::getGracePreiod()
{
  return instance->gracePreiod;
}

void AbsIntegrator::setTimeCallBack(CallBack<TIME,empty,empty,empty>* t)
{
  this->getTimeCallBack=t;
}


TIME AbsIntegrator::getAdjustedGracePeriod()
{
  return convertToMyTime(instance->simTimeMetric,instance->gracePreiod);
}

TIME AbsIntegrator::getCurSimeTime()
{
  TIME t=(*(instance->getTimeCallBack))();
  
  return convertToFrameworkTime(instance->simTimeMetric,t);
}

  
time_metric AbsIntegrator::getCurSimMetric()
{
  return instance->simTimeMetric;
}

ObjectCommInterface* AbsIntegrator::getCommInterface(string objectName)
{
  thi 
}

int AbsIntegrator::getRank()
{
  return instance->currentInterface->getMyRank();
}

ObjectCommInterface* AbsIntegrator::getCommInterface(char* objectName)
{

}

int AbsIntegrator::getNumberOfCommNodes()
{
  return instance->numberOfCommNodes;
}

}
