#include "config.h"

#include <assert.h> /* for assert */
#include <stdlib.h> /* for size_t */
#include <sys/time.h> /* for gettimeofday */

#include <string>

#include <zmq.h>

#include "zmqhelper.h"

using namespace std;


int s_recv(void *socket, string &buf) {
    char buffer [256];
    int size = zmq_recv (socket, buffer, 255, 0);
    assert(size >= 0);
    if (size > 255) {
        size = 255;
    }
    buffer[size] = 0;
    buf.assign(buffer);
    return size;
}
int s_recv(void *socket, uint8_t* &buf, uint32_t buf_size) {
    int size;
    buf = new uint8_t[buf_size];
    size = zmq_recv(socket, buf, buf_size, 0);
    assert(size >= 0 && size <= buf_size);
    return size;
}


int s_irecv(void *socket, string &buf) {
    char buffer [256];
    int size = zmq_recv(socket, buffer, 255, ZMQ_DONTWAIT);
    if (size == -1) {
        buf.clear();
        return size;
    }
    if (size > 255) {
        size = 255;
    }
    buffer[size] = 0;
    buf.assign(buffer);
    return size;
}


int s_send(void *socket, const string &s) {
    int size;
    size = zmq_send(socket, s.data(), s.size(), 0);
    assert(size == s.size());
    return size;
}
int s_send(void *socket, uint8_t *buf, uint32_t buf_size) {
    int size;
    size = zmq_send(socket, buf, buf_size, 0);
    assert(size >= 0 && size <= buf_size);
    return size;
}


int s_sendmore(void *socket, const string &s) {
    int size;
    size = zmq_send(socket, s.data(), s.size(), ZMQ_SNDMORE);
    assert(size == s.size());
    return size;
}
int s_sendmore(void *socket, uint8_t *buf, uint32_t buf_size) {
    int size;
    size = zmq_send(socket, buf, buf_size, ZMQ_SNDMORE);
    assert(size == buf_size);
    return size;
}


int s_send(void *socket, int context, const string &command) {
    int totalSize = 0;
    totalSize += s_sendmore(socket, context);
    totalSize += s_send    (socket, command);
    return totalSize;
}


int s_sendmore(void *socket, int context, const string &command) {
    int totalSize = 0;
    totalSize += s_sendmore(socket, context);
    totalSize += s_sendmore(socket, command);
    return totalSize;
}


unsigned long s_utime()
{
    struct timeval tv;

    (void) gettimeofday(&tv, NULL);

    return tv.tv_sec*1000000 + tv.tv_usec;
}


#define randof(num)  (int) ((float) (num) * random () / (RAND_MAX + 1.0))
string gen_id() {
    char identity [10];
    srandom(s_utime());
    sprintf (identity, "%05d-%04X-%04X", getpid(), randof (0x10000), randof (0x10000));
    return string(identity);
}

