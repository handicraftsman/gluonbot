#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <tiny.h>

#include "utils.h"

char* gbu_strjoin(TVector* strings, char* separator) {
  return gbu_strjoin_offset(strings, separator, 0);
}

// found here: https://stackoverflow.com/a/4681415/5387903
char* gbu_strjoin_offset(TVector* strings, char* separator, int offset) {
  char* str = NULL;
  size_t total_length = 0;
  int i = 0;

  for (i = 0; i < strings->len; i++) total_length += strlen(strings->arr[i]->obj);
  total_length++;
  total_length += strlen(separator) * (strings->len - 1);

  str = (char*) t_malloc(total_length);
  str[0] = '\0';

  for (i = offset; i < strings->len; i++) {
    strcat(str, strings->arr[i]->obj);
    if (i < (strings->len - 1)) strcat(str, separator);
  }

  return str;
}
