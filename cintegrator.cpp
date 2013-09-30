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

#include <cassert>

#include "cintegrator.h"
//Chaomei not sure !
//#if HAVE_ZMQ
#include "zmqnetworkinterface.h"
//#endif
#include "integrator.h"
#include "objectcomminterface.h"
#include "callback.h"
#include "speculationtimecalculationstrategy.h"

using namespace sim_comm;

std::string myName;
std::map<std::string,ObjectCommInterface *> registeredInterfaces;


void fenix_initialize(int *arc,char ***argv)
{
}

void fenix_finalize(){
}

void timeStepStart(TIME currentTime)
{
  Integrator::timeStepStart(currentTime);
}


uint8_t isFinished()
{
  return (uint8_t)Integrator::isFinished();
}

 void initIntegratorGracePeriod(enum time_metric simTimeStep, 
				TIME packetLostPeriod, TIME initialTime){
   //MpiNetworkInterface *comm = new MpiNetworkInterface(MPI_COMM_WORLD, false);
   ZmqNetworkInterface *comm = new ZmqNetworkInterface(false);
   Integrator::initIntegratorGracePeriod(comm,simTimeStep,packetLostPeriod,initialTime);
 }
 
void initIntegratorOptimisticConstant(time_metric simTimeStep,
				      TIME packetLostPeriod, TIME initialTime, TIME specTime)
{
  //MpiNetworkInterface *comm = new MpiNetworkInterface(MPI_COMM_WORLD, false);
  ZmqNetworkInterface *comm = new ZmqNetworkInterface(false);
  ConstantSpeculationTimeStrategy *st=new ConstantSpeculationTimeStrategy(simTimeStep,specTime);
  Integrator::initIntegratorOptimistic(comm,simTimeStep,packetLostPeriod,initialTime,specTime,st);	
}

void initIntegratorOptimisticIncreasing(time_metric simTimeStep, 
					TIME packetLostPeriod, TIME initialTime, TIME specTime)
{
    ZmqNetworkInterface *comm = new ZmqNetworkInterface(false);
    IncreasingSpeculationTimeStrategy *st=new IncreasingSpeculationTimeStrategy(simTimeStep,specTime);
    Integrator::initIntegratorOptimistic(comm,simTimeStep,packetLostPeriod,initialTime,specTime,st);
}


 void initIntegratorConservativeSleepingTick(enum time_metric simTimeStep, 
				TIME packetLostPeriod, TIME initialTime,enum time_metric metrics[],int metricsSize)
{
  //MpiNetworkInterface *comm = new MpiNetworkInterface(MPI_COMM_WORLD, false);
  ZmqNetworkInterface *comm = new ZmqNetworkInterface(false);
  Integrator::initIntegratorConservativeSleepingTick(comm,simTimeStep,packetLostPeriod,initialTime,metrics,metricsSize);
}

void registerObject(char* name)
{
  ObjectCommInterface *com=Integrator::getCommInterface(name);
  myName= std::string(name);
  registeredInterfaces.insert(pair<std::string,ObjectCommInterface*>(myName,com));
}

void sendMesg(char *from,char *destination,char* msg, int size,int networked,TIME currentTime)
{
  string fromstr(from);
  string tostr(destination);
  ObjectCommInterface *com=registeredInterfaces[fromstr];
  //msgobj->setDelayThroughComm(false);
  
  Message *msgobj=new Message(fromstr,tostr,currentTime,(uint8_t *)msg,size);
  bool networkedflag = networked > 0 ? true : false;
  msgobj->setDelayThroughComm(networkedflag);
  com->send(msgobj);
}

void receive(char *from,char** buff, int* size)
{
  string fromstr(from);
  
  ObjectCommInterface *com=registeredInterfaces[fromstr];
  
  if(com->hasMoreMessages()){ 
      Message *msg=com->getNextInboxMessage();
      char* buff2=new char[msg->getSize()];
      memcpy(buff2,msg->getData(),msg->getSize());
      *buff=buff2;
      *size=msg->getSize();
      delete msg;
  }
  else{
    *buff=NULL;
  }
}


void finalizeRegistrations()
{
  Integrator::finalizeRegistrations();
}

void setregistercallback(TIME (*callback)())
{
  CallBack<TIME,empty,empty,empty> *cb=CreateCallback(callback);
  Integrator::setTimeCallBack(cb);
}

void stopIntegrator()
{
  Integrator::stopIntegrator();
}

TIME getNextTime(TIME currentTime, TIME nextTime)
{
  return Integrator::getNextTime(currentTime,nextTime);
}

void setOffset(TIME initialTime)
{
  Integrator::setOffset(initialTime);
}
