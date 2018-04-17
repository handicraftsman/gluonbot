#define _GNU_SOURCE

#include "Plugin.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GBPlugin* gb_plugin_new(char* name) {
  GBPlugin* self = (GBPlugin*) t_malloc(sizeof(GBPlugin));
  if (!self) {
    perror("gb_channeldesc_new");
    exit(1);
  }
  
  t_gcunit(self) = t_gcunit_new_(self, gb_plugin_destroy);
  
  self->name = strdup(name);
  
  self->l = (char*) malloc(strlen(self->name) + 2);
  sprintf(self->l, "?%s", self->name);
  
  memset(self->handlers, 0, (sizeof(GBEventHandleFunc) * _GBEventType_SIZE));
  
  self->handle = NULL; // implement this
  
  return self;
}

void gb_plugin_destroy(GBPlugin* self) {
  assert(self != NULL);
  
  t_free(self->name);
  t_free(self->l);
}

