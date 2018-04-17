#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tiny-log.h>

#include "IRCSocket.h"

IRCSocket* gb_ircsocket_new(char* name) {
  IRCSocket* self = (IRCSocket*) t_malloc(sizeof(IRCSocket));
  if (!self) {
    perror("gb_ircsocket_new");
    exit(1);
  }
  
  t_gcunit(self) = t_gcunit_new_(self, gb_ircsocket_destroy);
  self->name = strdup(name);
  
  self->l = (char*) malloc(strlen(self->name) + 2);
  sprintf(self->l, "!%s", self->name);
  
  self->nick = strdup("GluonBot");
  self->user = strdup("GluonBot");
  self->pass = NULL;
  self->rnam = strdup("An IRC bot in C");
  
  tl_important(self->l, "Hello, World!");
  
  return self;
}

void gb_ircsocket_destroy(IRCSocket* self) {
  if (self->name != NULL) free(self->name);
  if (self->l != NULL) free(self->l);
  
  if (self->nick != NULL) free(self->nick);
  if (self->user != NULL) free(self->user);
  if (self->pass != NULL) free(self->pass);
  if (self->rnam != NULL) free(self->rnam);
}
