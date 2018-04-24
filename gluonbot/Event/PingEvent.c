#include "../Event.h"
#include "../IRCSocket.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tiny-log.h>

static void gb_event_ping_destroy(GBEventPing* self);
static void gb_event_ping_handle(GBEventPing* self);

GBEvent* gb_event_ping_new(GBIRCSocket* sock, char* target) {
  assert(sock != NULL);
  assert(target != NULL);
  t_ref(sock);
  
  GBEventPing* self = (GBEventPing*) t_malloc(sizeof(GBEventPing));
  if (!self) {
    perror("gb_event_ping_new");
    exit(1);
  }
  
  t_gcunit((GBEvent*) self) = t_gcunit_new_(self, gb_event_ping_destroy);
  ((GBEvent*) self)->type   = GBEventType_PING;
  ((GBEvent*) self)->handle = (GBEventHandleFunc) gb_event_ping_handle;
  self->sock = sock;
  self->target = strdup(target);
  
  return (GBEvent*) self;
}

void gb_event_ping_destroy(GBEventPing* self) {
  assert(self != NULL);

  t_unref(self->sock);
  t_free(self->target);
}

void gb_event_ping_handle(GBEventPing* self) {
  assert(self != NULL);
  t_ref((GBEvent*) self);
  
  gb_ircsocket_write(self->sock, "PONG :%s\r\n", self->target);
  
  t_unref((GBEvent*) self);
}

