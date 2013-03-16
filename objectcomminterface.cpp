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


ObjectCommInterface::ObjectCommInterface(string objectName) {
#if DEBUG
    CERR << "ObjectCommInterface::ObjectCommInterface("
        << objectName << ")" << endl;
#endif
    this->attachedObjectName=objectName;
    this->notifyMessage=nullptr;
}


ObjectCommInterface::~ObjectCommInterface() {
#if DEBUG
    CERR << "ObjectCommInterface::~ObjectCommInterface()" << endl;
#endif
    this->inbox.clear();
    this->outbox.clear();
}


void ObjectCommInterface::send(Message* given) {
#if DEBUG
    CERR << "ObjectCommInterface::send(Message*)" << endl;
#endif
    this->outbox.push_back(given);
}


int ObjectCommInterface::getInboxMessagesCount() {
#if DEBUG
    CERR << "ObjectCommInterface::getInboxMessagesCount()" << endl;
#endif
    //we can write a better one!

     TIME timeframe=Integrator::getCurSimTime();
     TIME timeframe1=timeframe-Integrator::getGracePeriod();
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
}


std::vector< Message* > ObjectCommInterface::getAllInboxMessages() {
#if DEBUG
    CERR << "ObjectCommInterface::getAllInboxMessages()" << endl;
#endif
    TIME currentTime=Integrator::getCurSimTime();
    TIME graceTime=currentTime- Integrator::getGracePeriod();
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
        TIME graceTime=currentTime- Integrator::getGracePeriod();
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
    return this->inbox[*it];
}


void ObjectCommInterface::newMessage(Message* given) {
#if DEBUG
    CERR << "ObjectCommInterface::newMessage(Message*)" << endl;
#endif
    this->inbox.push_back(given);
    
    if(this->notifyMessage!=nullptr)
      (*notifyMessage)();
}


std::vector< Message* > ObjectCommInterface::getOutBox() {
#if DEBUG
    CERR << "ObjectCommInterface::getOutBox()" << endl;
#endif
    return outbox;
}


void ObjectCommInterface::clear() {
#if DEBUG
    CERR << "ObjectCommInterface::clear()" << endl;
#endif
    this->outbox.clear();
}

void ObjectCommInterface::setMessageNotifier(sim_comm::CallBack< void, empty, empty, empty >* tonotify)
{
  this->notifyMessage=tonotify;
}


} /* end namespace sim_comm */
