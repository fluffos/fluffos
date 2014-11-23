/*
 * rc.c
 * description: runtime configuration for lpmud
 * author: erikkay@mit.edu
 * last modified: July 4, 1994 [robo]
 * Mar 26, 1995: edited heavily by Beek
 * Aug 29, 1998: modified by Gorta
 */

#include "base/internal/rc.h"

#include <cstring>  // for strlen
#include <deque>
#include <fstream>
#include <sstream>
#include <stdlib.h>  // for exit
#include <string>

#include "base/internal/stralloc.h"
#include "base/internal/external_port.h"

static const int kMaxConfigLineLength = 120;

char *config_str[NUM_CONFIG_STRS];
int config_int[NUM_CONFIG_INTS];

char *external_cmd[NUM_EXTERNAL_CMDS];

static std::deque<std::string> config_lines;

static int scan_config_line(const char *, void *, int);
static void config_init(void); /* don't ask */

static void config_init() {
  int i;

  for (i = 0; i < NUM_CONFIG_INTS; i++) {
    config_int[i] = 0;
  }
  for (i = 0; i < NUM_CONFIG_STRS; i++) {
    config_str[i] = nullptr;
  }
}

/*
 * If the required flag is 0, it will only give a warning if the line is
 * missing from the config file.  Otherwise, it will give an error and exit
 * if the line isn't there.
 */
/* required:
      1  : Must have
      0  : optional
      -1 : warn if missing
      -2 : warn if found.
 */
static int scan_config_line(const char *fmt, void *dest, int required) {
  /* zero the destination.  It is either a pointer to an int or a char
     buffer, so this will work */
  *((int *)dest) = 0;

  bool found = false;
  for (auto line : config_lines) {
    if (sscanf(line.c_str(), fmt, dest) == 1) {
      found = true;
      break;
    }
  }

  std::string line(fmt);
  auto pos = line.find_first_of(':');
  if (pos != std::string::npos) {
    line = line.substr(0, pos);
  }

  if (found) {
    switch (required) {
      case -2:
        // obsolete
        fprintf(stderr, "*Warning: obsolete line in config file, please delete:\n\t%s\n",
                line.c_str());
        return 0;
    }
    return 1;
  } else {
    switch (required) {
      case -1:
        // optional but warn
        fprintf(stderr, "*Warning: Missing line in config file:\n\t%s\n", line.c_str());
        return 0;
      case 0:
        // optional
        return 0;
      case 1:
        // required
        fprintf(stderr, "*Error in config file.  Missing line:\n\t%s\n", line.c_str());
        exit(-1);
    }
  }
  return 0;
}

void read_config(char *filename) {
  /* needed for string_copy() below */
  CONFIG_INT(__MAX_STRING_LENGTH__) = 128;

  config_init();

  std::ifstream f(filename);
  if (f.is_open()) {
    fprintf(stderr, "using config file: %s\n", filename);
  } else {
    fprintf(stderr, "*Error: couldn't find or open config file: '%s'\n", filename);
    exit(-1);
  }
  std::stringstream buffer;
  buffer << f.rdbuf();

  char tmp[kMaxConfigLineLength];
  while (buffer.getline(&tmp[0], sizeof(tmp), '\n')) {
    if (strlen(tmp) == kMaxConfigLineLength - 1) {
      fprintf(stderr, "*Warning: possible truncated config line: %s", tmp);
    }
    if (strlen(tmp) == 0 || strcmp(tmp, "") == 0) {
      continue;
    }
    // Deal with possible CRLF
    if (strlen(tmp) > 2 && tmp[strlen(tmp) - 2] == '\r') {
      tmp[strlen(tmp) - 2] = '\0';
    }
    config_lines.push_back(std::string(tmp));
  }

  {
    scan_config_line("global include file : %[^\n]", tmp, 0);
    char *p;
    p = CONFIG_STR(__GLOBAL_INCLUDE_FILE__) = alloc_cstring(tmp, "config file: gif");

    /* check if the global include file is quoted */
    if (*p && *p != '"' && *p != '<') {
      char *ptr;

      fprintf(stderr,
              "Missing '\"' or '<' around global include file name; adding "
              "quotes.\n");
      for (ptr = p; *ptr; ptr++) {
        ;
      }
      ptr[2] = 0;
      ptr[1] = '"';
      while (ptr > p) {
        *ptr = ptr[-1];
        ptr--;
      }
      *p = '"';
    }
  }

  scan_config_line("name : %[^\n]", tmp, 1);
  CONFIG_STR(__MUD_NAME__) = alloc_cstring(tmp, "config file: mn");

  scan_config_line("mudlib directory : %[^\n]", tmp, 1);
  CONFIG_STR(__MUD_LIB_DIR__) = alloc_cstring(tmp, "config file: mld");

  scan_config_line("binary directory : %[^\n]", tmp, 1);
  CONFIG_STR(__BIN_DIR__) = alloc_cstring(tmp, "config file: bd");

  scan_config_line("log directory : %[^\n]", tmp, 1);
  CONFIG_STR(__LOG_DIR__) = alloc_cstring(tmp, "config file: ld");

  scan_config_line("include directories : %[^\n]", tmp, 1);
  CONFIG_STR(__INCLUDE_DIRS__) = alloc_cstring(tmp, "config file: id");

  scan_config_line("master file : %[^\n]", tmp, 1);
  CONFIG_STR(__MASTER_FILE__) = alloc_cstring(tmp, "config file: mf");

  scan_config_line("simulated efun file : %[^\n]", tmp, 0);
  CONFIG_STR(__SIMUL_EFUN_FILE__) = alloc_cstring(tmp, "config file: sef");

  scan_config_line("swap file : %[^\n]", tmp, 1);
  CONFIG_STR(__SWAP_FILE__) = alloc_cstring(tmp, "config file: sf");

  scan_config_line("debug log file : %[^\n]", tmp, -1);
  CONFIG_STR(__DEBUG_LOG_FILE__) = alloc_cstring(tmp, "config file: dlf");

  scan_config_line("default error message : %[^\n]", tmp, 0);
  CONFIG_STR(__DEFAULT_ERROR_MESSAGE__) = alloc_cstring(tmp, "config file: dem");

  {
    scan_config_line("default fail message : %[^\n]", tmp, 0);
    if (strlen(tmp) == 0) {
      strcpy(tmp, "What?\n");
    }
    if (strlen(tmp) <= kMaxConfigLineLength - 2) {
      strcat(tmp, "\n");
    }
    CONFIG_STR(__DEFAULT_FAIL_MESSAGE__) = alloc_cstring(tmp, "config file: dfm");
  }

  scan_config_line("mud ip : %[^\n]", tmp, 0);
  CONFIG_STR(__MUD_IP__) = alloc_cstring(tmp, "config file: mi");

  scan_config_line("time to clean up : %d\n", &CONFIG_INT(__TIME_TO_CLEAN_UP__), 1);
  scan_config_line("time to reset : %d\n", &CONFIG_INT(__TIME_TO_RESET__), 1);
  scan_config_line("time to swap : %d\n", &CONFIG_INT(__TIME_TO_SWAP__), 1);

// TODO(sunyc): process these.
#if 0
  /*
   * not currently used...see options.h
   */
  scan_config_line("evaluator stack size : %d\n",
                   &CONFIG_INT(__EVALUATOR_STACK_SIZE__), 0);
  scan_config_line("maximum local variables : %d\n",
                   &CONFIG_INT(__MAX_LOCAL_VARIABLES__), 0);
  scan_config_line("maximum call depth : %d\n",
                   &CONFIG_INT(__MAX_CALL_DEPTH__), 0);
  scan_config_line("living hash table size : %d\n",
                   &CONFIG_INT(__LIVING_HASH_TABLE_SIZE__), 0);
#endif

  scan_config_line("inherit chain size : %d\n", &CONFIG_INT(__INHERIT_CHAIN_SIZE__), 1);
  scan_config_line("maximum evaluation cost : %d\n", &CONFIG_INT(__MAX_EVAL_COST__), 1);

  scan_config_line("maximum array size : %d\n", &CONFIG_INT(__MAX_ARRAY_SIZE__), 1);
#ifndef NO_BUFFER_TYPE
  scan_config_line("maximum buffer size : %d\n", &CONFIG_INT(__MAX_BUFFER_SIZE__), 1);
#endif
  scan_config_line("maximum mapping size : %d\n", &CONFIG_INT(__MAX_MAPPING_SIZE__), 1);
  scan_config_line("maximum string length : %d\n", &CONFIG_INT(__MAX_STRING_LENGTH__), 1);
  scan_config_line("maximum bits in a bitfield : %d\n", &CONFIG_INT(__MAX_BITFIELD_BITS__), 1);

  scan_config_line("maximum byte transfer : %d\n", &CONFIG_INT(__MAX_BYTE_TRANSFER__), 1);
  scan_config_line("maximum read file size : %d\n", &CONFIG_INT(__MAX_READ_FILE_SIZE__), 1);

  scan_config_line("hash table size : %d\n", &CONFIG_INT(__SHARED_STRING_HASH_TABLE_SIZE__), 1);
  scan_config_line("object table size : %d\n", &CONFIG_INT(__OBJECT_HASH_TABLE_SIZE__), 1);

  {
    int i, port, port_start = 0;
    if (scan_config_line("port number : %d\n", &CONFIG_INT(__MUD_PORT__), 0)) {
      external_port[0].port = CONFIG_INT(__MUD_PORT__);
      external_port[0].kind = PORT_TELNET;
      port_start = 1;
    }

    /* check for ports */
    if (port_start == 1) {
      if (scan_config_line("external_port_1 : %[^\n]", tmp, 0)) {
        int port = CONFIG_INT(__MUD_PORT__);
        fprintf(stderr,
                "Warning: external_port_1 already defined to be 'telnet %i' by "
                "the line\n    'port number : %i'; ignoring the line "
                "'external_port_1 : %s'\n",
                port, port, tmp);
      }
    }
    for (i = port_start; i < 5; i++) {
      external_port[i].kind = 0;
      external_port[i].fd = -1;

      char kind[kMaxConfigLineLength];
      sprintf(kind, "external_port_%i : %%[^\n]", i + 1);
      if (scan_config_line(kind, tmp, 0)) {
        if (sscanf(tmp, "%s %d", kind, &port) == 2) {
          external_port[i].port = port;
          if (!strcmp(kind, "telnet")) {
            external_port[i].kind = PORT_TELNET;
          } else if (!strcmp(kind, "binary")) {
#ifdef NO_BUFFER_TYPE
            fprintf(stderr, "binary ports unavailable with NO_BUFFER_TYPE defined.\n");
            exit(-1);
#endif
            external_port[i].kind = PORT_BINARY;
          } else if (!strcmp(kind, "ascii")) {
            external_port[i].kind = PORT_ASCII;
          } else if (!strcmp(kind, "MUD")) {
            external_port[i].kind = PORT_MUD;
          } else if (!strcmp(kind, "websocket")) {
            external_port[i].kind = PORT_WEBSOCKET;
          } else {
            fprintf(stderr, "Unknown kind of external port: %s\n", kind);
            exit(-1);
          }
        } else {
          fprintf(stderr, "Syntax error in port specification\n");
          exit(-1);
        }
      }
    }
  }
#ifdef PACKAGE_EXTERNAL
  /* check for commands */
  {
    char kind[kMaxConfigLineLength];

    for (int i = 0; i < NUM_EXTERNAL_CMDS; i++) {
      sprintf(kind, "external_cmd_%i : %%[^\n]", i + 1);
      if (scan_config_line(kind, tmp, 0)) {
        external_cmd[i] = alloc_cstring(tmp, "external cmd");
      } else {
        external_cmd[i] = 0;
      }
    }
  }
#endif

  /*
   * from options.h
   */
  CONFIG_INT(__EVALUATOR_STACK_SIZE__) = CFG_EVALUATOR_STACK_SIZE;
  CONFIG_INT(__MAX_LOCAL_VARIABLES__) = CFG_MAX_LOCAL_VARIABLES;
  CONFIG_INT(__MAX_CALL_DEPTH__) = CFG_MAX_CALL_DEPTH;
  CONFIG_INT(__LIVING_HASH_TABLE_SIZE__) = CFG_LIVING_HASH_SIZE;

  // Complain about obsolete config lines.
  scan_config_line("address server ip : %[^\n]", tmp, -2);
  scan_config_line("address server port : %d\n", tmp, -2);
  scan_config_line("reserved size : %d\n", tmp, -2);
  scan_config_line("fd6 kind : %[^\n]", tmp, -2);
  scan_config_line("fd6 port : %d\n", tmp, -2);

  // Give all obsolete (thus untouched) config strings a value.
  for (int i = 0; i < NUM_CONFIG_STRS; i++) {
    if (config_str[i] == nullptr) {
      config_str[i] = alloc_cstring("", "rc_obsolete");
    }
  }

  // TODO: get rid of config_lines all together.
  config_lines.clear();
  config_lines.shrink_to_fit();
}
