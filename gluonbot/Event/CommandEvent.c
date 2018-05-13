#include "../Command.h"
#include "../Event.h"
#include "../Plugin.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>

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

#define getcurrenttime() \
  struct timeval ctimev; \
  gettimeofday(&ctimev, NULL); \
  long long ctime = (ctimev.tv_sec * 1000) + (ctimev.tv_usec / 1000); \
  long long* ctimeptr = (long long*) t_malloc(sizeof(long long)); \
  *ctimeptr = ctime;
  
#define justgetcurrenttime() \
  struct timeval ctimev; \
  gettimeofday(&ctimev, NULL); \
  long long ctime = (ctimev.tv_sec * 1000) + (ctimev.tv_usec / 1000);
  
static bool is_command_available(GBEventCommand* self, GBCommand* command) {
  TMapPair* serverp = t_map_get(command->cooldowns, self->sock->name);
  if (!serverp) {
    TMap* serverm = t_map_new();
    TMap* channelm = t_map_new();
    
    getcurrenttime();
    
    t_unref(t_map_set_(channelm, self->host, t_gcunit_new_(ctimeptr, t_free)));
    t_unref(t_map_set_(serverm, self->target, channelm));
    t_unref(t_map_set_(command->cooldowns, self->sock->name, serverm));
    
    t_unref(channelm);
    t_unref(serverm);
    return true;
  }
  TMap* serverm = serverp->unit->obj;
  
  TMapPair* channelp = t_map_get(serverm, self->target);
  if (!channelp) {
    TMap* channelm = t_map_new();
    
    getcurrenttime();
    
    t_unref(t_map_set_(channelm, self->host, t_gcunit_new_(ctimeptr, t_free)));
    t_unref(t_map_set_(serverm, self->target, channelm));
    
    t_unref(channelm);
    t_unref(serverp);
    return true;    
  }
  TMap* channelm = channelp->unit->obj;
  
  TMapPair* hostp = t_map_get(channelm, self->host);
  if (!hostp) {
    getcurrenttime();
    
    t_unref(t_map_set(channelm, self->host, t_gcunit_new_(ctimeptr, t_free)));
    
    t_unref(channelp);
    t_unref(serverp);
    return true;
  }
  
  getcurrenttime();
  
  long long* ltimeptr = hostp->unit->obj;
  long long ltime = *ltimeptr;
  
  bool ret = ((ctime - ltime) >= ((long long) command->cooldown * (long long) 1000));
  
  if (ret) {
    t_gcunit_unref(hostp->unit);
    hostp->unit = t_gcunit_new_(ctimeptr, t_free);
  } else {
    t_free(ctimeptr);
  }
  
  t_unref(hostp);
  t_unref(channelp);
  t_unref(serverp);
  
  return ret;
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
        if (strcmp(command->flag, "world") == 0 || gb_flag_is_set(flag)) {
          if (is_command_available(self, command)) {
            command->handler(self); 
          }
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
