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

#include <utility>

#include "abscomminterface.h"
#include "objectcomminterface.h"
#include "integrator.h"
#include "communicatorsimulatorsyncalgo.h"

using namespace std;


namespace sim_comm {


AbsCommInterface::AbsCommInterface() {
#if DEBUG
    CERR << "AbsCommInterface::AbsCommInterface()" << endl;
#endif
    this->receiveCount=0;
    this->sendCount=0;
    this->receiverRunning=false;
    this->allowRegistrations=true;
}


bool AbsCommInterface::isReceiverRunning() {
#if DEBUG
    CERR << "AbsCommInterface::isReceiverRunning()" << endl;
#endif
    return this->receiverRunning;
}


void AbsCommInterface::addObjectInterface(
        string objectName,
        ObjectCommInterface* given) {
#if DEBUG
    CERR << "AbsCommInterface::addObjectInterface("
        << objectName << ","
        << given << ")" << endl;
#endif
    if (this->allowRegistrations) {
        if (this->interfaces.count(objectName) != 0) {
            /* enforce unique names and one-time registrations */
            throw ObjectInterfaceRegistrationException(__FILE__,__LINE__);
        }
        this->interfaces.insert(pair<string,ObjectCommInterface*>(objectName,given));
    }
    else {
        throw ObjectInterfaceRegistrationException(__FILE__,__LINE__);
    }
}


ObjectCommInterface* AbsCommInterface::getObjectInterface(string objectName) {
#if DEBUG
    CERR << "AbsCommInterface::getObjectInterface("
        << objectName << ")" << endl;
#endif
    if (this->interfaces.count(objectName) == 0) {
        /* enforce unique names and one-time registrations */
        throw ObjectInterfaceRegistrationException(__FILE__,__LINE__);
    }
    return this->interfaces[objectName];
}


void AbsCommInterface::finalizeRegistrations() {
#if DEBUG
    CERR << "AbsCommInterface::finalizeRegistrations()" << endl;
#endif
    this->allowRegistrations = false;
}


void AbsCommInterface::startReceiver() {
#if DEBUG
    CERR << "AbsCommInterface::startReceiver()" << endl;
#endif
    this->receiverRunning=true;
}


void AbsCommInterface::stopReceiver() {
#if DEBUG
    CERR << "AbsCommInterface::stopReceiver()" << endl;
#endif
    this->receiverRunning=false;
}


void AbsCommInterface::sendAll() {
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
                    if (outmessges[i]->isBroadCast()) {
                        int scount = this->realBroadcastMessage(outmessges[i]);
                        if(doincrementCountersInSendReceive)
                            sendCount +=scount;
                    } 
                    else {
                        if(doincrementCountersInSendReceive)
                            sendCount += 1;
                        this->realSendMessage(outmessges[i]);
                    }
                }
                catch(InterfaceErrorException e) {

                    std::cerr << "Send operation failed on interface ";
                }
            }
        //}
    }
}


AbsCommInterface::~AbsCommInterface() {
#if DEBUG
    CERR << "AbsCommInterface::~AbsCommInterface()" << endl;
#endif
    map<string,ObjectCommInterface*>::iterator it=this->interfaces.begin();
    for(;it!=interfaces.end();++it) {
        delete it->second;
    }
    this->interfaces.clear();
}


void AbsCommInterface::packetLost(AbsSyncAlgorithm* given) {
#if DEBUG
    CERR << "AbsCommInterface::packetLost(AbsSyncAlgorithm*)" << endl;
#endif
    CommunicatorSimulatorSyncalgo *syncAlgo=dynamic_cast<CommunicatorSimulatorSyncalgo *>(given);

    if(syncAlgo==NULL) {
        throw SyncAlgoErrorException();
    }

    this->receiveCount++;
}


void AbsCommInterface::messageReceived(Message *message) {
#if DEBUG
    CERR << "AbsCommInterface::messageReceived(Message*)" << endl;
#endif
    //Get Time frame to accept the messageReceived
    TIME timeframe=Integrator::getCurSimTime()-Integrator::getGracePeriod()*2;

    if(message->getTime()<timeframe){ //old message drop
        delete message;
    }

    //let it throw an exception if the key is not found.
    ObjectCommInterface *comm=getObjectInterface(message->getTo());

    if (this->doincrementCountersInSendReceive) {
        this->receiveCount++;
    }

    comm->newMessage(message);
}


void AbsCommInterface::setNoCounterIncrement(AbsSyncAlgorithm* given, bool state) {
#if DEBUG
    CERR << "AbsCommInterface::setNoCounterIncrement("
        << "AbsSyncAlgorithm*,"
        << "state=" << state << ")" << endl;
#endif
    CommunicatorSimulatorSyncalgo *syncAlgo=dynamic_cast<CommunicatorSimulatorSyncalgo *>(given);

    if(syncAlgo==NULL) {
        throw SyncAlgoErrorException();
    }

    this->doincrementCountersInSendReceive=state;
}


} /* end namespace sim_comm */

