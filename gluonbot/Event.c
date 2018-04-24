#include "Event.h"

#include <assert.h>
#include <stdio.h>

#include <pthread.h>

/*
 * Generic methods
 */

static void* gb_event_handle(GBEvent* e) {
  assert(e != NULL);
  
  e->handle(e);
  
  t_unref(e);
  return NULL;
}

void gb_event_fire(GBEvent* e) {
  assert(e != NULL);
  t_ref(e);
  
  pthread_t thr;
  if (pthread_create(&thr, NULL, (void*(*)(void*)) gb_event_handle, e) != 0) {
    perror("gb_event_fire{pthread_create}");
    t_unref(e);
    return;
  }
  pthread_detach(thr);
}
