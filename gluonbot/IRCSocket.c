#define _GNU_SOURCE

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>

#include <tiny-log.h>

#include "ChannelDesc.h"
#include "Event.h"
#include "IRCSocket.h"

// line 301

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
  
  self->queue = t_list_new();
  memset(&self->queue_mtx, 0, sizeof(self->queue_mtx));
  pthread_mutex_init(&self->queue_mtx, NULL);
  self->last_write = 0;
  
  self->user_cache = gb_user_cache_new();
  
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

static void gb_ircsocket_write_(GBIRCSocket* self, char* msg) {
  assert(self != NULL);
  t_ref(self);
  
  size_t l = strlen(msg);
  write(self->fd, msg, l);
  msg[l-2] = '\0';
  tl_io(self->l, "W> %s", msg);
  
  t_unref(self);
}

static void gb_ircsocket_authenticate(GBIRCSocket* self) {
  assert(self != NULL);
  assert(self->nick != NULL);
  assert(self->user != NULL);
  assert(self->rnam != NULL);
  t_ref(self);
  
  if (self->pass != NULL) {
    char* pmsg;
    asprintf(&pmsg, "PASS %s\r\n", self->pass);
    gb_ircsocket_write_(self, pmsg);
    t_free(pmsg);
  }
  
  char* nmsg;
  asprintf(&nmsg, "NICK %s\r\n", self->nick);
  gb_ircsocket_write_(self, nmsg);
  t_free(nmsg);
  
  char* umsg;
  asprintf(&umsg, "USER %s 0 * :%s\r\n", self->user, self->rnam);
  gb_ircsocket_write_(self, umsg);
  t_free(umsg);
  
  t_unref(self);
}

void gb_ircsocket_connect(GBIRCSocket* self) {
  assert(self != NULL);
  t_ref(self);
  
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

  gb_ircsocket_authenticate(self);
  gb_ircsocket_io_loop(self);
  
  t_unref(self);
}

static char* gb_ircsocket_read_line(GBIRCSocket* self, TError** err) {
  assert(self != NULL);
  t_ref(self);

  struct timeval tv = {
    .tv_sec = 0,
    .tv_usec = 10000
  };
  
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(self->fd, &readfds);
  
  int ret = select(self->fd+1, &readfds, NULL, NULL, &tv);
  if (ret == -1) {
    *err = t_error_new("unable to read from the socket", false);
    t_unref(self);
    return NULL;
  } else if (!ret) {
    return NULL;
  }
  
  char* data = calloc(513, sizeof(char));
  int i = 0;
  while (i <= 512) {
    int ok = read(self->fd, &data[i], 1);
    if (ok < 1) {
      *err = t_error_new("unable to read from the socket", false);
      t_free(data);
      t_unref(self);
      return NULL;
    }
    if (i > 0 && data[i-1] == '\r' && data[i] == '\n') {
      break;
    }
    if (i == 512) {
      tl_warning(self->l, "I've read too long line!");
      t_free(data);
      t_unref(self);
      return NULL;
    }
    ++i;
  }
  data[strlen(data)-2] = '\0';
  
  t_unref(self);
  return data;
}

void gb_ircsocket_io_loop(GBIRCSocket* self) {
  assert(self != NULL);
  t_ref(self);
  
  while (self->running) {
    usleep(100);
    
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
    GBEvent* e = gb_event_message_new(self, l);
    gb_event_fire(e);
    t_unref(e);
    t_free(l);
    
    struct timeval ctimev;
  l_write:
    gettimeofday(&ctimev, NULL);
    long long ctime = (ctimev.tv_sec * 1000) + (ctimev.tv_usec / 1000);
    if (self->last_write == 0 || ctime - self->last_write >= 700) {
      struct timeval tv = {
        .tv_sec = 0,
        .tv_usec = 10000
      };
      
      fd_set writefds;
      FD_ZERO(&writefds);
      FD_SET(self->fd, &writefds);
      
      int ret = select(self->fd+1, NULL, &writefds, NULL, &tv);
      if (ret == -1) {
        tl_error(self->l, "unable to write to the socket");
        GBEvent* e = gb_event_disconnect_new(self);
        gb_event_fire(e);
        t_unref(e);
        break;
      } else if (!ret) {
        continue;
      }
      
      pthread_mutex_lock(&self->queue_mtx);
      TListNode* msgn = t_list_first(self->queue);
      if (msgn != NULL) {
        t_list_remove(self->queue, msgn);
        pthread_mutex_unlock(&self->queue_mtx);
        if (write(self->fd, (char*) msgn->unit->obj, strlen(msgn->unit->obj)) == -1) {
          tl_error(self->l, "unable to write to the socket");
          GBEvent* e = gb_event_disconnect_new(self);
          gb_event_fire(e);
          t_unref(e);
          break;
        }
        ((char*) msgn->unit->obj)[strlen((char*) msgn->unit->obj)-2] = '\0';
        tl_io(self->l, "W> %s", (char*) msgn->unit->obj);
        t_unref(msgn);
        self->last_write = ctime;
      } else {
        pthread_mutex_unlock(&self->queue_mtx);
        continue;
      }
    } else {
      continue;
    }
  }
  
  t_unref(self);
}


/*
 * User API
 */


void gb_ircsocket_write(GBIRCSocket* self, char* fmt, ...) {
  assert(self != NULL);
  t_ref(self);
  
  va_list args;
  va_start(args, fmt);
  char* msg;
  vasprintf(&msg, fmt, args);
  va_end(args);
  
  pthread_mutex_lock(&self->queue_mtx);
  TGCUnit* u = t_gcunit_new_(msg, t_free);
  t_unref(t_list_append_(self->queue, u));
  t_unref(u);
  pthread_mutex_unlock(&self->queue_mtx);
  
  t_unref(self);
}

void gb_ircsocket_join(GBIRCSocket* self, char* chan, char* pass) {
  assert(self != NULL);
  assert(chan != NULL);
  t_ref(self);
  
  if (pass != NULL) {
    gb_ircsocket_write(self, "JOIN %s %s\r\n", chan, pass);
  } else {
    gb_ircsocket_write(self, "JOIN %s\r\n", chan);
  }
  
  t_unref(self);
}

void gb_ircsocket_privmsg(GBIRCSocket* self, char* target, char* msg) {
  assert(self   != NULL);
  assert(target != NULL);
  assert(msg    != NULL);
  t_ref(self);
  
  size_t len = strlen(msg);
  
  for (int i = 0; i < (len / 400); i++) {
    char* m = strndup(msg+ 400 * i, 400);
    gb_ircsocket_write(self, "PRIVMSG %s :%s\r\n", target, m);
    t_free(m);
  }
  
  t_unref(self);
}

void gb_ircsocket_notice(GBIRCSocket* self, char* target, char* msg) {
  assert(self   != NULL);
  assert(target != NULL);
  assert(msg    != NULL);
  t_ref(self);
  
  size_t len = strlen(msg);
  size_t sz = (len / 400);
  sz = sz == 0 ? 1 : sz;
  
  for (int i = 0; i < sz; i++) {
    char* m = strndup(msg+ 400 * i, 400);
    gb_ircsocket_write(self, "NOTICE %s :%s\r\n", target, m);
    t_free(m);
  }
  
  t_unref(self);
}
