#include <stddef.h>
#include <assert.h>
#include "etutils/zep/ev_zk.h"
#include "ev_private.h"

static void ev_zk_reloop(struct ev_loop* loop,ev_zk* self);
static void ev_zk_rio_cb(struct ev_loop* loop,ev_io* w,int revents);
static void ev_zk_wio_cb(struct ev_loop* loop,ev_io* w,int revents);
static void ev_zk_timer_cb(struct ev_loop* loop,ev_timer* w,int revents);

void ev_zk_init(ev_zk* self,ev_zk_callback cb,zhandle_t* zh) {
	ev_init(self,cb);
	self->fd = -1;
	ev_init(&self->rio,ev_zk_rio_cb);
	ev_init(&self->wio,ev_zk_wio_cb);
	ev_init(&self->timer,ev_zk_timer_cb);
	ev_zk_set(self,zh);
}

void ev_zk_set(ev_zk* self,zhandle_t* zh) {
	self->zkh = zh;
}

void ev_zk_start(struct ev_loop* loop,ev_zk* self) {
	int zerr;
	struct timeval tv;
	int zevents = 0;

	if( ev_is_active(self) ) {
		return;
	}
	tv.tv_sec = tv.tv_usec = 0;
	zerr = zookeeper_interest(self->zkh,&self->fd,&zevents,&tv);
	if( zerr != ZOK ) {
		return;
	}
	assert(self->fd != -1);
	
	ev_start(loop,(ev_watcher*)self,1);
	ev_io_set(&self->rio,self->fd,EV_READ);
	ev_io_set(&self->wio,self->fd,EV_WRITE);

	if( zevents & ZOOKEEPER_READ ) {
		ev_io_start(loop,&self->rio);
	}
	if( zevents & ZOOKEEPER_WRITE ) {
		ev_io_start(loop,&self->wio);
	}
	if( tv.tv_sec || tv.tv_usec ) {
		self->timer.repeat = tv.tv_sec + ( tv.tv_usec * 0.000001 );
		ev_timer_again(loop,&self->timer);
	}
}

void ev_zk_stop(struct ev_loop* loop,ev_zk* self) {
	if( ! ev_is_active(self) ) {
		return;
	}

	ev_stop(loop,(ev_watcher*)self);
	if( ev_is_active(&self->wio) ) {
		ev_io_stop(loop,&self->wio);
	}
	if( ev_is_active(&self->rio) ) {
		ev_io_stop(loop,&self->wio);
	}
	if( ev_is_active(&self->timer) ) {
		ev_timer_stop(loop,&self->timer);
	}
	self->fd = -1;
}


static void ev_zk_reloop(struct ev_loop* loop,ev_zk* self) {
	int zerr = 0;
	int zevents = 0;
	int fd = -1;
	struct timeval tv;

	tv.tv_sec = tv.tv_usec = 0;
	zerr = zookeeper_interest(self->zkh,&self->fd,&zevents,&tv);
	if( zerr != ZOK ) {
		return;
	}
	assert(fd != -1);
	assert(zevents);
	
	if( fd != self->fd ) {
		ev_zk_stop(loop,self);
		self->fd = fd;
		ev_io_set(&self->rio,self->fd,EV_READ);
		ev_io_set(&self->wio,self->fd,EV_WRITE);
		if( zevents & ZOOKEEPER_READ ) {
			ev_io_start(loop,&self->rio);
		}
		if( zevents & ZOOKEEPER_WRITE ) {
			ev_io_start(loop,&self->wio);
		}
		if( tv.tv_sec || tv.tv_usec ) {
			self->timer.repeat = tv.tv_sec + ( tv.tv_usec * 0.000001 );
			ev_timer_again(loop,&self->timer);
		}
	} else {
		if( zevents & ZOOKEEPER_READ ) {
			ev_io_start(loop,&self->rio);
		} else if( ev_is_active(&self->rio) ) {
			ev_io_stop(loop,&self->rio);
		}

		if( zevents & ZOOKEEPER_WRITE ) {
			ev_io_start(loop,&self->wio);
		} else if( ev_is_active(&self->wio) ) {
			ev_io_stop(loop,&self->wio);
		}

		if( tv.tv_sec || tv.tv_usec ) {
			self->timer.repeat = tv.tv_sec + ( tv.tv_usec * 0.000001 );
			ev_timer_again(loop,&self->timer);
		} else if( ev_is_active(&self->timer) ) {
			ev_timer_stop(loop,&self->timer);
		}
	}
}

static void ev_zk_rio_cb(struct ev_loop* loop,ev_io* w,int revents) {
	int zerr;
	ev_zk* self = (ev_zk*)( ((char*)w) - offsetof(ev_zk,rio) );
	if( revents & EV_ERROR ) {
		if( self->cb ) {
			self->cb(loop,self,ZSYSTEMERROR);
		}
		return;
	}

	zerr = zookeeper_process(self->zkh,ZOOKEEPER_READ);
	if( zerr != ZOK ) {
		ev_zk_stop(loop,self);
		if( self->cb ) {
			self->cb(loop,self,zerr);
		}
	} else {
		ev_zk_reloop(loop,self);
	}
}

static void ev_zk_wio_cb(struct ev_loop* loop,ev_io* w,int revents) {
	int zerr;
	ev_zk* self = (ev_zk*)( ((char*)w) - offsetof(ev_zk,wio) );
	if( revents & EV_ERROR ) {
		if( self->cb ) {
			self->cb(loop,self,ZSYSTEMERROR);
		}
		return;
	}
	zerr = zookeeper_process(self->zkh,ZOOKEEPER_WRITE);
	if( zerr != ZOK ) {
		ev_zk_stop(loop,self);
		if( self->cb ) {
			self->cb(loop,self,zerr);
		}
	} else {
		ev_zk_reloop(loop,self);
	}
}

static void ev_zk_timer_cb(struct ev_loop* loop,ev_timer* w,int revents) {
	int zerr;
	ev_zk* self = (ev_zk*)( ((char*)w) - offsetof(ev_zk,timer) );
	if( revents & EV_ERROR ) {
		if( self->cb ) {
			self->cb(loop,self,ZSYSTEMERROR);
		}
		return;
	}
	zerr = zookeeper_process(self->zkh,0);
	if( zerr != ZOK ) {
		ev_zk_stop(loop,self);
		if( self->cb ) {
			self->cb(loop,self,zerr);
		}
	} else {
		ev_zk_reloop(loop,self);
	}
}

