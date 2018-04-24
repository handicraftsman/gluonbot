#pragma once

#include "../Event.h"
#include "../IRCSocket.h"

/// \addtogroup Events
/// @{
typedef struct GBEventCode {
  GBEvent _parent;
  
  GBIRCSocket* sock;
  int code;
  char* extra;
} GBEventCode;

GBEvent* gb_event_code_new(GBIRCSocket* sock, int code, char* extra);
/// @}


