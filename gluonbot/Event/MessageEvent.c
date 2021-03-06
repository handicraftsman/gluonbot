#include "../Event.h"
#include "../IRCSocket.h"
#include "../utils/utils.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <tiny-log.h>

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>


/*
 * Regexes
 */

static pcre2_code* rgx_privmsg;
static pcre2_code* rgx_code;
static pcre2_code* rgx_ping;
static pcre2_code* rgx_join;
static pcre2_code* rgx_part;
static pcre2_code* rgx_nick;

static void init_regexes() {
  static bool done = false;
  if (done) return;
  done = true;
  
  rgx_privmsg = new_regex("^:(.+?)!(.+?)@(.+?) PRIVMSG (.+?) :(.+)$");
  rgx_code = new_regex("^:.+? (\\d\\d\\d) .+? (.+)$");
  rgx_ping = new_regex("^PING :(.+)$");
  rgx_join = new_regex("^:(.+?)!(.+?)@(.+?) JOIN (.+)$");
  rgx_part = new_regex("^:(.+?)!(.+?)@(.+?) PART (.+?) :(.+)$");
  rgx_nick = new_regex("^:(.+?)!(.+?)@(.+?) NICK :(.+)$");
}

/*
 * The event itself
 */

static void gb_event_message_destroy(GBEventMessage* self);
static void gb_event_message_handle(GBEventMessage* self);

GBEvent* gb_event_message_new(GBIRCSocket* sock, char* msg) {  
  assert(sock != NULL);
  t_ref(sock);
  
  GBEventMessage* self = (GBEventMessage*) t_malloc(sizeof(GBEventMessage));
  if (!self) {
    perror("gb_event_message_new");
    exit(1);
  }
  
  t_gcunit((GBEvent*) self) = t_gcunit_new_(self, gb_event_message_destroy);
  ((GBEvent*) self)->type   = GBEventType_MESSAGE;
  ((GBEvent*) self)->handle = (GBEventHandleFunc) gb_event_message_handle;
  self->sock = sock;
  self->msg  = strdup(msg);
  
  return (GBEvent*) self;
}

void gb_event_message_destroy(GBEventMessage* self) {
  assert(self != NULL);
  
  t_unref(self->sock);
  t_free(self->msg);
}

void gb_event_message_handle(GBEventMessage* self) {
  assert(self != NULL);
  t_ref((GBEvent*) self);
  
  init_regexes();
  
  tl_io(self->sock->l, "R> %s", self->msg);
  
  pcre2_match_data* md = NULL;
  
  if (try_match(rgx_privmsg, self->msg, &md)) {
    char* nick = get_substring(md, 1);
    char* user = get_substring(md, 2);
    char* host = get_substring(md, 3);
    char* target = get_substring(md, 4);
    char* message = get_substring(md, 5);
    
    GBEvent* e = gb_event_privmsg_new(self->sock, nick, user, host, target, message);
    gb_event_fire(e);
    t_unref(e);
    
    pcre2_substring_free((PCRE2_UCHAR8*) nick);
    pcre2_substring_free((PCRE2_UCHAR8*) user);
    pcre2_substring_free((PCRE2_UCHAR8*) host);
    pcre2_substring_free((PCRE2_UCHAR8*) target);
    pcre2_substring_free((PCRE2_UCHAR8*) message);    
  } else if (try_match(rgx_code, self->msg, &md)) {
    char* cstr = get_substring(md, 1);
    char* extra = get_substring(md, 2);
    int code = atoi(cstr);
    
    GBEvent* e = gb_event_code_new(self->sock, code, extra);
    gb_event_fire(e);
    t_unref(e);
    
    pcre2_substring_free((PCRE2_UCHAR8*) cstr);
    pcre2_substring_free((PCRE2_UCHAR8*) extra);
  } else if (try_match(rgx_ping, self->msg, &md)) {
    char* target = get_substring(md, 1);
    
    GBEvent* e = gb_event_ping_new(self->sock, target);
    gb_event_fire(e);
    t_unref(e);
    
    pcre2_substring_free((PCRE2_UCHAR8*) target);
  } else if (try_match(rgx_join, self->msg, &md)) {
    char* nick = get_substring(md, 1);
    char* user = get_substring(md, 2);
    char* host = get_substring(md, 3);
    char* chan = get_substring(md, 4);
    
    GBEvent* e = gb_event_join_new(self->sock, nick, user, host, chan);
    gb_event_fire(e);
    t_unref(e);
    
    pcre2_substring_free((PCRE2_UCHAR8*) nick);
    pcre2_substring_free((PCRE2_UCHAR8*) user);
    pcre2_substring_free((PCRE2_UCHAR8*) host);
    pcre2_substring_free((PCRE2_UCHAR8*) chan);
  } else if (try_match(rgx_part, self->msg, &md)) {
    char* nick = get_substring(md, 1);
    char* user = get_substring(md, 2);
    char* host = get_substring(md, 3);
    char* chan = get_substring(md, 4);
    char* reason = get_substring(md, 5);
    
    GBEvent* e = gb_event_part_new(self->sock, nick, user, host, chan, reason);
    gb_event_fire(e);
    t_unref(e);
    
    pcre2_substring_free((PCRE2_UCHAR8*) nick);
    pcre2_substring_free((PCRE2_UCHAR8*) user);
    pcre2_substring_free((PCRE2_UCHAR8*) host);
    pcre2_substring_free((PCRE2_UCHAR8*) chan);
    pcre2_substring_free((PCRE2_UCHAR8*) reason);
  } else if (try_match(rgx_nick, self->msg, &md)) {
    char* nick = get_substring(md, 1);
    char* user = get_substring(md, 2);
    char* host = get_substring(md, 3);
    char* new_nick = get_substring(md, 4);
    
    GBEvent* e = gb_event_nick_new(self->sock, nick, user, host, new_nick);
    gb_event_fire(e);
    t_unref(e);
    
    pcre2_substring_free((PCRE2_UCHAR8*) nick);
    pcre2_substring_free((PCRE2_UCHAR8*) user);
    pcre2_substring_free((PCRE2_UCHAR8*) host);
    pcre2_substring_free((PCRE2_UCHAR8*) new_nick);
  }
  
  if (md != NULL) pcre2_match_data_free(md);

  t_unref((GBEvent*) self);
}
