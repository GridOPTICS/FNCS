#ifndef _ZMQHELPER_H_
#define _ZMQHELPER_H_

#include <assert.h> /* for assert */
#include <stdlib.h> /* for size_t */
#include <sys/time.h> /* for gettimeofday */

static int s_recv(void *socket, string &buf) {
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
static int s_recv(void *socket, uint8_t* &buf, uint32_t buf_size) {
    int size;
    buf = new uint8_t[buf_size];
    size = zmq_recv(socket, buf, buf_size, 0);
    assert(size >= 0 && size <= buf_size);
    return size;
}
template <typename T>
static int s_recv(void *socket, T &buf) {
    int size;
    size = zmq_recv(socket, &buf, sizeof(T), 0);
    assert(size == sizeof(T));
    return size;
}


static int s_irecv(void *socket, string &buf) {
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
template <typename T>
static int s_irecv(void *socket, T &buf) {
    int size = zmq_recv(socket, &buf, sizeof(T), ZMQ_DONTWAIT);
    assert((size == -1 && errno == EAGAIN)
            || size == sizeof(T));
    return size;
}


static int s_send(void *socket, const string &s) {
    int size;
    size = zmq_send(socket, s.data(), s.size(), 0);
    assert(size == s.size());
    return size;
}
static int s_send(void *socket, uint8_t *buf, uint32_t buf_size) {
    int size;
    size = zmq_send(socket, buf, buf_size, 0);
    assert(size >= 0 && size <= buf_size);
    return size;
}
template <typename T>
static int s_send(void *socket, const T &what) {
    int size;
    size = zmq_send(socket, &what, sizeof(T), 0);
    assert(size == sizeof(T));
    return size;
}


static int s_sendmore(void *socket, const string &s) {
    int size;
    size = zmq_send(socket, s.data(), s.size(), ZMQ_SNDMORE);
    assert(size == s.size());
    return size;
}
static int s_sendmore(void *socket, uint8_t *buf, uint32_t buf_size) {
    int size;
    size = zmq_send(socket, buf, buf_size, ZMQ_SNDMORE);
    assert(size == buf_size);
    return size;
}
template <typename T>
static int s_sendmore(void *socket, const T &what) {
    int size;
    size = zmq_send(socket, &what, sizeof(T), ZMQ_SNDMORE);
    assert(size == sizeof(T));
    return size;
}


static int s_send(void *socket, int context, const string &command) {
    int totalSize = 0;
    totalSize += s_sendmore(socket, context);
    totalSize += s_send    (socket, command);
    return totalSize;
}
template <typename T>
static int s_send(
        void *socket, int context, const string &command, const T &t) {
    int totalSize = 0;
    totalSize += s_sendmore(socket, context);
    totalSize += s_sendmore(socket, command);
    totalSize += s_send    (socket, t);
    return totalSize;
}


static int s_sendmore(void *socket, int context, const string &command) {
    int totalSize = 0;
    totalSize += s_sendmore(socket, context);
    totalSize += s_sendmore(socket, command);
    return totalSize;
}
template <typename T>
static int s_sendmore(
        void *socket, int context, const string &command, const T &t) {
    int totalSize = 0;
    totalSize += s_sendmore(socket, context);
    totalSize += s_sendmore(socket, command);
    totalSize += s_sendmore(socket, t);
    return totalSize;
}


static unsigned long utime()
{
    struct timeval tv;

    (void) gettimeofday(&tv, NULL);

    return tv.tv_sec*1000000 + tv.tv_usec;
}


#define randof(num)  (int) ((float) (num) * random () / (RAND_MAX + 1.0))
static string gen_id() {
    char identity [10];
    srandom(utime());
    sprintf (identity, "%05d-%04X-%04X", getpid(), randof (0x10000), randof (0x10000));
    return string(identity);
}

#endif /* _ZMQHELPER_H_ */
