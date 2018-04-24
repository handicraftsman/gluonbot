#pragma once

#include <stdbool.h>

#include <pthread.h>

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
  
  int fd;
  bool running;
  pthread_mutex_t running_mtx;
  
  TList* queue;
  pthread_mutex_t queue_mtx;
  long long last_write;
} GBIRCSocket;

GBIRCSocket* gb_ircsocket_new(char* name);
void gb_ircsocket_destroy(GBIRCSocket* self);

void gb_ircsocket_dump(GBIRCSocket* self);

void gb_ircsocket_connect(GBIRCSocket* self);
void gb_ircsocket_io_loop(GBIRCSocket* self);

void gb_ircsocket_write(GBIRCSocket* self, char* fmt, ...);
void gb_ircsocket_join(GBIRCSocket* self, char* chan, char* pass);
