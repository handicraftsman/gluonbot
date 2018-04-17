#include "Bot.h"
#include "IRCSocket.h"

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
}

static void init_server(xmlNodePtr node) {
  char* name = NULL;
  
  for (xmlAttrPtr attr = node->properties; attr != NULL; attr = attr->next) {
    if (strcmp((char*) attr->name, "name") == 0) {
      name = (char*) xmlNodeListGetString(node->doc, attr->children, 1);
    }
  }
  
  // todo - load address, credentials and autojoin list
  
  bool ok = true;
  
  if (name == NULL) {
    tl_error(l, "Noticed an unnamed server node (line %d)!", node->line);
    ok = false;
  }
  
  if (ok) {
    IRCSocket* sock = gb_ircsocket_new(name);
  
    t_unref(t_map_set_(GBBot.sockets, name, sock));
    
    t_unref(sock);
  }
  
  if (name != NULL) xmlFree(name);
  
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
  
  for(xmlNodePtr c = root->children; c != NULL; c = c->next) {
    if (strcmp((char*) c->name, "server") == 0) {
      init_server(c);
    } else if (c->type != XML_TEXT_NODE) {
      tl_warning(l, "Invalid node name: /gluonbot-config/%s (line %d). Skipping...", c->name, c->line);
    }
  }
  
  xmlFreeDoc(doc);
  
  xmlCleanupParser();
}

void gb_bot_deinit() {
  t_unref(GBBot.sockets);
  t_unref(GBBot.plugins);
}

