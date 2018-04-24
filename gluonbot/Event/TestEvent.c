#include "../Event.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <tiny-log.h>

static void gb_event_test_destroy(GBEventTest* self);
static void gb_event_test_handle(GBEventTest* self);

GBEvent* gb_event_test_new() {  
  GBEventTest* self = (GBEventTest*) t_malloc(sizeof(GBEventTest));
  if (!self) {
    perror("gb_event_test_new");
    exit(1);
  }
  
  t_gcunit((GBEvent*) self) = t_gcunit_new_(self, gb_event_test_destroy);
  ((GBEvent*) self)->type   = GBEventType_TEST;
  ((GBEvent*) self)->handle = (GBEventHandleFunc) gb_event_test_handle;
  
  return (GBEvent*) self;
}

void gb_event_test_destroy(GBEventTest* self) {
  assert(self != NULL);
}

void gb_event_test_handle(GBEventTest* self) {}

