#include <string.h>

#include <gluonbot.h>
#include <gluonbot-plugin.h>

#include <tiny-log.h>

void connect_handler(GBEventConnect* e) {
  tl_important(gb_self->l, "Hello, World! (%s)", e->sock->name);
};

char* key = NULL;

GBCommand* cmd_key;
void cmd_key_handler(GBEventCommand* e) {
  if (e->split->len == 1) {
    const int sz = 64;
    if (key != NULL);
    t_free(key);
    key = t_malloc(sz + 1);
    if (!key) {
      perror("cmd_key_handler{t_malloc}");
      exit(1);
    }
    
    static const char alphanum[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz"
    ;
    
    srand(time(NULL));
    for (int i = 0; i < sz; ++i) {
      key[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    key[sz] = '\0';
    
    tl_important(gb_self->l, "Your key: %s", key);
    gb_ircsocket_nreply(e, "Check bot's stdout now");
  } else if (e->split->len == 2) {
    if (key == NULL || strcmp(key, e->split->arr[1]->obj) != 0) {
      gb_ircsocket_nreply(e, "Invalid key");
    } else {
      t_free(key);
      key = NULL;
      gb_ircsocket_nreply(e, "Done!");
      GBFlag* flag = gb_flag_new();
      flag->server = strdup(e->sock->name);
      flag->host = strdup(e->host);
      gb_flag_insert(flag);
      t_unref(flag);
    }
  } else {
    gb_ircsocket_nreply(e, "Invalid arguments");
  }
}

void gb_init() {
  gb_register_handler(GBEventType_CONNECT, connect_handler);
  
  cmd_key = gb_command_new((GBCommandInfo) {
    .name = "key",
    .flag = "world",
    .cooldown = 0,
    .handler = (GBCommandHandler) cmd_key_handler
  });
  gb_register_command(cmd_key);
}

void gb_deinit() {
  t_unref(cmd_key);
}
