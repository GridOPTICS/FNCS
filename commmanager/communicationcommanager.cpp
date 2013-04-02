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


#include "communicationcommanager.h"
#include "objectcomminterface.h"
#include "callback.h"

namespace sim_comm{
  
  
    CommunicationComManager::CommunicationComManager(AbsNetworkInterface *interface) : AbsCommManager(interface)
    {
      CallBack<void,Message*,empty,empty> *msgCallback=CreateObjCallback<CommunicationComManager *, void (CommunicationComManager::*)(Message*), void,Message*>(this, &CommunicationComManager::messageReceived);
  
 
      this->currentInterface->setMessageCallBack(msgCallback);
    }

    CommunicationComManager::~CommunicationComManager()
    {

    }
    
    void CommunicationComManager::packetLost()
    {
      //when packet is lost just increment the receive counter
      this->receiveCount++;
    }
    
    void CommunicationComManager::sendAll()
    {
#if DEBUG
      CERR << "AbsCommInterface::sendAll()" << endl;
#endif
      map<string,ObjectCommInterface*>::iterator it=this->interfaces.begin();

      for(; it!=this->interfaces.end(); it++) {

	  ObjectCommInterface *in=it->second;
	  //if(in->getInboxMessagesCount()>0) {

	      vector<Message*>  outmessges=in->getOutBox();
	      for(int i=0; i<outmessges.size(); i++) {
		  try {
		      outmessges[i]->setDeliveryTime(Integrator::getCurSimTime());
		      if (outmessges[i]->isBroadCast()) {
			  int scount = this->currentInterface->broadcast(outmessges[i]);
			    // sendCount +=scount;
		      } 
		      else {
			  
			    // sendCount += 1;
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
#if DEBUG
      CERR << "CommunicationComManager::messageReceived(Message*)" << endl;
#endif
      //Get Time frame to accept the messageReceived
      TIME currentTime=Integrator::getCurSimTime();
      TIME graceTime=currentTime- Integrator::getPacketLostPeriod();
      if(graceTime>currentTime){ //overflowed
	graceTime=0;
      }

      if(message->getTime()<graceTime){ //old message drop
	  delete message;
	  return;
      }
#if DEBUG
      CERR << message->getTo() << endl;
#endif
      ObjectCommInterface *comm=getObjectInterface(message->getFrom());
      //handle bcast
      if(message->isBroadCast()){
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
      }
    
     
    }

  
}
