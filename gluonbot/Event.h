#pragma once

#include "Bot.h"
#include "IRCSocket.h"

#include <tiny.h>

typedef enum GBEventType {
  GBEventType_NONE,
  GBEventType_CONNECT,
  _GBEventType_SIZE
} GBEventType;

typedef struct GBEvent GBEvent;

typedef void (*GBEventHandleFunc)(GBEvent*);

typedef struct GBEventVTable {
  GBEventHandleFunc handle;
} GBEventVTable;

typedef struct GBEvent {
  t_gcunit_use();
  
  GBEventType type;
  GBEventVTable* vtable;
} GBEvent;


void gb_event_fire(GBEvent* e);


GBEvent* gb_event_connect_new(GBIRCSocket* sock);
