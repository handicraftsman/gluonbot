#include <gluonbot.h>
#include <gluonbot-plugin.h>

#include <tiny-log.h>

void connect_handler(GBEventConnect* e) {
  tl_important(gb_self->l, "Hello, World! (%s)", e->sock->name);
};

GBCommand* cmd_ping;
void cmd_ping_handler(GBEventCommand* e) {
  gb_ircsocket_reply(e, "Pong!");
}

void gb_init() {
  gb_register_handler(GBEventType_CONNECT, connect_handler);
  
  cmd_ping = gb_command_new((GBCommandInfo) {
    .name = "ping",
    .flag = "ping",
    .cooldown = 10,
    .handler = (GBCommandHandler) cmd_ping_handler
  });
  gb_register_command(cmd_ping);
}

void gb_deinit() {
  t_unref(cmd_ping);
}
