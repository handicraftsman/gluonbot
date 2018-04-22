#define _GNU_SOURCE

#include "Plugin.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include <dlfcn.h>
#include <tiny-log.h>

typedef void (*GBSetupFunc)();

GBPlugin* gb_plugin_new(char* name) {
  GBPlugin* self = (GBPlugin*) t_malloc(sizeof(GBPlugin));
  if (!self) {
    perror("gb_channeldesc_new");
    exit(1);
  }
  
  t_gcunit(self) = t_gcunit_new_(self, gb_plugin_destroy);
  
  self->name = strdup(name);
  
  self->l = (char*) malloc(strlen(self->name) + 2);
  sprintf(self->l, "?%s", self->name);
  
  memset(self->handlers, 0, (sizeof(GBEventHandleFunc) * _GBEventType_SIZE));
  
  char* plugin_paths[] = {
    "./libgb-%s.so",
    "/usr/lib/libgb-%s.so",
    "/usr/local/lib/libgb-%s.so",
    NULL
  };
    
  char* path = NULL;
  
  for (int i = 0; plugin_paths[i] != NULL; ++i) {
    char* p;
    asprintf(&p, plugin_paths[i], self->name);
    if (access(p, F_OK) != -1) {
      path = p;
    } else {
      free(p);
    }
  }
  
  if (path == NULL) {
    tl_critical(self->l, "Unable to find myself");
    exit(1);
  }
  
  char* error;
  
  self->handle = dlopen(path, RTLD_NOW | RTLD_LOCAL);
  if ((error = dlerror()) != NULL) {
    tl_critical(self->l, "Unable to load myself: %s", error);
    exit(1);
  }
    
  tl_important(self->l, "Hello, bot!");
  
  GBPlugin** p = (GBPlugin**) dlsym(self->handle, "gb_self");
  if ((error = dlerror()) != NULL) {
    tl_critical(self->l, "Unable to find reference to me in myself. Did you #include <gluonbot-plugin.h> in the main file?");
    tl_critical(self->l, "%s", error);
    exit(1);
  }
  *p = self;
  
  GBSetupFunc init = (GBSetupFunc) dlsym(self->handle, "gb_init");
  if ((error = dlerror()) == NULL) {
    init();
  }
  
  free(path);
  
  return self;
}

void gb_plugin_destroy(GBPlugin* self) {
  assert(self != NULL);
  
  char* error;
    
  GBSetupFunc deinit = (GBSetupFunc) dlsym(self->handle, "gb_deinit");
  if ((error = dlerror()) == NULL) {
    deinit();
  }
  
  t_free(self->name);
  t_free(self->l);
  
  if (self->handle != NULL) dlclose(self->handle);
}

void gb_plugin_register_handler(GBPlugin* self, GBEventType type, GBEventHandleFunc handler) {
  assert(self != NULL);
  t_ref(self);
  
  self->handlers[type] = handler;
  
  t_unref(self);
}
