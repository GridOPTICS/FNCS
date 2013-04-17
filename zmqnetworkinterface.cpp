#include "config.h"

#include <zmq.h>

#include <cassert>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <list>
#include <string>

#include "message.h"
#include "zmqhelper.h"
#include "zmqnetworkinterface.h"

using namespace std;
using namespace sim_comm;


ZmqNetworkInterface::ZmqNetworkInterface(bool iAmNetSim)
    :   AbsNetworkInterface()
    ,   zmq_ctx(NULL)
    ,   zmq_req(NULL)
    ,   ID()
    ,   iAmNetSim(iAmNetSim)
    ,   receivedMessages()
    ,   globalObjectCount(0)
{
    int zmq_connect_req_retval = 0;
    int zmq_connect_sub_retval = 0;
    int zmq_setsockopt_id_retval = 0;
    int zmq_setsockopt_retval = 0;

#if DEBUG
    CERR << "ZmqNetworkInterface::ZmqNetworkInterface()" << endl;
#endif

    srandom(unsigned(time(NULL)));
    this->ID = gen_id();
    cout << "ZmqNetworkInterface ID=" << this->ID << endl;

    /* create zmq context */
    this->zmq_ctx = zmq_ctx_new();
    assert(this->zmq_ctx);

    /* create and connect zmq socket for req/res to broker */
    this->zmq_req = zmq_socket(this->zmq_ctx, ZMQ_DEALER);
    assert(this->zmq_req);
    zmq_setsockopt_id_retval = zmq_setsockopt(
            this->zmq_req, ZMQ_IDENTITY, this->ID.data(), this->ID.size());
    if (0 != zmq_setsockopt_id_retval) {
        perror("zmq_setsockopt ZMQ_IDENTITY");
    }
    assert(0 == zmq_setsockopt_id_retval);
    zmq_connect_req_retval = zmq_connect(this->zmq_req, "tcp://localhost:5555");
    if (0 != zmq_connect_req_retval) {
        perror("zmq_connect ZMQ_DEALER");
    }
    assert(0 == zmq_connect_req_retval);

    /* create and connect zmq socket for kill signal */
    this->zmq_die = zmq_socket(this->zmq_ctx, ZMQ_SUB);
    assert(this->zmq_die);
    zmq_setsockopt_retval = zmq_setsockopt(this->zmq_die, ZMQ_SUBSCRIBE, "", 0);
    if (0 != zmq_setsockopt_retval) {
        perror("zmq_setsockopt ZMQ_SUBSCRIBE");
    }
    assert(0 == zmq_setsockopt_retval);
    zmq_setsockopt_id_retval = zmq_setsockopt(
            this->zmq_die, ZMQ_IDENTITY, this->ID.data(), this->ID.size());
    if (0 != zmq_setsockopt_id_retval) {
        perror("zmq_setsockopt ZMQ_IDENTITY");
    }
    assert(0 == zmq_setsockopt_id_retval);
    zmq_connect_sub_retval = zmq_connect(this->zmq_die, "tcp://localhost:5556");
    if (0 != zmq_connect_sub_retval) {
        perror("zmq_connect ZMQ_SUB");
    }
    assert(0 == zmq_connect_sub_retval);

    /* send hello to broker */
    if (iAmNetSim) {
        (void) s_send(this->zmq_req, "HELLO_NETSIM");
    }
    else {
        (void) s_send(this->zmq_req, "HELLO");
    }

    /* get ack from broker */
    string ack;
    (void) s_recv(this->zmq_req, ack);
    assert(ack == "ACK");

    cout << this->ID << " ACK" << endl;
}


ZmqNetworkInterface::~ZmqNetworkInterface()
{
    int zmq_close_retval = 0;
    int zmq_ctx_destroy_retval = 0;

#if DEBUG
    CERR << "ZmqNetworkInterface::~ZmqNetworkInterface()" << endl;
#endif

    makeProgress();

    zmq_close_retval = zmq_close(this->zmq_req);
    assert(0 == zmq_close_retval);

    zmq_close_retval = zmq_close(this->zmq_die);
    assert(0 == zmq_close_retval);

    zmq_ctx_destroy_retval = zmq_ctx_destroy(this->zmq_ctx);
    assert(0 == zmq_ctx_destroy_retval);
}


void ZmqNetworkInterface::send(Message *message)
{
    const uint8_t *data = message->getData();
    uint32_t dataSize = message->getSize();
    uint8_t *envelop = NULL;
    uint32_t envelopSize = 0;

#if DEBUG
    CERR << "ZmqNetworkInterface::send()" << endl;
#endif

    makeProgress();

    message->serializeHeader(envelop,envelopSize);

    if (iAmNetSim) {
        (void) s_sendmore(this->zmq_req, "ROUTE");
    }
    else {
        (void) s_sendmore(this->zmq_req, "DELAY");
    }
    (void) s_sendmore(this->zmq_req, envelop, envelopSize);
    (void) s_send(this->zmq_req, const_cast<uint8_t*>(data), dataSize);
}


uint64_t ZmqNetworkInterface::broadcast(Message *message)
{
#if DEBUG
    CERR << "ZmqNetworkInterface::broadcast()" << endl;
#endif
    if (iAmNetSim) {
        NETWORK_EXCEPTION("network simulator should not broadcast");
    }

    message->setTo(Message::DESTIONATION_BCAST);
    send(message);

    return this->globalObjectCount;
}


Message* ZmqNetworkInterface::receive()
{
    Message *message = NULL;

#if DEBUG
    CERR << "ZmqNetworkInterface::receive()" << endl;
#endif

    makeProgress();

    if (!receivedMessages.empty()) {
        message = receivedMessages.front();
        receivedMessages.pop_front();
    }

    return message;
}


vector<Message*> ZmqNetworkInterface::receiveAll()
{
    vector<Message*> messages;

#if DEBUG
    CERR << "ZmqNetworkInterface::receiveAll()" << endl;
#endif
    makeProgress();

    messages.reserve(receivedMessages.size());
    messages.assign(receivedMessages.begin(), receivedMessages.end());
    receivedMessages.clear();

    return messages;
}


uint64_t ZmqNetworkInterface::reduceMinTime(uint64_t myTime)
{
    unsigned long _myTime = static_cast<unsigned long>(myTime);
    unsigned long retval;

#if DEBUG
    CERR << "ZmqNetworkInterface::reduceMinTime()" << endl;
#endif
    makeProgress();

    (void) s_sendmore(this->zmq_req, "REDUCE_MIN_TIME");
    (void) s_send(this->zmq_req, &_myTime, sizeof(unsigned long));
    (void) s_recv(this->zmq_req, retval);

    return retval;
}


uint64_t ZmqNetworkInterface::reduceTotalSendReceive(
        uint64_t sent, uint64_t received)
{
    unsigned long m_sent;
    unsigned long m_recv;

#if DEBUG
    CERR << "ZmqNetworkInterface::reduceTotalSendReceive()" << endl;
#endif
    makeProgress();

    (void) s_sendmore(this->zmq_req, "REDUCE_SEND_RECV");
    (void) s_sendmore(this->zmq_req, sent);
    (void) s_send    (this->zmq_req, received);
    (void) s_recv(this->zmq_req, m_sent);
    (void) s_recv(this->zmq_req, m_recv);
    assert(m_sent >= m_recv);

    return static_cast<uint64_t>(m_sent - m_recv);
}


void ZmqNetworkInterface::registerObject(string name)
{
#if DEBUG
    CERR << "ZmqNetworkInterface::registerObject()" << endl;
#endif
    AbsNetworkInterface::registerObject(name);

    (void) s_sendmore(this->zmq_req, "REGISTER_OBJECT");
    (void) s_send    (this->zmq_req, name);
}


void ZmqNetworkInterface::finalizeRegistrations()
{
#if DEBUG
    CERR << "ZmqNetworkInterface::finalizeRegistrations()" << endl;
#endif
    AbsNetworkInterface::finalizeRegistrations();

    (void) s_send(this->zmq_req, "FINALIZE_REGISTRATIONS");
    (void) s_recv(this->zmq_req, this->globalObjectCount);
}


void ZmqNetworkInterface::barier()
{
    string ack;

#if DEBUG
    CERR << "ZmqNetworkInterface::barier()" << endl;
#endif

    (void) s_send(this->zmq_req, "BARRIER");
    (void) s_recv(this->zmq_req, ack);
    assert(ack == "ACK");
}


AbsNetworkInterface* ZmqNetworkInterface::duplicateInterface()
{
#if DEBUG
    CERR << "ZmqNetworkInterface::duplicateInterface()" << endl;
#endif
    /* TODO: the clone needs to create its own sockets */
    assert(0);
    ZmqNetworkInterface *toReturn=new ZmqNetworkInterface(*this);
    return toReturn;
}


void ZmqNetworkInterface::makeProgress()
{
    bool incoming = true;

#if DEBUG
    CERR << "ZmqNetworkInterface::makeProgress()" << endl;
#endif

    if (isAcceptingRegistrations()) {
        NETWORK_EXCEPTION("object registration must first be finalized");
    }

    while (incoming) {
#if DEBUG
        CERR << "testing for incoming messages" << endl;
#endif
        zmq_pollitem_t items[1];
        items[0].socket = this->zmq_req;
        items[0].events = ZMQ_POLLIN;
        int rc = zmq_poll(items, 1, 0);
        assert(rc >= 0);
        if (items[0].revents & ZMQ_POLLIN) {
            string control;
            uint8_t *envelope;
            uint32_t envelopeSize;
            uint8_t *data;
            uint32_t dataSize;
            Message *message;

            (void) s_recv(this->zmq_req, control);
            if ("ROUTE" != control) {
                cerr << "invalid control message" << endl;
                (void) s_send(this->zmq_req, "DIE");
            }
            envelopeSize = s_recv(this->zmq_req, envelope, 256);
            message = new Message(envelope,envelopeSize);
            dataSize = message->getSize();
            if (dataSize > 0) {
                data = new uint8_t[dataSize];
                (void) s_recv(this->zmq_req, data, dataSize);
                message->setData(data,dataSize);
            }

            if (this->messageCallBack) {
                (*(this->messageCallBack))(message);
            }
            else {
                this->receivedMessages.push_back(message);
            }
        }
        else {
            incoming = false;
        }
    }
}
