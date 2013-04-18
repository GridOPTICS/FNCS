#ifndef _ZMQHELPER_H_
#define _ZMQHELPER_H_

#include <assert.h> /* for assert */
#include <stdlib.h> /* for size_t */
#include <stdint.h> /* for uint8_t, uint32_t */
#include <sys/time.h> /* for gettimeofday */

#include <zmq.h>

#include <string>

using namespace std;


int s_recv(void *socket, string &buf);
int s_recv(void *socket, uint8_t* &buf, uint32_t buf_size);
template <typename T>
int s_recv(void *socket, T &buf) {
    int size;
    size = zmq_recv(socket, &buf, sizeof(T), 0);
    assert(size == sizeof(T));
    return size;
}


int s_irecv(void *socket, string &buf);
template <typename T>
int s_irecv(void *socket, T &buf) {
    int size = zmq_recv(socket, &buf, sizeof(T), ZMQ_DONTWAIT);
    assert((size == -1 && errno == EAGAIN)
            || size == sizeof(T));
    return size;
}


int s_send(void *socket, const string &s);
int s_send(void *socket, uint8_t *buf, uint32_t buf_size);
template <typename T>
int s_send(void *socket, const T &what) {
    int size;
    size = zmq_send(socket, &what, sizeof(T), 0);
    assert(size == sizeof(T));
    return size;
}


int s_sendmore(void *socket, const string &s);
int s_sendmore(void *socket, uint8_t *buf, uint32_t buf_size);
template <typename T>
int s_sendmore(void *socket, const T &what) {
    int size;
    size = zmq_send(socket, &what, sizeof(T), ZMQ_SNDMORE);
    assert(size == sizeof(T));
    return size;
}


int s_send(void *socket, int context, const string &command);
template <typename T>
int s_send(void *socket, int context, const string &command, const T &t) {
    int totalSize = 0;
    totalSize += s_sendmore(socket, context);
    totalSize += s_sendmore(socket, command);
    totalSize += s_send    (socket, t);
    return totalSize;
}


int s_sendmore(void *socket, int context, const string &command);
template <typename T>
int s_sendmore(void *socket, int context, const string &command, const T &t) {
    int totalSize = 0;
    totalSize += s_sendmore(socket, context);
    totalSize += s_sendmore(socket, command);
    totalSize += s_sendmore(socket, t);
    return totalSize;
}

unsigned long s_utime();

string gen_id();

#endif /* _ZMQHELPER_H_ */
