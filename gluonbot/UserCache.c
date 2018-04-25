#include "UserCache.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tiny-log.h>

static GBUserInfo* gb_user_info_new(char* nick);
static void gb_user_info_destroy(GBUserInfo* self);

/*
 * User Cache
 */

GBUserCache* gb_user_cache_new() {
  GBUserCache* self = (GBUserCache*) t_malloc(sizeof(GBUserCache));
  if (!self) {
    perror("gb_usercache_new");
    exit(1);
  }
  
  t_gcunit(self) = t_gcunit_new_(self, gb_user_cache_destroy);
  self->info = t_map_new();
  
  return self;
}

void gb_user_cache_destroy(GBUserCache* self) {
  assert(self != NULL);
  
  t_unref(self->info);
}

GBUserInfo* gb_user_cache_get(GBUserCache* self, char* nick) {
  assert(self != NULL);
  assert(nick != NULL);
  t_ref(self);
  
  tl_debug("user-cache", "cache <-> %s", nick);
  
  TMapPair* p = t_map_get(self->info, nick);
  if (p != NULL) {
    GBUserInfo* i = p->unit->obj;
    t_ref(i);
    t_unref(p);
    t_unref(self);
    return i;
  } else {
    GBUserInfo* i = gb_user_info_new(nick);
    t_unref(t_map_set_(self->info, nick, i));
    t_unref(self);
    return i;
  }
}

/*
 * User Info
 */

static GBUserInfo* gb_user_info_new(char* nick) {
  GBUserInfo* self = (GBUserInfo*) t_malloc(sizeof(GBUserInfo));
  if (!self) {
    perror("gb_userinfo_new");
  }
  
  t_gcunit(self) = t_gcunit_new_(self, gb_user_info_destroy);
  memset(&self->mtx, 0, sizeof(self->mtx));
  pthread_mutex_init(&self->mtx, NULL);
  self->nick = strdup(nick);
  self->user = NULL;
  self->host = NULL;
  
  return self;
}

static void gb_user_info_destroy(GBUserInfo* self) {
  assert(self != NULL);

  if (self->nick != NULL) t_free(self->nick);
  if (self->user != NULL) t_free(self->user);
  if (self->host != NULL) t_free(self->host);
}
