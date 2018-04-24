#include <gluonbot.h>
#include <gluonbot-plugin.h>

#include <tiny-log.h>

void test_handler(GBEventConnect* e) {
  tl_important(gb_self->l, "Hello, World! (%s)", e->sock->name);
};

void gb_init() {
  gb_register_handler(GBEventType_CONNECT, test_handler);
}
