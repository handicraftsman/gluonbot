#pragma once

#include "../Event.h"
#include "../IRCSocket.h"

/// \addtogroup Events
/// @{
typedef struct GBEventPRIVMSG {
  GBEvent _parent;
  
  GBIRCSocket* sock;
  char* nick;
  char* user;
  char* host;
  char* target;
  char* message;
  char* reply_to;
} GBEventPRIVMSG;

GBEvent* gb_event_privmsg_new(GBIRCSocket* sock, char* nick, char* user, char* host, char* target, char* message);
/// @}


