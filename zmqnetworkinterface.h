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
    void *zmq_die;
    string ID;
    bool iAmNetSim;
    list<Message*> receivedMessages;
    uint64_t globalObjectCount;

protected:
    void makeProgress();

    template <typename T>
    int i_recv(void *socket, T &buf);
    
public:
    /**
     * Constructs.
     */
    ZmqNetworkInterface(bool iAmNetSim);

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

    /** @copydoc AbsNetworkInterface::duplicateInterface()*/
    virtual AbsNetworkInterface* duplicateInterface();

    /** @copydoc AbsNetworkInterface::sendFinishedSignal()*/
    virtual void sendFinishedSignal();
};


template <typename T>
int ZmqNetworkInterface::i_recv(void *socket, T &buf)
{
    int size;

    CERR << "ZmqNetworkInterface::i_recv" << endl;

    while (true) {
        zmq_pollitem_t items[2];
        items[0].socket = socket;
        items[0].events = ZMQ_POLLIN;
        items[1].socket = this->zmq_die;
        items[1].events = ZMQ_POLLIN;
        int rc = zmq_poll(items, 1, 0);
        assert(rc >= 0);
        if (items[0].revents & ZMQ_POLLIN) {
            size = s_recv(socket, buf);
            break;
        }
        if (items[1].revents & ZMQ_POLLIN) {
            string control;

            (void) s_recv(this->zmq_die, control);

            if ("DIE" == control) {
                CERR << "DIE received from SUB" << endl;
                /* an application terminated abrubtly, so do we */
                (void) s_send(this->zmq_req, "DIE");
                exit(EXIT_FAILURE);
            }
            else if ("FINISHED" == control) {
                CERR << "FINISHED received from SUB" << endl;
                /* an application terminated cleanly, so do we */
                (void) s_send(this->zmq_req, "FINISHED");
                exit(EXIT_SUCCESS);
            }
            else {
                CERR << "'" << control << "' received from SUB" << endl;
                (void) s_send(this->zmq_req, "DIE");
                exit(EXIT_FAILURE);
            }

        }
    }

    return size;
}


} /* end namespace sim_comm */

#endif /* ZMQNETWORKINTERFACE_H_ */
