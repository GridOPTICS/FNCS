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

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <zmq.h>

#include "integrator.h"
#include "message.h"
#include "zmqhelper.h"

using namespace std;
using namespace sim_comm;

map<string,string> obj_to_ID;
map<string,unsigned long> m_reduce_min_time;
map<string,unsigned long> m_reduce_sent;
map<string,unsigned long> m_reduce_recv;
set<string> connections;
set<string> finalized;
set<string> barrier;
set<string> finished;
string netSimID;
void *zmq_ctx = NULL;
void *broker = NULL;
void *killer = NULL;
int world_size = 0;

typedef map<string,unsigned long> ReduceMap;
typedef pair<string,unsigned long> ReducePair;

struct ReduceMinPairLess
{
    bool operator()(const ReducePair& left, const ReducePair& right) const
    {
        return left.second < right.second;
    }
};

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
        CERR << "missing world_size argument" << endl;
        exit(EXIT_FAILURE);
    }
    else if (3 <= argc) {
        CERR << "too many arguments" << endl;
        exit(EXIT_FAILURE);
    }
    world_size = atoi(argv[1]);
    if (world_size <= 1) {
        CERR << "world size must be > 1" << endl;
        exit(EXIT_FAILURE);
    }

    zmq_ctx = zmq_ctx_new();
    if (NULL == zmq_ctx) {
        perror("zmq_ctx_new");
        graceful_death(EXIT_FAILURE);
    }

    /* create socket for receiving messages from simulations */
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

    /* create socket for sending kill message to simulations */
    killer = zmq_socket(zmq_ctx, ZMQ_PUB);
    if (NULL == killer) {
        perror("zmq_socket(zmq_ctx, ZMQ_PUB)");
        graceful_death(EXIT_FAILURE);
    }

    /* bind killer to physical address */
    retval = zmq_bind(killer, "tcp://*:5556");
    if (-1 == retval) {
        perror("zmq_bind(killer, \"tcp://*:5556\")");
        graceful_death(EXIT_FAILURE);
    }

    while (1) {
        string identity; // simulation identifier
        string control; // control field

        (void) s_recv(broker, identity);
        (void) s_recv(broker, control);
        CERR << "broker recv - identity '" << identity << "' control '"
            << control << "'" << endl;

        if ("HELLO" == control || "HELLO_NETSIM" == control) {
            /* a sim is notifying it wants to connect */
            /* we wait until all sims connect before replying */
            connections.insert(identity);
            if ("HELLO_NETSIM" == control) {
                if (netSimID.empty()) {
                    netSimID = identity;
                }
                else {
                    CERR << "a net sim is already connected" << endl;
                    graceful_death(EXIT_FAILURE);
                }
            }
            if (connections.size() > world_size) {
                graceful_death(EXIT_FAILURE);
            }
            else if (connections.size() == world_size) {
                for (set<string>::iterator it=connections.begin();
                        it != connections.end(); ++it) {
                    (void) s_sendmore(broker, *it);
                    (void) s_send(broker, "ACK");
                }
            }
        }
        else if ("ROUTE" == control) {
            /* net sim finished routing message;
             * needs to route payload to dest sim */
            uint8_t *envelope;
            uint32_t envelopeSize;
            uint8_t *data;
            uint32_t dataSize;
            Message *message;

            envelopeSize = s_recv(broker, envelope, 256);
            message = new Message(envelope, envelopeSize);
            dataSize = message->getSize();
            if (dataSize > 0) {
                data = new uint8_t[dataSize];
                (void) s_recv(broker, data, dataSize);
                message->setData(data, dataSize);
            }

            if (0 == obj_to_ID.count(message->getTo())) {
                CERR << "object '" << message->getTo()
                    << "' not registered" << endl;
                graceful_death(EXIT_FAILURE);
            }
            string identity = obj_to_ID[message->getTo()];

            (void) s_sendmore(broker, identity);
            (void) s_sendmore(broker, "ROUTE");
            if (dataSize > 0) {
                (void) s_sendmore(broker, envelope, envelopeSize);
                (void) s_send    (broker, data, dataSize);
            }
            else {
                (void) s_send    (broker, envelope, envelopeSize);
            }
        }
        else if ("DELAY" == control) {
            /* a sim needs to route message through net sim */
            uint8_t *envelope;
            uint32_t envelopeSize;
            uint8_t *data;
            uint32_t dataSize;
            Message *message;

            envelopeSize = s_recv(broker, envelope, 256);
            message = new Message(envelope, envelopeSize);
            CERR << "broker - recv message: " << *message << endl;
            dataSize = message->getSize();
            if (dataSize > 0) {
                data = new uint8_t[dataSize];
                (void) s_recv(broker, data, dataSize);
                message->setData(data, dataSize);
            }

            (void) s_sendmore(broker, netSimID);
            (void) s_sendmore(broker, "DELAY");
            if (dataSize > 0) {
                (void) s_sendmore(broker, envelope, envelopeSize);
                (void) s_send    (broker, data, dataSize);
            }
            else {
                (void) s_send    (broker, envelope, envelopeSize);
            }
        }
        else if ("REDUCE_MIN_TIME" == control) {
            unsigned long time;
            if (1 == m_reduce_min_time.count(identity)) {
                CERR << "sim with ID '" << identity
                    << "' duplicate REDUCE_MIN_TIME" << endl;
                graceful_death(EXIT_FAILURE);
            }
            (void) s_recv(broker, time);
            m_reduce_min_time[identity] = time;
            if (m_reduce_min_time.size() == world_size) {
                ReducePair min = *min_element(m_reduce_min_time.begin(),
                        m_reduce_min_time.end(), ReduceMinPairLess());
                /* send result to all sims */
                for (set<string>::iterator it=connections.begin();
                        it != connections.end(); ++it) {
                    (void) s_sendmore(broker, *it);
                    (void) s_send(broker, min.second);
                }
                /* clear the map in preparation for next round */
                m_reduce_min_time.clear();
            }
        }
        else if ("REDUCE_SEND_RECV" == control) {
            unsigned long sent = 0;
            unsigned long recv = 0;
            unsigned long m_sent = 0;
            unsigned long m_recv = 0;
            if (1 == m_reduce_sent.count(identity)) {
                assert(1 == m_reduce_recv.count(identity));
                CERR << "sim with ID '" << identity
                    << "' duplicate REDUCE_SEND_RECV" << endl;
                graceful_death(EXIT_FAILURE);
            }
            (void) s_recv(broker, sent);
            (void) s_recv(broker, recv);
            m_reduce_sent[identity] = sent;
            m_reduce_recv[identity] = recv;
            if (m_reduce_sent.size() == world_size) {
                assert(m_reduce_recv.size() == world_size);
                ReduceMap::const_iterator it;
                for (it=m_reduce_sent.begin(); it!=m_reduce_sent.end(); ++it) {
                    m_sent += it->second;
                }
                for (it=m_reduce_recv.begin(); it!=m_reduce_recv.end(); ++it) {
                    m_recv += it->second;
                }
                /* send result to all sims */
                for (set<string>::iterator it=connections.begin();
                        it != connections.end(); ++it) {
                    (void) s_sendmore(broker, *it);
                    (void) s_sendmore(broker, m_sent);
                    (void) s_send    (broker, m_recv);
                }
                /* clear the map in preparation for next round */
                m_reduce_sent.clear();
                m_reduce_recv.clear();
            }
        }
        else if ("REGISTER_OBJECT" == control) {
            string name;
            (void) s_recv(broker, name);
            /* don't register net sim objects */
            if (identity == netSimID) {
                continue;
            }
            if (1 == obj_to_ID.count(name)) {
                CERR << "object '" << name << "' already registered to sim '"
                    << identity << "'" << endl;
                graceful_death(EXIT_FAILURE);
            }
            obj_to_ID[name] = identity;
        }
        else if ("FINALIZE_REGISTRATIONS" == control) {
            finalized.insert(identity);
            if (finalized.size() > world_size) {
                graceful_death(EXIT_FAILURE);
            }
            else if (finalized.size() == world_size) {
                for (set<string>::iterator it=finalized.begin();
                        it != finalized.end(); ++it) {
                    (void) s_sendmore(broker, *it);
                    (void) s_send    (broker, obj_to_ID.size());
                }
            }
        }
        else if ("BARRIER" == control) {
            if (1 == barrier.count(identity)) {
                CERR << "barrier already initialized for sim '"
                    << identity << "'" << endl;
                graceful_death(EXIT_FAILURE);
            }
            barrier.insert(identity);
            if (barrier.size() > world_size) {
                CERR << "barrier size > world size" << endl;
                graceful_death(EXIT_FAILURE);
            }
            else if (barrier.size() == world_size) {
                for (set<string>::iterator it=barrier.begin();
                        it != barrier.end(); ++it) {
                    (void) s_sendmore(broker, *it);
                    (void) s_send    (broker, "ACK");
                }
                barrier.clear();
            }
        }
        else if ("DIE" == control) {
            /* a simulation wants to terminate abrubtly */
            graceful_death(EXIT_FAILURE);
        }
        else if ("FINISHED" == control) {
            /* a simulation wants to terminate nicely */
            /* publish term message to all sims */
            if (1 == finished.count(identity)) {
                CERR << "additional FINISHED received from '" << identity
                    << "'" << endl;
            }
            finished.insert(identity);
            if (finished.size() > world_size) {
                CERR << "finished size > world size" << endl;
                graceful_death(EXIT_FAILURE);
            }
            else if (finished.size() == world_size) {
                break;
            }
            (void) s_send(killer, "FINISHED");
        }
        else {
            CERR << "unrecognized control header '" << control << "'" << endl;
            graceful_death(EXIT_FAILURE);
        }
    }

    graceful_death(EXIT_SUCCESS);
}

