#pragma once

#include <stdint.h>

#include <tiny.h>

typedef struct IRCSocket {
  t_gcunit_use();
  
  char* name;
  char* l;
  
  char* host;
  int   port;
  
  char* nick;
  char* user;
  char* pass;
  char* rnam;
  
  TList* autojoin;
} IRCSocket;

IRCSocket* gb_ircsocket_new(char* name);
void gb_ircsocket_destroy(IRCSocket* self);

void gb_ircsocket_dump(IRCSocket* self);
