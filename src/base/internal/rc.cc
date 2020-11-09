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
#include <iostream>
#include <sstream>
#include <stdlib.h>  // for exit
#include <string>

#include "base/internal/external_port.h"
#include "base/internal/stralloc.h"
#include "base/internal/strutils.h"
#include "log.h"

char *config_str[NUM_CONFIG_STRS];
int config_int[NUM_CONFIG_INTS];
char *external_cmd[g_num_external_cmds];

namespace {

const int kMaxConfigLineLength = 120;
std::deque<std::string> config_lines;

struct flagEntry {
  std::string key;
  int pos;
  int defaultValue;
  int minValue = 0;
  int maxValue = INT_MAX;
};

const flagEntry intFlags[] = {
    {"time to clean up", __TIME_TO_CLEAN_UP__, 600},
    {"time to reset", __TIME_TO_RESET__, 900},
    {"time to swap", __TIME_TO_SWAP__, 300},

    // __COMPILER_STACK_SIZE__
    {"evaluator stack size", __EVALUATOR_STACK_SIZE__, CFG_EVALUATOR_STACK_SIZE},

    {"inherit chain size", __INHERIT_CHAIN_SIZE__, 30},
    {"maximum evaluation cost", __MAX_EVAL_COST__, 30000000},
    {"maximum local variables", __MAX_LOCAL_VARIABLES__, 64, 64, UINT8_MAX},
    {"maximum call depth", __MAX_CALL_DEPTH__, CFG_MAX_CALL_DEPTH},

    {"maximum array size", __MAX_ARRAY_SIZE__, 15000},
    {"maximum buffer size", __MAX_BUFFER_SIZE__, 400000},
    {"maximum mapping size", __MAX_MAPPING_SIZE__, 150000},
    {"maximum string length", __MAX_STRING_LENGTH__, 200000},
    {"maximum bits in a bitfield", __MAX_BITFIELD_BITS__, 12000},
    {"maximum byte transfer", __MAX_BYTE_TRANSFER__, 200000},
    {"maximum read file size", __MAX_READ_FILE_SIZE__, 200000},

    {"hash table size", __SHARED_STRING_HASH_TABLE_SIZE__, 7001},
    {"object table size", __OBJECT_HASH_TABLE_SIZE__, 1501},
    {"living hash table size", __LIVING_HASH_TABLE_SIZE__, CFG_LIVING_HASH_SIZE},

    {"gametick msec", __RC_GAMETICK_MSEC__, 1000},
    {"heartbeat interval msec", __RC_HEARTBEAT_INTERVAL_MSEC__, 1000},
    {"sane explode string", __RC_SANE_EXPLODE_STRING__, 1},
    {"reversible explode string", __RC_REVERSIBLE_EXPLODE_STRING__, 0},
    {"sane sorting", __RC_SANE_SORTING__, 1},
    {"warn tab", __RC_WARN_TAB__, 0},
    {"wombles", __RC_WOMBLES__, 0},
    {"call other type check", __RC_CALL_OTHER_TYPE_CHECK__, 0},
    {"call other warn", __RC_CALL_OTHER_WARN__, 0},
    {"mudlib error handler", __RC_MUDLIB_ERROR_HANDLER__, 1},
    {"no resets", __RC_NO_RESETS__, 0},
    {"lazy resets", __RC_LAZY_RESETS__, 0},
    {"randomized resets", __RC_RANDOMIZED_RESETS__, 1},
    {"no ansi", __RC_NO_ANSI__, 1},
    {"strip before process input", __RC_STRIP_BEFORE_PROCESS_INPUT__, 1},
    {"this_player in call_out", __RC_THIS_PLAYER_IN_CALL_OUT__, 1},
    {"trace", __RC_TRACE__, 1},
    {"trace code", __RC_TRACE_CODE__, 0},
    {"interactive catch tell", __RC_INTERACTIVE_CATCH_TELL__, 0},
    {"receive snoop", __RC_RECEIVE_SNOOP__, 1},
    {"snoop shadowed", __RC_SNOOP_SHADOWED__, 0},
    {"reverse defer", __RC_REVERSE_DEFER__, 0},
    {"has console", __RC_HAS_CONSOLE__, 1},
    {"noninteractive stderr write", __RC_NONINTERACTIVE_STDERR_WRITE__, 0},
    {"trap crashes", __RC_TRAP_CRASHES__, 1},
    {"old type behavior", __RC_OLD_TYPE_BEHAVIOR__, 0},
    {"old range behavior", __RC_OLD_RANGE_BEHAVIOR__, 0},
    {"warn old range behavior", __RC_WARN_OLD_RANGE_BEHAVIOR__, 1},
    {"suppress argument warnings", __RC_SUPPRESS_ARGUMENT_WARNINGS__, 1},
    {"enable_commands call init", __RC_ENABLE_COMMANDS_CALL_INIT__, 1},
    {"sprintf add_justified ignore ANSI colors", __RC_SPRINTF_ADD_JUSTFIED_IGNORE_ANSI_COLORS__, 1},
    {"call_out(0) nest level", __RC_CALL_OUT_ZERO_NEST_LEVEL__, 1000},
    {"trace lpc execution context", __RC_TRACE_CONTEXT__, 0},
    {"trace lpc instructions", __RC_TRACE_INSTR__, 0},
    {"enable mxp", __RC_ENABLE_MXP__, 0},
    {"enable gmcp", __RC_ENABLE_GMCP__, 0},
    {"enable zmp", __RC_ENABLE_ZMP__, 0},
    {"enable mssp", __RC_ENABLE_MSSP__, 1},
};

void config_init() {
  int i;

  for (i = 0; i < NUM_CONFIG_INTS; i++) {
    config_int[i] = 0;
  }
  for (i = 0; i < NUM_CONFIG_STRS; i++) {
    config_str[i] = nullptr;
  }

  // populate default value for int flags.
  for (const auto &flag : intFlags) {
    CONFIG_INT(flag.pos) = flag.defaultValue;
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
const int kMustHave = 1;
const int kOptional = 0;
const int kWarnMissing = -1;
const int kWarnFound = -2;

bool scan_config_line(const char *fmt, void *dest, int required) {
  /* zero the destination.  It is either a pointer to an int or a char
   buffer, so this will work */
  *(reinterpret_cast<int *>(dest)) = 0;

  bool found = false;
  for (const auto &line : config_lines) {
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
      case kWarnFound:
        // obsolete
        debug_message("*Warning: obsolete line in config file, please delete:\n\t%s\n",
                      line.c_str());
        return false;
    }
    return true;
  } else {
    switch (required) {
      case kWarnMissing:
        // optional but warn
        debug_message("*Warning: Missing line in config file:\n\t%s\n", line.c_str());
        return false;
      case kOptional:
        // optional
        return false;
      case kMustHave:
        // required
        debug_message("*Error in config file.  Missing line:\n\t%s\n", line.c_str());
        exit(-1);
    }
  }
  return false;
}

}  // namespace

void read_config(char *filename) {
  config_init();

  debug_message("Processing config file: %s\n", filename);

  std::ifstream f(filename);
  if (!f.is_open()) {
    perror("Error: couldn't open config file: ");
    exit(-1);
  }

  std::stringstream buffer;
  buffer << f.rdbuf();

  char tmp[kMaxConfigLineLength];
  while (buffer.getline(&tmp[0], sizeof(tmp), '\n')) {
    if (strlen(tmp) == kMaxConfigLineLength - 1) {
      debug_message("*Warning: possible truncated config line: %s\n", tmp);
    }

    std::string v(tmp);

    // ignore anything after # in the line.
    auto pos = v.find_first_of('#');
    if (pos != std::string::npos) {
      v.erase(pos);
    }
    v = trim(v);
    if (v.empty()) {
      continue;
    }
    config_lines.push_back(v + "\n");
  }

  // Process global include file.
  {
    scan_config_line("global include file : %[^\n]", tmp, 0);

    /* check if the global include file is quoted */
    std::string v(tmp);
    if (!starts_with(v, "\"") && !starts_with(v, "<")) {
      debug_message("Missing '\"' or '<' around global include file name; adding quotes.\n");
      // not very efficient, but who cares.
      CONFIG_STR(__GLOBAL_INCLUDE_FILE__) =
          alloc_cstring(("\"" + v + "\"").c_str(), "config file: gif");
    } else {
      CONFIG_STR(__GLOBAL_INCLUDE_FILE__) = alloc_cstring(tmp, "config file: gif");
    }
  }

  scan_config_line("name : %[^\n]", tmp, 1);
  CONFIG_STR(__MUD_NAME__) = alloc_cstring(tmp, "config file: mn");

  scan_config_line("mudlib directory : %[^\n]", tmp, 1);
  CONFIG_STR(__MUD_LIB_DIR__) = alloc_cstring(tmp, "config file: mld");

  scan_config_line("log directory : %[^\n]", tmp, 1);
  CONFIG_STR(__LOG_DIR__) = alloc_cstring(tmp, "config file: ld");

  scan_config_line("include directories : %[^\n]", tmp, 1);
  CONFIG_STR(__INCLUDE_DIRS__) = alloc_cstring(tmp, "config file: id");

  scan_config_line("master file : %[^\n]", tmp, 1);
  CONFIG_STR(__MASTER_FILE__) = alloc_cstring(tmp, "config file: mf");

  scan_config_line("simulated efun file : %[^\n]", tmp, 0);
  CONFIG_STR(__SIMUL_EFUN_FILE__) = alloc_cstring(tmp, "config file: sef");

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

  /* Process ports */
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
        debug_message(
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
            external_port[i].kind = PORT_BINARY;
          } else if (!strcmp(kind, "ascii")) {
            external_port[i].kind = PORT_ASCII;
          } else if (!strcmp(kind, "MUD")) {
            external_port[i].kind = PORT_MUD;
          } else if (!strcmp(kind, "websocket")) {
            external_port[i].kind = PORT_WEBSOCKET;
            scan_config_line("websocket http dir : %[^\n]", tmp, kMustHave);
            CONFIG_STR(__RC_WEBSOCKET_HTTP_DIR__) = alloc_cstring(tmp, "config file: whd");
          } else {
            debug_message("Unknown kind of external port: %s\n", kind);
            exit(-1);
          }
        } else {
          debug_message("Syntax error in port specification\n");
          exit(-1);
        }
      }
    }
  }
#ifdef PACKAGE_EXTERNAL
  /* check for commands */
  {
    char kind[kMaxConfigLineLength];

    for (int i = 0; i < g_num_external_cmds; i++) {
      sprintf(kind, "external_cmd_%i : %%[^\n]", i + 1);
      if (scan_config_line(kind, tmp, 0)) {
        external_cmd[i] = alloc_cstring(tmp, "external cmd");
      } else {
        external_cmd[i] = 0;
      }
    }
  }
#endif

  // Complain about obsolete config lines.
  scan_config_line("address server ip : %[^\n]", tmp, -2);
  scan_config_line("address server port : %d\n", tmp, -2);
  scan_config_line("reserved size : %d\n", tmp, -2);
  scan_config_line("fd6 kind : %[^\n]", tmp, -2);
  scan_config_line("fd6 port : %d\n", tmp, -2);
  scan_config_line("binary directory : %[^\n]", tmp, kWarnFound);
  scan_config_line("swap file : %[^\n]", tmp, kWarnFound);

  // Give all obsolete (thus untouched) config strings a value.
  for (auto &i : config_str) {
    if (i == nullptr) {
      i = alloc_cstring("", "rc_obsolete");
    }
  }

  // process int flags
  for (const auto &flag : intFlags) {
    int value = 0;
    char buf[256];
    sprintf(buf, "%s : %%d\n", flag.key.c_str());

    if (scan_config_line(buf, &value, kOptional)) {
      if (value != flag.defaultValue) {
        if (value < flag.minValue || value > flag.maxValue) {
          debug_message("%s: invalid new value, resetting to default.\n", flag.key.c_str());
          value = flag.defaultValue;
        }
        CONFIG_INT(flag.pos) = value;
      }
    }
  }
  // TODO: get rid of config_lines all together.
  config_lines.clear();
  config_lines.shrink_to_fit();
}

void print_rc_table() {
  for (const auto &flag : intFlags) {
    auto val = CONFIG_INT(flag.pos);
    if (val != flag.defaultValue) {
      debug_message("%s : %d # default: %d\n", flag.key.c_str(), val, flag.defaultValue);
    } else {
      debug_message("%s : %d\n", flag.key.c_str(), val);
    }
  }
}
