#include "config.h"

#include <assert.h> /* for assert */
#include <stdlib.h> /* for size_t */
#include <sys/time.h> /* for gettimeofday */
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <zmq.h>


#include "zmqhelper.h"
#if DEBUG
#include "integrator.h"
#endif

using namespace std;

zmqx_sigfunc _zmqx_sigfunc=NULL;
int   _zmqx_sigfunc_context=0;
void *_zmqx_sigfunc_object=NULL;
int _zmqx_interrupted=0;
static int _zmqx_interrupted_in_process=0;


void zmqx_register_handler(zmqx_sigfunc function, int context, void *object)
{
    _zmqx_sigfunc = function;
    _zmqx_sigfunc_context = context;
    _zmqx_sigfunc_object = object;
    //reset signal checks
    _zmqx_interrupted_in_process=0;
    _zmqx_interrupted=0;
}


void zmqx_signal_handler (int signal_value)
{
    _zmqx_interrupted = 1;
}


void zmqx_catch_signals (void)
{
    struct sigaction action;
    action.sa_handler = zmqx_signal_handler;
    action.sa_flags = 0;
    sigemptyset (&action.sa_mask);
    sigaction (SIGINT, &action, NULL);
    sigaction (SIGTERM, &action, NULL);
}


void zmqx_interrupt_check()
{
/* check for interrupt but don't recheck if already processing interrupt */
    if (_zmqx_interrupted && !_zmqx_interrupted_in_process) {
#if DEBUG
        CERR << "zmqx_interrupt_check interrupt detected" << endl;
#endif
        _zmqx_interrupted_in_process = 1;
        if (_zmqx_sigfunc != NULL) {
#if DEBUG
            CERR << "zmqx_interrupt_check calling callback" << endl;
#endif
            _zmqx_sigfunc(_zmqx_sigfunc_object);
        }
    }
}


int zmqx_recv(void *socket, string &buf) {
    char buffer [256];
    int size = 0;

    zmqx_interrupt_check();
    size = zmq_recv (socket, buffer, 255, 0);
    if (size >= 0) {
        if (size > 255) {
            size = 255;
        }
        buffer[size] = 0;
        buf.assign(buffer);
    }
    else if (size == -1) {
        assert(errno == EINTR);
    }
    else {
        assert(0);
    }

    zmqx_interrupt_check();
    return size;
}


int zmqx_recv(void *socket, uint8_t* &buf, uint32_t buf_size) {
    int size;

    zmqx_interrupt_check();
    buf = new uint8_t[buf_size];
    size = zmq_recv(socket, buf, buf_size, 0);
    assert((size >= 0 && size <= buf_size)
            || (size == -1 && errno == EINTR)
            );

    zmqx_interrupt_check();
    return size;
}


int zmqx_irecv(void *socket, string &buf) {
    char buffer [256];
    int size = 0;

    zmqx_interrupt_check();
    size = zmq_recv(socket, buffer, 255, ZMQ_DONTWAIT);
    if (size >= 0) {
        if (size > 255) {
            size = 255;
        }
        buffer[size] = 0;
        buf.assign(buffer);
    }
    else if (size == -1) {
        assert(errno == EINTR || errno == EAGAIN);
        buf.clear();
    }
    else {
        assert(0);
    }

    zmqx_interrupt_check();
    return size;
}


int zmqx_send(void *socket, const string &s) {
    int size;

    zmqx_interrupt_check();
    do{
      size = zmq_send(socket, s.data(), s.size(), 0);
  
      assert(size == s.size()
	      || (size == -1 && errno == EINTR)
	      );

      zmqx_interrupt_check();
    }while(size < 0);
    return size;
}


int zmqx_send(void *socket, uint8_t *buf, uint32_t buf_size) {
    int size;

    zmqx_interrupt_check();
    size = zmq_send(socket, buf, buf_size, 0);
  
    assert((size >= 0 && size <= buf_size)
            || (size == -1 && errno == EINTR)
            );

    zmqx_interrupt_check();
    return size;
}


int zmqx_sendmore(void *socket, const string &s) {
    int size;

    zmqx_interrupt_check();
    do{
      size = zmq_send(socket, s.data(), s.size(), ZMQ_SNDMORE);
      assert(size == s.size()
	      || (size == -1 && errno == EINTR)
	      );

      zmqx_interrupt_check();
      //if we are here then we ignored the signal.
    }while(size < 0);
    return size;
}


int zmqx_sendmore(void *socket, uint8_t *buf, uint32_t buf_size) {
    int size;

    zmqx_interrupt_check();
    size = zmq_send(socket, buf, buf_size, ZMQ_SNDMORE);
  
    assert(size == buf_size
            || (size == -1 && errno == EINTR)
            );

    zmqx_interrupt_check();
    return size;
}


int zmqx_send(void *socket, int context, const string &command) {
    int totalSize = 0;

    zmqx_interrupt_check();
    totalSize += zmqx_sendmore(socket, context);
    totalSize += zmqx_send    (socket, command);

    zmqx_interrupt_check();
    return totalSize;
 
}


int zmqx_sendmore(void *socket, int context, const string &command) {
    int totalSize = 0;

    zmqx_interrupt_check();
    totalSize += zmqx_sendmore(socket, context);
    totalSize += zmqx_sendmore(socket, command);

    zmqx_interrupt_check();
    return totalSize;
}

int zmqx_poll(zmq_pollitem_t* socks, uint32_t buff_size)
{

   zmqx_interrupt_check();
   int rc = zmq_poll(socks, buff_size, 0);
   assert(rc >= 0
            || (rc == -1 && errno == EINTR)
            );
   zmqx_interrupt_check();
   return rc;
}



unsigned long zmqx_utime()
{
    struct timeval tv;

    zmqx_interrupt_check();
    (void) gettimeofday(&tv, NULL);

    zmqx_interrupt_check();
    return tv.tv_sec*1000000 + tv.tv_usec;
}


#define randof(num)  (int) ((float) (num) * random () / (RAND_MAX + 1.0))
string gen_id() {
    char identity [16];
    srandom(zmqx_utime());
    sprintf (identity, "%05d-%04X-%04X", getpid(), randof (0x10000), randof (0x10000));
    return string(identity);
}

