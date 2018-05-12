#include "../Command.h"
#include "../Event.h"
#include "../Plugin.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tiny-log.h>

static void gb_event_command_destroy(GBEventCommand* self);
static void gb_event_command_handle(GBEventCommand* self);

GBEvent* gb_event_command_new(GBIRCSocket* sock, char* nick, char* user, char* host, char* target, char* message, TVector* split) {  
  assert(sock != NULL);
  assert(nick != NULL);
  assert(user != NULL);
  assert(host != NULL);
  assert(target != NULL);
  assert(message != NULL);
  assert(split != NULL);
  t_ref(sock);
  t_ref(split);
  
  GBEventCommand* self = (GBEventCommand*) t_malloc(sizeof(GBEventCommand));
  if (!self) {
    perror("gb_event_command_new");
    exit(1);
  }
  
  t_gcunit((GBEvent*) self) = t_gcunit_new_(self, gb_event_command_destroy);
  ((GBEvent*) self)->type   = GBEventType_COMMAND;
  ((GBEvent*) self)->handle = (GBEventHandleFunc) gb_event_command_handle;
  self->sock = sock;
  self->nick = strdup(nick);
  self->user = strdup(user);
  self->host = strdup(host);
  self->target = strdup(target);
  self->message = strdup(message);
  self->split = split;
  
  return (GBEvent*) self;
}

void gb_event_command_destroy(GBEventCommand* self) {
  assert(self != NULL);
  t_unref(self->sock);
  t_free(self->nick);
  t_free(self->user);
  t_free(self->host);
  t_free(self->target);
  t_free(self->message);
  t_unref(self->split);
}

void gb_event_command_handle(GBEventCommand* self) {
  t_ref((GBEvent*) self);
  
  TGCUnit* cmdu = t_vector_get(self->split, 0);
  char* cmd = cmdu->obj;
  
  t_list_foreach(GBBot.plugins->pairs, pluginpn) {
    TMapPair* pluginp = pluginpn->unit->obj;
    GBPlugin* plugin = pluginp->unit->obj;
    t_list_foreach(plugin->commands->pairs, commandpn) {
      TMapPair* commandp = commandpn->unit->obj;
      GBCommand* command = commandp->unit->obj;
      if (strcmp(command->name, cmd) == 0) {
        GBFlag* flag  = gb_flag_new();
        flag->server  = strdup(self->sock->name);
        flag->channel = strdup(self->target);
        flag->host    = strdup(self->host);
        flag->plugin  = strdup(plugin->name);
        flag->name    = strdup(command->flag);
        if (gb_flag_is_set(flag)) {
          command->handler(self); 
        }
        t_unref(flag);
        t_unref(commandpn);
        break;
      }
    }
  }
  
  t_unref(cmdu);
  
  t_unref((GBEvent*) self);
}
