#include "../Event.h"
#include "../IRCSocket.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tiny-log.h>

typedef struct GBEventMessage {
  GBEvent _parent;
  
  GBIRCSocket* sock;
  char* msg;
} GBEventMessage;

static void gb_event_message_destroy(GBEventMessage* self);
static void gb_event_message_handle(GBEventMessage* self);

static GBEventVTable gb_event_message_vtable = {
  .handle = (GBEventHandleFunc) gb_event_message_handle
};

GBEvent* gb_event_message_new(GBIRCSocket* sock, char* msg) {  
  assert(sock != NULL);
  t_ref(sock);
  
  GBEventMessage* self = (GBEventMessage*) t_malloc(sizeof(GBEventMessage));
  if (!self) {
    perror("gb_event_message_new");
    exit(1);
  }
  
  t_gcunit((GBEvent*) self) = t_gcunit_new_(self, gb_event_message_destroy);
  ((GBEvent*) self)->type   = GBEventType_MESSAGE;
  ((GBEvent*) self)->vtable = &gb_event_message_vtable;
  self->sock = sock;
  self->msg  = strdup(msg);
  
  return (GBEvent*) self;
}

void gb_event_message_destroy(GBEventMessage* self) {
  assert(self != NULL);
  
  t_unref(self->sock);
  t_free(self->msg);
}

void gb_event_message_handle(GBEventMessage* self) {
  assert(self != NULL);
  t_ref((GBEvent*) self);
  
  tl_io(self->sock->l, "R> %s", self->msg);

  t_unref((GBEvent*) self);
}
