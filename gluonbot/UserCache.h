#pragma once

#include <pthread.h>

#include <tiny.h>

/// \addtogroup User-Cache
/// @{

typedef struct GBUserCache {
  t_gcunit_use();
  TMap* info;
} GBUserCache;

typedef struct GBUserInfo {
  t_gcunit_use();
  pthread_mutex_t mtx; ///< please lock this mutex if you need to access an instance of this struct
  
  char* nick;
  char* user;
  char* host;
} GBUserInfo;

GBUserCache* gb_user_cache_new(); ///< \private
void gb_user_cache_destroy(GBUserCache* self); ///< \private
GBUserInfo* gb_user_cache_get(GBUserCache* self, char* nick);
GBUserInfo* gb_user_cache_rename(GBUserCache* self, char* nick, char* new_nick);

/// @}
