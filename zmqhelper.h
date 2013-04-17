#ifndef _ZMQHELPER_H_
#define _ZMQHELPER_H_

#include <stdlib.h> /* for size_t */

static int s_recv(void *socket, string &buf) {
    char buffer [256];
    int size = zmq_recv (socket, buffer, 255, 0);
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
static int s_recv(void *socket, T &buf) {
    return zmq_recv(socket, &buf, sizeof(T), 0);
}
template <typename T>
static int s_recv(void *socket, T* &buf, size_t count) {
    buf = new T[count];
    return zmq_recv(socket, buf, sizeof(T)*count, 0);
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
    return zmq_recv(socket, &buf, sizeof(T), ZMQ_DONTWAIT);
}
template <typename T>
static int s_irecv(void *socket, T* &buf, size_t count) {
    buf = new T[count];
    return zmq_recv(socket, buf, sizeof(T)*count, ZMQ_DONTWAIT);
}

static int s_send(void *socket, const string &s) {
    return zmq_send(socket, s.data(), s.size(), 0);
}
static int s_send(void *socket, void *buf, size_t size) {
    return zmq_send(socket, buf, size, 0);
}
template <typename T>
static int s_send(void *socket, const T &what) {
    return zmq_send(socket, &what, sizeof(T), 0);
}
template <typename T>
static int s_send(void *socket, const T* &what, size_t count) {
    return zmq_send(socket, what, sizeof(T)*count, 0);
}

static int s_sendmore(void *socket, const string &s) {
    return zmq_send(socket, s.data(), s.size(), ZMQ_SNDMORE);
}
static int s_sendmore(void *socket, void *buf, size_t size) {
    return zmq_send(socket, buf, size, ZMQ_SNDMORE);
}
template <typename T>
static int s_sendmore(void *socket, const T &what) {
    return zmq_send(socket, &what, sizeof(T), ZMQ_SNDMORE);
}
template <typename T>
static int s_sendmore(void *socket, const T* &what, size_t count) {
    return zmq_send(socket, what, sizeof(T)*count, ZMQ_SNDMORE);
}

#define randof(num)  (int) ((float) (num) * random () / (RAND_MAX + 1.0))
static string gen_id() {
    char identity [10];
    sprintf (identity, "%04X-%04X", randof (0x10000), randof (0x10000));
    return string(identity);
}

#endif /* _ZMQHELPER_H_ */
