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

/* C++ STL */
#include <cassert>
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


MpiCommInterface::MpiCommInterface(MPI_Comm comm_)
    :   AbsCommInterface(0)
    ,   comm(MPI_COMM_NULL)
{
    int ierr;
    int rank;

    ierr = MPI_Comm_dup(comm_, &comm);
    assert(MPI_SUCCESS == ierr);
    ierr = MPI_Comm_rank(comm, &rank);
    assert(MPI_SUCCESS == ierr);
    assert(rank >= 0);
    this->myRank = static_cast<uint32_t>(rank);
}


MpiCommInterface::~MpiCommInterface()
{
    int ierr;

    ierr = MPI_Comm_free(&comm);
    assert(MPI_SUCCESS == ierr);
}


void MpiCommInterface::realSendMessage(Message *given)
{
    MpiIsendPacket bundle;
    map<string,ObjectCommInterface*>::iterator iter;

    make_progress();

    iter = interfaces.find(given->getTo());
    if (iter == interfaces.end()) {
        cerr << "[" << myRank << "] "
             << "message destination not registered: " << given->getTo() << endl;
        MPI_Abort(comm, 1);
    }

    bundle.destination_rank = iter->second->getMyRank();
    bundle.message = new char[sizeof(TIME)];
    MPI_Isend(bundle.message, sizeof(TIME), MPI_CHAR,
            bundle.destination_rank, FNCS_TAG, comm, &(bundle.request));
    sentMessages.push_back(bundle);

    make_progress();
}


Message* MpiCommInterface::realGetMessage()
{
    make_progress();

    make_progress();
}


uint64_t MpiCommInterface::realReduceMinTime(uint64_t myTime)
{
    uint64_t retval;
    MPI_Datatype datatype;

    make_progress();

#if   SIZEOF_UINT64_T == SIZEOF_UNSIGNED_CHAR
    datatype = MPI_UNSIGNED_CHAR;
#elif SIZEOF_UINT64_T == SIZEOF_UNSIGNED_SHORT
    datatype = MPI_UNSIGNED_SHORT;
#elif SIZEOF_UINT64_T == SIZEOF_UNSIGNED_INT
    datatype = MPI_UNSIGNED;
#elif SIZEOF_UINT64_T == SIZEOF_UNSIGNED_LONG
    datatype = MPI_UNSIGNED_LONG;
#else
#   error cannot determine MPI_Datatype for uint64_t
#endif

    MPI_Allreduce(&myTime, &retval, 1, datatype, MPI_MIN, comm);

    return retval;
}


uint64_t MpiCommInterface::realReduceTotalSendReceive(
        uint64_t send, uint64_t receive)
{
    uint64_t recvbuf[2];
    uint64_t sendbuf[2];
    MPI_Datatype datatype;

    make_progress();

#if   SIZEOF_UINT64_T == SIZEOF_UNSIGNED_CHAR
    datatype = MPI_UNSIGNED_CHAR;
#elif SIZEOF_UINT64_T == SIZEOF_UNSIGNED_SHORT
    datatype = MPI_UNSIGNED_SHORT;
#elif SIZEOF_UINT64_T == SIZEOF_UNSIGNED_INT
    datatype = MPI_UNSIGNED;
#elif SIZEOF_UINT64_T == SIZEOF_UNSIGNED_LONG
    datatype = MPI_UNSIGNED_LONG;
#else
#   error cannot determine MPI_Datatype for uint64_t
#endif

    sendbuf[0] = send;
    sendbuf[1] = receive;
    MPI_Allreduce(sendbuf, recvbuf, 2, datatype, MPI_SUM, comm);

    return recvbuf[0]; /* @TODO: shouldn't we return both values?? */
}


void MpiCommInterface::addObjectInterface(
        string objectName, ObjectCommInterface *given)
{
    map<string,ObjectCommInterface*>::iterator iter;
    
    make_progress();

    iter = interfaces.find(objectName);
    if (iter != interfaces.end()) {
        cerr << "[" << myRank << "] "
             << "object registered already exists: " << objectName << endl;
        MPI_Abort(comm, 1);
    }
}


void MpiCommInterface::startReceiver()
{
    receiverRunning = true;

    make_progress();
}


bool MpiCommInterface::isReceiverRunning()
{
    make_progress();

    return receiverRunning;
}


void MpiCommInterface::stopReceiver()
{
    make_progress();

    receiverRunning = false;
}


void MpiCommInterface::sendAll()
{
    make_progress();
}


void MpiCommInterface::make_progress()
{
    int ierr = MPI_SUCCESS;
    int incoming = 1;
    list<MpiIsendPacket>::iterator iter;

    if (!isReceiverRunning()) {
        return;
    }

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

        ierr = MPI_Iprobe(MPI_ANY_SOURCE, FNCS_TAG, comm, &incoming, &status);
        assert(MPI_SUCCESS == ierr);

        if (incoming) {
            char *message;
            int size;

            ierr = MPI_Get_count(&status, MPI_CHAR, &size);
            assert(MPI_SUCCESS == ierr);
            message = new char[size];

            ierr = MPI_Recv(message, size, MPI_CHAR,
                    status.MPI_SOURCE, status.MPI_TAG,
                    comm, MPI_STATUS_IGNORE);
            assert(MPI_SUCCESS == ierr);

            /* @TODO do something with the message! */
            delete [] message;
        }
    }
}

