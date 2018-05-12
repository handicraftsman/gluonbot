#pragma once

#include "../Event.h"

/// \addtogroup Events
/// @{
typedef struct GBEventPart {
  GBEvent _parent;
  
  GBIRCSocket* sock;
  char* nick;
  char* user;
  char* host;
  char* chan;
  char* reason;
} GBEventPart;

GBEvent* gb_event_part_new(GBIRCSocket* sock, char* nick, char* user, char* host, char* chan, char* reason);
/// @}


