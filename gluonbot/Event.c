#include "Event.h"
#include "Plugin.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

#include <tiny.h>

/*
 * Generic methods
 */

typedef struct PEPair {
  GBPlugin* p;
  GBEvent*  e;
} PEPair;

static void* gb_event_phandle(PEPair* info) {
  assert(info != NULL);
  t_ref(info->p);
  t_ref(info->e);
  
  GBEventHandleFunc f = info->p->handlers[info->e->type];
  if (f != NULL) {
    f(info->e);
  }
  
  t_unref(info->p);
  t_unref(info->e);
  t_free(info);
  return NULL;
}

static void* gb_event_handle(GBEvent* e) {
  assert(e != NULL);
  
  e->handle(e);
  
  t_list_foreach(GBBot.plugins->pairs, n) {
    TMapPair* pair = n->unit->obj;
    GBPlugin* plugin = pair->unit->obj;
    pthread_t thr;
    t_ref(e);
    PEPair* info = (PEPair*) t_malloc(sizeof(PEPair));
    if (!info) {
      perror("gb_event_handle{t_malloc}");
      exit(1);
    }
    info->p = plugin;
    info->e = e;
    if (pthread_create(&thr, NULL, (void*(*)(void*)) gb_event_phandle, info) != 0) {
      perror("gb_event_handle{pthread_create}");
      t_unref(e);
    }
    pthread_detach(thr);
  }
  
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
