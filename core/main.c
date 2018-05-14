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

GBCommand* cmd_help;
void cmd_help_handler(GBEventCommand* e) { // check this
  if (e->split->len == 1) {
    gb_ircsocket_nreply(e,
      "Type `%slist` to list all plugins. "
      "Type `%slist <plugin>` to list all commands in the given plugin. "
      "Type `%shelp` to show this message. "
      "Type `%shelp <command>` to print a help message for the given command. ",
      GBBot.prefix, GBBot.prefix, GBBot.prefix, GBBot.prefix
    );
  } else if (e->split->len == 2) {
    bool found = false;
    t_list_foreach(GBBot.plugins->pairs, pluginn) {
      TMapPair* pluginp = pluginn->unit->obj;
      GBPlugin* plugin = pluginp->unit->obj;
  
      t_list_foreach(plugin->commands->pairs, commandn) {
        TMapPair* commandp = commandn->unit->obj;
        GBCommand* command = commandp->unit->obj;
        
        if (strcmp(command->name, e->split->arr[1]->obj) == 0) {
          found = true;
          
          bool can_execute = false;
          if (strcmp(command->flag, "world") == 0) {
            can_execute = true;
          } else {
            GBFlag* filter  = gb_flag_new();
            filter->server  = strdup(e->sock->name);
            filter->channel = strdup(e->target);
            filter->host    = strdup(e->host);
            filter->plugin  = strdup(pluginp->key);
            filter->name    = strdup(commandp->key);
            
            can_execute = gb_flag_is_set(filter);
            
            t_unref(filter);
          }
          
          gb_ircsocket_nreply(
            e,
            "%s/%s | Usage: %s | Description: %s | Cooldown: %ds | Flag: %s | %s",
            pluginp->key,
            commandp->key,
            command->usage,
            command->description,
            command->cooldown,
            command->flag,
            can_execute == true ? "You can execute this command" : "You cannot execute this command"
          );
          
          t_unref(commandn);
          break;
        }
      }
      
      if (found) {
        t_unref(pluginn);
        break;
      }
    }
  } else {
    gb_ircsocket_nreply(e, "Invalid arguments");
  }
}

GBCommand* cmd_list;
void cmd_list_handler(GBEventCommand* e) {
  if (e->split->len == 1) {
    TVector* pnames_vec = t_vector_new(0, 0);
    t_list_foreach(GBBot.plugins->pairs, pluginp) {
      TMapPair* pluginn = pluginp->unit->obj;
      GBPlugin* plugin = pluginn->unit->obj;
      t_vector_push_(pnames_vec, t_gcunit_new_(strdup(plugin->name), t_free));
    }
    char* pnames = gbu_strjoin(pnames_vec, ", ");
    gb_ircsocket_nreply(e, "Plugins: %s", pnames);
    t_free(pnames);
    t_unref(pnames_vec);    
  } else if (e->split->len == 2) {
    t_list_foreach(GBBot.plugins->pairs, pluginn) {
      TMapPair* pluginp = pluginn->unit->obj;
      GBPlugin* plugin = pluginp->unit->obj;
      
      if (strcmp(plugin->name, e->split->arr[1]->obj) == 0) {
        TVector* cnames_vec = t_vector_new(0, 0);
        t_list_foreach(plugin->commands->pairs, commandn) {
          TMapPair* commandp = commandn->unit->obj;
          GBPlugin* command = commandp->unit->obj;
          t_vector_push_(cnames_vec, t_gcunit_new_(strdup(command->name), t_free));
        }
        char* cnames = gbu_strjoin(cnames_vec, ", ");
        gb_ircsocket_nreply(e, "%s: %s", plugin->name, cnames);
        t_free(cnames);
        t_unref(cnames_vec);
        t_unref(pluginn);
        break;
      }
    }
  } else {
    gb_ircsocket_nreply(e, "Invalid arguments");
  }
}

GBCommand* cmd_join;
void cmd_join_handler(GBEventCommand* e) {
  if (e->split->len == 2) {
    char* channel = e->split->arr[1]->obj;
    gb_ircsocket_join(e->sock, channel, NULL);
    gb_ircsocket_nreply(e, "Done!");
  } else if (e->split->len == 3) {
    char* channel = e->split->arr[1]->obj;
    char* password = e->split->arr[2]->obj;
    gb_ircsocket_join(e->sock, channel, password);
    gb_ircsocket_nreply(e, "Done!");    
  } else {
    gb_ircsocket_nreply(e, "Invalid arguments");
  }
}

GBCommand* cmd_part;
void cmd_part_handler(GBEventCommand* e) {
  char* channel;
  char* reason = NULL;
  if (e->split->len == 2) {
    channel = e->split->arr[1]->obj;
  } else if (e->split->len > 2) {
    channel = e->split->arr[1]->obj;
    reason = gbu_strjoin_offset(e->split, " ", 2);
  } else {
    gb_ircsocket_nreply(e, "Invalid arguments");
    return;
  }
  gb_ircsocket_part(e->sock, channel, reason);
}

void gb_init() {
  gb_register_handler(GBEventType_CONNECT, connect_handler);
  
  cmd_key = gb_command_new((GBCommandInfo) {
    .name = "key",
    .usage = "key [key]",
    .description = "Generates a key and prints it to stdout. If you give this command the printed key, you'll get admin permissions",
    .flag = "world",
    .cooldown = 0,
    .handler = (GBCommandHandler) cmd_key_handler
  });
  gb_register_command(cmd_key);
  
  cmd_help = gb_command_new((GBCommandInfo) {
    .name = "help",
    .usage = "help [command]",
    .description = "Displays a general help message or a help message for the given command",
    .flag = "world",
    .cooldown = 3,
    .handler = (GBCommandHandler) cmd_help_handler
  });
  gb_register_command(cmd_help);
  
  cmd_list = gb_command_new((GBCommandInfo) {
    .name = "list",
    .usage = "list [plugin]",
    .description = "Lists plugins and their commands",
    .flag = "world",
    .cooldown = 3,
    .handler = (GBCommandHandler) cmd_list_handler
  });
  gb_register_command(cmd_list);
  
  cmd_join = gb_command_new((GBCommandInfo) {
    .name = "join",
    .usage = "join <channel> [password]",
    .description = "Joins the given channel",
    .flag = "joinpart",
    .cooldown = 0,
    .handler = (GBCommandHandler) cmd_join_handler
  });
  gb_register_command(cmd_join);
  
  cmd_part = gb_command_new((GBCommandInfo) {
    .name = "part",
    .usage = "part <channel> [reason]",
    .description = "Parts the given channel",
    .flag = "joinpart",
    .cooldown = 0,
    .handler = (GBCommandHandler) cmd_part_handler
  });
  gb_register_command(cmd_part);
}

void gb_deinit() {
  t_unref(cmd_key);
  t_unref(cmd_help);
  t_unref(cmd_list);
  t_unref(cmd_join);
}
