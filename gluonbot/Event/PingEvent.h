#pragma once

#include "../Event.h"
#include "../IRCSocket.h"

/// \addtogroup Events
/// @{
typedef struct GBEventPing {
  GBEvent _parent;
  
  GBIRCSocket* sock;
  char* target;
} GBEventPing;

GBEvent* gb_event_ping_new(GBIRCSocket* sock, char* target);
/// @}

