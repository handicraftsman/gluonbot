#include <gluonbot.h>
#include <gluonbot-plugin.h>

#include <tiny-log.h>

void connect_handler(GBEventConnect* e) {
  tl_important(gb_self->l, "Hello, World! (%s)", e->sock->name);
};

GBCommand* cmd_key;
void cmd_key_handler(GBEventCommand* e) {
  gb_ircsocket_reply(e, "NYI");
}

void gb_init() {
  gb_register_handler(GBEventType_CONNECT, connect_handler);
  
  cmd_key = gb_command_new((GBCommandInfo) {
    .name = "key",
    .flag = "world",
    .cooldown = 10,
    .handler = (GBCommandHandler) cmd_key_handler
  });
  gb_register_command(cmd_key);
}

void gb_deinit() {
  t_unref(cmd_key);
}
