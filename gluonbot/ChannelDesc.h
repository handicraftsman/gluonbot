#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <tiny.h>

typedef struct GBChannelDesc {
  t_gcunit_use();
  
  char* name;
  char* pass;
} GBChannelDesc;

GBChannelDesc* gb_channeldesc_new(char* name, char* pass);
void gb_channeldesc_destroy(GBChannelDesc* self);
