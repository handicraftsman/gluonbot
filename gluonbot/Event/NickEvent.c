#include "../Event.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tiny-log.h>

static void gb_event_nick_destroy(GBEventNick* self);
static void gb_event_nick_handle(GBEventNick* self);

GBEvent* gb_event_nick_new(GBIRCSocket* sock, char* nick, char* user, char* host, char* new_nick) {  
  assert(sock != NULL);
  assert(nick != NULL);
  assert(user != NULL);
  assert(host != NULL);
  assert(new_nick != NULL);
  t_ref(sock);
  
  GBEventNick* self = (GBEventNick*) t_malloc(sizeof(GBEventNick));
  if (!self) {
    perror("gb_event_nick_new");
    exit(1);
  }
  
  t_gcunit((GBEvent*) self) = t_gcunit_new_(self, gb_event_nick_destroy);
  ((GBEvent*) self)->type   = GBEventType_NICK;
  ((GBEvent*) self)->handle = (GBEventHandleFunc) gb_event_nick_handle;
  self->sock = sock;
  self->nick = strdup(nick);
  self->user = strdup(user);
  self->host = strdup(host);
  self->new_nick = strdup(new_nick);
  
  return (GBEvent*) self;
}

void gb_event_nick_destroy(GBEventNick* self) {
  assert(self != NULL);
}

void gb_event_nick_handle(GBEventNick* self) {
  assert(self != NULL);
  t_ref((GBEvent*) self);
  
  gb_user_cache_rename(self->sock->user_cache, self->nick, self->new_nick);
  
  t_unref((GBEvent*) self);
}


