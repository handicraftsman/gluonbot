#include "utils.h"

pcre2_code* new_regex(char* pattern) {
  int errornumber;
  PCRE2_SIZE erroroffset;
  pcre2_code* re = pcre2_compile((PCRE2_SPTR8) pattern, PCRE2_ZERO_TERMINATED, 0, &errornumber, &erroroffset, NULL);

  if (re == NULL) {
    PCRE2_UCHAR buffer[256];
    pcre2_get_error_message(errornumber, buffer, sizeof(buffer));
    tl_critical("regex", "%s", pattern);
    tl_critical("regex", "%s (%d)", buffer, erroroffset);
    exit(1);
  }
  
  tl_debug("regex", "regexes <- %s", pattern);
  
  return re;
}

bool try_match(pcre2_code* rgx, char* str, pcre2_match_data** md) {
  if (*md != NULL) {
    pcre2_match_data_free(*md);
  }
  *md = pcre2_match_data_create_from_pattern(rgx, NULL);
  int ret = pcre2_match(rgx, (PCRE2_SPTR8) str, strlen(str), 0, 0, *md, NULL);
  if (ret >= 0) {
    return true;
  } else {
    pcre2_match_data_free(*md);
    *md = NULL;
    return false;
  }
}

char* get_substring(pcre2_match_data* md, int idx) {
  char* buf; PCRE2_SIZE len;
  pcre2_substring_get_bynumber(md, idx, (PCRE2_UCHAR8**) &buf, &len);
  return buf;
}
