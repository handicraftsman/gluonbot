#pragma once

#include "Bot.h"
#include "IRCSocket.h"

#include <tiny.h>

/// \addtogroup Events
/// @{

/*!
 * \summary Event types used for calling plugin-defined handlers
 */
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
  GBEventType_NICK,
  GBEventType_PRIVMSG,
  GBEventType_COMMAND,
  _GBEventType_SIZE
} GBEventType;

typedef struct GBEvent GBEvent;

typedef void (*GBEventHandleFunc)(GBEvent*);

typedef struct GBEvent {
  t_gcunit_use(); ///< \private
  
  GBEventType type; ///< \summary Event type
  GBEventHandleFunc handle; ///< \summary Event handler
} GBEvent;

void gb_event_fire(GBEvent* e); ///< \summary Fires a new event
/// @}

#include "Event/Event.h"
