#pragma once

#include "../Event.h"

/// \addtogroup Events
/// @{
typedef struct GBEventJoin {
  GBEvent _parent;
  
  GBIRCSocket* sock;
  char* nick;
  char* user;
  char* host;
  char* chan;
} GBEventJoin;

GBEvent* gb_event_join_new();
/// @}

