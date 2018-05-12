#pragma once

#include "../Event.h"
#include "../IRCSocket.h"

/// \addtogroup Events
/// @{
typedef struct GBEventNick {
  GBEvent _parent;
  
  GBIRCSocket* sock;
  char* nick;
  char* user;
  char* host;
  char* new_nick;
} GBEventNick;

GBEvent* gb_event_nick_new(GBIRCSocket* sock, char* nick, char* user, char* host, char* new_nick);
/// @}
