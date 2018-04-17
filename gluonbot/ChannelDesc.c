#include "ChannelDesc.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GBChannelDesc* gb_channeldesc_new(char* name, char* pass) {
  GBChannelDesc* self = (GBChannelDesc*) t_malloc(sizeof(GBChannelDesc));
  if (!self) {
    perror("gb_channeldesc_new");
    exit(1);
  }
  
  t_gcunit(self) = t_gcunit_new_(self, gb_channeldesc_destroy);
  self->name = strdup(name);
  self->pass = pass != NULL ? strdup(pass) : NULL;
  
  return self;
}

void gb_channeldesc_destroy(GBChannelDesc* self) {
  assert(self != NULL);
  
  if (self->name != NULL) t_free(self->name);
  if (self->pass != NULL) t_free(self->pass);
}
