#pragma once

#include <tiny.h>

#include "Bot.h"

typedef struct GBGroup {
  t_gcunit_use();
  char* name;
  TList* flags;
} GBGroup;

GBGroup* gb_group_new(char* name);
void gb_group_destroy(GBGroup* self);
void gb_group_add_flag(GBGroup* self, GBFlag* flag);
void gb_group_give(GBGroup* self, GBFlag* info);
void gb_group_take(GBGroup* self, GBFlag* info);
