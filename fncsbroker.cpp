/*
 *  Copyright (c) 2013, <copyright holder> <email>
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *      * Neither the name of the <organization> nor the names of its
 *      contributors may be used to endorse or promote products derived from
 *      this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY <copyright holder> <email> ''AS IS'' AND ANY
 *  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL <copyright holder> <email> BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 *  DAMAGE.
 */
#include "config.h"

#define DEBUG 1

#include <algorithm>
#include <cassert>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <algorithm>
#include <zmq.h>

#include "integrator.h"
#include "message.h"
#include "zmqhelper.h"

using namespace std;
using namespace sim_comm;

typedef map<string,unsigned long> ReduceMap;
typedef pair<string,unsigned long> ReducePair;

vector<map<string,string> > obj_to_ID;
vector<ReduceMap> reduce_min_time;
vector<ReduceMap> reduce_sent;
vector<ReduceMap> reduce_recv;
string newNetSimID;
set<string> newConnections;
vector<set<string> > contexts;
vector<set<string> > finalized;
vector<set<string> > barrier;
vector<set<string> > finished;
vector<string> netSimID;
vector<size_t> netSimObjCount;
void *zmq_ctx = NULL;
void *broker = NULL;
void *async_broker = NULL;
void *killer = NULL;
int world_size = 0;

struct ReduceMinPairLess
{
    bool operator()(const ReducePair& left, const ReducePair& right) const
    {
        return left.second < right.second;
    }
};

static int add_context();
static bool starts_with(const string &str, const string &prefix);
static void graceful_death(int exit_code);
static void hello_handler(const string &identity, const int &context, const string &control);
static void route_handler(const string &identity, const int &context, const string &control);
static void delay_handler(const string &identity, const int &context, const string &control);
static void reduce_min_time_handler(const string &identity, const int &context, const string &control);
static void reduce_send_recv_handler(const string &identity, const int &context, const string &control);
static void register_handler(const string &identity, const int &context, const string &control);
static void finalize_handler(const string &identity, const int &context, const string &control);
static void barrier_handler(const string &identity, const int &context, const string &control);
static bool finished_handler(const string &identity, const int &context, const string &control);


/* handle signals */
static int s_interrupted = 0;
static void s_signal_handler(int signal_value)
{
    s_interrupted = 1;
}
static void s_catch_signals()
{
    struct sigaction action;
    action.sa_handler = s_signal_handler;
    action.sa_flags = 0;
    sigemptyset(&action.sa_mask);
    sigaction(SIGINT, &action, NULL);
    sigaction(SIGTERM, &action, NULL);
}


int main(int argc, char **argv)
{
    int retval;
    const int ONE = 1;

#if DEBUG
#   if DEBUG_TO_FILE
    ostringstream ferrName;
    ferrName << "tracer." << PID << ".log";
    echo.open(ferrName.str().c_str());
#   endif
#endif
    
    if (1 >= argc) {
        cerr << "missing world_size argument" << endl;
        exit(EXIT_FAILURE);
    }
    else if (3 <= argc) {
        cerr << "too many arguments" << endl;
        exit(EXIT_FAILURE);
    }
    world_size = atoi(argv[1]);
    if (world_size <= 1) {
        cerr << "world size must be > 1" << endl;
        exit(EXIT_FAILURE);
    }

    zmq_ctx = zmq_ctx_new();
    if (NULL == zmq_ctx) {
        perror("zmq_ctx_new");
        graceful_death(EXIT_FAILURE);
    }

    /* create socket for receiving barrier/sync messages from simulations */
    broker = zmq_socket(zmq_ctx, ZMQ_ROUTER);
    if (NULL == broker) {
        perror("zmq_socket(zmq_ctx, ZMQ_ROUTER)");
        graceful_death(EXIT_FAILURE);
    }

    /* broker should not silently drop packets that can't be sent out */
    retval = zmq_setsockopt(broker, ZMQ_ROUTER_MANDATORY, &ONE, sizeof(int));
    if (-1 == retval) {
        perror("zmq_setsockopt(broker, ZMQ_ROUTER_MANDATORY, &ONE, sizeof(int))");
        graceful_death(EXIT_FAILURE);
    }

    /* bind broker to physical address */
    retval = zmq_bind(broker, "tcp://*:5555");
    if (-1 == retval) {
        perror("zmq_bind(broker, \"tcp://*:5555\")");
        graceful_death(EXIT_FAILURE);
    }

    /* create socket for receiving sim messages from simulations */
    async_broker = zmq_socket(zmq_ctx, ZMQ_ROUTER);
    if (NULL == async_broker) {
        perror("zmq_socket(zmq_ctx, ZMQ_ROUTER)");
        graceful_death(EXIT_FAILURE);
    }

    /* async_broker should not silently drop packets that can't be sent out */
    retval = zmq_setsockopt(async_broker, ZMQ_ROUTER_MANDATORY, &ONE, sizeof(int));
    if (-1 == retval) {
        perror("zmq_setsockopt(async_broker, ZMQ_ROUTER_MANDATORY, &ONE, sizeof(int))");
        graceful_death(EXIT_FAILURE);
    }

    /* bind async_broker to physical address */
    retval = zmq_bind(async_broker, "tcp://*:5556");
    if (-1 == retval) {
        perror("zmq_bind(async_broker, \"tcp://*:5556\")");
        graceful_death(EXIT_FAILURE);
    }

    /* create socket for sending kill message to simulations */
    killer = zmq_socket(zmq_ctx, ZMQ_PUB);
    if (NULL == killer) {
        perror("zmq_socket(zmq_ctx, ZMQ_PUB)");
        graceful_death(EXIT_FAILURE);
    }

    /* bind killer to physical address */
    retval = zmq_bind(killer, "tcp://*:5557");
    if (-1 == retval) {
        perror("zmq_bind(killer, \"tcp://*:5557\")");
        graceful_death(EXIT_FAILURE);
    }

    s_catch_signals();

    while (1) {
        string identity; // simulation identifier
        int context; // which group of sims?
        string control; // control field

        zmq_pollitem_t items[] = {
            { broker,       0, ZMQ_POLLIN, 0 },
            { async_broker, 0, ZMQ_POLLIN, 0 },
        };

        int rc = zmq_poll(items, 2, -1);
        if (s_interrupted) {
            graceful_death(EXIT_FAILURE);
        }
        else {
            assert(rc >= 0);
        }

        if (items[0].revents & ZMQ_POLLIN) {
            (void) s_recv(broker, identity);
            (void) s_recv(broker, context);
            (void) s_recv(broker, control);
#if DEBUG
            CERR << "broker recv - identity '" << identity
                << "' context '" << context
                << "' control '" << control
                << "'" << endl;
#endif
            if (context < 0
                    || (context >= contexts.size() && !contexts.empty())) {
                cerr << "broker received invalid context" << endl;
                graceful_death(EXIT_FAILURE);
            }
            if ("HELLO" == control || "HELLO_NETSIM" == control) {
                hello_handler(identity, context, control);
            }
            else if ("REDUCE_MIN_TIME" == control) {
                reduce_min_time_handler(identity, context, control);
            }
            else if ("REDUCE_SEND_RECV" == control) {
                reduce_send_recv_handler(identity, context, control);
            }
            else if ("REGISTER_OBJECT" == control) {
                register_handler(identity, context, control);
            }
            else if ("FINALIZE_REGISTRATIONS" == control) {
                finalize_handler(identity, context, control);
            }
            else if ("BARRIER" == control) {
                barrier_handler(identity, context, control);
            }
            else if ("DIE" == control) {
                /* a simulation wants to terminate abrubtly */
                graceful_death(EXIT_FAILURE);
            }
            else if ("FINISHED" == control) {
                if (finished_handler(identity, context, control)) {
                    break;
                }
            }
            else {
                cerr << "unrecognized control header '"
                    << control << "'" << endl;
                graceful_death(EXIT_FAILURE);
            }
        }
        if (items[1].revents & ZMQ_POLLIN) {
            (void) s_recv(async_broker, identity);
            (void) s_recv(async_broker, context);
            (void) s_recv(async_broker, control);
#if DEBUG
            CERR << "async_broker recv - identity '" << identity
                << "' context '" << context
                << "' control '" << control
                << "'" << endl;
#endif
            if (context < 0
                    || (context >= contexts.size() && !contexts.empty())) {
#if DEBUG
                CERR << "async_broker received invalid context" << endl;
#endif
                graceful_death(EXIT_FAILURE);
            }
            if ("ROUTE" == control) {
                route_handler(identity, context, control);
            }
            else if ("DELAY" == control) {
                delay_handler(identity, context, control);
            }
            else {
#if DEBUG
                CERR << "unrecognized control header '"
                    << control << "'" << endl;
#endif
                graceful_death(EXIT_FAILURE);
            }
        }
    }

    graceful_death(EXIT_SUCCESS);
}


static int add_context()
{
    size_t size;

    netSimID.push_back(newNetSimID);
    netSimObjCount.push_back(0);
    contexts.push_back(newConnections);
    size = contexts.size();

    obj_to_ID.resize(size);
    reduce_min_time.resize(size);
    reduce_sent.resize(size);
    reduce_recv.resize(size);
    finalized.resize(size);
    barrier.resize(size);
    finished.resize(size);

    return int(size-1);
}


static bool starts_with(const string &str, const string &prefix)
{
    return str.substr(0, prefix.size()) == prefix;
}


static void graceful_death(int exit_code)
{
    int retval;

    /* send term message to all sims */
    if (killer) {
        (void) s_send(killer, "DIE");
    }

    /* clean up killer */
    if (killer) {
        retval = zmq_close(killer);
        if (-1 == retval) {
            perror("zmq_close(killer)");
            exit_code = EXIT_FAILURE;
        }
    }

    /* clean up async_broker */
    if (async_broker) {
        retval = zmq_close(async_broker);
        if (-1 == retval) {
            perror("zmq_close(async_broker)");
            exit_code = EXIT_FAILURE;
        }
    }

    /* clean up broker */
    if (broker) {
        retval = zmq_close(broker);
        if (-1 == retval) {
            perror("zmq_close(broker)");
            exit_code = EXIT_FAILURE;
        }
    }

    /* clean up context */
    if (zmq_ctx) {
        retval = zmq_ctx_destroy(zmq_ctx);
        if (-1 == retval) {
            perror("zmq_ctx_destroy");
            exit_code = EXIT_FAILURE;
        }
    }

#if DEBUG && DEBUG_TO_FILE
    echo.close();
#endif

    exit(exit_code);
}


/**
 * a sim is notifying it wants to connect
 * we wait until all sims connect before replying
 */
static void hello_handler(
        const string &identity,
        const int &context,
        const string &control)
{
    newConnections.insert(identity);
    if ("HELLO_NETSIM" == control) {
        if (newNetSimID.empty()) {
            newNetSimID = identity;
        }
        else {
            cerr << "a net sim is already connected" << endl;
            graceful_death(EXIT_FAILURE);
        }
    }
    if (newConnections.size() > world_size) {
        graceful_death(EXIT_FAILURE);
    }
    else if (newConnections.size() == world_size) {
        int contextID = add_context();
        for (set<string>::iterator it=newConnections.begin();
                it != newConnections.end(); ++it) {
            (void) s_sendmore(broker, *it);
            (void) s_send(broker, contextID);
        }
        /* prep for next group of connections */
        newConnections.clear();
        newNetSimID.clear();
    }
}


/**
 * net sim finished routing message;
 * needs to route payload to dest sim
 */
static void route_handler(
        const string &identity,
        const int &context,
        const string &control)
{
    uint8_t *envelope;
    uint32_t envelopeSize;
    uint8_t *data;
    uint32_t dataSize;
    Message *message;

    envelopeSize = s_recv(async_broker, envelope, 256);
    message = new Message(envelope, envelopeSize);
    dataSize = message->getSize();
    if (dataSize > 0) {
        data = new uint8_t[dataSize];
        (void) s_recv(async_broker, data, dataSize);
        message->setData(data, dataSize);
    }

    if (0 == obj_to_ID[context].count(message->getTo())) {
        cerr << "object '" << message->getTo()
            << "' not registered" << endl;
        graceful_death(EXIT_FAILURE);
    }
    string object_owner = obj_to_ID[context][message->getTo()];

    (void) s_sendmore(async_broker, object_owner);
    (void) s_sendmore(async_broker, "ROUTE");
    if (dataSize > 0) {
        (void) s_sendmore(async_broker, envelope, envelopeSize);
        (void) s_send    (async_broker, data, dataSize);
    }
    else {
        (void) s_send    (async_broker, envelope, envelopeSize);
    }

    delete message;
    delete [] envelope;
    if (dataSize > 0) {
        delete [] data;
    }
}


/** a sim needs to route message through net sim */
static void delay_handler(
        const string &identity,
        const int &context,
        const string &control)
{
    uint8_t *envelope;
    uint32_t envelopeSize;
    uint8_t *data;
    uint32_t dataSize;
    Message *message;

    envelopeSize = s_recv(async_broker, envelope, 256);
#if DEBUG
    CERR << "envelopeSize=" << envelopeSize << endl;
#endif
    message = new Message(envelope, envelopeSize);
#if DEBUG
    CERR << "async_broker - recv message: " << *message << endl;
#endif
    dataSize = message->getSize();
    if (dataSize > 0) {
        dataSize = s_recv(async_broker, data, dataSize);
        message->setData(data, dataSize);
    }

    (void) s_sendmore(async_broker, netSimID[context]);
    (void) s_sendmore(async_broker, "DELAY");
    if (dataSize > 0) {
        (void) s_sendmore(async_broker, envelope, envelopeSize);
        (void) s_send    (async_broker, data, dataSize);
    }
    else {
        (void) s_send    (async_broker, envelope, envelopeSize);
    }

    delete message;
    delete [] envelope;
    if (dataSize > 0) {
        delete [] data;
    }
}


static void reduce_min_time_handler(
        const string &identity,
        const int &context,
        const string &control)
{
    unsigned long time;
    if (1 == reduce_min_time[context].count(identity)) {
        cerr << "sim with ID '" << identity
            << "' duplicate REDUCE_MIN_TIME" << endl;
        graceful_death(EXIT_FAILURE);
    }
    (void) s_recv(broker, time);
    reduce_min_time[context][identity] = time;
    if (reduce_min_time[context].size() == world_size) {
        ReducePair min = *min_element(reduce_min_time[context].begin(),
                reduce_min_time[context].end(), ReduceMinPairLess());
        /* send result to all sims */
        for (set<string>::iterator it=contexts[context].begin();
                it != contexts[context].end(); ++it) {
            (void) s_sendmore(broker, *it);
            (void) s_send(broker, min.second);
        }
        /* clear the map in preparation for next round */
        reduce_min_time[context].clear();
    }
}


static void reduce_send_recv_handler(
        const string &identity,
        const int &context,
        const string &control)
{
    unsigned long sent = 0;
    unsigned long recv = 0;
    unsigned long m_sent = 0;
    unsigned long m_recv = 0;
    if (1 == reduce_sent[context].count(identity)) {
        assert(1 == reduce_recv[context].count(identity));
        cerr << "sim with ID '" << identity
            << "' duplicate REDUCE_SEND_RECV" << endl;
        graceful_death(EXIT_FAILURE);
    }
    (void) s_recv(broker, sent);
    (void) s_recv(broker, recv);
    reduce_sent[context][identity] = sent;
    reduce_recv[context][identity] = recv;
    if (reduce_sent[context].size() == world_size) {
        assert(reduce_recv[context].size() == world_size);
        ReduceMap::const_iterator it;
        for (it=reduce_sent[context].begin();
                it!=reduce_sent[context].end(); ++it) {
            m_sent += it->second;
        }
        for (it=reduce_recv[context].begin();
                it!=reduce_recv[context].end(); ++it) {
            m_recv += it->second;
        }
        /* send result to all sims */
        for (set<string>::iterator it=contexts[context].begin();
                it != contexts[context].end(); ++it) {
            (void) s_sendmore(broker, *it);
            (void) s_sendmore(broker, m_sent);
            (void) s_send    (broker, m_recv);
        }
        /* clear the map in preparation for next round */
        reduce_sent[context].clear();
        reduce_recv[context].clear();
    }
}


static void register_handler(
        const string &identity,
        const int &context,
        const string &control)
{
    string name;
    (void) s_recv(broker, name);
    /* don't register net sim objects, but keep track of how many */
    if (identity == netSimID[context]) {
        netSimObjCount[context]++;
        return;
    }
    if (1 == obj_to_ID[context].count(name)) {
        cerr << "object '" << name << "' already registered to sim '"
            << identity << "'" << endl;
        graceful_death(EXIT_FAILURE);
    }
    obj_to_ID[context][name] = identity;
}



static void finalize_handler(
        const string &identity,
        const int &context,
        const string &control)
{
    finalized[context].insert(identity);
    if (finalized[context].size() > world_size) {
        graceful_death(EXIT_FAILURE);
    }
    else if (finalized[context].size() == world_size) {
        for (set<string>::iterator it=finalized[context].begin();
                it != finalized[context].end(); ++it) {
            (void) s_sendmore(broker, *it);
            /* some objects aren't network enabled, so we send back only the
             * number of network registered objects */
            /*(void) s_send    (broker, obj_to_ID[context].size());*/
            (void) s_send    (broker, netSimObjCount[context]);
        }
    }
}


static void barrier_handler(
        const string &identity,
        const int &context,
        const string &control)
{
    if (1 == barrier[context].count(identity)) {
        cerr << "barrier already initialized for sim '"
            << identity << "'" << endl;
        graceful_death(EXIT_FAILURE);
    }
    barrier[context].insert(identity);
    if (barrier[context].size() > world_size) {
        cerr << "barrier size > world size" << endl;
        graceful_death(EXIT_FAILURE);
    }
    else if (barrier[context].size() == world_size) {
        for (set<string>::iterator it=barrier[context].begin();
                it != barrier[context].end(); ++it) {
            (void) s_sendmore(broker, *it);
            (void) s_send    (broker, "ACK");
        }
        barrier[context].clear();
    }
}


static bool finished_handler(
        const string &identity,
        const int &context,
        const string &control)
{
    /* a simulation wants to terminate nicely */
    /* publish term message to all sims */
    if (1 == finished[context].count(identity)) {
#if DEBUG
        CERR << "additional FINISHED received from '" << identity
            << "'" << endl;
#endif
    }
    finished[context].insert(identity);
    if (finished[context].size() > world_size) {
        cerr << "finished size > world size" << endl;
        graceful_death(EXIT_FAILURE);
    }
    else if (finished[context].size() == world_size) {
        return true;
    }
    (void) s_send(killer, "FINISHED");

    return false;
}
