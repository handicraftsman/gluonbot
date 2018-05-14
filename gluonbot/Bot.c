#define _GNU_SOURCE

#include "Bot.h"
#include "Bot.h"
#include "ChannelDesc.h"
#include "IRCSocket.h"
#include "Plugin.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libxml/tree.h>
#include <libxml/parser.h>

#include <tiny.h>
#include <tiny-log.h>

_GBBot GBBot = {};

static char* l = "bot";

void gb_bot_init() {
  tl_important(l, "Hello, main!");
  
  GBBot.sockets = t_map_new();
  GBBot.plugins = t_map_new();
  
  GBBot.prefix = strdup("!");
  GBBot.db_path = strdup("./gluonbot.db");
}

void gb_bot_deinit() {
  t_unref(GBBot.sockets);
  t_unref(GBBot.plugins);
  
  t_free(GBBot.prefix);
  if (GBBot.db_path != NULL) t_free(GBBot.db_path);
  if (GBBot.database != NULL) sqlite3_close(GBBot.database);
}


static void init_server(xmlNodePtr node) {
  char* name = NULL;
  char* host = NULL;
  int port = 6667;
  
  char* nick = NULL;
  char* user = NULL;
  char* pass = NULL;
  char* rnam = NULL;
  
  TList* autojoin = t_list_new();
  
  for (xmlAttrPtr attr = node->properties; attr != NULL; attr = attr->next) {
    if (strcmp((char*) attr->name, "name") == 0) {
      name = (char*) xmlNodeListGetString(node->doc, attr->children, 1);
    }
  }
  
  for (xmlNodePtr c = node->children; c != NULL; c = c->next) {
    if (strcmp((char*) c->name, "address") == 0) {
      for (xmlAttrPtr addr_attr = c->properties; addr_attr != NULL; addr_attr = addr_attr->next) {
        if (strcmp((char*) addr_attr->name, "host") == 0) {
          host = (char*) xmlNodeListGetString(c->doc, addr_attr->children, 1);
        } else if (strcmp((char*) addr_attr->name, "port") == 0) {
          char* pstr = (char*) xmlNodeListGetString(c->doc, addr_attr->children, 1);
          port = atoi(pstr);
          xmlFree(pstr);
        } 
      }
    } else if (strcmp((char*) c->name, "credentials") == 0) {
      for (xmlAttrPtr cred_attr = c->properties; cred_attr != NULL; cred_attr = cred_attr->next) {
        if (strcmp((char*) cred_attr->name, "nick") == 0) {
          nick = (char*) xmlNodeListGetString(c->doc, cred_attr->children, 1);
        } else if (strcmp((char*) cred_attr->name, "user") == 0) {
          user = (char*) xmlNodeListGetString(c->doc, cred_attr->children, 1);
        } else if (strcmp((char*) cred_attr->name, "pass") == 0) {
          pass = (char*) xmlNodeListGetString(c->doc, cred_attr->children, 1);
        } else if (strcmp((char*) cred_attr->name, "rnam") == 0) {
          rnam = (char*) xmlNodeListGetString(c->doc, cred_attr->children, 1);
        }
      }
    } else if (strcmp((char*) c->name, "channel") == 0) {
      char* cname = NULL;
      char* pass = NULL;
      for (xmlAttrPtr chan_attr = c->properties; chan_attr != NULL; chan_attr = chan_attr->next) {
        if (strcmp((char*) chan_attr->name, "name") == 0) {
          cname = (char*) xmlNodeListGetString(c->doc, chan_attr->children, 1);
        } else if (strcmp((char*) chan_attr->name, "pass") == 0) {
          pass= (char*) xmlNodeListGetString(c->doc, chan_attr->children, 1);
        }
      }
      
      if (cname != NULL) {
        GBChannelDesc* chan = gb_channeldesc_new(cname, pass);
        t_unref(t_list_append_(autojoin, chan));
        t_unref(chan);
      }
      
      if (cname != NULL) xmlFree(cname);
      if (pass  != NULL) xmlFree(pass);
    }
  }
  
  bool ok = true;
  
  if (name == NULL) {
    tl_error(l, "Noticed an unnamed server node! (line %d)", node->line);
    ok = false;
  }
  
  if (host == NULL) {
    tl_error(l, "No host specified! (line %d)", node->line);
    ok = false;
  }
  
  if (nick == NULL) {
    tl_error(l, "No nick specified! (line %d)", node->line);
    ok = false;
  }
  
  if (ok) {
    GBIRCSocket* sock = gb_ircsocket_new(name);
  
    free(sock->host);
    sock->host = strdup(host);
    sock->port = port;
   
    if (nick != NULL) {
      free(sock->nick);
      sock->nick = strdup(nick);
    }
    
    if (user != NULL) {
      free(sock->user);
      sock->user = strdup(user);
    }
    
    if (pass != NULL) {
      sock->pass = strdup(pass);
    }
    
    if (rnam != NULL) {
      free(sock->rnam);
      sock->rnam = strdup(rnam);
    }
    
    sock->autojoin = autojoin;
    
    gb_ircsocket_dump(sock);
    
    t_unref(t_map_set_(GBBot.sockets, name, sock));
    
    t_unref(sock);
  }
  
  if (name != NULL) xmlFree(name);
  if (host != NULL) xmlFree(host);
  if (nick != NULL) xmlFree(nick);
  if (user != NULL) xmlFree(user);
  if (pass != NULL) xmlFree(pass);
  if (rnam != NULL) xmlFree(rnam);
  
  if (!ok) return;
}

void gb_bot_load_config(char* config_path) {
  tl_info(l, "Loading config...");

  LIBXML_TEST_VERSION

  xmlDocPtr doc = xmlReadFile(config_path, NULL, 0);
  if (doc == NULL) {
    tl_critical(l, "Failed to parse %s", config_path);
    exit(1);
  }
  
  xmlNodePtr root = xmlDocGetRootElement(doc);
  
  if (strcmp((char*) root->name, "gluonbot-config") != 0) {
    tl_critical(l, "Invalid root element name: %s", root->name);
    exit(1);
  }
  
  for (xmlAttrPtr attr = root->properties; attr != NULL; attr = attr->next) {
    if (strcmp((char*) attr->name, "prefix") == 0) {
      char* p = (char*) xmlNodeListGetString(root->doc, attr->children, 1);
      GBBot.prefix = strdup(p);
      tl_debug(l, "prefix <- %s", GBBot.prefix); 
      xmlFree(p);      
    } else if (strcmp((char*) attr->name, "db-path") == 0) {
      char* p = (char*) xmlNodeListGetString(root->doc, attr->children, 1);
      GBBot.db_path = strdup(p);
      xmlFree(p);
    }
  }
  
  for(xmlNodePtr c = root->children; c != NULL; c = c->next) {
    if (strcmp((char*) c->name, "server") == 0) {
      init_server(c);
    } else if (strcmp((char*) c->name, "plugin") == 0) {
      char* name = NULL;
      
      for (xmlAttrPtr attr = c->properties; attr != NULL; attr = attr->next) {
        if (strcmp((char*) attr->name, "name") == 0) {
          name = (char*) xmlNodeListGetString(root->doc, attr->children, 1);
          tl_debug(l, "plugins <- %s", name);
        }
      }
      
      if (name == NULL) {
        tl_error(l, "Noticed an unnamed plugin node! (line %d)", c->line);
        continue;
      }
      
      GBPlugin* p = gb_plugin_new(name, c);
      t_unref(t_map_set_(GBBot.plugins, name, p));
      t_unref(p);
      
      xmlFree(name);
    }
  }
  
  bool has_core = false;
  t_list_foreach(GBBot.plugins->pairs, pluginn) {
    TMapPair* pluginp = pluginn->unit->obj;
    GBPlugin* plugin = pluginp->unit->obj;
    if (strcmp(plugin->name, "core") == 0) {
      has_core = true;
      t_unref(pluginn);
      break;
    }
  }
  if (!has_core) {
    tl_error(l, "No core plugin!");
    exit(1);
  }
  
  xmlFreeDoc(doc);
  
  xmlCleanupParser();
}

void gb_bot_load_database() {
  sqlite3_open(GBBot.db_path, &GBBot.database);
  if (!GBBot.database) {
    const char* emsg = sqlite3_errmsg(GBBot.database);
    tl_error(l, "%s", emsg);
    exit(1);
  }
  
  char* emsg = NULL;
  
  sqlite3_exec(
    GBBot.database, 
    "CREATE TABLE IF NOT EXISTS flags ("
      "server VARCHAR(64) NOT NULL, "
      "channel VARCHAR(64), "
      "host VARCHAR(64), "
      "plugin VARCHAR(64), "
      "name VARCHAR(64)"
    ");",
    NULL,
    NULL,
    &emsg
  );
  if (emsg) {
    tl_error(l, "%s", emsg);
    exit(1);
  }
}

void gb_bot_connect() {
  t_list_foreach(GBBot.sockets->pairs, n) {
    TMapPair* p = n->unit->obj;
    GBIRCSocket* sock = p->unit->obj;
    pthread_t thr;
    pthread_create(&thr, NULL, (void*(*)(void*)) gb_ircsocket_connect, sock);
    pthread_detach(thr);
  }
}


GBFlag* gb_flag_new() {
  GBFlag* self = (GBFlag*) t_malloc(sizeof(GBFlag));
  if (!self) {
    perror("gb_flag_new");
    exit(1);
  }
  
  t_gcunit(self) = t_gcunit_new_(self, gb_flag_destroy);
  self->server  = NULL;
  self->channel = NULL;
  self->host    = NULL;
  self->plugin  = NULL;
  self->name    = NULL;
  
  return self;
}

void gb_flag_destroy(GBFlag* self) {
  assert(self != NULL);
  if (self->server != NULL)  t_free(self->server);
  if (self->channel != NULL) t_free(self->channel);
  if (self->host != NULL)    t_free(self->host);
  if (self->plugin != NULL)  t_free(self->plugin);
  if (self->name != NULL)    t_free(self->name);
}


bool gb_flag_is_set(GBFlag* self) {
  assert(self != NULL);
  assert(self->server != NULL);
  assert(self->channel != NULL);
  assert(self->host != NULL);
  assert(self->plugin != NULL);
  assert(self->name != NULL);
  
  sqlite3_stmt* stmt;
  if (sqlite3_prepare(
    GBBot.database,
    "SELECT "
      "EXISTS(SELECT name FROM flags WHERE server=? AND channel=? AND host=? AND plugin=? AND name=? LIMIT 1) "
    "OR "
      "EXISTS(SELECT name FROM flags WHERE server=? AND channel=? AND plugin=? AND name=? AND host IS NULL LIMIT 1) "
    "OR "
      "EXISTS(SELECT name FROM flags WHERE server=? AND host=? AND plugin=? AND name=? AND channel IS NULL LIMIT 1) "
    "OR "
      "EXISTS(SELECT name FROM flags WHERE server=? AND plugin=? AND name=? AND NOT channel IS NULL AND host IS NULL LIMIT 1) "
    "OR "
  
      "EXISTS(SELECT name FROM flags WHERE server=? AND channel=? AND host=? AND plugin=? AND name IS NULL LIMIT 1) "
    "OR "
      "EXISTS(SELECT name FROM flags WHERE server=? AND channel=? AND plugin=? AND host IS NULL AND name IS NULL LIMIT 1) "
    "OR "
      "EXISTS(SELECT name FROM flags WHERE server=? AND host=? AND plugin=? AND channel IS NULL AND name IS NULL LIMIT 1) "
    "OR "
      "EXISTS(SELECT name FROM flags WHERE server=? AND plugin=? AND channel IS NULL AND host IS NULL AND name IS NULL LIMIT 1) "
    "OR "
  
      "EXISTS(SELECT name FROM flags WHERE server=? AND channel=? AND host=? AND plugin IS NULL AND name IS NULL LIMIT 1) "
    "OR "
      "EXISTS(SELECT name FROM flags WHERE server=? AND channel=? AND host IS NULL AND plugin IS NULL AND name IS NULL LIMIT 1) "
    "OR "
      "EXISTS(SELECT name FROM flags WHERE server=? AND host=? AND channel IS NULL AND plugin IS NULL AND name IS NULL LIMIT 1) "
    "OR "
      "EXISTS(SELECT name FROM flags WHERE server=? AND channel IS NULL AND host IS NULL AND plugin IS NULL AND name IS NULL LIMIT 1);"
    ,
    -1,
    &stmt,
    NULL
  ) != SQLITE_OK) {
    tl_error(l, "%s", sqlite3_errmsg(GBBot.database));
    return false;
  }
  
  sqlite3_bind_text(stmt, 1, self->server, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, self->channel, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 3, self->host, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 4, self->plugin, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 5, self->name, -1, SQLITE_TRANSIENT);
  
  sqlite3_bind_text(stmt, 6, self->server, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 7, self->channel, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 8, self->plugin, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 9, self->name, -1, SQLITE_TRANSIENT);

  sqlite3_bind_text(stmt, 10, self->server, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 11, self->host, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 12, self->plugin, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 13, self->name, -1, SQLITE_TRANSIENT);
  
  sqlite3_bind_text(stmt, 14, self->server, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 15, self->plugin, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 16, self->name, -1, SQLITE_TRANSIENT);
  
  
  sqlite3_bind_text(stmt, 17, self->server, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 18, self->channel, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 19, self->host, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 20, self->plugin, -1, SQLITE_TRANSIENT);
  
  sqlite3_bind_text(stmt, 21, self->server, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 22, self->channel, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 23, self->plugin, -1, SQLITE_TRANSIENT);
    
  sqlite3_bind_text(stmt, 24, self->server, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 25, self->host, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 26, self->plugin, -1, SQLITE_TRANSIENT);
  
  sqlite3_bind_text(stmt, 27, self->server, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 28, self->plugin, -1, SQLITE_TRANSIENT);


  sqlite3_bind_text(stmt, 29, self->server, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 30, self->channel, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 31, self->host, -1, SQLITE_TRANSIENT);
  
  sqlite3_bind_text(stmt, 32, self->server, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 33, self->channel, -1, SQLITE_TRANSIENT);
    
  sqlite3_bind_text(stmt, 34, self->server, -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 35, self->host, -1, SQLITE_TRANSIENT);
  
  sqlite3_bind_text(stmt, 36, self->server, -1, SQLITE_TRANSIENT);
  
  int r = sqlite3_step(stmt);
  if (r == SQLITE_ROW) {
    int out = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return (bool) out;
  } else {
    sqlite3_finalize(stmt);
    return false;
  }
}

typedef struct GBFlagListerState {
  TList* flags;
  GBFlag* filter;
} GBFlagListerState;

static int _gb_flag_lister(GBFlagListerState* state, int argc, char** argv, char** col_names) {
  bool ok = true;
  
  char* server;
  char* channel;
  char* host;
  char* plugin;
  char* name;
  
  for (int i = 0; i < argc; ++i) {    
    int serverc = strcmp(col_names[i], "server");
    
    if (serverc == 0 && argv[i] != NULL && state->filter->server != NULL && strcmp(argv[i], state->filter->server) != 0) {
      ok = false;
      break;
    } else if (serverc == 0 && argv[i] != NULL) {
      server = strdup(argv[i]);
    } else if (serverc == 0) {
      tl_error(l, "Error: flag.server is null");
      exit(1);
    }
    
    int channelc = strcmp(col_names[i], "channel");
    if (channelc == 0 && argv[i] != NULL && state->filter->channel != NULL && strcmp(argv[i], state->filter->channel) != 0) {
      ok = false;
      break;
    } else if (channelc == 0 && argv[i] != NULL) {
      channel = strdup(argv[i]);
    } else if (channelc == 0) {
      channel = NULL;
    }
    
    int hostc = strcmp(col_names[i], "host"); 
    if (hostc == 0 && argv[i] != NULL && state->filter->host != NULL && strcmp(argv[i], state->filter->host) != 0) {
      ok = false;
      break;
    } else if (hostc == 0 && argv[i] != NULL) {
      host = strdup(argv[i]);
    } else if (hostc == 0) {
      host = NULL;
    }
    
    int pluginc = strcmp(col_names[i], "plugin");
    if (pluginc == 0 && argv[i] != NULL && state->filter->plugin != NULL && strcmp(argv[i], state->filter->plugin) != 0) {
      ok = false;
      break;
    } else if (pluginc == 0 && argv[i] != NULL) {
      plugin = strdup(argv[i]);
    } else if (pluginc == 0) {
      plugin = NULL;
    }
    
    int namec = strcmp(col_names[i], "name");
    if (namec == 0 && argv[i] != NULL && state->filter->name != NULL && strcmp(argv[i], state->filter->name) != 0) {
      ok = false;
      break;
    } else if (namec == 0 && argv[i] != NULL) {
      name = strdup(argv[i]);
    } else if (namec == 0) {
      name = NULL;
    }
  }
  
  if (ok) {
    GBFlag* flag  = gb_flag_new();
    flag->server  = server;
    flag->channel = channel;
    flag->host    = host;
    flag->plugin  = plugin;
    flag->name    = name;    
    t_unref(t_list_append_(state->flags, flag));
    t_unref(flag);
  } else {
    t_free(server);
    t_free(channel);
    t_free(host);
    t_free(plugin);
    t_free(name);
  }
  
  return 0;
}

TList* gb_flag_list(GBFlag* filter) {
  assert(filter != NULL);
  assert(filter->server != NULL);
  t_ref(filter);
  
  TList* flags = t_list_new();
  
  GBFlagListerState* state = (GBFlagListerState*) t_malloc(sizeof(GBFlagListerState));
  if (!state) {
    perror("gb_flag_list{t_malloc}");
    exit(1);
  }
  state->flags  = flags;
  state->filter = filter;
  
  char* emsg;
  sqlite3_exec(
    GBBot.database,
    "SELECT * FROM flags;",
    (int(*)(void*,int,char**,char**)) _gb_flag_lister,
    state,
    &emsg
  );
  if (emsg) {
    tl_error(l, emsg);
    exit(1);
  }
  
  t_unref(filter);
  return flags;
}

void gb_flag_remove(GBFlag* filter) {
  assert(filter != NULL);
  assert(filter->server != NULL);
  t_ref(filter);
  
  char* channelp;
  char* hostp;
  char* pluginp;
  char* namep;
  
  if (filter->channel) {
    asprintf(&channelp, " AND channel=:channel");
  } else {
    channelp = strdup("");
  }
  
  if (filter->host) {
    asprintf(&hostp, " AND host=:host");
  } else {
    hostp = strdup("");
  }
  
  if (filter->plugin) {
    asprintf(&pluginp, " AND plugin=:plugin");
  } else {
    pluginp = strdup("");
  }
  
  if (filter->name) {
    asprintf(&namep, " AND name=:name");
  } else {
    namep = strdup("");
  }
  
  char* req;
  asprintf(&req, "DELETE FROM flags WHERE server=:server%s%s%s%s;", channelp, hostp, pluginp, namep);
  
  sqlite3_stmt* stmt;
  if (sqlite3_prepare(
    GBBot.database,
    req,
    -1,
    &stmt,
    NULL
  ) != SQLITE_OK) {
    tl_error(l, "%s", sqlite3_errmsg(GBBot.database));
    goto gb_flag_remove_dealloc;
  }
  
  int servern, channeln, hostn, pluginn, namen;
  servern  = sqlite3_bind_parameter_index(stmt, ":server");
  channeln = sqlite3_bind_parameter_index(stmt, ":channel");
  hostn    = sqlite3_bind_parameter_index(stmt, ":host");
  pluginn  = sqlite3_bind_parameter_index(stmt, ":plugin");
  namen    = sqlite3_bind_parameter_index(stmt, ":name");
  
  if (servern && filter->server)   sqlite3_bind_text(stmt, servern, filter->server, -1, SQLITE_TRANSIENT);
  if (channeln && filter->channel) sqlite3_bind_text(stmt, channeln, filter->channel, -1, SQLITE_TRANSIENT);
  if (hostn && filter->host)       sqlite3_bind_text(stmt, hostn, filter->host, -1, SQLITE_TRANSIENT);
  if (pluginn && filter->plugin)   sqlite3_bind_text(stmt, pluginn, filter->plugin, -1, SQLITE_TRANSIENT);
  if (namen && filter->name)       sqlite3_bind_text(stmt, namen, filter->name, -1, SQLITE_TRANSIENT);
  
  int ret;
gb_flag_remove_step:
  ret = sqlite3_step(stmt);
  if (ret == SQLITE_ROW) goto gb_flag_remove_step;
  if (ret == SQLITE_ERROR) tl_error(l, "%s", sqlite3_errmsg(GBBot.database));
  sqlite3_finalize(stmt);
    
gb_flag_remove_dealloc:
  t_free(channelp);
  t_free(hostp);
  t_free(pluginp);
  t_free(namep);
  t_free(req);
  
  t_unref(filter);
}

void gb_flag_insert(GBFlag* filter) {
  assert(filter != NULL);
  assert(filter->server != NULL);
  t_ref(filter);
  
  char* channelp1;
  char* channelp2;
  
  char* hostp1;
  char* hostp2;
  
  char* pluginp1;
  char* pluginp2;
  
  char* namep1;
  char* namep2;
  
  if (filter->channel) {
    asprintf(&channelp1, ",channel");
    asprintf(&channelp2, ",:channel");
  } else {
    channelp1 = strdup("");
    channelp2 = strdup("");
  }
  
  if (filter->host) {
    asprintf(&hostp1, ",host");
    asprintf(&hostp2, ",:host");
  } else {
    hostp1 = strdup("");
    hostp2 = strdup("");
  }
  
  if (filter->plugin) {
    asprintf(&pluginp1, ",plugin");
    asprintf(&pluginp2, ",:plugin");
  } else {
    pluginp1 = strdup("");
    pluginp2 = strdup("");
  }
  
  if (filter->name) {
    asprintf(&namep1, ",name");
    asprintf(&namep2, ",:name");
  } else {
    namep1 = strdup("");
    namep2 = strdup("");
  }
  
  char* req;
  asprintf(&req, "INSERT INTO flags (server%s%s%s%s) VALUES (:server%s%s%s%s);", channelp1, hostp1, pluginp1, namep1, channelp2, hostp2, pluginp2, namep2);
  
  sqlite3_stmt* stmt;
  if (sqlite3_prepare(
    GBBot.database,
    req,
    -1,
    &stmt,
    NULL
  ) != SQLITE_OK) {
    tl_error(l, "%s", sqlite3_errmsg(GBBot.database));
    goto gb_flag_insert_dealloc;
  }
  
  int servern, channeln, hostn, pluginn, namen;
  servern  = sqlite3_bind_parameter_index(stmt, ":server");
  channeln = sqlite3_bind_parameter_index(stmt, ":channel");
  hostn    = sqlite3_bind_parameter_index(stmt, ":host");
  pluginn  = sqlite3_bind_parameter_index(stmt, ":plugin");
  namen    = sqlite3_bind_parameter_index(stmt, ":name");
  
  if (servern && filter->server)   sqlite3_bind_text(stmt, servern, filter->server, -1, SQLITE_TRANSIENT);
  if (channeln && filter->channel) sqlite3_bind_text(stmt, channeln, filter->channel, -1, SQLITE_TRANSIENT);
  if (hostn && filter->host)       sqlite3_bind_text(stmt, hostn, filter->host, -1, SQLITE_TRANSIENT);
  if (pluginn && filter->plugin)   sqlite3_bind_text(stmt, pluginn, filter->plugin, -1, SQLITE_TRANSIENT);
  if (namen && filter->name)       sqlite3_bind_text(stmt, namen, filter->name, -1, SQLITE_TRANSIENT);
  
  int ret;
gb_flag_insert_step:
  ret = sqlite3_step(stmt);
  if (ret == SQLITE_ROW) goto gb_flag_insert_step;
  if (ret == SQLITE_ERROR) tl_error(l, "%s", sqlite3_errmsg(GBBot.database));
  sqlite3_finalize(stmt);
    
gb_flag_insert_dealloc:
  t_free(channelp1);
  t_free(channelp2);
  t_free(hostp1);
  t_free(hostp2);
  t_free(pluginp1);
  t_free(pluginp2);
  t_free(namep1);
  t_free(namep2);
  t_free(req);
  
  t_unref(filter);
}
