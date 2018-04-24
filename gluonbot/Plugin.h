#pragma once

#include "Event.h"

#include <tiny.h>

typedef struct GBPlugin {
  t_gcunit_use();
  
  char* name;
  char* l;
  
  GBEventHandleFunc handlers[_GBEventType_SIZE];
  
  void* handle;
} GBPlugin;

GBPlugin* gb_plugin_new(char* name);
void gb_plugin_destroy(GBPlugin* self);

void gb_plugin_register_handler(GBPlugin* self, GBEventType type, GBEventHandleFunc handler);
#define gb_register_handler(type, handler) gb_plugin_register_handler(gb_self, type, (GBEventHandleFunc) handler)
