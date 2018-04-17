#pragma once

#include "IRCSocket.h"

#include <tiny.h>

typedef struct _GBBot {
  TMap* sockets;
  TMap* plugins;
} _GBBot;

_GBBot GBBot;

void gb_bot_init();
void gb_bot_load_config(char* config_path);
void gb_bot_deinit();

