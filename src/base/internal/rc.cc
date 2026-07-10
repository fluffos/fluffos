/*
 * rc.c
 * description: runtime configuration for lpmud
 * author: erikkay@mit.edu
 * last modified: July 4, 1994 [robo]
 * Mar 26, 1995: edited heavily by Beek
 * Aug 29, 1998: modified by Gorta
 */

#include "base/internal/rc.h"

#include <cstdio>   // for printf
#include <cstring>  // for strlen
#include <deque>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdlib>  // for exit
#include <string>

#include "base/internal/external_port.h"
#include "base/internal/stralloc.h"
#include "base/internal/strutils.h"
#include "log.h"

char* config_str[NUM_CONFIG_STRS];
int config_int[NUM_CONFIG_INTS];
char* external_cmd[g_num_external_cmds];

namespace {

const int K_MAX_CONFIG_LINE_LENGTH = 120;
std::deque<std::string> config_lines;

// ─────────────────────────────────────────────────────────────────────────────
// SOURCE OF TRUTH for docs/driver/config.md.
//
// INT_FLAGS[] (below) and STR_FLAGS[] (further down) define every recognized
// runtime config option together with its documentation. The driver parses
// config files directly from these tables, and docs/gen_config_docs.py
// generates docs/driver/config.md from them -- so the `category` and
// `description` fields below ARE the documentation.
//
// After adding, removing, or changing an entry, regenerate the docs and commit
// the result:
//     python3 docs/gen_config_docs.py
//
// CI (.github/workflows/config-docs.yml) re-runs the generator with --check and
// fails the build if docs/driver/config.md is out of date. Keep descriptions to
// a single line and avoid the '|' character (it breaks the Markdown table).
// ─────────────────────────────────────────────────────────────────────────────
struct FlagEntry {
  std::string key;          // recognized config-file setting name
  int pos;                  // slot constant from runtime_config.h
  int defaultValue;         // value used when the option is omitted
  int minValue;             // inclusive lower bound (else reset to default)
  int maxValue;             // inclusive upper bound (else reset to default)
  std::string category;     // grouping heading in the generated docs
  std::string description;  // doc prose: one line, no '|' characters
};

const FlagEntry INT_FLAGS[] = {
    {"time to clean up", __TIME_TO_CLEAN_UP__, 600, 0, INT_MAX, "Timing & Lifecycle",
     "Seconds an object may be idle before clean_up() is called on it; should be well above 'time "
     "to swap'."},
    {"time to reset", __TIME_TO_RESET__, 900, 0, INT_MAX, "Timing & Lifecycle",
     "Seconds between successive reset() calls on an object."},
    {"time to swap", __TIME_TO_SWAP__, 300, 0, INT_MAX, "Timing & Lifecycle",
     "Seconds an unused object stays in memory before being swapped out; 0 disables swapping."},

    {"evaluator stack size", __EVALUATOR_STACK_SIZE__, CFG_EVALUATOR_STACK_SIZE, 0, INT_MAX,
     "Limits",
     "Maximum size of the evaluator stack, which holds all local variables and call arguments."},
    {"inherit chain size", __INHERIT_CHAIN_SIZE__, 30, 0, INT_MAX, "Limits",
     "Maximum depth of an object's inheritance chain."},
    {"maximum evaluation cost", __MAX_EVAL_COST__, 30000000, 0, INT_MAX, "Limits",
     "Maximum eval cost a single thread may consume before execution is aborted."},
    {"maximum local variables", __MAX_LOCAL_VARIABLES__, 64, 64, UINT8_MAX, "Limits",
     "Maximum number of local variables in a single function."},
    {"maximum call depth", __MAX_CALL_DEPTH__, CFG_MAX_CALL_DEPTH, 0, INT_MAX, "Limits",
     "Maximum nesting depth of LPC function calls."},

    {"maximum array size", __MAX_ARRAY_SIZE__, 15000, 0, INT_MAX, "Limits",
     "Maximum number of elements in a single array."},
    {"maximum buffer size", __MAX_BUFFER_SIZE__, 1 << 20, 0, INT_MAX, "Limits",
     "Maximum size, in bytes, of a single buffer variable."},
    {"maximum mapping size", __MAX_MAPPING_SIZE__, 150000, 0, INT_MAX, "Limits",
     "Maximum number of entries in a single mapping."},
    {"maximum string length", __MAX_STRING_LENGTH__, 1 << 20, 0, INT_MAX, "Limits",
     "Maximum length, in bytes, of a single string variable."},
    {"maximum bits in a bitfield", __MAX_BITFIELD_BITS__, 12000, 0, INT_MAX, "Limits",
     "Maximum number of bits in a bitfield (stored 6 bits per printable byte)."},
    {"maximum byte transfer", __MAX_BYTE_TRANSFER__, 1 << 18, 0, INT_MAX, "Limits",
     "Maximum number of bytes a single read_bytes()/write_bytes() call may transfer."},
    {"maximum read file size", __MAX_READ_FILE_SIZE__, 1 << 18, 0, INT_MAX, "Limits",
     "Maximum size, in bytes, of a file that read_file() may read."},

    {"hash table size", __SHARED_STRING_HASH_TABLE_SIZE__, 65536, 7001, INT_MAX, "Hash Tables",
     "Size of the shared-string hash table; should be prime, roughly 1/5 of the number of distinct "
     "strings."},
    {"object table size", __OBJECT_HASH_TABLE_SIZE__, 4096, 1024, INT_MAX, "Hash Tables",
     "Size of the object hash table; roughly 1/4 of the number of objects in the game."},
    {"living hash table size", __LIVING_HASH_TABLE_SIZE__, 256, 256, INT_MAX, "Hash Tables",
     "Size of the find_living() hash table; must be one of 4, 16, 64, 256, 1024, or 4096."},

    {"gametick msec", __RC_GAMETICK_MSEC__, 1000, 0, INT_MAX, "Timing & Lifecycle",
     "Granularity of in-game time in milliseconds (the shortest visible time interval)."},
    {"heartbeat interval msec", __RC_HEARTBEAT_INTERVAL_MSEC__, 1000, 0, INT_MAX,
     "Timing & Lifecycle", "Heartbeat interval in milliseconds."},
    {"sane explode string", __RC_SANE_EXPLODE_STRING__, 1, 0, INT_MAX, "Language Behavior",
     "explode() strips at most one leading delimiter (and still one trailing delimiter)."},
    {"reversible explode string", __RC_REVERSIBLE_EXPLODE_STRING__, 0, 0, INT_MAX,
     "Language Behavior",
     "Make implode(explode(x, y), y) always equal x; overrides 'sane explode string'."},
    {"sane sorting", __RC_SANE_SORTING__, 1, 0, INT_MAX, "Language Behavior",
     "Use a well-defined, stable ordering for the driver's sorting operations."},
    {"call other type check", __RC_CALL_OTHER_TYPE_CHECK__, 0, 0, INT_MAX, "Type Checking",
     "Enable type checking for call_other() (the -> operator on objects)."},
    {"call other warn", __RC_CALL_OTHER_WARN__, 0, 0, INT_MAX, "Type Checking",
     "Emit warnings instead of errors for call_other() type mismatches."},
    {"mudlib error handler", __RC_MUDLIB_ERROR_HANDLER__, 1, 0, INT_MAX, "Error Handling",
     "Pass runtime errors to the master object's error_handler() instead of handling them in the "
     "driver."},
    {"no resets", __RC_NO_RESETS__, 0, 0, INT_MAX, "Reset Behavior",
     "Completely disable the periodic calling of reset()."},
    {"lazy resets", __RC_LAZY_RESETS__, 0, 0, INT_MAX, "Reset Behavior",
     "Only call reset() when an object is touched via call_other() or move_object()."},
    {"randomized resets", __RC_RANDOMIZED_RESETS__, 1, 0, INT_MAX, "Reset Behavior",
     "Spread reset() calls over a randomized interval rather than firing them all at once."},
    {"no ansi", __RC_NO_ANSI__, 1, 0, INT_MAX, "Player I/O",
     "Replace ANSI escape characters (ASCII 27) in user input with a space before add_actions "
     "run."},
    {"strip before process input", __RC_STRIP_BEFORE_PROCESS_INPUT__, 1, 0, INT_MAX, "Player I/O",
     "Strip ANSI before process_input() sees the input, rather than only before add_actions are "
     "called."},
    {"this_player in call_out", __RC_THIS_PLAYER_IN_CALL_OUT__, 1, 0, INT_MAX, "Language Behavior",
     "Make this_player() usable from within call_out() callbacks."},
    {"trace", __RC_TRACE__, 1, 0, INT_MAX, "Diagnostics",
     "Enable the trace() and traceprefix() efuns (leaving it off runs slightly faster)."},
    {"trace code", __RC_TRACE_CODE__, 0, 0, INT_MAX, "Diagnostics",
     "Include the preceding lines of LPC code in error traces (slower)."},
    {"interactive catch tell", __RC_INTERACTIVE_CATCH_TELL__, 0, 0, INT_MAX, "Player I/O",
     "Call catch_tell() on interactive users as well as on NPCs."},
    {"receive snoop", __RC_RECEIVE_SNOOP__, 1, 0, INT_MAX, "Player I/O",
     "Send snoop text to receive_snoop() in the snooper instead of directly via add_message()."},
    {"snoop shadowed", __RC_SNOOP_SHADOWED__, 0, 0, INT_MAX, "Player I/O",
     "Report snooped output even when the target's catch_tell() is shadowed (prefixed with $$)."},
    {"reverse defer", __RC_REVERSE_DEFER__, 0, 0, INT_MAX, "Language Behavior",
     "Run deferred functions registered with defer() in reverse order."},
    {"has console", __RC_HAS_CONSOLE__, 1, 0, INT_MAX, "Diagnostics",
     "Allow the driver's interactive console via the -C command-line argument."},
    {"noninteractive stderr write", __RC_NONINTERACTIVE_STDERR_WRITE__, 0, 0, INT_MAX, "Player I/O",
     "Write tells/messages sent to non-interactive objects to stderr, prefixed with ']' (legacy "
     "behavior)."},
    {"trap crashes", __RC_TRAP_CRASHES__, 1, 0, INT_MAX, "Error Handling",
     "Call crash() in the master object and shut down cleanly on signals that would otherwise "
     "crash the driver."},
    {"old type behavior", __RC_OLD_TYPE_BEHAVIOR__, 0, 0, INT_MAX, "Type Checking",
     "Reintroduce a legacy type-checking bug for backwards compatibility."},
    {"old range behavior", __RC_OLD_RANGE_BEHAVIOR__, 0, 0, INT_MAX, "Language Behavior",
     "Treat negative range indices in strings/buffers as counting from the end (rvalue use only)."},
    {"warn old range behavior", __RC_WARN_OLD_RANGE_BEHAVIOR__, 1, 0, INT_MAX, "Language Behavior",
     "Warn when code relies on 'old range behavior'."},
    {"suppress argument warnings", __RC_SUPPRESS_ARGUMENT_WARNINGS__, 1, 0, INT_MAX, "Diagnostics",
     "Suppress unused-argument warnings, warning only about unused local variables."},
    {"enable_commands call init", __RC_ENABLE_COMMANDS_CALL_INIT__, 1, 0, INT_MAX,
     "Language Behavior", "Call init() in an object when enable_commands() is invoked on it."},
    {"sprintf add_justified ignore ANSI colors", __RC_SPRINTF_ADD_JUSTFIED_IGNORE_ANSI_COLORS__, 1,
     0, INT_MAX, "Language Behavior",
     "Make sprintf() column justification ignore ANSI color codes when computing field width."},
    {"call_out(0) nest level", __RC_CALL_OUT_ZERO_NEST_LEVEL__, 1000, 0, INT_MAX,
     "Language Behavior",
     "Maximum nesting level for chains of call_out(0) within a single backend cycle."},
    {"trace lpc execution context", __RC_TRACE_CONTEXT__, 0, 0, INT_MAX, "Diagnostics",
     "Record LPC execution context for tracing and debugging."},
    {"trace lpc instructions", __RC_TRACE_INSTR__, 0, 0, INT_MAX, "Diagnostics",
     "Trace individual LPC instructions for debugging."},
    {"display preload progress", __RC_DISPLAY_PRELOAD_PROGRESS__, 1, 0, INT_MAX, "Diagnostics",
     "Print each file name to the debug log while preload files are loaded at boot."},
    {"enable mxp", __RC_ENABLE_MXP__, 0, 0, INT_MAX, "Protocol Support",
     "Advertise and enable the MXP telnet protocol."},
    {"enable gmcp", __RC_ENABLE_GMCP__, 0, 0, INT_MAX, "Protocol Support",
     "Advertise and enable the GMCP telnet protocol."},
    {"enable zmp", __RC_ENABLE_ZMP__, 0, 0, INT_MAX, "Protocol Support",
     "Advertise and enable the ZMP telnet protocol."},
    {"enable mssp", __RC_ENABLE_MSSP__, 1, 0, INT_MAX, "Protocol Support",
     "Advertise and enable the MSSP telnet protocol."},
    {"enable msp", __RC_ENABLE_MSP__, 1, 0, INT_MAX, "Protocol Support",
     "Advertise and enable the MSP telnet protocol."},
    {"enable msdp", __RC_ENABLE_MSDP__, 0, 0, INT_MAX, "Protocol Support",
     "Advertise and enable the MSDP telnet protocol."},
};

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
const int K_WARN_FOUND = -2;

// String config options. Like INT_FLAGS, this table is the source of truth for
// both parsing and docs/driver/config.md: the simple "key : <text>" options are
// parsed directly from it in read_config(). A few string-valued options with
// special handling (the external ports, external commands, the global include
// file, and the default fail message) are parsed separately but still
// documented -- that hand-maintained section lives in docs/gen_config_docs.py.
// After editing this table, regenerate the docs (python3 docs/gen_config_docs.py).
struct StrFlagEntry {
  std::string key;          // recognized config-file setting name
  int pos;                  // slot constant from runtime_config.h
  int required;             // kMustHave / kOptional / kWarnMissing
  std::string tag;          // allocation label (debugging aid)
  std::string category;     // grouping heading in the generated docs
  std::string description;  // doc prose: one line, no '|' characters
};

const StrFlagEntry STR_FLAGS[] = {
    {"name", __MUD_NAME__, kMustHave, "config file: mn", "Identity & Network", "Name of this MUD."},
    {"mudlib directory", __MUD_LIB_DIR__, kMustHave, "config file: mld", "Directory Structure",
     "Absolute path to the mudlib root (this path is not relative to the mudlib)."},
    {"log directory", __LOG_DIR__, kMustHave, "config file: ld", "Directory Structure",
     "Filesystem directory for debug.log and stats files, resolved relative to the driver's "
     "working directory (leading slashes are stripped); not a mudlib virtual path."},
    {"include directories", __INCLUDE_DIRS__, kMustHave, "config file: id", "Directory Structure",
     "Colon-separated list of directories searched by `#include <...>`."},
    {"master file", __MASTER_FILE__, kMustHave, "config file: mf", "Core Files",
     "Path to the object that defines the master object."},
    {"simulated efun file", __SIMUL_EFUN_FILE__, kOptional, "config file: sef", "Core Files",
     "Path to the object that defines global simulated efuns."},
    {"debug log file", __DEBUG_LOG_FILE__, kWarnMissing, "config file: dlf", "Logging",
     "Filename (within the log directory) for the driver's debug log."},
    {"default error message", __DEFAULT_ERROR_MESSAGE__, kOptional, "config file: dem",
     "Error Handling", "Message shown to players when error() occurs."},
    {"mud ip", __MUD_IP__, kOptional, "config file: mi", "Identity & Network",
     "IP address to bind to; useful on hosts with multiple network addresses."},
    {"ffi allowed libraries", __FFI_ALLOWED_LIBRARIES__, kOptional, "config file: fal", "Security",
     "Colon-separated allow-list of shared-library paths that ffi_load() may open (package_ffi). "
     "Empty means the driver imposes no path restriction and defers entirely to the master apply "
     "valid_ffi(); every ffi_load/symbol/prepare/callback is gated by that apply regardless."},
};

bool scan_config_line(const char* fmt, void* dest, int required) {
  /* zero the destination.  It is either a pointer to an int or a char
   buffer, so this will work */
  *(reinterpret_cast<int*>(dest)) = 0;

  bool found = false;
  for (const auto& line : config_lines) {
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
      case K_WARN_FOUND:
        // obsolete
        debug_message("*Warning: obsolete line in config file, please delete:\n\t%s\n",
                      line.c_str());
        return false;
    }
    return true;
  }
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

  return false;
}

}  // namespace

void config_init() {
  int i;

  for (i = 0; i < NUM_CONFIG_INTS; i++) {
    config_int[i] = 0;
  }
  for (i = 0; i < NUM_CONFIG_STRS; i++) {
    config_str[i] = nullptr;
  }

  // populate default value for int flags.
  for (const auto& flag : INT_FLAGS) {
    CONFIG_INT(flag.pos) = flag.defaultValue;
  }
}

void read_config(const char* filename) {
  config_init();

  debug_message("Processing config file: %s\n", filename);

  std::ifstream f(filename);
  if (!f.is_open()) {
    perror("Error: couldn't open config file: ");
    exit(-1);
  }

  std::stringstream buffer;
  buffer << f.rdbuf();

  char tmp[K_MAX_CONFIG_LINE_LENGTH];
  while (buffer.getline(&tmp[0], sizeof(tmp), '\n')) {
    if (strlen(tmp) == K_MAX_CONFIG_LINE_LENGTH - 1) {
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
    std::string const v(tmp);
    if (!starts_with(v, "\"") && !starts_with(v, "<")) {
      debug_message("Missing '\"' or '<' around global include file name; adding quotes.\n");
      // not very efficient, but who cares.
      CONFIG_STR(__GLOBAL_INCLUDE_FILE__) =
          alloc_cstring(("\"" + v + "\"").c_str(), "config file: gif");
    } else {
      CONFIG_STR(__GLOBAL_INCLUDE_FILE__) = alloc_cstring(tmp, "config file: gif");
    }
  }

  // Process the simple string options from the STR_FLAGS table.
  for (const auto& flag : STR_FLAGS) {
    char buf[256];
    sprintf(buf, "%s : %%[^\n]", flag.key.c_str());
    scan_config_line(buf, tmp, flag.required);
    CONFIG_STR(flag.pos) = alloc_cstring(tmp, flag.tag.c_str());
  }

  {
    scan_config_line("default fail message : %[^\n]", tmp, 0);
    if (strlen(tmp) == 0) {
      strcpy(tmp, "What?\n");
    }
    if (strlen(tmp) <= K_MAX_CONFIG_LINE_LENGTH - 2) {
      strcat(tmp, "\n");
    }
    CONFIG_STR(__DEFAULT_FAIL_MESSAGE__) = alloc_cstring(tmp, "config file: dfm");
  }

  /* Process ports */
  {
    int i, port, port_start = 0;
    if (scan_config_line("port number : %d\n", &CONFIG_INT(__MUD_PORT__), 0)) {
      external_port[0].port = CONFIG_INT(__MUD_PORT__);
      external_port[0].kind = PORT_TYPE_TELNET;
      port_start = 1;
    }

    /* check for ports */
    if (port_start == 1) {
      if (scan_config_line("external_port_1 : %[^\n]", tmp, 0)) {
        int const port = CONFIG_INT(__MUD_PORT__);
        debug_message(
            "Warning: external_port_1 already defined to be 'telnet %i' by "
            "the line\n    'port number : %i'; ignoring the line "
            "'external_port_1 : %s'\n",
            port, port, tmp);
      }
    }
    for (i = port_start; i < 5; i++) {
      external_port[i].kind = PORT_TYPE_UNDEFINED;
      external_port[i].fd = -1;

      char kind[K_MAX_CONFIG_LINE_LENGTH];
      sprintf(kind, "external_port_%i : %%[^\n]", i + 1);
      if (scan_config_line(kind, tmp, 0)) {
        if (sscanf(tmp, "%s %d", kind, &port) == 2) {
          external_port[i].port = port;
          if (!strcmp(kind, "telnet")) {
            external_port[i].kind = PORT_TYPE_TELNET;
          } else if (!strcmp(kind, "binary")) {
            external_port[i].kind = PORT_TYPE_BINARY;
          } else if (!strcmp(kind, "ascii")) {
            external_port[i].kind = PORT_TYPE_ASCII;
          } else if (!strcmp(kind, "MUD")) {
            external_port[i].kind = PORT_TYPE_MUD;
          } else if (!strcmp(kind, "websocket")) {
            external_port[i].kind = PORT_TYPE_WEBSOCKET;
            if (!CONFIG_STR(__RC_WEBSOCKET_HTTP_DIR__)) {
              scan_config_line("websocket http dir : %[^\n]", tmp, kMustHave);
              CONFIG_STR(__RC_WEBSOCKET_HTTP_DIR__) = alloc_cstring(tmp, "config file: whd");
            }
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
    // TLS support status
    for (i = port_start; i < 5; i++) {
      if (external_port[i].kind != PORT_TYPE_UNDEFINED) {
        char kind[K_MAX_CONFIG_LINE_LENGTH];
        sprintf(kind, "external_port_%i_tls : %%[^\n]", i + 1);
        if (scan_config_line(kind, tmp, 0)) {
          char cert[255 + 1]{}, key[255 + 1]{};
          if (sscanf(tmp, "cert=%255s key=%255s", cert, key) == 2) {
            if (strlen(cert) == 0 || strlen(key) == 0) {
              debug_message("cert/key path can't be empty.\n");
              exit(-1);
            }
            external_port[i].tls_cert = cert;
            external_port[i].tls_key = key;
          }
        }
      }
    }
  }
#ifdef PACKAGE_EXTERNAL
  /* check for commands */
  {
    char kind[K_MAX_CONFIG_LINE_LENGTH];

    for (int i = 0; i < g_num_external_cmds; i++) {
      sprintf(kind, "external_cmd_%i : %%[^\n]", i + 1);
      if (scan_config_line(kind, tmp, 0)) {
        external_cmd[i] = alloc_cstring(tmp, "external cmd");
      } else {
        external_cmd[i] = nullptr;
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
  scan_config_line("binary directory : %[^\n]", tmp, K_WARN_FOUND);
  scan_config_line("swap file : %[^\n]", tmp, K_WARN_FOUND);
  scan_config_line("wombles : %d\n", tmp, K_WARN_FOUND);
  scan_config_line("warn tab : %d\n", tmp, K_WARN_FOUND);

  // Give all obsolete (thus untouched) config strings a value.
  for (auto& i : config_str) {
    if (i == nullptr) {
      i = alloc_cstring("", "rc_obsolete");
    }
  }

  // process int flags
  for (const auto& flag : INT_FLAGS) {
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
  for (const auto& flag : INT_FLAGS) {
    auto val = CONFIG_INT(flag.pos);
    if (val != flag.defaultValue) {
      debug_message("%s : %d # default: %d\n", flag.key.c_str(), val, flag.defaultValue);
    } else {
      debug_message("%s : %d\n", flag.key.c_str(), val);
    }
  }
}

// Starter values for the required/recommended string options. Integer options
// emit their compiled-in defaults; these paths and the MUD name have no sensible
// default, so they get obvious placeholders the operator must edit.
namespace {
// Print `text` as one or more "# ..." comment lines, wrapped on whitespace.
// The config parser drops any line >= K_MAX_CONFIG_LINE_LENGTH chars (it trips
// failbit and stops reading the file), so comment lines must stay short.
void print_comment(const std::string& text) {
  const size_t width = 76;
  std::istringstream words(text);
  std::string word, line;
  while (words >> word) {
    if (!line.empty() && line.size() + 1 + word.size() > width) {
      printf("# %s\n", line.c_str());
      line.clear();
    }
    line += line.empty() ? word : " " + word;
  }
  if (!line.empty()) {
    printf("# %s\n", line.c_str());
  }
}

std::string template_str_value(const std::string& key) {
  if (key == "name") return "CHANGE_ME";
  if (key == "mudlib directory") return "/path/to/your/mudlib";
  if (key == "log directory") return "log";
  if (key == "include directories") return "/include";
  if (key == "master file") return "/single/master";
  if (key == "debug log file") return "debug.log";
  if (key == "simulated efun file") return "/single/simul_efun";
  if (key == "mud ip") return "127.0.0.1";
  return "";
}
}  // namespace

void print_config_template() {
  static const char* const CATEGORY_ORDER[] = {
      "Identity & Network", "Directory Structure", "Core Files",       "Logging",
      "Error Handling",     "Timing & Lifecycle",  "Limits",           "Hash Tables",
      "Reset Behavior",     "Language Behavior",   "Type Checking",    "Player I/O",
      "Diagnostics",        "Performance",         "Protocol Support",
  };

  printf(
      "###############################################################################\n"
      "# FluffOS configuration file - generated by `driver --generate-config`.\n"
      "#\n"
      "# Integer options are shown at their compiled-in defaults. Required values\n"
      "# with no sensible default (the MUD name, the mudlib path) use placeholders\n"
      "# you must edit. Options that need external resources (websocket, TLS,\n"
      "# external commands) are commented out -- uncomment and edit to enable them.\n"
      "###############################################################################\n\n");

  for (const auto* category : CATEGORY_ORDER) {
    bool header_printed = false;
    auto ensure_header = [&]() {
      if (!header_printed) {
        printf("# ----- %s -----\n\n", category);
        header_printed = true;
      }
    };

    for (const auto& s : STR_FLAGS) {
      if (s.category != category) {
        continue;
      }
      ensure_header();
      const char* tag = s.required == kMustHave      ? " (required)"
                        : s.required == kWarnMissing ? " (recommended)"
                                                     : " (optional)";
      print_comment(s.description + tag);
      std::string const value = template_str_value(s.key);
      if (s.required == kOptional) {
        printf("# %s : %s\n\n", s.key.c_str(), value.c_str());
      } else {
        printf("%s : %s\n\n", s.key.c_str(), value.c_str());
      }
    }

    for (const auto& f : INT_FLAGS) {
      if (f.category != category) {
        continue;
      }
      ensure_header();
      print_comment(f.description);
      printf("%s : %d\n\n", f.key.c_str(), f.defaultValue);
    }

    // The listening ports belong conceptually with Identity & Network.
    if (std::string(category) == "Identity & Network") {
      printf("# ----- Ports & Connections -----\n\n");
      printf("# Plain telnet listener.\n");
      printf("external_port_1 : telnet 4000\n\n");
      printf("# Optional extra listeners -- uncomment and edit to enable:\n");
      printf("# external_port_2 : binary 4001\n");
      printf("# external_port_3 : websocket 8080\n");
      printf("# websocket http dir : www                # a websocket port requires this\n");
      printf("# external_port_3_tls : cert=etc/cert.pem key=etc/key.pem\n\n");
    }
  }

  // Options parsed specially in read_config() (not in the tables above).
  printf("# ----- Other -----\n\n");
  printf("# Header automatically #include'd in every compiled object (optional).\n");
  printf("# global include file : \"/include/globals.h\"\n\n");
  printf("# Message used when an action returns 0 and no notify_fail() was set.\n");
  printf("default fail message : What?\n\n");
  printf("# External programs callable via external_start() (requires PACKAGE_EXTERNAL).\n");
  printf("# external_cmd_1 : /bin/cat\n\n");
}
