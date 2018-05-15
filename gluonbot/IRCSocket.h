#pragma once

#include "UserCache.h"

#include <stdbool.h>
#include <string.h>

#include <pthread.h>

#include <tiny.h>

/// \addtogroup IRC-Socket
/// @{

typedef struct GBIRCSocket {
  t_gcunit_use(); ///< \private
  
  char* name; ///< name of the server
  char* l; ///< name of the log
  
  char* host; ///< remote host
  int   port; ///< remote port
  
  char* nick; ///< our nickname
  char* user; ///< our username
  char* pass; ///< our password (nullable)
  char* rnam; ///< our realname/GECOS
  
  TList* autojoin; ///< list of channels (GBChannelDesc*) bot should join automatically
  
  int fd; ///< \private
  bool running; ///< \private
  pthread_mutex_t running_mtx; ///< \private
  
  TList* queue; ///< \private
  pthread_mutex_t queue_mtx; ///< \private
  long long last_write; ///< \private
  int burst;
  
  GBUserCache* user_cache; ///< \summary stores info about all known users
} GBIRCSocket;

GBIRCSocket* gb_ircsocket_new(char* name); ///< \private
void gb_ircsocket_destroy(GBIRCSocket* self); ///< \private

/// \details Dumps socket info to the log
void gb_ircsocket_dump(GBIRCSocket* self);

/// \details Connects socket to the remote server
void gb_ircsocket_connect(GBIRCSocket* self);
void gb_ircsocket_io_loop(GBIRCSocket* self); ///< \private

/// \details Writes given message to the socket. Messages should end with \\r\\n
void gb_ircsocket_write(GBIRCSocket* self, char* fmt, ...);

/// \description Joins the given channel
/// \param chan Channel to join
/// \param pass Channel password (nullable)
void gb_ircsocket_join(GBIRCSocket* self, char* chan, char* pass);

/// Parts the given channel
/// \param chan Channel to part
/// \param reason Part reason (nullable)
void gb_ircsocket_part(GBIRCSocket* self, char* chan, char* reason);

/// Sends given message to the given target
/// \param target Channel/user to send to
/// \param message Message to send
void gb_ircsocket_privmsg(GBIRCSocket* self, char* target, char* fmt, ...);

/// Sends given notice to the given target
/// \param target Channel/user to notice
/// \param message Message to send
void gb_ircsocket_notice(GBIRCSocket* self, char* target, char* fmt, ...);

#define gb_ircsocket_reply(e, fmt, ...) gb_ircsocket_privmsg(e->sock, strcmp(e->target, e->sock->nick) == 0 ? e->nick : e->target, fmt, ##__VA_ARGS__)
#define gb_ircsocket_nreply(e, fmt, ...) gb_ircsocket_notice(e->sock, e->nick, fmt, ##__VA_ARGS__)

/// @}
