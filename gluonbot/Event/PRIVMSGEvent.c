#include "../Event.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tiny-log.h>

static void gb_event_privmsg_destroy(GBEventPRIVMSG* self);
static void gb_event_privmsg_handle(GBEventPRIVMSG* self);

GBEvent* gb_event_privmsg_new(GBIRCSocket* sock, char* nick, char* user, char* host, char* target, char* message) {  
  assert(sock != NULL);
  assert(nick != NULL);
  assert(user != NULL);
  assert(host != NULL);
  assert(target != NULL);
  assert(message != NULL);
  t_ref(sock);
  
  GBEventPRIVMSG* self = (GBEventPRIVMSG*) t_malloc(sizeof(GBEventPRIVMSG));
  if (!self) {
    perror("gb_event_privmsg_new");
    exit(1);
  }
  
  t_gcunit((GBEvent*) self) = t_gcunit_new_(self, gb_event_privmsg_destroy);
  ((GBEvent*) self)->type   = GBEventType_PRIVMSG;
  ((GBEvent*) self)->handle = (GBEventHandleFunc) gb_event_privmsg_handle;
  self->sock = sock;
  self->nick = strdup(nick);
  self->user = strdup(user);
  self->host = strdup(host);
  self->target = strdup(target);
  self->message = strdup(message);
  
  return (GBEvent*) self;
}

void gb_event_privmsg_destroy(GBEventPRIVMSG* self) {
  t_unref(self->sock);
  t_free(self->nick);
  t_free(self->user);
  t_free(self->host);
  t_free(self->target);
  t_free(self->message);
  
  assert(self != NULL);
}

void gb_event_privmsg_handle(GBEventPRIVMSG* self) {
  assert(self != NULL);
  t_ref((GBEvent*) self);
  
  int res = strncmp(self->message, GBBot.prefix, strlen(GBBot.prefix));
  if (res == 0) {
    TVector* split = t_vector_new(0, 0);
    char* m = strdup(self->message) + strlen(GBBot.prefix);
    char* s = strtok(m, " \t\r\n");
    while (s != NULL) {
      t_vector_push_(split, t_gcunit_new_(strdup(s), t_free));
      s = strtok(NULL, " \t\r\n");
    }
    if (split->len > 0) {
      GBEvent* e = gb_event_command_new(self->sock, self->nick, self->user, self->host, self->target, self->message, split);
      gb_event_fire(e);
      t_unref(e);
    } else {
      t_unref(split);
    }
  }
  
  t_unref((GBEvent*) self);
}
