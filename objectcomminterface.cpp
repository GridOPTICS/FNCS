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

#include "objectcomminterface.h"


namespace sim_comm {


ObjectCommInterface::ObjectCommInterface(string objectName, BufferStrategy *st) {
#if DEBUG
    CERR << "ObjectCommInterface::ObjectCommInterface("
        << objectName << ")" << endl;
#endif
    this->attachedObjectName=objectName;
    this->notifyMessage=nullptr;
    this->syncAlgoCallBackSend=nullptr;
    
    this->st=nullptr;
    if(st!=nullptr){
      st->setCommInterface(this);
      this->st=st;
    }
}

void ObjectCommInterface::setSyncAlgoCallBack(sim_comm::CallBack< bool, Message*, empty, empty,empty >* syncAlgoCallBackSend)
{
#ifdef DEBUG
    CERR << "ObjectCommInterface::setSyncAlgoCallBack(" << syncAlgoCallBackSend << ")" << endl;
#endif
  this->syncAlgoCallBackSend=syncAlgoCallBackSend;
}


ObjectCommInterface::~ObjectCommInterface() {
#if DEBUG
    CERR << "ObjectCommInterface::~ObjectCommInterface()" << endl;
#endif
    this->inbox.clear();
    this->outbox.clear();
    if(st)
      delete st;
}


void ObjectCommInterface::send(Message* given){
#if DEBUG
    CERR << "ObjectCommInterface::send(Message*)" << endl;
#endif
    if(syncAlgoCallBackSend!=nullptr && 
      !(*(this->syncAlgoCallBackSend))(given)){
      return;
    }
    this->outbox.push_back(given);
}


/*int ObjectCommInterface::getInboxMessagesCount() {
#if DEBUG
    CERR << "ObjectCommInterface::getInboxMessagesCount()" << endl;
#endif
    //we can write a better one!

     TIME timeframe=Integrator::getCurSimTime();
     TIME timeframe1=timeframe-Integrator::getPacketLostPeriod();
      if(timeframe1>timeframe){ //overflowed!
	timeframe1=0;
      }
      
    int toReturn=0;
    for(int i=0; i<inbox.size(); i++) {

        Message *msg=inbox[i];
        if(msg->getTime()<=timeframe && msg->getTime()>=timeframe1) {
            toReturn++;
        }
    }

    return toReturn;
}*/


std::vector< Message* > ObjectCommInterface::getAllInboxMessages() {
#if DEBUG
    CERR << "ObjectCommInterface::getAllInboxMessages()" << endl;
#endif
    TIME currentTime=Integrator::getCurSimTime();
    TIME graceTime=currentTime- Integrator::getPacketLostPeriod();
    if(graceTime>currentTime){ //overflowed
      graceTime=0;
    }
    vector<Message *> toReturn;

    vector<int> locs;

    for(int i=0; i<inbox.size(); i++) {

        Message *msg=inbox[i];
        if(msg->getTime()<=currentTime && msg->getTime()>=graceTime) {
            locs.push_back(i);
            toReturn.push_back(msg);
        }
    }

    for(int i=0; i<locs.size(); i++) {

        inbox.erase(inbox.begin()+locs[i]);
    }

    return toReturn;
}


bool ObjectCommInterface::hasMoreMessages() {
#if DEBUG
    CERR << "ObjectCommInterface::hasMoreMessages()" << endl;
#endif
    if(this->msgs.size()==0) { //find the locations of the messages to return
        TIME currentTime=Integrator::getCurSimTime();
        TIME graceTime=currentTime- Integrator::getPacketLostPeriod();
	 if(graceTime>currentTime){ //overflowed
	      graceTime=0;
	 }
        //this->msgs.clear();
        for(int i=0; i<inbox.size(); i++) {

            Message *msg=inbox[i];
            if(msg->getTime()<=currentTime && msg->getTime()>=graceTime) {
                this->msgs.push_back(i);

            }
        }
        if(msgs.size()==0)
	  return false;
        this->it=msgs.begin();//safety!
        return true;
    }
    else {
   
          ++this->it;
	  if(this->it==this->msgs.end()) { //no more messages to return
	    uint32_t shift=0;
            for(int i=0; i<msgs.size(); i++) {

               inbox.erase(inbox.begin()+(msgs[i]-shift));
		shift++;
            }
            msgs.clear();
            this->it=msgs.begin();
            return false;
        }
       return true;
    }
}


Message* ObjectCommInterface::getNextInboxMessage() {
#if DEBUG
    CERR << "ObjectCommInterface::getNextInboxMessage() " << *it << endl;
#endif
    Message *toReturn=this->inbox[*it];
    this->inbox[*it]=nullptr;
    return toReturn;
}


void ObjectCommInterface::newMessage(Message* given) {
#if DEBUG
    CERR << "ObjectCommInterface::newMessage(Message*)" << endl;
#endif
    if(st!=nullptr && !st->doBufferMessage(given))
      return;
    
    this->inbox.push_back(given);
    
    if(this->notifyMessage!=nullptr)
      (*notifyMessage)();
}


std::vector< Message* > ObjectCommInterface::getOutBox() {

    return outbox;
}


void ObjectCommInterface::clear() {
    for(int i=0;i<this->outbox.size();i++){
      delete outbox[i];
      outbox[i]=nullptr;
    }
    this->outbox.clear();
}

void ObjectCommInterface::setMessageNotifier(sim_comm::CallBack< void, empty, empty, empty,empty >* tonotify)
{
  this->notifyMessage=tonotify;
}

BufferStrategy::BufferStrategy()
{
  this->controlInterface=nullptr;
}

void BufferStrategy::setCommInterface(ObjectCommInterface* given)
{
  this->controlInterface=given;
}


void BufferStrategy::clearInbox()
{

  this->controlInterface->inbox.clear();
}

int BufferStrategy::getNumberOfMessage()
{
  
  this->controlInterface->inbox.size();
}

Message* BufferStrategy::getMessage(int index)
{
 ;
  return this->controlInterface->inbox[index];
}

void BufferStrategy::removeMessage(int index)
{
  this->controlInterface->inbox.erase(this->controlInterface->inbox.begin()+index);
}

KeepLastStrategy::KeepLastStrategy(): BufferStrategy()
{

}

bool KeepLastStrategy::doBufferMessage(Message* given)
{
  if(getNumberOfMessage()==0){
    return true;
  }

  if(given->getTime() >= getMessage(0)->getTime())
    return true;
  
  return false;
}

bool KeepFirstStrategy::doBufferMessage(Message* given)
{
  if(getNumberOfMessage()==0)
    return true;
  
  return false;
}

KeepFirstStrategy::KeepFirstStrategy(): BufferStrategy()
{

}



} /* end namespace sim_comm */
