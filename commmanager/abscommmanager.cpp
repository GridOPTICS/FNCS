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
/* autoconf header */
#include "config.h"

#include "abscommmanager.h"
#include "callback.h"
#include "objectcomminterface.h"

namespace sim_comm{
 
 
  AbsCommManager::AbsCommManager(AbsNetworkInterface *current){
   #if DEBUG
    CERR << "AbsCommManager::AbsCommManager()" << endl;
    #endif
    this->receiveCount=0;
    this->sendCount=0;
    this->receiverRunning=false;
    this->allowRegistrations=true;
    this->currentInterface=current;
    CallBack<void,Message*,empty,empty> *msgCallback=CreateObjCallback<AbsCommManager *, void (AbsCommManager::*)(Message*), void,Message*>(this, &AbsCommManager::messageReceived);
  
    this->syncAlgoCallBackRecv=NULL;
    this->syncAlgoCallBackSend=NULL;
    this->currentInterface->setMessageCallBack(msgCallback);
    this->minNetworkDelay=Infinity;
  }
  
  AbsCommManager::~AbsCommManager()
  {
    #if DEBUG
    CERR << "AbsCommManager::~AbsCommManager()" << endl;
  #endif
    map<string,ObjectCommInterface*>::iterator it=this->interfaces.begin();
    for(;it!=interfaces.end();++it) {
        delete it->second;
    }
    this->interfaces.clear();
    delete this->currentInterface;
  }
  
  void AbsCommManager::waitforAll()
  {
  #if DEBUG
    CERR << "AbsCommManager::waitforAll()" << endl;
  #endif
    this->currentInterface->barier();
  }


  void AbsCommManager::messageReceived(Message *message)
  {
#if DEBUG
    CERR << "AbsCommManager::messageReceived(Message*)" << endl;
#endif
    //Get Time frame to accept the messageReceived
    TIME currentTime=Integrator::getCurSimTime();
    TIME graceTime=currentTime- Integrator::getPacketLostPeriod();
    if(graceTime>currentTime){ //overflowed
	graceTime=0;
    }

    if(message->getTime()<graceTime){ //old message!!
        delete message;
	return;
    }

    if(message->isBroadCast()){
      throw InterfaceErrorException();
    }
    
    //let it throw an exception if the key is not found.
    ObjectCommInterface *comm=getObjectInterface(message->getTo());

   
        this->receiveCount++;
#if DEBUG
    CERR << "Message to " << message->getTo() << " in total " << this->receiveCount << endl;
#endif
    comm->newMessage(message);
    
     if(this->syncAlgoCallBackRecv){
	 bool val=(*(this->syncAlgoCallBackRecv))(message);
	 if(!val) //syncalgo signaled ignore message!
	   return;
     }
     this->adjustNetworkDelay(message->getDeliveryPeriod());
  }
  
  bool AbsCommManager::isReceiverRunning() {
#if DEBUG
    CERR << "AbsCommInterface::isReceiverRunning()" << endl;
#endif
    return this->receiverRunning;
  }


  void AbsCommManager::addObjectInterface(
	  string objectName,
	  ObjectCommInterface* given) {
  #if DEBUG
      CERR << "AbsCommManager::addObjectInterface("
	  << objectName << ","
	  << given << ")" << endl;
  #endif
      if (this->allowRegistrations) {
	  if (this->interfaces.count(objectName) != 0) {
	      /* enforce unique names and one-time registrations */
	      throw ObjectInterfaceRegistrationException(objectName,__FILE__,__LINE__);
	  }
	  this->interfaces.insert(pair<string,ObjectCommInterface*>(objectName,given));
	  this->currentInterface->registerObject(objectName);
      }
      else {
	  throw ObjectInterfaceRegistrationException(__FILE__,__LINE__);
      }
  }
  
  void AbsCommManager::startReceiver()
  {
	this->receiverRunning=true;
	
  }

  void AbsCommManager::stopReceiver()
  {
	this->receiverRunning=false;
  }

  
  void AbsCommManager::finalizeRegistrations() {
    #if DEBUG
    CERR << "AbsCommManager::finalizeRegistrations()" << endl;
    #endif
    this->allowRegistrations = false;
    this->currentInterface->finalizeRegistrations();
  }



  ObjectCommInterface* AbsCommManager::getObjectInterface(string objectName) {
  #if DEBUG
      CERR << "AbsCommManager::getObjectInterface("
	  << objectName << ")" << endl;
  #endif
      if (this->interfaces.count(objectName) == 0) {
	  /* enforce unique names and one-time registrations */
	  throw ObjectInterfaceRegistrationException(objectName,__FILE__,__LINE__);
      }
      return this->interfaces[objectName];
  }
  

  uint64_t AbsCommManager::reduceNetworkDelay()
  {
     #if DEBUG
      CERR << "AbsCommManager::reduceTotalSendReceive()" << endl;
    #endif
      return this->currentInterface->reduceMinTime(this->minNetworkDelay);
  }

  uint64_t AbsCommManager::reduceTotalSendReceive()
  {

    #if DEBUG
      CERR << "AbsCommManager::reduceTotalSendReceive()" << endl;
    #endif
      return this->currentInterface->reduceTotalSendReceive(this->sendCount,this->receiveCount);
  }
  
  
  uint64_t AbsCommManager::reduceMinTime(uint64_t currentTime)
  {
  #if DEBUG
      CERR << "AbsCommManager::reduceMinTime("
	  << currentTime << ")" << endl;
  #endif
      return this->currentInterface->reduceMinTime(currentTime);
  }


  void AbsCommManager::adjustNetworkDelay(TIME msgDeliveryTime)
  {
      if(msgDeliveryTime<this->minNetworkDelay)
	this->minNetworkDelay=msgDeliveryTime;
  }

}

