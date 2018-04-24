#include "../Event.h"
#include "../ChannelDesc.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tiny-log.h>

static void gb_event_code_destroy(GBEventCode* self);
static void gb_event_code_handle(GBEventCode* self);

GBEvent* gb_event_code_new(GBIRCSocket* sock, int code, char* extra) {  
  assert(sock != NULL);
  assert(extra != NULL);
  t_ref(sock);
  
  GBEventCode* self = (GBEventCode*) t_malloc(sizeof(GBEventCode));
  if (!self) {
    perror("gb_event_code_new");
    exit(1);
  }
  
  t_gcunit((GBEvent*) self) = t_gcunit_new_(self, gb_event_code_destroy);
  ((GBEvent*) self)->type   = GBEventType_CODE;
  ((GBEvent*) self)->handle = (GBEventHandleFunc) gb_event_code_handle;
  self->sock = sock;
  self->code = code;
  self->extra = strdup(extra);
  
  return (GBEvent*) self;
}

void gb_event_code_destroy(GBEventCode* self) {
  assert(self != NULL);
  
  t_unref(self->sock);
  t_free(self->extra);
}

void gb_event_code_handle(GBEventCode* self) {
  assert(self != NULL);
  t_ref((GBEvent*) self);
  
  if (self->code == 1) {
    t_list_foreach(self->sock->autojoin, n) {
      GBChannelDesc* d = n->unit->obj;
      gb_ircsocket_join(self->sock, d->name, d->pass);
    }
  }
  
  t_unref((GBEvent*) self);
}

