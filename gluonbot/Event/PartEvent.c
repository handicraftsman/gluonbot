#include "../Event.h"
#include "../UserCache.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tiny-log.h>

static void gb_event_part_destroy(GBEventPart* self);
static void gb_event_part_handle(GBEventPart* self);

GBEvent* gb_event_part_new(GBIRCSocket* sock, char* nick, char* user, char* host, char* chan, char* reason) {
  assert(sock != NULL);
  assert(nick != NULL);
  assert(user != NULL);
  assert(host != NULL);
  assert(chan != NULL);
  assert(reason != NULL);
  t_ref(sock);
  
  GBEventPart* self = (GBEventPart*) t_malloc(sizeof(GBEventPart));
  if (!self) {
    perror("gb_event_part_new");
    exit(1);
  }
  
  t_gcunit((GBEvent*) self) = t_gcunit_new_(self, gb_event_part_destroy);
  ((GBEvent*) self)->type   = GBEventType_PART;
  ((GBEvent*) self)->handle = (GBEventHandleFunc) gb_event_part_handle;
  self->sock = sock;
  self->nick = strdup(nick);
  self->user = strdup(user);
  self->host = strdup(host);
  self->chan = strdup(chan);
  self->reason = strdup(reason);
  
  return (GBEvent*) self;
}

void gb_event_part_destroy(GBEventPart* self) {
  assert(self != NULL);
  
  t_unref(self->sock);
  t_free(self->nick);
  t_free(self->user);
  t_free(self->host);
  t_free(self->chan);
}

void gb_event_part_handle(GBEventPart* self) {}
