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
#ifndef MPICOMMINTERFACE_H_
#define MPICOMMINTERFACE_H_

#include <list>
#include <string>

#include <mpi.h>

#include "abscomminterface.h"

using namespace std;

namespace sim_comm {

class Message;
class ObjectCommInterface;

#define FNCS_TAG 3627

class MpiIsendPacket {
public:
    MpiIsendPacket() {}

    MpiIsendPacket(int destination_rank, char *message, MPI_Request request)
        :   destination_rank(destination_rank)
        ,   message(message)
        ,   request(request)
    {}

    ~MpiIsendPacket() {
        delete [] message;
    }

    int destination_rank;
    char *message;
    MPI_Request request;
};

class MpiCommInterface : public AbsCommInterface {
private:
    MPI_Comm comm;
    int rank;
    list<MpiIsendPacket> sentMessages;

protected:
    void make_progress();

public:
    MpiCommInterface(MPI_Comm comm);

    virtual ~MpiCommInterface();

    /** @copydoc AbsCommInterface::realSendMessage(Message*) */
    virtual void realSendMessage(Message *given);

    /** @copydoc AbsCommInterface::realGetMessage() */
    virtual Message* realGetMessage();

    /** @copydoc AbsCommInterface::realReduceMinTime(uint64_t) */
    virtual uint64_t realReduceMinTime();

    /** @copydoc AbsCommInterface::realReduceTotalSendReceive(uint64_t,uint64_t) */
    virtual uint64_t realReduceTotalSendReceive();

    /** @copydoc AbsCommInterface::addObjectInterface(string,ObjectCommInterface*) */
    void addObjectInterface(string objectName,ObjectCommInterface *given);

    /** @copydoc AbsCommInterface::startReceiver() */
    void startReceiver();

    /** @copydoc AbsCommInterface::isReceiverRunning() */
    bool isReceiverRunning();

    /** @copydoc AbsCommInterface::stopReceiver() */
    void stopReceiver();

    /** @copydoc AbsCommInterface::sendAll() */
    void sendAll();
};

}

#endif /* MPICOMMINTERFACE_H_ */
