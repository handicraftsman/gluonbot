#pragma once

typedef enum GBEventType {
  GBEventType_NONE,
  GBEventType_TEST,
  _GBEventType_SIZE
} GBEventType;

typedef struct GBEvent GBEvent;

typedef char* (*GBEventToStringFunc)(GBEvent*);
typedef void (*GBEventHandleFunc)(GBEvent*);

typedef struct GBEventVTable {
  GBEventToStringFunc to_string;
  GBEventHandleFunc handle;
} GBEventVTable;

typedef struct GBEvent {
  GBEventType type;
  
} GBEvent;
