#include "../Event.h"
#include "../ChannelDesc.h"
#include "../UserCache.h"
#include "../utils/utils.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#include <tiny-log.h>

static pcre2_code* rgx_whoreply;

static void init_regexes() {
  static bool done = false;
  if (done) return;
  done = true;
  
  rgx_whoreply = new_regex("^.+? (.+?) (.+?) .+? (.+?) .*");
}

static void gb_event_code_destroy(GBEventCode* self);
static void gb_event_code_handle(GBEventCode* self);

GBEvent* gb_event_code_new(GBIRCSocket* sock, int code, char* extra) {  
  assert(sock != NULL);
  assert(extra != NULL);
  t_ref(sock);
  
  GBEventCode* self = (GBEventCode*) t_malloc(sizeof(GBEventCode));
  if (!self) {
    perror("gb_event_code_new");
    exit(1);
  }
  
  t_gcunit((GBEvent*) self) = t_gcunit_new_(self, gb_event_code_destroy);
  ((GBEvent*) self)->type   = GBEventType_CODE;
  ((GBEvent*) self)->handle = (GBEventHandleFunc) gb_event_code_handle;
  self->sock = sock;
  self->code = code;
  self->extra = strdup(extra);
  
  return (GBEvent*) self;
}

void gb_event_code_destroy(GBEventCode* self) {
  assert(self != NULL);
  
  t_unref(self->sock);
  t_free(self->extra);
}

void gb_event_code_handle(GBEventCode* self) {
  assert(self != NULL);
  t_ref((GBEvent*) self);
  
  init_regexes();
  
  if (self->code == 1) { // welcome
    t_list_foreach(self->sock->autojoin, n) {
      GBChannelDesc* d = n->unit->obj;
      gb_ircsocket_join(self->sock, d->name, d->pass);
    }
  } else if (self->code == 352) { // whoreply
    pcre2_match_data* md = NULL;
    
    if (try_match(rgx_whoreply, self->extra, &md)) {
      char* nick = get_substring(md, 3);
      char* user = get_substring(md, 1);
      char* host = get_substring(md, 2);
      
      GBUserInfo* info = gb_user_cache_get(self->sock->user_cache, nick);
      pthread_mutex_lock(&info->mtx);
      if (info->user != NULL) {
        t_free(info->user);
      }
      info->user = strdup(user);
      if (info->host != NULL) {
        t_free(info->host);
      }
      info->user = strdup(host);
      pthread_mutex_unlock(&info->mtx);
      
      pcre2_substring_free((PCRE2_UCHAR8*) nick);
      pcre2_substring_free((PCRE2_UCHAR8*) user);
      pcre2_substring_free((PCRE2_UCHAR8*) host);
    }
    
    if (md != NULL) pcre2_match_data_free(md);
  }
  
  t_unref((GBEvent*) self);
}

