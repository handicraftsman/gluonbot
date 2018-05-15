#pragma once

#include "IRCSocket.h"

#include <sqlite3.h>

#include <tiny.h>

/// \addtogroup Bot
/// @{

typedef struct _GBBot {
  TMap* sockets;
  TMap* plugins;
  TMap* groups;
  
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

/// @}
/// \addtogroup Flags
/// @{

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
TList* gb_flag_list(GBFlag* filter);
void gb_flag_remove(GBFlag* filter);
void gb_flag_insert(GBFlag* filter);

/// @}
