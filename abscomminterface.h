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
#ifndef ABSCOMMINTERFACE_H
#define ABSCOMMINTERFACE_H

#include <pthread.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "message.h"

using namespace std;

namespace sim_comm {

class ObjectCommInterface;

class ObjectInterfaceRegistrationException : exception {
    virtual const char* what() const throw() {
        return "Cannot attach interface to a non-remote object!";
    }
};

class InterfaceErrorException : exception {
    virtual const char* what() const throw() {
        return "Interface operation failed!!";
    }
};

class AbsCommInterface {
protected:
    map<string,ObjectCommInterface*> interfaces; /**< @TODO doc */
    bool receiverRunning; /**< @TODO doc */
    uint64_t sendCount; /**< @TODO doc */
    uint64_t receiveCount; /**< @TODO doc */

public:
    /**
     * Constructor.
     */
    AbsCommInterface();

    /**
     * Destructor.
     */
    virtual ~AbsCommInterface();

    /**
     * The real send message method, calling this will send the message to
     * another sim_comm.
     *
     * @param[in] given the Message instance
     * @throw InterfaceErrorException if the send operation fails
     */
    virtual void realSendMessage(Message *given) =0;

    /**
     * Reall receive message method, calling this will block until a message is
     * received from a sim_comm.
     *
     * @throw InterfaceErrorException if the receive operation fails
     * @return the Message instance
     */
    virtual Message* realGetMessage() =0;

    /**
     * Reduce min time operation.
     *
     * @throw InterfaceErrorException when the reduce operation fails
     * @return the min time
     */
    virtual uint64_t realReduceMinTime(uint64_t myTime) =0;

    /**
     * Reduce total send receive operation.
     *
     * @throw InterfaceErrorException when the reduce operation fails
     */
    virtual uint64_t realReduceTotalSendReceive() =0;

    /**
     * Used by the integrator to register an object.
     *
     * @param[in] objectName TODO
     * @param[in] given TODO
     */
    void addObjectInterface(string objectName,ObjectCommInterface *given);

    /**
     * Starts the receiver thread.
     */
    void startReceiver();

    /**
     * Returns true if the reciever thread is running.
     */
    bool isReceiverRunning();

    /**
     * Kills the receiver thread.
     */
    void stopReceiver();

    /**
     * Called by the integrator to send all the messages.
     */
    void sendAll();
};

}

#endif // ABSCOMMINTERFACE_H
