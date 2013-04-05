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

/* C++ STL */
#include <cassert>
#include <climits>
#include <cstring>
#include <iostream>
#include <list>
#include <map>
#include <string>
#include <vector>

/* 3rd party headers */
#include <mpi.h>

/* our headers */
#include "absnetworkinterface.h"
#include "integrator.h"
#include "message.h"
#include "mpinetworkinterface.h"
#include "objectcomminterface.h"

using namespace std;
using namespace sim_comm;


#if   SIZEOF_UINT64_T == SIZEOF_UNSIGNED_CHAR
static MPI_Datatype FNCS_MPI_UINT64 = MPI_UNSIGNED_CHAR;
#elif SIZEOF_UINT64_T == SIZEOF_UNSIGNED_SHORT
static MPI_Datatype FNCS_MPI_UINT64 = MPI_UNSIGNED_SHORT;
#elif SIZEOF_UINT64_T == SIZEOF_UNSIGNED_INT
static MPI_Datatype FNCS_MPI_UINT64 = MPI_UNSIGNED;
#elif SIZEOF_UINT64_T == SIZEOF_UNSIGNED_LONG
static MPI_Datatype FNCS_MPI_UINT64 = MPI_UNSIGNED_LONG;
#else
#   error cannot determine MPI_Datatype for uint64_t
#endif


MpiNetworkInterface::MpiNetworkInterface(MPI_Comm comm_, bool iAmNetSim)
    :   AbsNetworkInterface()
    ,   comm(MPI_COMM_NULL)
    ,   commRank(-1)
    ,   commSize(-1)
    ,   sentMessages()
    ,   localObjectCount(0)
    ,   globalObjectCount(0)
    ,   iAmNetSim(iAmNetSim)
    ,   netSimRank(-1)
    ,   objectRank()
    ,   netObject(NULL) {
#if DEBUG
    CERR << "MpiNetworkInterface::MpiNetworkInterface(MPI_Comm,"
        << "iAmNetSim=" << iAmNetSim << ")" << endl;
#endif
    int ierr=0;
    int netsim_count=0;

    ierr = MPI_Comm_dup(comm_, &this->comm);
    assert(MPI_SUCCESS == ierr);
    ierr = MPI_Comm_rank(comm, &this->commRank);
    assert(MPI_SUCCESS == ierr);
    ierr = MPI_Comm_size(comm, &this->commSize);
    assert(MPI_SUCCESS == ierr);

    /* assert that there is only one network simulator */
    if (iAmNetSim) {
        int ONE = 1;
        ierr = MPI_Allreduce(&ONE, &netsim_count,
                1, MPI_INT, MPI_SUM, this->comm);
        assert(MPI_SUCCESS == ierr);
    }
    else {
        int ZERO = 0;
        ierr = MPI_Allreduce(&ZERO, &netsim_count,
                1, MPI_INT, MPI_SUM, this->comm);
        assert(MPI_SUCCESS == ierr);
    }
    assert(1 == netsim_count);

    /* determine which rank is the network simulator */
    if (iAmNetSim) {
        ierr = MPI_Allreduce(&this->commRank, &this->netSimRank,
                1, MPI_INT, MPI_SUM, this->comm);
        assert(MPI_SUCCESS == ierr);
    }
    else {
        int ZERO = 0;
        ierr = MPI_Allreduce(&ZERO, &this->netSimRank,
                1, MPI_INT, MPI_SUM, this->comm);
        assert(MPI_SUCCESS == ierr);
    }
#if DEBUG
    CERR << "netSimRank=" << netSimRank << endl;
#endif
}


MpiNetworkInterface::~MpiNetworkInterface() {
#if DEBUG
    CERR << "MpiNetworkInterface::~MpiNetworkInterface()" << endl;
#endif
    int ierr;
    makeProgress(); //clean the mpi buffer
    ierr = MPI_Comm_free(&comm);
    assert(MPI_SUCCESS == ierr);
}

void MpiNetworkInterface::barier()
{
   int ierr;
  
   ierr = MPI_Barrier(comm);
   assert(MPI_SUCCESS == ierr);
   makeProgress(); //clean the mpi buffer
}



void MpiNetworkInterface::send(Message *message) {
#if DEBUG
    CERR << "MpiNetworkInterface::send(Message*)" << endl;
#endif
    MpiIsendPacket *envelopeBundle;
    MpiIsendPacket *dataBundle;
    int rank = 0;
    const uint8_t *data = message->getData();
    uint32_t dataSize = message->getSize();
    uint8_t *envelope = NULL;
    uint32_t envelopeSize = 0;
    TIME time = Integrator::getCurSimTime();

    makeProgress();

    /* serialize envelope */
    message->serializeHeader(envelope,envelopeSize);

    if (iAmNetSim) {
        /* net sim sends to simulator containing object */
        map<string,int>::const_iterator rank_it;
        rank_it = objectRank.find(message->getTo());
        assert(objectRank.end() != rank_it);
        rank = rank_it->second;
#if DEBUG
        CERR << "netsim sending message to " << message->getTo() << endl;
#endif
    }
    else {
        /* all other sims always send to the net sim */
        rank = this->netSimRank;
#if DEBUG
        CERR << "gensim "
            << message->getFrom()
            << " sending message to "
            << message->getTo()
            << " via netsim" << endl;
#endif
    }

#if DEBUG
    CERR << "sending envelope to " << message->getTo() << endl;
#endif
    envelopeBundle = new MpiIsendPacket;
    envelopeBundle->destination_rank = rank;
    envelopeBundle->message = envelope;
    MPI_Isend(envelope, envelopeSize, MPI_UNSIGNED_CHAR,
            rank, FNCS_TAG_ENVELOPE, comm, &(envelopeBundle->request));
    sentMessages.push_back(envelopeBundle);

#if DEBUG
    CERR << "sending data to " << message->getTo() << endl;
#endif
    dataBundle = new MpiIsendPacket;
    dataBundle->destination_rank = rank;
    dataBundle->message = data;
    MPI_Isend(reinterpret_cast<void*>(const_cast<uint8_t*>(data)),
            dataSize, MPI_UNSIGNED_CHAR,
            rank, FNCS_TAG_DATA, comm, &(dataBundle->request));
    sentMessages.push_back(dataBundle);

#if DEBUG
    CERR << "sending complete to " << message->getTo() << endl;
#endif

    makeProgress();
}


uint64_t MpiNetworkInterface::broadcast(Message *message) {
#if DEBUG
    CERR << "MpiNetworkInterface::broadcast(Message*)" << endl;
#endif
    message->setTo(Message::DESTIONATION_BCAST);
    send(message);
    if (iAmNetSim) {
        NETWORK_EXCEPTION("network simulator should not broadcast");
    }

    return globalObjectCount;
}

Message* MpiNetworkInterface::receive() {
#if DEBUG
    CERR << "MpiNetworkInterface::receive()" << endl;
#endif
    Message *message = NULL;

    makeProgress();

    if (!receivedMessages.empty()) {
        message = receivedMessages.front();
        receivedMessages.pop_front();
    }

    return message;
}


vector<Message*> MpiNetworkInterface::receiveAll() {
#if DEBUG
    CERR << "MpiNetworkInterface::receiveAll()" << endl;
#endif
    vector<Message*> messages;
    
    makeProgress();

    messages.reserve(receivedMessages.size());
    messages.assign(receivedMessages.begin(), receivedMessages.end());
    receivedMessages.clear();

    return messages;
}


uint64_t MpiNetworkInterface::reduceMinTime(uint64_t myTime) {
#if DEBUG
    CERR << "MpiNetworkInterface::reduceMinTime("
        << "myTime=" << myTime << ")" << endl;
#endif
    unsigned long _myTime = static_cast<unsigned long>(myTime);
    unsigned long retval;

    makeProgress();

#if DEBUG && 0
    CERR << "MPI_Allreduce("
         << &_myTime << ","
         << &retval << ","
         << 1 << ","
         << MPI_UNSIGNED_LONG << ","
         << MPI_MIN << ","
         << comm << ")" << endl;
#endif

    MPI_Allreduce(&_myTime, &retval, 1, MPI_UNSIGNED_LONG, MPI_MIN, comm);
    MPI_Barrier(comm);

    return retval;
}


uint64_t MpiNetworkInterface::reduceTotalSendReceive(
        uint64_t sent,
        uint64_t received) {
#if DEBUG
    CERR << "MpiNetworkInterface::reduceTotalSendReceive()" << endl;
#endif
    unsigned long recvbuf[2];
    unsigned long sendbuf[2];

    makeProgress();

    sendbuf[0] = static_cast<unsigned long>(sent);
    sendbuf[1] = static_cast<unsigned long>(received);
    MPI_Allreduce(sendbuf, recvbuf, 2, MPI_UNSIGNED_LONG, MPI_SUM, comm);
    assert(recvbuf[0] >= recvbuf[1]);

    return static_cast<uint64_t>(recvbuf[0] - recvbuf[1]);
}


void MpiNetworkInterface::registerObject(string name) {
#if DEBUG
    CERR << "MpiNetworkInterface::registerObject("
        << "name=" << name << ")" << endl;
#endif
    AbsNetworkInterface::registerObject(name);
    ++this->localObjectCount;
}


void MpiNetworkInterface::finalizeRegistrations() {
#if DEBUG
    CERR << "MpiNetworkInterface::finalizeRegistrations()" << endl;
#endif
    int ierr = MPI_SUCCESS;
    uintmax_t ZERO = 0;
    uintmax_t *objectCounts = new uintmax_t[commSize];
    uintptr_t objectCountsSum = 0;

    AbsNetworkInterface::finalizeRegistrations();

    /* gather the number of registered objects to the net sim */
    if (iAmNetSim) {
        ierr = MPI_Gather(&ZERO, 1, FNCS_MPI_UINT64,
                objectCounts, 1, FNCS_MPI_UINT64, netSimRank, comm);
        assert(MPI_SUCCESS == ierr);
    }
    else {
        ierr = MPI_Gather(&localObjectCount, 1, FNCS_MPI_UINT64,
                objectCounts, 1, FNCS_MPI_UINT64, netSimRank, comm);
        assert(MPI_SUCCESS == ierr);
    }

    /* sum up object counts so we can broadcast back to other sims */
    if (iAmNetSim) {
        for (int i=0; i<commSize; ++i) {
            objectCountsSum += objectCounts[i];
#if DEBUG
            CERR << "rank " << i << " has "
                 << objectCounts[i] << " objects" << endl;
#endif
        }
    }

    /* broadcast */
    ierr = MPI_Bcast(&objectCountsSum, 1, FNCS_MPI_UINT64, netSimRank, comm);
    assert(MPI_SUCCESS == ierr);
#if DEBUG
    CERR << "[" << commRank << "] global object count = "
         << objectCountsSum << endl;
#endif
    this->globalObjectCount = objectCountsSum;

    /* net sim needs to know which objects are associated with which ranks so
     * it can route messages */
    /** @todo TODO this is the worst possible way to implement obejct->rank
     * mapping, consider MPI_Allgatherv */
    if (iAmNetSim) {
        char name[LINE_MAX];
        for (int i=0; i<commSize; ++i) {
            if (i != commRank) {
                for (int j=0; j<objectCounts[i]; ++j) {
                    int size;
                    ierr = MPI_Recv(&size, 1, MPI_INT, i,
                            FNCS_TAG+i+1, comm, MPI_STATUS_IGNORE);
                    assert(MPI_SUCCESS == ierr);
                    assert(size < LINE_MAX);
                    ierr = MPI_Recv(name, LINE_MAX, MPI_CHAR, i,
                            FNCS_TAG+i+1, comm, MPI_STATUS_IGNORE);
                    string str_name(name);
                    assert(objectRank.count(str_name) == 0);
                    objectRank[str_name] = i;
#if DEBUG
                    CERR << "objectRank[" << str_name << "] = " << i << endl;
#endif
                }
            }
        }
    }
    else {
        vector<string>::const_iterator it;
        for (it=myObjects.begin(); it!=myObjects.end(); ++it) {
            int size = it->size() + 1; /* include null terminator */
            MPI_Send(&size, 1, MPI_INT, netSimRank, FNCS_TAG+commRank+1, comm);
            MPI_Send(const_cast<char*>(it->c_str()), size, MPI_CHAR,
                    netSimRank, FNCS_TAG+commRank+1, comm);
#if DEBUG
            CERR << "gensim sending registered name: " << *it << endl;
#endif
        }
    }

    delete [] objectCounts;
}


void MpiNetworkInterface::makeProgress() {
#if DEBUG
    CERR << "MpiNetworkInterface::makeProgress()" << endl;
#endif
    int ierr = MPI_SUCCESS;
    int incoming = 1;
    list<MpiIsendPacket*>::iterator iter;

    if (isAcceptingRegistrations()) {
        NETWORK_EXCEPTION("object registration must first be finalized");
    }

    /* make progress on sent messages */
    for (iter=sentMessages.begin(); iter!=sentMessages.end(); /*++iter*/) {
        int flag = 0;

        ierr = MPI_Test(&((*iter)->request), &flag, MPI_STATUS_IGNORE);
        assert(MPI_SUCCESS == ierr);

        if (flag) {
            //delete *iter; I think this is not needed, erase calls the destructor otherwise we get double free error?
            iter = sentMessages.erase(iter);
        }
        else {
            ++iter;
        }
    }

    /* check for incoming messages */
    while (incoming) {
#if DEBUG
        CERR << "testing for incoming messages" << endl;
#endif
        MPI_Status status;

        ierr = MPI_Iprobe(MPI_ANY_SOURCE, FNCS_TAG_ENVELOPE,
                comm, &incoming, &status);
        assert(MPI_SUCCESS == ierr);

        if (incoming) {
#if DEBUG
            CERR << "found incoming message" << endl;
#endif
            Message *message=NULL;
            uint8_t *envelope=NULL;
            uint8_t *data=NULL;
            int envelopeSize=0;
            uint32_t dataSize=0;

            ierr = MPI_Get_count(&status, MPI_UNSIGNED_CHAR, &envelopeSize);
            assert(MPI_SUCCESS == ierr);
            envelope = new uint8_t[envelopeSize];

            ierr = MPI_Recv(envelope, envelopeSize, MPI_UNSIGNED_CHAR,
                            status.MPI_SOURCE, FNCS_TAG_ENVELOPE,
                            comm, MPI_STATUS_IGNORE);
            assert(MPI_SUCCESS == ierr);

            message = new Message(envelope,envelopeSize);
            dataSize = message->getSize();

#if DEBUG
            CERR << "retrieved incoming message with size " << dataSize << endl;
            CERR << *message << endl;
#endif
            if (dataSize > 0) {
                data = new uint8_t[dataSize];
                ierr = MPI_Recv(data, dataSize, MPI_UNSIGNED_CHAR,
                        status.MPI_SOURCE, FNCS_TAG_DATA,
                        comm, MPI_STATUS_IGNORE);
                assert(MPI_SUCCESS == ierr);
                message->setData(data,dataSize);
            }

            if (this->messageCallBack) {
                (*(this->messageCallBack))(message);
            }
            else {
	            this->receivedMessages.push_back(message);
            }
            
           
        }
    }
}

