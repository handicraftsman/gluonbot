#pragma once

#include <tiny.h>

typedef struct IRCSocket {
  t_gcunit_use();
  
  char* name;
  char* l;
  
  char* nick;
  char* user;
  char* pass;
  char* rnam;
} IRCSocket;

IRCSocket* gb_ircsocket_new(char* name);
void gb_ircsocket_destroy(IRCSocket* self);
