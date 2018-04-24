#include "../Event.h"
#include "../IRCSocket.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

#include <tiny-log.h>

static void gb_event_disconnect_destroy(GBEventDisconnect* self);
static void gb_event_disconnect_handle(GBEventDisconnect* self);

GBEvent* gb_event_disconnect_new(GBIRCSocket* sock) {  
  assert(sock != NULL);
  t_ref(sock);
  
  GBEventDisconnect* self = (GBEventDisconnect*) t_malloc(sizeof(GBEventDisconnect));
  if (!self) {
    perror("gb_event_disconnect_new");
    exit(1);
  }
  
  t_gcunit((GBEvent*) self) = t_gcunit_new_(self, gb_event_disconnect_destroy);
  ((GBEvent*) self)->type   = GBEventType_DISCONNECT;
  ((GBEvent*) self)->handle = (GBEventHandleFunc) gb_event_disconnect_handle;
  self->sock = sock;
  
  return (GBEvent*) self;
}

void gb_event_disconnect_destroy(GBEventDisconnect* self) {
  assert(self != NULL);
  
  t_unref(self->sock);
}

void gb_event_disconnect_handle(GBEventDisconnect* self) {
  assert(self != NULL);
  
  pthread_mutex_lock(&self->sock->running_mtx);
  tl_important(self->sock->l, "Disconnected!");
  self->sock->running = true;
  pthread_mutex_unlock(&self->sock->running_mtx);
  // todo - implement reconnecting
}

