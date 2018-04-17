#include <gluonbot.h>
#include <gluonbot-plugin.h>

#include <tiny-log.h>

void gb_init() {
  tl_info(gb_self->l, "Hello, World!");
}

void gb_deinit() {
  tl_info(gb_self->l, "Bye, World!");
}
