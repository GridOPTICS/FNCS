#include "config.h"

#include <assert.h> /* for assert */
#include <stdlib.h> /* for size_t */
#include <sys/time.h> /* for gettimeofday */
#include <sys/types.h>
#include <unistd.h>
#include <string>

#include <zmq.h>


#include "zmqhelper.h"

using namespace std;

zmq_sigfunc s_zmq_sigfunc=NULL;
void *s_zmq_sigfunc_arg=NULL;
int s_interrupted=0;


int s_recv(void *socket, string &buf) {
    char buffer [256];
    int size = 0;

    s_interrupt_check();
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

    s_interrupt_check();
    return size;
}


int s_recv(void *socket, uint8_t* &buf, uint32_t buf_size) {
    int size;

    s_interrupt_check();
    buf = new uint8_t[buf_size];
    size = zmq_recv(socket, buf, buf_size, 0);
    assert((size >= 0 && size <= buf_size)
            || (size == -1 && errno == EINTR)
            );

    s_interrupt_check();
    return size;
}


int s_irecv(void *socket, string &buf) {
    char buffer [256];
    int size = 0;

    s_interrupt_check();
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

    s_interrupt_check();
    return size;
}


int s_send(void *socket, const string &s) {
    int size;

    s_interrupt_check();
    size = zmq_send(socket, s.data(), s.size(), 0);
    assert(size == s.size()
            || (size == -1 && errno == EINTR)
            );

    s_interrupt_check();
    return size;
}


int s_send(void *socket, uint8_t *buf, uint32_t buf_size) {
    int size;

    s_interrupt_check();
    size = zmq_send(socket, buf, buf_size, 0);
    assert((size >= 0 && size <= buf_size)
            || (size == -1 && errno == EINTR)
            );

    s_interrupt_check();
    return size;
}


int s_sendmore(void *socket, const string &s) {
    int size;

    s_interrupt_check();
    size = zmq_send(socket, s.data(), s.size(), ZMQ_SNDMORE);
    assert(size == s.size()
            || (size == -1 && errno == EINTR)
            );

    s_interrupt_check();
    return size;
}


int s_sendmore(void *socket, uint8_t *buf, uint32_t buf_size) {
    int size;

    s_interrupt_check();
    size = zmq_send(socket, buf, buf_size, ZMQ_SNDMORE);
    assert(size == buf_size
            || (size == -1 && errno == EINTR)
            );

    s_interrupt_check();
    return size;
}


int s_send(void *socket, int context, const string &command) {
    int totalSize = 0;

    s_interrupt_check();
    totalSize += s_sendmore(socket, context);
    totalSize += s_send    (socket, command);

    s_interrupt_check();
    return totalSize;
}


int s_sendmore(void *socket, int context, const string &command) {
    int totalSize = 0;

    s_interrupt_check();
    totalSize += s_sendmore(socket, context);
    totalSize += s_sendmore(socket, command);

    s_interrupt_check();
    return totalSize;
}


unsigned long s_utime()
{
    struct timeval tv;

    s_interrupt_check();
    (void) gettimeofday(&tv, NULL);

    s_interrupt_check();
    return tv.tv_sec*1000000 + tv.tv_usec;
}


#define randof(num)  (int) ((float) (num) * random () / (RAND_MAX + 1.0))
string gen_id() {
    char identity [16];
    srandom(s_utime());
    sprintf (identity, "%05d-%04X-%04X", getpid(), randof (0x10000), randof (0x10000));
    return string(identity);
}

