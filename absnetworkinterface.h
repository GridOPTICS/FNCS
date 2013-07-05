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
#ifndef ABSNETWORKINTERFACE_H_
#define ABSNETWORKINTERFACE_H_

#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

#include <exception>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "callback.h"

using namespace std;

namespace sim_comm {

/* forward declarations */
class Message;


/**
 * Generic excpetion generated by the network.
 */
class NetworkException : public exception {
public:
    NetworkException(
            const char *file,
            const char *func,
            int line,
            const char *description) throw () {
        ostringstream s;
        s << "[" << getpid() << "] " << file << ":" << line << ":" << func
            << ": Exception `" << description << "'" << endl;
        this->message = s.str();
    }
    ~NetworkException() throw() {}
    virtual const char* what() const throw() {
        return message.c_str();
    }
private:
    string message;
};

/* for brevity */
#define NETWORK_EXCEPTION(DESC) \
    throw NetworkException(__FILE__,__func__,__LINE__,(DESC))


class AbsNetworkInterface {
protected:
    vector<string> myObjects;
    bool registrationsAreFinalized;
    CallBack<void,Message*,empty,empty> *messageCallBack;
    
public:
    /**
     * Constructs.
     */
    AbsNetworkInterface();

    AbsNetworkInterface(const AbsNetworkInterface &that);
    /**
     * Destroys.
     */
    virtual ~AbsNetworkInterface();

    /**
     * Injects a message into the network.
     *
     * @param[in] message the Message instance
     * @throw NetworkException if the send operation fails
     */
    virtual void send(Message *message) =0;

    /**
     * Sends the message to every network endpoint.
     *
     * @param[in] message the Message instance
     * @throw NetworkException if the send operation fails
     */
    virtual uint64_t broadcast(Message *message) =0;

    /**
     * Returns the first available Message, or NULL.
     *
     * This method is for callers which actively pull Message instances from
     * the network, as opposed to registering a callback function when a
     * Message arrives.
     *
     * @throw NetworkException if the receive operation fails
     * @return the Message instance, or NULL
     */
    virtual Message* receive() =0;

    /**
     * Returns all available messages.
     *
     * This method is for callers which actively pull Message instances from
     * the network, as opposed to registering a callback function when a
     * Message arrives.
     *
     * @throw NetworkException if the receive operation fails
     * @return all Message instances, or an empty vector
     */
    virtual vector<Message*> receiveAll();

    /**
     * Registers a callback function to invoke when a Message is received.
     */
    void setMessageCallBack(CallBack<void,Message*,empty,empty> *messageCallBack) {
        this->messageCallBack = messageCallBack;
    }

    /**
     * Reduces min time operation.
     *
     * @throw NetworkException when the reduce operation fails
     * @return the min time
     */
    virtual uint64_t reduceMinTime(uint64_t myTime) =0;

    /**
     * Reduces total sent and received counts.
     *
     * @throw NetworkException when the reduce operation fails
     */
    virtual uint64_t reduceTotalSendReceive(uint64_t sent, uint64_t received) =0;

    /**
     * Registers an object for Message delivery.
     *
     * @param[in] name TODO
     * @param[in] object TODO
     */
    virtual void registerObject(string name);

    /**
     * Indicates that communication object registrations have completed.
     *
     * This method is collective across all AbsNetworkInterface instances in
     * order to efficiently exchange metadata.
     */
    virtual void finalizeRegistrations();

    /**
     * Indicates that registrations have been completed.
     */
    inline bool isAcceptingRegistrations() {
        return !this->registrationsAreFinalized;
    }
    
    /**
     * Barier function.
     */
    virtual void barier() =0;
    
    /**
     * Sleep function.
     */
    virtual void sleep() =0;
    
    /**
     * Duplicates the current interfce, creates a new interface that
     * has the same connections as the previous.
     */
    virtual AbsNetworkInterface* duplicateInterface() =0;

    /**
     * Signals that we are finished.
     */
    virtual void sendFinishedSignal();
};

} /* end namespace sim_comm */

#endif /* ABSNETWORKINTERFACE_H_ */
