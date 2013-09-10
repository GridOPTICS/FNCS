#ifndef _ZMQHELPER_H_
#define _ZMQHELPER_H_

#include <stdint.h> /* for uint8_t, uint32_t */
#include <sys/time.h> /* for gettimeofday */

#include <cassert> /* for assert */
#include <cstdlib> /* for size_t */
#include <csignal> /* for sigaction */
#include <string>

#include <zmq.h>

using namespace std;

// Signal handling
//
// Call zmqx_catch_signals() in your application at startup, and then
// exit your main loop if _zmqx_interrupted is ever 1. Works especially
// well with zmq_poll.

typedef void (*zmqx_sigfunc)(void*);
extern zmqx_sigfunc _zmqx_sigfunc;
extern void *_zmqx_sigfunc_arg;
extern int _zmqx_interrupted;

void zmqx_register_handler(zmqx_sigfunc function, void *object);

void zmqx_signal_handler (int signal_value);

void zmqx_catch_signals (void);

void zmqx_interrupt_check();

int zmqx_recv(void *socket, string &buf);

int zmqx_recv(void *socket, uint8_t* &buf, uint32_t buf_size);

template <typename T>
int zmqx_recv(void *socket, T &buf)
{
    int size = 0;

    zmqx_interrupt_check();
    size = zmq_recv(socket, &buf, sizeof(T), 0);
    assert(size == sizeof(T)
            || (size == -1 && errno == EINTR)
          );

    zmqx_interrupt_check();
    return size;
}

int zmqx_irecv(void *socket, string &buf);

template <typename T>
int zmqx_irecv(void *socket, T &buf)
{
    int size = 0;

    zmqx_interrupt_check();
    size = zmq_recv(socket, &buf, sizeof(T), ZMQ_DONTWAIT);
    assert(size == sizeof(T)
            || (size == -1 && errno == EAGAIN)
            || (size == -1 && errno == EINTR)
          );

    zmqx_interrupt_check();
    return size;
}

int zmqx_send(void *socket, const string &s);

int zmqx_send(void *socket, uint8_t *buf, uint32_t buf_size);

template <typename T>
int zmqx_send(void *socket, const T &what)
{
    int size;

    zmqx_interrupt_check();
    size = zmq_send(socket, &what, sizeof(T), 0);
    assert(size == sizeof(T)
            || (size == -1 && errno == EINTR)
          );

    zmqx_interrupt_check();
    return size;
}

int zmqx_sendmore(void *socket, const string &s);

int zmqx_sendmore(void *socket, uint8_t *buf, uint32_t buf_size);

template <typename T>
int zmqx_sendmore(void *socket, const T &what)
{
    int size;

    zmqx_interrupt_check();
    size = zmq_send(socket, &what, sizeof(T), ZMQ_SNDMORE);
    assert(size == sizeof(T)
            || (size == -1 && errno == EINTR)
          );

    zmqx_interrupt_check();
    return size;
}

int zmqx_send(void *socket, int context, const string &command);

template <typename T>
int zmqx_send(void *socket, int context, const string &command, const T &t)
{
    int totalSize = 0;

    zmqx_interrupt_check();
    totalSize += zmqx_sendmore(socket, context);
    totalSize += zmqx_sendmore(socket, command);
    totalSize += zmqx_send    (socket, t);

    zmqx_interrupt_check();
    return totalSize;
}

int zmqx_sendmore(void *socket, int context, const string &command);

template <typename T>
int zmqx_sendmore(void *socket, int context, const string &command, const T &t)
{
    int totalSize = 0;

    zmqx_interrupt_check();
    totalSize += zmqx_sendmore(socket, context);
    totalSize += zmqx_sendmore(socket, command);
    totalSize += zmqx_sendmore(socket, t);

    zmqx_interrupt_check();
    return totalSize;
}

unsigned long zmqx_utime();

string gen_id();

#endif /* _ZMQHELPER_H_ */
