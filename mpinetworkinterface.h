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
#ifndef MPINETWORKINTERFACE_H_
#define MPINETWORKINTERFACE_H_

#include <list>
#include <string>

#include <mpi.h>

#include "absnetworkinterface.h"

using std::list;
using std::string;

namespace sim_comm {

/* forward declarations */
class Message;
class ObjectCommInterface;

#define FNCS_TAG_BASE 3627
#define FNCS_TAG_ENVELOPE (FNCS_TAG_BASE + 1)
#define FNCS_TAG_DATA     (FNCS_TAG_BASE + 2)
#define FNCS_TAG          (FNCS_TAG_BASE + 3)

class MpiIsendPacket {
public:
    MpiIsendPacket() {}

    MpiIsendPacket(int destination_rank, const uint8_t *message, MPI_Request request)
        :   destination_rank(destination_rank)
        ,   message(message)
        ,   request(request)
    {}

    ~MpiIsendPacket() {
        if (nullptr != message) {
            delete [] message;
        }
    }

    int destination_rank;
    const uint8_t *message;
    MPI_Request request;
};

class MpiNetworkInterface : public AbsNetworkInterface {
private:
    MPI_Comm comm;
    int commRank;
    int commSize;
    uint64_t localObjectCount;
    uint64_t globalObjectCount;
    bool iAmNetSim;
    int netSimRank;
    map<string,int> objectRank;
    ObjectCommInterface *netObject;
    list<MpiIsendPacket*> sentMessages;
    list<Message*> receivedMessages;

protected:
    void makeProgress();

public:
    MpiNetworkInterface(MPI_Comm comm, bool iAmNetSim);

    virtual ~MpiNetworkInterface();

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
};

}

#endif /* MPINETWORKINTERFACE_H_ */
