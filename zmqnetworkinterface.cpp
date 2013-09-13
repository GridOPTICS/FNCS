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


static void cleanup_handler(void *object)
{
    ZmqNetworkInterface *interface = NULL;
    interface = reinterpret_cast<ZmqNetworkInterface*>(object);
    interface->cleanup();
    exit(EXIT_FAILURE);
}


void ZmqNetworkInterface::init(bool sendHello)
{
    int zmq_connect_req_retval = 0;
    int zmq_connect_sub_retval = 0;
    int zmq_setsockopt_id_retval = 0;
    int zmq_setsockopt_retval = 0;

#if DEBUG
    CERR << "ZmqNetworkInterface::init()" << endl;
#endif


#if DEBUG
    CERR << "ZmqNetworkInterface ID=" << this->ID << endl;
#endif

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

    if (sendHello) {
        /* send hello to broker */
        /* send parent context as part of hello when this ::init() is called as
         * part of copy constructor */
        int ZERO = 0;
        if (this->iAmNetSim) {
            (void) zmqx_send(this->zmq_req, ZERO, "HELLO_NETSIM", this->context);
        }
        else {
            (void) zmqx_send(this->zmq_req, ZERO, "HELLO", this->context);
        }

        /* get ack from broker */
        (void) i_recv(this->context);
        assert(this->context >= 0);

        zmqx_register_handler(cleanup_handler, this->zmq_req, this->context, this);
        zmqx_catch_signals();
    }
    else {
        waitForHeartbeat();
    }

#if DEBUG
    CERR << this->ID << " context=" << this->context << endl;
#endif
}


ZmqNetworkInterface::ZmqNetworkInterface(bool iAmNetSim)
    :   AbsNetworkInterface()
    ,   zmq_ctx(NULL)
    ,   zmq_req(NULL)
    ,   zmq_async(NULL)
    ,   ID(gen_id())
    ,   context(-1)
    ,   iAmNetSim(iAmNetSim)
    ,   receivedMessages()
    ,   globalObjectCount(0)
    ,   heartbeat(false)
{
    
    init(true);
}


ZmqNetworkInterface::ZmqNetworkInterface(ZmqNetworkInterface &that)
    :   AbsNetworkInterface(that)
    ,   zmq_ctx(NULL)
    ,   zmq_req(NULL)
    ,   zmq_async(NULL)
    ,   context(that.context)
    ,   ID(that.ID)
    ,   iAmNetSim(that.iAmNetSim)
    ,   receivedMessages()
    ,   globalObjectCount(that.globalObjectCount)
    ,   heartbeat(false)
{
    uint64_t globalObjectCountAgain;

    if(Integrator::isChild()){
    	this->ID=gen_id();
    }
    
    init(Integrator::isChild());

    for (vector<string>::const_iterator it=myObjects.begin();
            it!=myObjects.end(); ++it) {
        (void) zmqx_send(this->zmq_req, this->context, "REGISTER_OBJECT", *it);
    }

    (void) zmqx_send(this->zmq_req, this->context, "FINALIZE_REGISTRATIONS");
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
#if DEBUG
    CERR << "envelopeSize=" << envelopeSize << endl;
#endif
    if (iAmNetSim) {
        (void) zmqx_sendmore(this->zmq_async, this->context, "ROUTE");
    }
    else if (message->getDelayThroughComm()) {
        (void) zmqx_sendmore(this->zmq_async, this->context, "DELAY");
    }
    else {
        (void) zmqx_sendmore(this->zmq_async, this->context, "ROUTE");
    }
    if (dataSize > 0) {
        (void) zmqx_sendmore(this->zmq_async, envelope, envelopeSize);
        (void) zmqx_send    (this->zmq_async, const_cast<uint8_t*>(data), dataSize);
    }
    else {
        (void) zmqx_send    (this->zmq_async, envelope, envelopeSize);
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

    (void) zmqx_sendmore(this->zmq_req, this->context, "REDUCE_MIN_TIME");
    (void) zmqx_send(this->zmq_req, _myTime);
    (void) i_recv(retval);

#if DEBUG
    CERR << "\treceived " << retval << endl;
#endif

    return retval;
}

uint64_t* ZmqNetworkInterface::getNextTimes(uint64_t nextTime,uint32_t &worldSize)
{
#if DEBUG
    CERR << "ZmqNetworkInterface::getNextTimes("
        << nextTime << ")" << endl;
#endif
	makeProgress();
	
    (void) zmqx_sendmore(this->zmq_req, this->context, "ALL_GATHER_NEXT_TIME");
    (void) zmqx_send(this->zmq_req, nextTime);
    (void) i_recv(worldSize);
    assert(worldSize < 1000);
    //uint64_t* toReturn=new uint64_t[worldSize];
    uint8_t *buff;
    zmqx_recv(this->zmq_req,buff,sizeof(uint64_t)*worldSize);
    return reinterpret_cast<uint64_t*>(buff);
    
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

    (void) zmqx_sendmore(this->zmq_req, this->context, "REDUCE_SEND_RECV");
    (void) zmqx_sendmore(this->zmq_req, sent);
    (void) zmqx_send    (this->zmq_req, received);
    (void) i_recv(m_sent);
    (void) i_recv(m_recv);

#if DEBUG
    CERR << "\treceived m_sent=" << m_sent
         << " m_recv=" << m_recv << endl;
#endif

    if (m_sent < m_recv) {
	//cout << "FAILING!!!!!!!!" << endl;
        (void) zmqx_send(this->zmq_req, this->context, "DIE");
    }

    return static_cast<uint64_t>(m_sent - m_recv);
}


void ZmqNetworkInterface::registerObject(string name)
{
#if DEBUG
    CERR << "ZmqNetworkInterface::registerObject()" << endl;
#endif
    AbsNetworkInterface::registerObject(name);

    (void) zmqx_send(this->zmq_req, this->context, "REGISTER_OBJECT", name);
}


void ZmqNetworkInterface::finalizeRegistrations()
{
#if DEBUG
    CERR << "ZmqNetworkInterface::finalizeRegistrations()" << endl;
#endif
    AbsNetworkInterface::finalizeRegistrations();

    (void) zmqx_send(this->zmq_req, this->context, "FINALIZE_REGISTRATIONS");
    (void) i_recv(this->globalObjectCount);
}


void ZmqNetworkInterface::barier()
{
    string ack;

#if DEBUG
    CERR << "ZmqNetworkInterface::barier()" << endl;
#endif

    (void) zmqx_send(this->zmq_req, this->context, "BARRIER");
    (void) i_recv(ack);
    assert(ack == "ACK");
}


bool ZmqNetworkInterface::sleep()
{
    string ack;

#if DEBUG
    CERR << "ZmqNetworkInterface::sleep()" << endl;
#endif

    (void) zmqx_send(this->zmq_req, this->context, "SLEEP");
    (void) zmqx_recv(this->zmq_req, ack);
    //assert(ack == "ACK");
    if(ack != "ACK"){
      string msg("Message is not ack ");
      msg += ack;
      throw NetworkException(__FILE__,"sleep()",__LINE__,msg.c_str());
    }
    
    zmq_pollitem_t items[] = {
            { this->zmq_async, 0, ZMQ_POLLIN, 0 },
        };
        int rc = zmq_poll(items, 1, 0);
        assert(rc >= 0);
        if (items[0].revents & ZMQ_POLLIN) {
           return true;
        }
     return false;
}


AbsNetworkInterface* ZmqNetworkInterface::duplicateInterface()
{
#if DEBUG
    CERR << "ZmqNetworkInterface::duplicateInterface()" << endl;
#endif
    return new ZmqNetworkInterface(*this);
}


bool ZmqNetworkInterface::processAsyncMessage()
{
    string control;
    uint8_t *envelope;
    uint32_t envelopeSize;
    uint8_t *data;
    uint32_t dataSize;
    Message *message;


    (void) zmqx_recv(this->zmq_async, control);

    if ("HEARTBEAT" == control) {
        heartbeat = true;
        return true;
    }

    if (iAmNetSim) {
        if ("DELAY" != control) {
#if DEBUG
            CERR << "net sim invalid control message '"
                << control << "'" << endl;
#endif
            (void) zmqx_send(this->zmq_req, this->context, "DIE");
        }
    }
    else {
        if ("ROUTE" != control) {
#if DEBUG
            CERR << "gen sim invalid control message '"
                << control << "'" << endl;
#endif
		(void) zmqx_send(this->zmq_req, this->context, "DIE");
        }
    }
    envelopeSize = zmqx_recv(this->zmq_async, envelope, 256);
    message = new Message(envelope,envelopeSize);
    dataSize = message->getSize();
    if (dataSize > 0) {
        data = new uint8_t[dataSize];
        (void) zmqx_recv(this->zmq_async, data, dataSize);
        message->setData(data,dataSize);
    }

    if (this->messageCallBack) {
        (*(this->messageCallBack))(message);
    }
    else {
        this->receivedMessages.push_back(message);
    }

    delete [] envelope;

    return false;
}


void ZmqNetworkInterface::processSubMessage()
{
    string control;

    (void) zmqx_recv(this->zmq_die, control);
    if ("DIE" == control) {
#if DEBUG
        CERR << "DIE received from SUB" << endl;
#endif
	/* an application terminated abrubtly, so do we */
        cleanup();
        exit(EXIT_FAILURE);
    }
    else if ("FINISHED" == control) {
#if DEBUG
        CERR << "FINISHED received from SUB" << endl;
#endif
        /* an application terminated cleanly, so do we */
        (void) zmqx_send(this->zmq_req, this->context, "FINISHED");
        cleanup();
        exit(EXIT_SUCCESS);
    }
    else if ("DIE_CHILD" == control) {
        /* if I'm child, I die */
        bool IAmChild = Integrator::isChild();
        if (IAmChild) {
            unsigned long time = Integrator::getCurSimTime();
	    (void) zmqx_send(this->zmq_req, this->context, "REDUCE_FAIL_TIME", time);
            cleanup();
	    cout << "Child exiting " <<endl;
            exit(EXIT_SUCCESS);
        } 
    }
    else if ("CHILD_DIED" == control) {
      bool IAmParent = !Integrator::isChild();
      if(IAmParent){
        unsigned long time;
        (void) zmqx_recv(this->zmq_die, time);
        Integrator::childDied(time);
      }
    }
    else if("DIE_PARENT" == control){
      bool IAmParent = !Integrator::isChild();
      if(IAmParent){
	cleanup();
	exit(EXIT_SUCCESS);
      }
    }
    else {
#if DEBUG
        CERR << "'" << control << "' received from SUB" << endl;
#endif
	(void) zmqx_send(this->zmq_req, "DIE");
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

void ZmqNetworkInterface::sendFailed()
{
  //this will start the process for grafully killing all the sims at this->context
  (void) zmqx_send(this->zmq_req, this->context, "CHILD_FAILED");
  string bogus;
  while(i_recv(bogus) > 0); //at this point we just wait the DIE child signal
}

void ZmqNetworkInterface::sendSuceed()
{
  (void) zmqx_send(this->zmq_req, this->context, "CHILD_SUCCESS");
}



void ZmqNetworkInterface::sendFinishedSignal()
{
    (void) zmqx_send(this->zmq_req, this->context, "FINISHED");
}


void ZmqNetworkInterface::cleanup()
{
    int zmq_close_retval = 0;
    int zmq_ctx_destroy_retval = 0;

#if DEBUG
    CERR << "ZmqNetworkInterface::cleanup()" << endl;
#endif

    heartbeat = false;

    //makeProgress();

    zmq_close_retval = zmq_close(this->zmq_req);
    assert(0 == zmq_close_retval);

    zmq_close_retval = zmq_close(this->zmq_async);
    assert(0 == zmq_close_retval);

    zmq_close_retval = zmq_close(this->zmq_die);
    assert(0 == zmq_close_retval);

    zmq_ctx_destroy_retval = zmq_ctx_destroy(this->zmq_ctx);
    assert(0 == zmq_ctx_destroy_retval);
}


void ZmqNetworkInterface::waitForHeartbeat()
{
    while (!heartbeat) {
        char ignore;
        (void) zmqx_send(this->zmq_req, this->context, "HEARTBEAT");
        (void) i_recv(ignore, 1000); // timeout in milliseconds
    }
}

