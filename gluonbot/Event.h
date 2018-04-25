#pragma once

#include "Bot.h"
#include "IRCSocket.h"

#include <tiny.h>

typedef enum GBEventType {
  GBEventType_NONE,
  GBEventType_TEST,
  GBEventType_CONNECT,
  GBEventType_DISCONNECT,
  GBEventType_MESSAGE,
  GBEventType_CODE,
  GBEventType_PING,
  GBEventType_JOIN,
  GBEventType_PART,
  _GBEventType_SIZE
} GBEventType;

typedef struct GBEvent GBEvent;

typedef void (*GBEventHandleFunc)(GBEvent*);

typedef struct GBEvent {
  t_gcunit_use();
  
  GBEventType type;
  GBEventHandleFunc handle;
} GBEvent;

/// \addtogroup Events
/// @{
void gb_event_fire(GBEvent* e); ///< Fires a new event
/// @}

#include "Event/Event.h"
