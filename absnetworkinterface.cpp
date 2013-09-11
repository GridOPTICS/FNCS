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

#include <algorithm>
#include <map>
#include <string>
#include <sstream>
#include <utility>
#include <vector>

#include "absnetworkinterface.h"
#include "integrator.h"
#include "echo.h"

using std::endl;
using std::find;
using std::map;
using std::ostringstream;
using std::string;
using std::vector;

//#if DEBUG && DEBUG_TO_FILE
//Echo echo;
//#endif

using namespace sim_comm;


AbsNetworkInterface::AbsNetworkInterface()
    :   myObjects()
    ,   registrationsAreFinalized(false)
  
{
#if DEBUG && DEBUG_TO_FILE
    ostringstream ferrName;
    ferrName << "tracer." << PID << ".log";
    echo.open(ferrName.str().c_str());
#endif
}


AbsNetworkInterface::~AbsNetworkInterface() {
#if DEBUG && DEBUG_TO_FILE
    echo.close();
#endif
}


AbsNetworkInterface::AbsNetworkInterface(const AbsNetworkInterface& that)
{
  this->myObjects=that.myObjects;
  this->messageCallBack=that.messageCallBack;
  this->registrationsAreFinalized=that.registrationsAreFinalized;
 
}


vector<Message*> AbsNetworkInterface::receiveAll() {
    vector<Message*> messages;
    Message *message;

    while (NULL != (message = receive())) {
        messages.push_back(message);
    }

    return messages;
}


void AbsNetworkInterface::registerObject(string name) {
    vector<string>::iterator it;

    if (this->registrationsAreFinalized) {
        NETWORK_EXCEPTION("object registration has already been finalized");
    }

    it = find(myObjects.begin(), myObjects.end(), name);
    if (myObjects.end() == it) {
        myObjects.push_back(name);
    }
    else {
        ostringstream desc;
        desc << "object `" << name << "' already registered";
        NETWORK_EXCEPTION(desc.str().c_str());
    }
}


void AbsNetworkInterface::finalizeRegistrations() {
    this->registrationsAreFinalized = true;
}


void AbsNetworkInterface::sendFinishedSignal()
{
}



