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
#include "abscomminterface.h"
#include "message.h"
#include "mpicomminterface.h"
#include "objectcomminterface.h"

using namespace std;
using namespace sim_comm;

#define DEBUG 0

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


MpiCommInterface::MpiCommInterface(MPI_Comm comm_, bool iAmNetSim)
    :   AbsCommInterface()
    ,   comm(MPI_COMM_NULL)
    ,   commRank(-1)
    ,   commSize(-1)
    ,   sentMessages()
    ,   localObjectCount(0)
    ,   globalObjectCount(0)
    ,   iAmNetSim(iAmNetSim)
    ,   objectRank()
    ,   net_oci(NULL)
{
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
    cerr << "netSimRank=" << netSimRank << endl;
#endif
}


MpiCommInterface::~MpiCommInterface() {
    int ierr;
    make_progress(); //clean the mpi buffer
    ierr = MPI_Comm_free(&comm);
    assert(MPI_SUCCESS == ierr);
}


void MpiCommInterface::realSendMessage(Message *given) {
    MpiIsendPacket envelopeBundle;
    MpiIsendPacket dataBundle;
    int rank = 0;
    const uint8_t *data = given->getData();
    uint32_t dataSize = given->getSize();
    uint8_t *envelope = NULL;
    uint32_t envelopeSize = 0;
    TIME time = Integrator::getCurSimTime();

    assert(!this->allowRegistrations);

    make_progress();

    /* serialize envelope */
    given->serializeHeader(envelope,envelopeSize);

    if (iAmNetSim) {
        /* net sim sends to simulator containing object */
        map<string,int>::const_iterator rank_it;
        rank_it = objectRank.find(given->getTo());
        assert(objectRank.end() != rank_it);
        rank = rank_it->second;
    }
    else {
        /* all other sims always send to the net sim */
        rank = this->netSimRank;
    }

    envelopeBundle.destination_rank = rank;
    envelopeBundle.message = envelope;
    MPI_Isend(envelope, envelopeSize, MPI_UNSIGNED_CHAR,
            rank, FNCS_TAG_ENVELOPE, comm, &(envelopeBundle.request));
    sentMessages.push_back(envelopeBundle);

    dataBundle.destination_rank = rank;
    dataBundle.message = data;
    MPI_Isend(reinterpret_cast<void*>(const_cast<uint8_t*>(data)),
            dataSize, MPI_UNSIGNED_CHAR,
            rank, FNCS_TAG_DATA, comm, &(dataBundle.request));
    sentMessages.push_back(dataBundle);

    make_progress();
}


uint64_t MpiCommInterface::realBroadcastMessage(Message *given) {
    assert(0); // TODO
}

Message* MpiCommInterface::realGetMessage() {
    assert(!this->allowRegistrations);

    make_progress();

    make_progress();
}


uint64_t MpiCommInterface::realReduceMinTime(uint64_t myTime) {
    unsigned long _myTime = static_cast<unsigned long>(myTime);
    unsigned long retval;

    assert(!this->allowRegistrations);

    make_progress();

/*#if DEBUG
    cerr << "[" << commRank << "] MPI_Allreduce("
         << &_myTime << ","
         << &retval << ","
         << 1 << ","
         << MPI_UNSIGNED_LONG << ","
         << MPI_MIN << ","
         << comm << ")" << endl;
#endif*/
    MPI_Allreduce(&_myTime, &retval, 1, MPI_UNSIGNED_LONG, MPI_MIN, comm);
    MPI_Barrier(comm);

    return retval;
}


uint64_t MpiCommInterface::realReduceTotalSendReceive() {
    unsigned long recvbuf[2];
    unsigned long sendbuf[2];

    assert(!this->allowRegistrations);

    make_progress();

    sendbuf[0] = sendCount;
    sendbuf[1] = receiveCount;
    MPI_Allreduce(sendbuf, recvbuf, 2, MPI_UNSIGNED_LONG, MPI_SUM, comm);
    assert(recvbuf[0] >= recvbuf[1]);

    return recvbuf[0] - recvbuf[1];
}


void MpiCommInterface::addObjectInterface(
        string objectName,
        ObjectCommInterface *given) {
    map<string,ObjectCommInterface*>::iterator iter;

    AbsCommInterface::addObjectInterface(objectName,given);
    ++this->localObjectCount;
}


void MpiCommInterface::finalizeRegistrations() {
    int ierr = MPI_SUCCESS;
    uintmax_t ZERO = 0;
    uintmax_t *object_counts = new uintmax_t[commSize];
    uintptr_t object_counts_sum = 0;

    AbsCommInterface::finalizeRegistrations();

    /* gather the number of registered objects to the net sim */
    if (iAmNetSim) {
        ierr = MPI_Gather(&ZERO, 1, FNCS_MPI_UINT64,
                object_counts, 1, FNCS_MPI_UINT64, netSimRank, comm);
        assert(MPI_SUCCESS == ierr);
    }
    else {
        ierr = MPI_Gather(&localObjectCount, 1, FNCS_MPI_UINT64,
                object_counts, 1, FNCS_MPI_UINT64, netSimRank, comm);
        assert(MPI_SUCCESS == ierr);
    }

    /* sum up object counts so we can broadcast back to other sims */
    if (iAmNetSim) {
        for (int i=0; i<commSize; ++i) {
            object_counts_sum += object_counts[i];
#if DEBUG
            cerr << "rank " << i << " has "
                 << object_counts[i] << " objects" << endl;
#endif
        }
    }

    /* broadcast */
    ierr = MPI_Bcast(&object_counts_sum, 1, FNCS_MPI_UINT64, netSimRank, comm);
    assert(MPI_SUCCESS == ierr);
#if DEBUG
    cerr << "[" << commRank << "] global object count = "
         << object_counts_sum << endl;
#endif

    /* net sim needs to know which objects are associated with which ranks so
     * it can route messages */
    /** @todo TODO this is the worst possible way to implement obejct->rank
     * mapping */
    if (iAmNetSim) {
        char name[LINE_MAX];
        for (int i=0; i<commSize; ++i) {
            if (i != commRank) {
                for (int j=0; j<object_counts[i]; ++j) {
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
                }
            }
        }
    }
    else {
        map<string,ObjectCommInterface*>::const_iterator it;
        for (it=interfaces.begin(); it!=interfaces.end(); ++it) {
            int size = it->first.size() + 1; /* include null terminator */
            MPI_Send(&size, 1, MPI_INT, netSimRank, FNCS_TAG+commRank+1, comm);
            MPI_Send(const_cast<char*>(it->first.c_str()), size+1, MPI_CHAR,
                    netSimRank, FNCS_TAG+commRank+1, comm);
        }
    }
}


void MpiCommInterface::startReceiver() {
    receiverRunning = true;

    make_progress();
}


bool MpiCommInterface::isReceiverRunning() {
    assert(!this->allowRegistrations);

    return receiverRunning;
}


void MpiCommInterface::stopReceiver() {
    assert(!this->allowRegistrations);

    make_progress();

    receiverRunning = false;
}


void MpiCommInterface::sendAll() {
    assert(!this->allowRegistrations);

    make_progress();
}


void MpiCommInterface::make_progress() {
    int ierr = MPI_SUCCESS;
    int incoming = 1;
    list<MpiIsendPacket>::iterator iter;

    if (!this->receiverRunning) {
        return;
    }

    assert(!this->allowRegistrations);

    /* make progress on sent messages */
    for (iter=sentMessages.begin(); iter!=sentMessages.end(); /*++iter*/) {
        int flag = 0;

        ierr = MPI_Test(&(iter->request), &flag, MPI_STATUS_IGNORE);
        assert(MPI_SUCCESS == ierr);

        if (flag) {
            iter = sentMessages.erase(iter);
        }
        else {
            ++iter;
        }
    }

    /* check for incoming messages */
    while (incoming) {
        MPI_Status status;

        ierr = MPI_Iprobe(MPI_ANY_SOURCE, FNCS_TAG_ENVELOPE,
                comm, &incoming, &status);
        assert(MPI_SUCCESS == ierr);

        if (incoming) {
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

            if (dataSize > 0) {
                data = new uint8_t[dataSize];
                ierr = MPI_Recv(data, dataSize, MPI_UNSIGNED_CHAR,
                        status.MPI_SOURCE, FNCS_TAG_DATA,
                        comm, MPI_STATUS_IGNORE);
                assert(MPI_SUCCESS == ierr);
                message->setData(data,dataSize);
            }

	        this->messageReceived(message);
        }
    }
}

