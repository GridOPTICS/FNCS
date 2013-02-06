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


#include "abscomminterface.h"
#include "integrator.h"

namespace sim_comm{
AbsCommInterface::AbsCommInterface(uint32_t myRank)
{
  this->receiveCount=0;
  this->sendCount=0;
  this->receiverRunning=false;
  this->myRank=myRank;
}

bool AbsCommInterface::isReceiverRunning()
{
  return this->receiverRunning;
}

void AbsCommInterface::addObjectInterface(string objectName, ObjectCommInterface* given)
{
  if(given->getMyRank()==myRank)
    throw ObjectInterfaceRegistrationException();
  
  this->interfaces.insert(pair<string,ObjectCommInterface*>(objectName,given));
}

void AbsCommInterface::startReceiver()
{
  this->receiverRunning=true;
}

void AbsCommInterface::stopReceiver()
{
  this->receiverRunning=false;
}

void AbsCommInterface::sendAll()
{
  map<string,ObjectCommInterface*>::iterator it=this->interfaces.begin();
  
  for(;it!=this->interfaces.end();it++){
  
      ObjectCommInterface *in=it->second;
      if(in->getInboxMessagesCount()>0){
      
	vector<AbsMessage*>  outmessges=in->getOutBox();
	for(int i=0;i<outmessges.size();i++){
	  try{
	    this->realSendMessage(outmessges[i]);
	    if(outmessges[i]->isBroadCast()){
	    
	      sendCount+=Integrator::getNumberOfCommNodes();
	    }
	    else{
	    
	      sendCount+=1;
	    }
	  }
	  catch(InterfaceErrorException e){
	  
	    std::cerr << "Send operation failed on interface ";
	  }
	}
      }
  }
}

AbsCommInterface::~AbsCommInterface()
{
  this->interfaces.clear();
}



}
