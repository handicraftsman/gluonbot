#pragma once

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <tiny-log.h>

#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

pcre2_code* new_regex(char* pattern);
bool try_match(pcre2_code* rgx, char* str, pcre2_match_data** md);
char* get_substring(pcre2_match_data* md, int idx);
