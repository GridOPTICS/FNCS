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
#ifndef ZMQNETWORKINTERFACE_H_
#define ZMQNETWORKINTERFACE_H_

#include <errno.h>
#include <unistd.h>

#include <zmq.h>

#include <list>

#include "absnetworkinterface.h"
#include "integrator.h"
#include "zmqhelper.h"

using std::list;


namespace sim_comm {

/* forward declarations */
class Message;
class ObjectCommInterface;

class ZmqNetworkInterface : public AbsNetworkInterface {
private:
    void *zmq_ctx;
    void *zmq_req;
    void *zmq_async;
    void *zmq_die;
    string ID;
    int context;
    bool iAmNetSim;
    list<Message*> receivedMessages;
    uint64_t globalObjectCount;

protected:
    void init();
    void processAsyncMessage();
    void processSubMessage();
    void makeProgress();
    template <typename T> int i_recv(T &buf);
    
public:
    /**
     * Constructs.
     */
    ZmqNetworkInterface(bool iAmNetSim);

    /**
     * Copy constructs.
     */
    ZmqNetworkInterface(const ZmqNetworkInterface &that);

    /**
     * Destroys.
     */
    virtual ~ZmqNetworkInterface();

    /** @copydoc AbsNetworkInterface::send(Message*) */
    virtual void send(Message *message);

    /** @copydoc AbsNetworkInterface::broadcast(Message*) */
    virtual uint64_t broadcast(Message *message);

    /** @copydoc AbsNetworkInterface::receive() */
    virtual Message* receive();

    /** @copydoc AbsNetworkInterface::receiveAll() */
    virtual vector<Message*> receiveAll();

    /** @copydoc AbsNetworkInterface::reduceMinTime(uint64_t) */
    virtual uint64_t reduceMinTime(uint64_t myTime);

    /** @copydoc AbsNetworkInterface::reduceTotalSendReceive() */
    virtual uint64_t reduceTotalSendReceive(uint64_t sent, uint64_t received);

    /** @copydoc AbsNetworkInterface::registerObject(string) */
    virtual void registerObject(string name);

    /** @copydoc AbsNetworkInterface::finalizeRegistrations() */
    virtual void finalizeRegistrations();

    /** @copydoc AbsNetworkInterface::barier()*/
    virtual void barier();

    /** @copydoc AbsNetworkInterface::sleep()*/
    virtual bool sleep();

    /** @copydoc AbsNetworkInterface::duplicateInterface()*/
    virtual AbsNetworkInterface* duplicateInterface();

    /** @copydoc AbsNetworkInterface::sendFinishedSignal()*/
    virtual void sendFinishedSignal();
    
    /** @copydoc AbsNetworkInterface::getNextTimes()*/
    virtual uint64_t* getNextTimes(uint64_t nextTime,uint32_t &worldSize);

    /** @copydoc AbsNetworkInterface::cleanup()*/
    virtual void cleanup();
    
    /** @copydoc AbsNetworkInterface::sendFailed()*/
    virtual void sendFailed();
    
    /** @copydoc AbsNetworkInterface::sendSuceed()*/
    virtual void sendSuceed();
};


template <typename T>
int ZmqNetworkInterface::i_recv(T &buf)
{
    bool done;
    int size;
#if DEBUG
    CERR << "ZmqNetworkInterface::i_recv" << endl;
#endif
    done = false;
    while (!done) {
        zmq_pollitem_t items[] = {
            { this->zmq_req,   0, ZMQ_POLLIN, 0 },
            { this->zmq_async, 0, ZMQ_POLLIN, 0 },
            { this->zmq_die,   0, ZMQ_POLLIN, 0 }
        };
        int rc = zmq_poll(items, 3, -1);
        zmqx_interrupt_check();
        assert(rc >= 0 || (rc == -1 && errno == EINTR));
        if (items[0].revents & ZMQ_POLLIN) {
            size = zmqx_recv(this->zmq_req, buf);
#if DEBUG
            CERR << "ZmqNetworkInterface:i_recv got '" << buf << "'" << endl;
#endif
	    done = true;
        }
        if (items[1].revents & ZMQ_POLLIN) {
            processAsyncMessage();
        }
        if (items[2].revents & ZMQ_POLLIN) {
            processSubMessage();
        }
    }

    return size;
}


} /* end namespace sim_comm */

#endif /* ZMQNETWORKINTERFACE_H_ */
