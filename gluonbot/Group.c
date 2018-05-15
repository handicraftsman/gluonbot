#include "Group.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <tiny.h>

GBGroup* gb_group_new(char* name) {
  GBGroup* self = (GBGroup*) t_malloc(sizeof(GBGroup));
  if (!self) {
    perror("gb_group_new");
    exit(1);
  }
  
  t_gcunit(self) = t_gcunit_new_(self, gb_group_destroy);
  self->name  = strdup(name);
  self->flags = t_list_new();
  
  return self;
}

void gb_group_destroy(GBGroup* self) {
  assert(self != NULL);
  
  t_unref(self->flags);
}

void gb_group_add_flag(GBGroup* self, GBFlag* flag) {
  t_unref(t_list_append_(self->flags, flag));
}

void gb_group_give(GBGroup* self, GBFlag* info) {
  t_list_foreach(self->flags, flagn) {
    GBFlag* flag = flagn->unit->obj;
    
    GBFlag* to_set = gb_flag_new();
    to_set->server  = strdup(info->server);
    to_set->channel = info->channel != NULL ? strdup(info->channel) : NULL;
    to_set->host    = info->host    != NULL ? strdup(info->host)    : NULL;
    to_set->plugin  = flag->plugin  != NULL ? strdup(flag->plugin)  : NULL;
    to_set->name    = flag->name    != NULL ? strdup(flag->name)    : NULL;
    
    gb_flag_insert(to_set);
    
    t_unref(to_set);
  }
}

void gb_group_take(GBGroup* self, GBFlag* info) {
  t_list_foreach(self->flags, flagn) {
    GBFlag* flag = flagn->unit->obj;
    
    GBFlag* to_set = gb_flag_new();
    to_set->server  = strdup(info->server);
    to_set->channel = info->channel != NULL ? strdup(info->channel) : NULL;
    to_set->host    = info->host    != NULL ? strdup(info->host)    : NULL;
    to_set->plugin  = flag->plugin  != NULL ? strdup(flag->plugin)  : NULL;
    to_set->name    = flag->name    != NULL ? strdup(flag->name)    : NULL;
    
    gb_flag_remove(to_set);
    
    t_unref(to_set);
  }
}
