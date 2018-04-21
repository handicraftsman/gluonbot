#pragma once

#include <stdint.h>

#include <tiny.h>

typedef struct GBIRCSocket {
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
} GBIRCSocket;

GBIRCSocket* gb_ircsocket_new(char* name);
void gb_ircsocket_destroy(GBIRCSocket* self);

void gb_ircsocket_dump(GBIRCSocket* self);

void gb_ircsocket_connect(GBIRCSocket* self);
