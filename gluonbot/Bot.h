#pragma once

#include "IRCSocket.h"

#include <sqlite3.h>

#include <tiny.h>

typedef struct _GBBot {
  TMap* sockets;
  TMap* plugins;
  
  char* prefix;
  
  char* db_path;
  sqlite3* database;
} _GBBot;

_GBBot GBBot;

void gb_bot_init();
void gb_bot_deinit();

void gb_bot_load_config(char* config_path);
void gb_bot_load_database();
void gb_bot_connect();

typedef struct GBFlag {
  t_gcunit_use();
  
  char* server;
  char* channel;
  char* host;
  char* plugin;
  char* name;
} GBFlag;

GBFlag* gb_flag_new();
void gb_flag_destroy(GBFlag* self);
bool gb_flag_is_set(GBFlag* self);
