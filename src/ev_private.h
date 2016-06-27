#ifndef __EV_PRIVATE_H__
#define __EV_PRIVATE_H__

#include <ev.h>

static inline void pri_adjust (struct ev_loop* loop,ev_watcher* w) {
  int pri = ev_priority (w);
  (void)loop;

  pri = pri < EV_MINPRI ? EV_MINPRI : pri;
  pri = pri > EV_MAXPRI ? EV_MAXPRI : pri;
  ev_set_priority (w, pri);
}

static inline void ev_start (struct ev_loop* loop,ev_watcher* w, int active) {
  pri_adjust (loop, w);
  w->active = active;
  ev_ref (loop);
}

static inline void ev_stop (struct ev_loop* loop,ev_watcher* w) {
  ev_unref (loop);
  w->active = 0;
}


#endif

