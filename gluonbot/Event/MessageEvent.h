#pragma once

#include "../IRCSocket.h"

/// \addtogroup Events
/// @{
typedef struct GBEventMessage {
  GBEvent _parent;
  
  GBIRCSocket* sock;
  char* msg;
} GBEventMessage;

GBEvent* gb_event_message_new(GBIRCSocket* sock, char* msg);
/// @}
