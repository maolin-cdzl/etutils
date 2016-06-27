#ifndef __ZMQMOD_EV_ZMQ_H__
#define __ZMQMOD_EV_ZMQ_H__

#include <ev.h>
#include <czmq.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ev_zmq ev_zmq;
typedef void (*ev_zmq_callback)(struct ev_loop* loop,ev_zmq* w,int events);

struct ev_zmq {
	EV_WATCHER(ev_zmq)
	
	zsock_t*	zsock;

	ev_prepare	wprepare;
	ev_check	wcheck;
	ev_idle		widle;
	ev_io		wio;
};

#define ev_zmq_sock(ev)		((ev)->zsock)

void ev_zmq_init(ev_zmq* self,ev_zmq_callback cb,zsock_t* zsock,int events);

void ev_zmq_set(ev_zmq* self,zsock_t* zsock,int events);

void ev_zmq_start(struct ev_loop* loop,ev_zmq* w);

void ev_zmq_stop(struct ev_loop* loop,ev_zmq* w);

#ifdef __cplusplus
}
#endif
#endif


