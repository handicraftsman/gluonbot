#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>

#include <sys/select.h>

#include <getopt.h>

#include <tiny-log.h>

#include "../gluonbot.h"

void display_usage(char* pname) {
  fprintf(stderr, "Usage: %s [OPTIONS]\n\n", pname);
  fprintf(stderr, "Available options: \n");
  fprintf(stderr, "  -h        --help                Show this message\n");
  fprintf(stderr, "  -c FILE   --config-file FILE    Config file [./gluonbot.json]\n");
  //fprintf(stderr, "  -b FILE   --database-file FILE  Database file [./particlebot.db]\n");
  fprintf(stderr, "  -l LEVEL  --log-level LEVEL     Log level [info]\n");
  fprintf(stderr, "Possible log levels: debug, [io | irc], info, warning, error, important, critical\n");
  exit(1);
}

void set_log_level(char* ls) {
  char* c = ls;
  for (; *c; ++c) *c = tolower(*c);
  if (strcmp(ls, "debug") == 0) {
    tl_log_level = TLLevel_DEBUG;
  } else if (strcmp(ls, "io") == 0 || strcmp(ls, "irc") == 0) {
    tl_log_level = TLLevel_IO;
  } else if (strcmp(ls, "info") == 0) {
    tl_log_level = TLLevel_INFO;
  } else if (strcmp(ls, "warning") == 0) {
    tl_log_level = TLLevel_WARNING;
  } else if (strcmp(ls, "error") == 0) {
    tl_log_level = TLLevel_ERROR;
  } else if (strcmp(ls, "important") == 0) {
    tl_log_level = TLLevel_IMPORTANT;
  } else if (strcmp(ls, "critical") == 0) {
    tl_log_level = TLLevel_CRITICAL;
  } else {
    fprintf(stderr, "Invalid log level: %s\n", ls);
    exit(2);
  }
}

static void sighandler(int _) {
  putc('\r', stderr);
  tl_important("main", "Stopping...");
  gb_bot_deinit();
  exit(0);
}

int main(int argc, char** argv) {
  int opt;

  static struct option long_options[] = {
    {"help",          0,                 NULL, 'h'},
    {"config-file",   required_argument, NULL, 'c'},
    //{"database-file", required_argument, NULL, 'd'},
    {"log-level",     required_argument, NULL, 'l'},
  };

  char* config_file = "./gluonbot.json";

  while ((opt = getopt_long(argc, argv, "hc:d:l:", long_options, NULL)) != EOF)
    switch(opt) {
    case 'c':
      config_file = optarg;
      break;
    //case 'd':
    //  db_file = optarg;
    //  break;
    case 'l':
      set_log_level(optarg);
      break;
    case 'h':
      display_usage(argv[0]);
      exit(2);
    default:
      exit(2);
  }

  tl_important("main", "Hello, bot!");
  
  gb_bot_init();
  gb_bot_load_config(config_file);

  signal(SIGTERM, sighandler);
  signal(SIGINT, sighandler);
  signal(SIGHUP, sighandler);
  tl_important("main", "Press Ctrl+C to stop the bot");
  
  while (true) {
    int c = getc(stdin);
    if (c == EOF) sighandler(0);
    if (c == EOF) sighandler(0);
  }

  return 0;
}
