#include "etutils/zep/ev_zmq.h"
#include "ev_private.h"

static void ev_zmq_io_cb(struct ev_loop* loop,ev_io* w,int revents);
static void ev_zmq_idle_cb(struct ev_loop* loop,ev_idle* w,int revents);
static void ev_zmq_prepare_cb(struct ev_loop* loop,ev_prepare* w,int revents);
static void ev_zmq_check_cb(struct ev_loop* loop,ev_check* w,int revents);


void ev_zmq_init(ev_zmq* self,ev_zmq_callback cb,zsock_t* zsock,int events) {
	assert(self);
	assert(zsock);

	ev_init(&self->wio,ev_zmq_io_cb);
	ev_prepare_init(&self->wprepare,ev_zmq_prepare_cb);
	ev_check_init(&self->wcheck,ev_zmq_check_cb);
	ev_idle_init(&self->widle,ev_zmq_idle_cb);

	ev_init(self,cb);
	ev_zmq_set(self,zsock,events);
}

void ev_zmq_set(ev_zmq* self,zsock_t* zsock,int events) {
	self->zsock = zsock;

	ev_io_set(&self->wio,zsock_fd(zsock),events);
}

void ev_zmq_start(struct ev_loop* loop,ev_zmq* self) {
	if( ev_is_active(self) )
		return;

	ev_prepare_start(loop,&self->wprepare);
	ev_check_start(loop,&self->wcheck);

	ev_start(loop,(ev_watcher*)self,1);
}

void ev_zmq_stop(struct ev_loop* loop,ev_zmq* self) {
	if( ! ev_is_active(self) )
		return;

	ev_io_stop(loop,&self->wio);
	ev_prepare_stop(loop,&self->wprepare);
	ev_check_stop(loop,&self->wcheck);
	ev_idle_stop(loop,&self->widle);

	ev_stop(loop,(ev_watcher*)self);
}

static void ev_zmq_io_cb(struct ev_loop* loop,ev_io* w,int revents) {
	(void)loop;
	(void)w;
	(void)revents;
}

static void ev_zmq_idle_cb(struct ev_loop* loop,ev_idle* w,int revents) {
	(void)loop;
	(void)w;
	(void)revents;
}

static void ev_zmq_prepare_cb(struct ev_loop* loop,ev_prepare* w,int revents) {
	int zevents;
	ev_zmq* self = (ev_zmq*)( ((char*)w) - offsetof(ev_zmq,wprepare) );
	if( EV_ERROR & revents ) {
		self->cb(loop,self,EV_ERROR);
		return;
	}

	zevents = zsock_events(self->zsock);

	if( (zevents & ZMQ_POLLOUT) && (self->wio.events & EV_WRITE) ) {
		ev_idle_start(loop,&self->widle);
		return;
	}

	if( (zevents & ZMQ_POLLIN) && (self->wio.events & EV_READ) ) {
		ev_idle_start(loop,&self->widle);
		return;
	}

	ev_io_start(loop,&self->wio);
}

static void ev_zmq_check_cb(struct ev_loop* loop,ev_check* w,int revents) {
	int zevents;
	ev_zmq* self = (ev_zmq*)( ((char*)w) - offsetof(ev_zmq,wcheck) );
	if( EV_ERROR & revents ) {
		self->cb(loop,self,EV_ERROR);
		return;
	}

	ev_idle_stop(loop,&self->widle);
	ev_io_stop(loop,&self->wio);

	zevents = zsock_events(self->zsock);

	if( (zevents & ZMQ_POLLOUT) && (self->wio.events & EV_WRITE) ) {
		self->cb(loop,self,EV_WRITE);
	}

	if( (zevents & ZMQ_POLLIN) && (self->wio.events & EV_READ) ) {
		self->cb(loop,self,EV_READ);
	}
}


