#include "Bot.h"
#include "ChannelDesc.h"
#include "IRCSocket.h"
#include "Plugin.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libxml/tree.h>
#include <libxml/parser.h>

#include <tiny-log.h>

_GBBot GBBot = {};

static char* l = "bot";

void gb_bot_init() {
  tl_important(l, "Hello, main!");
  
  GBBot.sockets = t_map_new();
  GBBot.plugins = t_map_new();
  
  GBBot.prefix = strdup("!");
  
  GBPlugin* p = gb_plugin_new("core");
  t_unref(t_map_set_(GBBot.plugins, "core", p));
  t_unref(p);
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
    IRCSocket* sock = gb_ircsocket_new(name);
  
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
    }
  }
  
  for(xmlNodePtr c = root->children; c != NULL; c = c->next) {
    if (strcmp((char*) c->name, "server") == 0) {
      init_server(c);
    } else if (strcmp((char*) c->name, "plugin") == 0) {
      char* name = NULL;
      
      for (xmlAttrPtr attr = root->properties; attr != NULL; attr = attr->next) {
        if (strcmp((char*) attr->name, "name") == 0) {
          name = (char*) xmlNodeListGetString(root->doc, attr->children, 1);
          tl_debug(l, "plugins <- %s", name);
        }
      }
      
      if (name == NULL) {
        tl_error(l, "Noticed an unnamed plugin node! (line %d)", c->line);
        continue;
      }
      
      GBPlugin* p = gb_plugin_new(name);
      t_unref(t_map_set_(GBBot.plugins, name, p));
      t_unref(p);
    }
  }
  
  xmlFreeDoc(doc);
  
  xmlCleanupParser();
}

void gb_bot_deinit() {
  t_unref(GBBot.sockets);
  t_unref(GBBot.plugins);
  
  t_free(GBBot.prefix);
}

