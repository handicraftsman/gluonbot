#pragma once

#include "../Event.h"
#include "../IRCSocket.h"

/// \addtogroup Events
/// @{
typedef struct GBEventCommand {
  GBEvent _parent;
  
  GBIRCSocket* sock;
  char* nick;
  char* user;
  char* host;
  char* target;
  char* message;
  TVector* split;
} GBEventCommand;

GBEvent* gb_event_command_new(GBIRCSocket* sock, char* nick, char* user, char* host, char* target, char* message, TVector* split);
/// @}


