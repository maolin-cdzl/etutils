#ifndef __ZMQMOD_EV_ZK_H__
#define __ZMQMOD_EV_ZK_H__

#include <ev.h>
#include <zookeeper/zookeeper.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ev_zk	ev_zk;
typedef void (*ev_zk_callback)(struct ev_loop* loop,ev_zk* w,int events);

struct ev_zk {
	EV_WATCHER(ev_zk)
	
	int				fd;
	ev_io			wio;
	ev_io			rio;
	ev_timer		timer;
	zhandle_t*		zkh;
};

void ev_zk_init(ev_zk* self,ev_zk_callback cb,zhandle_t* zh);

void ev_zk_set(ev_zk* self,zhandle_t* zh);

void ev_zk_start(struct ev_loop* loop,ev_zk* self);

void ev_zk_stop(struct ev_loop* loop,ev_zk* self);

#ifdef __cplusplus
}
#endif
#endif

