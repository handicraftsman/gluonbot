#include "Command.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tiny-log.h>

GBCommand* gb_command_new(GBCommandInfo info) {
  assert(info.name != NULL);
  if (info.description == NULL) info.description = "No description available."; 
  if (info.usage == NULL) info.usage = "[OPTIONS...]"; 
  if (info.flag == NULL) {
    tl_warning("commands", "%s is not bound to a flag. Setting to `unbound`", info.name);
  }
  
  GBCommand* self = (GBCommand*) t_malloc(sizeof(GBCommand));
  if (!self) {
    perror("gb_command_new");
    exit(1);
  }
  
  t_gcunit(self) = t_gcunit_new_(self, gb_command_destroy);
  self->name = strdup(info.name);
  self->description = strdup(info.description);
  self->usage = strdup(info.usage);
  self->cooldown = info.cooldown;
  self->flag = info.flag == NULL ? strdup("unbound") : strdup(info.flag);
  self->handler = info.handler;
  self->cooldowns = t_map_new();
  
  return self;
}

void gb_command_destroy(GBCommand* self) {
  t_free(self->name);
  t_free(self->description);
  t_free(self->usage);
  if (self->flag != NULL) t_free(self->flag);
}
