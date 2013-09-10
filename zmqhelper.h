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
// Call s_catch_signals() in your application at startup, and then
// exit your main loop if s_interrupted is ever 1. Works especially
// well with zmq_poll.

typedef void (*zmq_sigfunc)(void*);
extern zmq_sigfunc s_zmq_sigfunc;
extern void *s_zmq_sigfunc_arg;
extern int s_interrupted;


static void s_register_handler(zmq_sigfunc function, void *object)
{
    s_zmq_sigfunc = function;
    s_zmq_sigfunc_arg = object;
}


static void s_signal_handler (int signal_value)
{
    s_interrupted = 1;
}


static void s_catch_signals (void)
{
    struct sigaction action;
    action.sa_handler = s_signal_handler;
    action.sa_flags = 0;
    sigemptyset (&action.sa_mask);
    sigaction (SIGINT, &action, NULL);
    sigaction (SIGTERM, &action, NULL);
}


static inline void s_interrupt_check()
{
    if (s_interrupted) {
        if (s_zmq_sigfunc) {
            s_zmq_sigfunc(s_zmq_sigfunc_arg);
        }
    }
}


int s_recv(void *socket, string &buf);
int s_recv(void *socket, uint8_t* &buf, uint32_t buf_size);
template <typename T>
int s_recv(void *socket, T &buf) {
    int size = 0;

    s_interrupt_check();
    size = zmq_recv(socket, &buf, sizeof(T), 0);
    assert(size == sizeof(T)
            || (size == -1 && errno == EINTR)
          );

    s_interrupt_check();
    return size;
}


int s_irecv(void *socket, string &buf);
template <typename T>
int s_irecv(void *socket, T &buf) {
    int size = 0;

    s_interrupt_check();
    size = zmq_recv(socket, &buf, sizeof(T), ZMQ_DONTWAIT);
    assert(size == sizeof(T)
            || (size == -1 && errno == EAGAIN)
            || (size == -1 && errno == EINTR)
          );

    s_interrupt_check();
    return size;
}


int s_send(void *socket, const string &s);
int s_send(void *socket, uint8_t *buf, uint32_t buf_size);
template <typename T>
int s_send(void *socket, const T &what) {
    int size;

    s_interrupt_check();
    size = zmq_send(socket, &what, sizeof(T), 0);
    assert(size == sizeof(T)
            || (size == -1 && errno == EINTR)
          );

    s_interrupt_check();
    return size;
}


int s_sendmore(void *socket, const string &s);
int s_sendmore(void *socket, uint8_t *buf, uint32_t buf_size);
template <typename T>
int s_sendmore(void *socket, const T &what) {
    int size;

    s_interrupt_check();
    size = zmq_send(socket, &what, sizeof(T), ZMQ_SNDMORE);
    assert(size == sizeof(T)
            || (size == -1 && errno == EINTR)
          );

    s_interrupt_check();
    return size;
}


int s_send(void *socket, int context, const string &command);
template <typename T>
int s_send(void *socket, int context, const string &command, const T &t) {
    int totalSize = 0;

    s_interrupt_check();
    totalSize += s_sendmore(socket, context);
    totalSize += s_sendmore(socket, command);
    totalSize += s_send    (socket, t);

    s_interrupt_check();
    return totalSize;
}


int s_sendmore(void *socket, int context, const string &command);
template <typename T>
int s_sendmore(void *socket, int context, const string &command, const T &t) {
    int totalSize = 0;

    s_interrupt_check();
    totalSize += s_sendmore(socket, context);
    totalSize += s_sendmore(socket, command);
    totalSize += s_sendmore(socket, t);

    s_interrupt_check();
    return totalSize;
}

unsigned long s_utime();

string gen_id();

#endif /* _ZMQHELPER_H_ */
