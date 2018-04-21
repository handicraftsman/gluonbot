#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tiny-log.h>

#include "ChannelDesc.h"
#include "Event.h"
#include "IRCSocket.h"

GBIRCSocket* gb_ircsocket_new(char* name) {
  GBIRCSocket* self = (GBIRCSocket*) t_malloc(sizeof(GBIRCSocket));
  if (!self) {
    perror("gb_ircsocket_new");
    exit(1);
  }
  
  t_gcunit(self) = t_gcunit_new_(self, gb_ircsocket_destroy);
  
  self->name = strdup(name);
  
  self->l = (char*) malloc(strlen(self->name) + 2);
  sprintf(self->l, "!%s", self->name);
  
  self->host = strdup("127.0.0.1");
  self->port = 6667;
  
  self->nick = strdup("GluonBot");
  self->user = strdup("GluonBot");
  self->pass = NULL;
  self->rnam = strdup("An IRC bot in C");
  
  self->autojoin = NULL;
  
  tl_important(self->l, "Hello, IRC!");
  
  return self;
}

void gb_ircsocket_destroy(GBIRCSocket* self) {
  assert(self != NULL);
  
  if (self->name != NULL) t_free(self->name);
  if (self->l != NULL) t_free(self->l);
  
  if (self->host != NULL) t_free(self->host);
  
  if (self->nick != NULL) t_free(self->nick);
  if (self->user != NULL) t_free(self->user);
  if (self->pass != NULL) t_free(self->pass);
  if (self->rnam != NULL) t_free(self->rnam);
  
  if (self->autojoin != NULL) t_unref(self->autojoin);
}

void gb_ircsocket_dump(GBIRCSocket* self) {
  assert(self != NULL);
  t_ref(self);
  
  tl_debug(self->l, "nick <- %s", self->nick);
  tl_debug(self->l, "user <- %s", self->user);
  tl_debug(self->l, "has-pass <- %d", self->pass != NULL);
  tl_debug(self->l, "rnam <- %s", self->rnam);
  
  t_list_foreach(self->autojoin, n) {
    GBChannelDesc* chan = n->unit->obj;
    tl_debug(self->l, "autojoin <- %s %d", chan->name, chan->pass != NULL);
  }
  
  t_unref(self);
}

void gb_ircsocket_connect(GBIRCSocket* self) {
  assert(self != NULL);
  t_ref(self);
  
  GBEvent* e = gb_event_connect_new(self);
  gb_event_fire(e);
  t_unref(e);
  
  t_unref(self);
}
