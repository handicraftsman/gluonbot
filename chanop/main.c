#include <gluonbot.h>
#include <gluonbot-plugin.h>

#include <tiny-log.h>

GBCommand* cmd_kick;
void cmd_kick_handler(GBEventCommand* e) {}

void gb_init() {
  cmd_kick = gb_command_new((GBCommandInfo) {
    .name = "kick",
    .usage = "kick [channel] <nickname/mask>",
    .description = "Kicks given user from the current/given channel",
    .flag = "kick",
    .cooldown = 0,
    .handler = (GBCommandHandler) cmd_kick_handler
  });
  gb_register_command(cmd_kick);
}

