#pragma once

#include "Event.h"
#include "Command.h"

#include <libxml/tree.h>

#include <tiny.h>

typedef struct GBPlugin {
  t_gcunit_use();
  
  char* name;
  char* l;
  
  GBEventHandleFunc handlers[_GBEventType_SIZE];
  TMap* commands;
  
  void* handle;
} GBPlugin;

GBPlugin* gb_plugin_new(char* name, xmlNodePtr root);
void gb_plugin_destroy(GBPlugin* self);

void gb_plugin_register_handler(GBPlugin* self, GBEventType type, GBEventHandleFunc handler);
#define gb_register_handler(type, handler) gb_plugin_register_handler(gb_self, type, (GBEventHandleFunc) handler)

void gb_plugin_register_command(GBPlugin* self, GBCommand* cmd);
#define gb_register_command(cmd) gb_plugin_register_command(gb_self, cmd)
