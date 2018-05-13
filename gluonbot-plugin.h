#pragma once

#include "gluonbot.h"

#include <libxml/tree.h>

GBPlugin* gb_self = NULL;

void gb_init();
void gb_post_init();
void gb_configure(xmlNodePtr root);
void gb_deinit();
