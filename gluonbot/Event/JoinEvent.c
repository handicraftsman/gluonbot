#include "../Event.h"
#include "../UserCache.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tiny-log.h>

static void gb_event_join_destroy(GBEventJoin* self);
static void gb_event_join_handle(GBEventJoin* self);

GBEvent* gb_event_join_new(GBIRCSocket* sock, char* nick, char* user, char* host, char* chan) {
  assert(sock != NULL);
  assert(nick != NULL);
  assert(user != NULL);
  assert(host != NULL);
  assert(chan != NULL);
  t_ref(sock);
  
  GBEventJoin* self = (GBEventJoin*) t_malloc(sizeof(GBEventJoin));
  if (!self) {
    perror("gb_event_join_new");
    exit(1);
  }
  
  t_gcunit((GBEvent*) self) = t_gcunit_new_(self, gb_event_join_destroy);
  ((GBEvent*) self)->type   = GBEventType_JOIN;
  ((GBEvent*) self)->handle = (GBEventHandleFunc) gb_event_join_handle;
  self->sock = sock;
  self->nick = strdup(nick);
  self->user = strdup(user);
  self->host = strdup(host);
  self->chan = strdup(chan);
  
  return (GBEvent*) self;
}

void gb_event_join_destroy(GBEventJoin* self) {
  assert(self != NULL);
  
  t_unref(self->sock);
  t_free(self->nick);
  t_free(self->user);
  t_free(self->host);
  t_free(self->chan);
}

void gb_event_join_handle(GBEventJoin* self) {
  assert(self != NULL);
  t_ref((GBEvent*) self);
  
  if (strcmp(self->nick, self->sock->nick) == 0) {
    gb_ircsocket_write(self->sock, "WHO %s\r\n", self->chan);
  }
  
  GBUserInfo* info = gb_user_cache_get(self->sock->user_cache, self->nick);
  pthread_mutex_lock(&info->mtx);
  if (info->user != NULL) {
    t_free(info->user);
  }
  info->user = strdup(self->user);
  if (info->host != NULL) {
    t_free(info->host);
  }
  info->user = strdup(self->host);
  pthread_mutex_unlock(&info->mtx);
  
  t_unref((GBEvent*) self);
}
