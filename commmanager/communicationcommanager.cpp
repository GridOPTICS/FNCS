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

#include "communicationcommanager.h"
#include "objectcomminterface.h"
#include "callback.h"

namespace sim_comm{
  
  
    CommunicationComManager::CommunicationComManager(AbsNetworkInterface *interface) : AbsCommManager(interface)
    {
      CallBack<void,Message*,empty,empty> *msgCallback=CreateObjCallback<CommunicationComManager *, void (CommunicationComManager::*)(Message*), void,Message*>(this, &CommunicationComManager::messageReceived);
  
 
      this->currentInterface->setMessageCallBack(msgCallback);
    }
    
    CommunicationComManager::CommunicationComManager(CommunicationComManager& given): AbsCommManager(given)
    {

    }

    AbsCommManager* CommunicationComManager::duplicate()
    {
      return new CommunicationComManager(*this);
    }

    CommunicationComManager::~CommunicationComManager()
    {

    }
    
    void CommunicationComManager::packetLostCalculator(TIME currentTime)
    {
      TIME leastTime = currentTime - Integrator::getPacketLostPeriod();
      if(leastTime > currentTime) //overflowed
	  leastTime=0;
      map<TIME,uint32_t>::iterator it=packets.begin();
      vector<TIME> toremove;
      for(;it!=packets.end();++it){
	if(it->first < leastTime){
	    
	  this->receiveCount+=it->second;
#ifdef DEBUG
	  CERR << "LOST A PACKET currentTime " << currentTime << " packetTime " << it->first << endl; 
#endif
	  toremove.push_back(it->first);
	}
      }
      for(int i=0;i<toremove.size();i++){
      
	  this->packets.erase(toremove[i]);
      }
      
    }

    
    
    void CommunicationComManager::sendAll()
    {
      map<string,ObjectCommInterface*>::iterator it=this->interfaces.begin();

      for(; it!=this->interfaces.end(); it++) {

	  ObjectCommInterface *in=it->second;
	  //if(in->getInboxMessagesCount()>0) {

	      vector<Message*>  outmessges=in->getOutBox();
	      for(int i=0; i<outmessges.size(); i++) {
		  try {
		      outmessges[i]->setDeliveryTime(Integrator::getCurSimTime());
		      if (outmessges[i]->isBroadCast()) { //wiried????? 
			  throw CommManagerOperationNotSupportedException("Broadcast is not support on commnetwork commanager");
			  //int scount = this->currentInterface->broadcast(outmessges[i]);
			    // sendCount +=scount;
		      } 
		      else {
#if DEBUG
      CERR << "CommunicationComManager::sendAll(" << (char*)outmessges[i]->getData() << ") time:" << Integrator::getCurSimTime() << endl;
#endif
			    // sendCount += 1;
			  if(removeMessageTimeout(outmessges[i]->getTime()))
			    this->currentInterface->send(outmessges[i]);
			  
		      }
		  }
		  catch(InterfaceErrorException e) {

		      std::cerr << "Send operation failed on interface ";
		  }
	      }
	      in->clear();
	  //}
      }

     }

    void CommunicationComManager::messageReceived(Message* message)
    {

      //Get Time frame to accept the messageReceived
      TIME currentTime=Integrator::getCurSimTime();
      TIME graceTime=currentTime- Integrator::getPacketLostPeriod();
      if(graceTime>currentTime){ //overflowed
	graceTime=0;
      }
#if DEBUG
    CERR << "AbsCommManager::messageReceived(" << (char*)message->getData() << ") " << currentTime << endl;
#endif
      if(message->getTime()<graceTime){ //old message drop
#if DEBUG
	CERR << (char*)message->getData() << " is old dropping" << endl;
#endif
	  delete message;
	  return;
      }
#if DEBUG
      CERR << message->getTo() << endl;
#endif
      ObjectCommInterface *comm=getObjectInterface(message->getFrom());
      //handle bcast
      if(message->isBroadCast()){
	addMessageTimeout(message->getTime(),true);
	map<string,ObjectCommInterface*>::iterator it=this->interfaces.begin();
	for(;it!=interfaces.end();++it){
	  if(it->first.compare(message->getFrom())==0)
	    continue;
	  Message *nm=new Message(message->getFrom(),it->first,message->getTime(),
		  message->getData(),message->getSize());
	  comm->newMessage(nm);
	  //it->second->newMessage(nm);
	}
	delete message;
      }
      else{
      //handle single message
      //let it throw an exception if the key is not found
	  comm->newMessage(message);
	  addMessageTimeout(message->getTime());
      }
    
     
    }

  void CommunicationComManager::addMessageTimeout(TIME msgTime, bool isbroadcast)
  {
      map<TIME,uint32_t>::iterator it=this->packets.find(msgTime);
      if(it==packets.end()){
      
	packets.insert(pair<TIME,uint32_t>(msgTime,0));
      }
      
      uint32_t val=packets[msgTime];
      if(isbroadcast)
	val+=this->interfaces.size()-1; //-1 is for node itself
      else
	val+=1;
      packets[msgTime]=val;
      
  }

  bool CommunicationComManager::removeMessageTimeout(TIME msgTime)
  {
    if(packets.find(msgTime)==packets.end())
      return false; //message too old return false so it is not send!
    uint32_t val=packets[msgTime];
    if(val > 0)
      packets[msgTime]=val - 1;
    
    if(packets[msgTime]==0)
      packets.erase(msgTime);
    return true;
  }

}
