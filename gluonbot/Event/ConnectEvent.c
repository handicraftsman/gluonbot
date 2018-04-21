#include "../Event.h"
#include "../IRCSocket.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <tiny-log.h>

/*
 * Connect
 */

typedef struct GBEventConnect {
  GBEvent _parent;

  GBIRCSocket* sock;
} GBEventConnect;

static void gb_event_connect_destroy(GBEventConnect* self);
static void gb_event_connect_handle(GBEventConnect* self);

static GBEventVTable gb_event_connect_vtable = {
  .handle = (GBEventHandleFunc) gb_event_connect_handle
};

GBEvent* gb_event_connect_new(GBIRCSocket* sock) {  
  assert(sock != NULL);
  t_ref(sock);
  
  GBEventConnect* self = (GBEventConnect*) t_malloc(sizeof(GBEventConnect));
  if (!self) {
    perror("gb_event_connect_new");
    exit(1);
  }
  
  t_gcunit((GBEvent*) self) = t_gcunit_new_(self, gb_event_connect_destroy);
  ((GBEvent*) self)->type   = GBEventType_CONNECT;
  ((GBEvent*) self)->vtable = &gb_event_connect_vtable;
  self->sock = sock;
  
  return (GBEvent*) self;
}

void gb_event_connect_destroy(GBEventConnect* self) {
  t_unref(self->sock);
}

void gb_event_connect_handle(GBEventConnect* self) {
  tl_important(self->sock->l, "Connected!");
}
