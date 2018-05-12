#pragma once

#include <tiny.h>

#include "Event.h"

typedef void (*GBCommandHandler)(GBEventCommand* e);

typedef struct GBCommand {
  t_gcunit_use();
  
  char* name;
  char* description;
  char* usage;
  int cooldown;
  char* flag;
  GBCommandHandler handler;
  
  // map    -> map     -> long long
  // server -> channel -> host
  TMap* cooldowns;
} GBCommand;

typedef struct GBCommandInfo {
  char* name;
  char* description;
  char* usage;
  int cooldown;
  char* flag;
  GBCommandHandler handler;
} GBCommandInfo;

GBCommand* gb_command_new(GBCommandInfo info);
void gb_command_destroy();
