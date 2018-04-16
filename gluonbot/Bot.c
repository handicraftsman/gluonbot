#include "Bot.h"

#include <json.h>
#include <tiny-log.h>

_GBBot GBBot = {};

static char* l = "bot";

void gb_bot_init() {
  tl_important(l, "Hello, main!");
  
  GBBot.sockets = t_map_new();
  GBBot.plugins = t_map_new();
}

void gb_bot_load_config(char* config_path) {
  //tl_info("Loading config...");
  // todo - load config
}

void gb_bot_deinit() {
  t_unref(GBBot.sockets);
  t_unref(GBBot.plugins);
}
