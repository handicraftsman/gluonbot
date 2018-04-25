#pragma once

#include "../Event.h"
#include "../IRCSocket.h"

/// \addtogroup Events
/// @{
typedef struct GBEventConnect {
  GBEvent _parent;

  GBIRCSocket* sock;
} GBEventConnect;

GBEvent* gb_event_connect_new(GBIRCSocket* sock);
/// @}
