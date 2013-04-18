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


void ZmqNetworkInterface::init()
{
    int zmq_connect_req_retval = 0;
    int zmq_connect_sub_retval = 0;
    int zmq_setsockopt_id_retval = 0;
    int zmq_setsockopt_retval = 0;

#if DEBUG
    CERR << "ZmqNetworkInterface::init()" << endl;
#endif

    this->ID = gen_id();
    CERR << "ZmqNetworkInterface ID=" << this->ID << endl;

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

    /* create and connect zmq socket for req/res to async broker */
    this->zmq_async = zmq_socket(this->zmq_ctx, ZMQ_DEALER);
    assert(this->zmq_async);
    zmq_setsockopt_id_retval = zmq_setsockopt(
            this->zmq_async, ZMQ_IDENTITY, this->ID.data(), this->ID.size());
    if (0 != zmq_setsockopt_id_retval) {
        perror("zmq_setsockopt ZMQ_IDENTITY");
    }
    assert(0 == zmq_setsockopt_id_retval);
    zmq_connect_req_retval = zmq_connect(this->zmq_async, "tcp://localhost:5556");
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
    zmq_connect_sub_retval = zmq_connect(this->zmq_die, "tcp://localhost:5557");
    if (0 != zmq_connect_sub_retval) {
        perror("zmq_connect ZMQ_SUB");
    }
    assert(0 == zmq_connect_sub_retval);

    /* send hello to broker */
    int ZERO = 0;
    if (this->iAmNetSim) {
        (void) s_send(this->zmq_req, ZERO, "HELLO_NETSIM");
    }
    else {
        (void) s_send(this->zmq_req, ZERO, "HELLO");
    }

    /* get ack from broker */
    (void) i_recv(this->context);
    assert(this->context >= 0);

    CERR << this->ID << " context=" << this->context << endl;
}


ZmqNetworkInterface::ZmqNetworkInterface(bool iAmNetSim)
    :   AbsNetworkInterface()
    ,   zmq_ctx(NULL)
    ,   zmq_req(NULL)
    ,   zmq_async(NULL)
    ,   ID()
    ,   iAmNetSim(iAmNetSim)
    ,   receivedMessages()
    ,   globalObjectCount(0)
{
    init();
}


ZmqNetworkInterface::ZmqNetworkInterface(const ZmqNetworkInterface &that)
    :   AbsNetworkInterface()
    ,   zmq_ctx(NULL)
    ,   zmq_req(NULL)
    ,   zmq_async(NULL)
    ,   ID()
    ,   iAmNetSim(that.iAmNetSim)
    ,   receivedMessages()
    ,   globalObjectCount(that.globalObjectCount)
{
    int globalObjectCountAgain;

    init();

    for (vector<string>::const_iterator it=myObjects.begin();
            it!=myObjects.end(); ++it) {
        (void) s_send(this->zmq_req, this->context, "REGISTER_OBJECT", *it);
    }

    (void) s_send(this->zmq_req, this->context, "FINALIZE_REGISTRATIONS");
    (void) i_recv(globalObjectCountAgain);

    assert(globalObjectCountAgain == this->globalObjectCount);
}


ZmqNetworkInterface::~ZmqNetworkInterface()
{
    cleanup();
}


void ZmqNetworkInterface::send(Message *message)
{
    const uint8_t *data = message->getData();
    uint32_t dataSize = message->getSize();
    uint8_t *envelope = NULL;
    uint32_t envelopeSize = 0;

#if DEBUG
    CERR << "ZmqNetworkInterface::send(message)" << endl;
    CERR << *message << endl;
#endif

    makeProgress();

    message->serializeHeader(envelope,envelopeSize);
    CERR << "envelopeSize=" << envelopeSize << endl;

    if (iAmNetSim) {
        (void) s_sendmore(this->zmq_async, this->context, "ROUTE");
    }
    else {
        (void) s_sendmore(this->zmq_async, this->context, "DELAY");
    }
    if (dataSize > 0) {
        (void) s_sendmore(this->zmq_async, envelope, envelopeSize);
        (void) s_send    (this->zmq_async, const_cast<uint8_t*>(data), dataSize);
    }
    else {
        (void) s_send    (this->zmq_async, envelope, envelopeSize);
    }

    delete [] envelope;
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
    CERR << "ZmqNetworkInterface::reduceMinTime(" << myTime << ")" << endl;
#endif
    makeProgress();

    (void) s_sendmore(this->zmq_req, this->context, "REDUCE_MIN_TIME");
    (void) s_send(this->zmq_req, _myTime);
    (void) i_recv(retval);

#if DEBUG
    CERR << "\treceived " << retval << endl;
#endif

    return retval;
}


uint64_t ZmqNetworkInterface::reduceTotalSendReceive(
        uint64_t sent, uint64_t received)
{
    unsigned long m_sent;
    unsigned long m_recv;

#if DEBUG
    CERR << "ZmqNetworkInterface::reduceTotalSendReceive("
        << sent << "," << received << ")" << endl;
#endif
    makeProgress();

    (void) s_sendmore(this->zmq_req, this->context, "REDUCE_SEND_RECV");
    (void) s_sendmore(this->zmq_req, sent);
    (void) s_send    (this->zmq_req, received);
    (void) i_recv(m_sent);
    (void) i_recv(m_recv);

#if DEBUG
    CERR << "\treceived m_sent=" << m_sent
         << " m_recv=" << m_recv << endl;
#endif

    if (m_sent < m_recv) {
        (void) s_send(this->zmq_req, this->context, "DIE");
    }

    return static_cast<uint64_t>(m_sent - m_recv);
}


void ZmqNetworkInterface::registerObject(string name)
{
#if DEBUG
    CERR << "ZmqNetworkInterface::registerObject()" << endl;
#endif
    AbsNetworkInterface::registerObject(name);

    (void) s_send(this->zmq_req, this->context, "REGISTER_OBJECT", name);
}


void ZmqNetworkInterface::finalizeRegistrations()
{
#if DEBUG
    CERR << "ZmqNetworkInterface::finalizeRegistrations()" << endl;
#endif
    AbsNetworkInterface::finalizeRegistrations();

    (void) s_send(this->zmq_req, this->context, "FINALIZE_REGISTRATIONS");
    (void) i_recv(this->globalObjectCount);
}


void ZmqNetworkInterface::barier()
{
    string ack;

#if DEBUG
    CERR << "ZmqNetworkInterface::barier()" << endl;
#endif

    (void) s_send(this->zmq_req, this->context, "BARRIER");
    (void) i_recv(ack);
    assert(ack == "ACK");
}


AbsNetworkInterface* ZmqNetworkInterface::duplicateInterface()
{
#if DEBUG
    CERR << "ZmqNetworkInterface::duplicateInterface()" << endl;
#endif
    return new ZmqNetworkInterface(*this);
}


void ZmqNetworkInterface::processAsyncMessage()
{
    string control;
    uint8_t *envelope;
    uint32_t envelopeSize;
    uint8_t *data;
    uint32_t dataSize;
    Message *message;

    (void) s_recv(this->zmq_async, control);
    if (iAmNetSim) {
        if ("DELAY" != control) {
            CERR << "net sim invalid control message '"
                << control << "'" << endl;
            (void) s_send(this->zmq_req, this->context, "DIE");
        }
    }
    else {
        if ("ROUTE" != control) {
            CERR << "gen sim invalid control message '"
                << control << "'" << endl;
            (void) s_send(this->zmq_req, this->context, "DIE");
        }
    }
    envelopeSize = s_recv(this->zmq_async, envelope, 256);
    message = new Message(envelope,envelopeSize);
    dataSize = message->getSize();
    if (dataSize > 0) {
        data = new uint8_t[dataSize];
        (void) s_recv(this->zmq_async, data, dataSize);
        message->setData(data,dataSize);
    }

    if (this->messageCallBack) {
        (*(this->messageCallBack))(message);
    }
    else {
        this->receivedMessages.push_back(message);
    }

    delete [] envelope;
}


void ZmqNetworkInterface::processSubMessage()
{
    string control;

    (void) s_recv(this->zmq_die, control);
    if ("DIE" == control) {
        CERR << "DIE received from SUB" << endl;
        /* an application terminated abrubtly, so do we */
        cleanup();
        exit(EXIT_FAILURE);
    }
    else if ("FINISHED" == control) {
        CERR << "FINISHED received from SUB" << endl;
        /* an application terminated cleanly, so do we */
        (void) s_send(this->zmq_req, this->context, "FINISHED");
        cleanup();
        exit(EXIT_SUCCESS);
    }
    else {
        CERR << "'" << control << "' received from SUB" << endl;
        (void) s_send(this->zmq_req, "DIE");
        cleanup();
        exit(EXIT_FAILURE);
    }
}


void ZmqNetworkInterface::makeProgress()
{
    bool done;

#if DEBUG
    CERR << "ZmqNetworkInterface::makeProgress()" << endl;
#endif

    if (isAcceptingRegistrations()) {
        NETWORK_EXCEPTION("object registration must first be finalized");
    }

    done = false;
    while (!done) {
        done = true;
#if DEBUG
        CERR << "testing for incoming messages" << endl;
#endif
        zmq_pollitem_t items[] = {
            { this->zmq_async, 0, ZMQ_POLLIN, 0 },
            { this->zmq_die,   0, ZMQ_POLLIN, 0 }
        };
        int rc = zmq_poll(items, 2, 0);
        assert(rc >= 0);
        if (items[0].revents & ZMQ_POLLIN) {
            processAsyncMessage();
            done = false; /* we had a message */
        }
        if (items[1].revents & ZMQ_POLLIN) {
            processSubMessage();
            done = false; /* we had a message */
        }
    }
}


void ZmqNetworkInterface::sendFinishedSignal()
{
    (void) s_send(this->zmq_req, this->context, "FINISHED");
}


void ZmqNetworkInterface::cleanup()
{
    int zmq_close_retval = 0;
    int zmq_ctx_destroy_retval = 0;

#if DEBUG
    CERR << "ZmqNetworkInterface::~ZmqNetworkInterface()" << endl;
#endif

    makeProgress();

    zmq_close_retval = zmq_close(this->zmq_req);
    assert(0 == zmq_close_retval);

    zmq_close_retval = zmq_close(this->zmq_async);
    assert(0 == zmq_close_retval);

    zmq_close_retval = zmq_close(this->zmq_die);
    assert(0 == zmq_close_retval);

    zmq_ctx_destroy_retval = zmq_ctx_destroy(this->zmq_ctx);
    assert(0 == zmq_ctx_destroy_retval);
}
