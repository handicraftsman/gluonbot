#pragma once

#include "../Event.h"
#include "../IRCSocket.h"

/// \addtogroup Events
/// @{
typedef struct GBEventDisconnect {
  GBEvent _parent;

  GBIRCSocket* sock;
} GBEventDisconnect;

GBEvent* gb_event_disconnect_new(GBIRCSocket* sock);
/// @}
