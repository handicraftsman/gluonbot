#define _GNU_SOURCE

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

#include <tiny-log.h>

#include "ChannelDesc.h"
#include "Event.h"
#include "IRCSocket.h"

GBIRCSocket* gb_ircsocket_new(char* name) {
  GBIRCSocket* self = (GBIRCSocket*) t_malloc(sizeof(GBIRCSocket));
  if (!self) {
    perror("gb_ircsocket_new");
    exit(1);
  }
  
  t_gcunit(self) = t_gcunit_new_(self, gb_ircsocket_destroy);
  
  self->name = strdup(name);
  
  self->l = (char*) malloc(strlen(self->name) + 2);
  sprintf(self->l, "!%s", self->name);
  
  self->host = strdup("127.0.0.1");
  self->port = 6667;
  
  self->nick = strdup("GluonBot");
  self->user = strdup("GluonBot");
  self->pass = NULL;
  self->rnam = strdup("An IRC bot in C");
  
  self->autojoin = NULL;
  
  self->fd = 0;
  self->running = false;
  memset(&self->running_mtx, 0, sizeof(self->running_mtx));
  pthread_mutex_init(&self->running_mtx, NULL);
  
  self->last_write = 0;
  
  tl_important(self->l, "Hello, IRC!");
  
  return self;
}

void gb_ircsocket_destroy(GBIRCSocket* self) {
  assert(self != NULL);
  
  if (self->name != NULL) t_free(self->name);
  if (self->l != NULL) t_free(self->l);
  
  if (self->host != NULL) t_free(self->host);
  
  if (self->nick != NULL) t_free(self->nick);
  if (self->user != NULL) t_free(self->user);
  if (self->pass != NULL) t_free(self->pass);
  if (self->rnam != NULL) t_free(self->rnam);
  
  if (self->autojoin != NULL) t_unref(self->autojoin);
}

void gb_ircsocket_dump(GBIRCSocket* self) {
  assert(self != NULL);
  t_ref(self);
  
  tl_debug(self->l, "nick <- %s", self->nick);
  tl_debug(self->l, "user <- %s", self->user);
  tl_debug(self->l, "has-pass <- %d", self->pass != NULL);
  tl_debug(self->l, "rnam <- %s", self->rnam);
  
  t_list_foreach(self->autojoin, n) {
    GBChannelDesc* chan = n->unit->obj;
    tl_debug(self->l, "autojoin <- %s %d", chan->name, chan->pass != NULL);
  }
  
  t_unref(self);
}

void gb_ircsocket_connect(GBIRCSocket* self) {
  assert(self != NULL);
  t_ref(self);
  
  //struct addrinfo hints, *res;
  //memset(&hints, 0, sizeof(hints));
  //hints.ai_family   = AF_INET;
  //hints.ai_socktype = SOCK_STREAM;
  
  //int ret;
  //char* p;
  //asprintf(&p, "%d", self->port);
  //if ((ret = getaddrinfo(self->host, p, &hints, &res))) {
    
  //}
  //free(p);
  
  struct addrinfo hints, *res;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family   = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  char* p;
  asprintf(&p, "%d", self->port);
  
  
  if (getaddrinfo(self->host, p, &hints, &res) < 0) {
    tl_error(self->l, "Cannot resolve %s:%d", self->host, self->port);
    free(p);
    return;
  }
  
  self->fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (self->fd == -1) {
    tl_error(self->l, "Cannot create a socket");
    freeaddrinfo(res);
    free(p);
    return;
  }
  
  if (connect(self->fd, res->ai_addr, res->ai_addrlen) == -1) {
    tl_error(self->l, "Cannot connect to %s:%d", self->host, self->port);
    close(self->fd);
    freeaddrinfo(res);
    free(p);
    return;
  }

  freeaddrinfo(res);
  free(p);
  
  pthread_mutex_lock(&self->running_mtx);
  tl_important(self->l, "Connected!");
  self->running = true;
  pthread_mutex_unlock(&self->running_mtx);
  
  GBEvent* e = gb_event_connect_new(self);
  gb_event_fire(e);
  t_unref(e);

  gb_ircsocket_io_loop(self);
  
  t_unref(self);
}

static char* gb_ircsocket_read_line(GBIRCSocket* self, TError** err) {
  assert(self != NULL);
  t_ref(self);

  // check if there's data
  // set err and return NULL on error
  // return NULL if there's no data
  // read data
  // set err and return NULL on error
  // return data
  
  t_unref(self);
  return NULL;
}

static void __attribute__((used)) gb_ircsocket_write_line(GBIRCSocket* self, char* data, TError** err) {
  assert(self != NULL);
  assert(data != NULL);
  t_ref(self);
  
  // try writing data to the socket
  // set err and return on error
  
  t_unref(self);
}

void gb_ircsocket_io_loop(GBIRCSocket* self) {
  assert(self != NULL);
  t_ref(self);
  
  while (self->running) {
    // sleep a bit
    
    TError* err;
    
    err = NULL;
    char* l = gb_ircsocket_read_line(self, &err);
    if (err != NULL) {
      tl_error(self->l, "%s", err->message);
      GBEvent* e = gb_event_disconnect_new(self);
      gb_event_fire(e);
      t_unref(e);
      break;
    }
    if (l == NULL) goto l_write;
    // emit an event
    
  l_write:
    err = NULL;
    // check if cooldown has been passed
    // continue to reading if it has not
    // check if socket is writable
    // continue to reading if it's not
    // try getting a line from the queue
    // continue to reading if there's no data
    // try writing a line
    // stop the loop on error
  }
  
  t_unref(self);
}
