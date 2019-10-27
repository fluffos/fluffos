#include "log.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <cerrno>

#include <fstream>
#include <iostream>

#include <unistd.h>

#include "rc.h"  // for CONFIG_*

#define E(x) \
  { #x, DBG_##x }

// !!! Keep in sync with defines in header file !!!
const debug_t levels[] = {
        E(call_out), E(addr_server), E(d_flag), E(connections), E(mapping), E(sockets),    E(comp_func_tab),
        E(LPC),      E(LPC_line),    E(event),  E(dns),         E(file),    E(add_action), E(telnet),
};

const size_t sizeof_levels = (sizeof(levels) / sizeof(levels[0]));

void debug_message_internal(const std::string fmt, fmt::format_args args) {
    static FILE *debug_message_fp = nullptr;
    std::string deb_buf;

    if (!debug_message_fp) {
        size_t i = 0;
    /*
     * check whether config file specified this option
     */
        std::string dlf = CONFIG_STR(__DEBUG_LOG_FILE__);

        deb_buf = CONFIG_STR(__LOG_DIR__);
        if (dlf.length()) {
            deb_buf += '/' + dlf;
        } else {
            deb_buf += "/debug.log";
        }
        try {
            while(deb_buf.at(i) == '/') {
                i++;
            }
        }
        catch(std::out_of_range &error) {
            errno = EINVAL;
        }
        if(i < deb_buf.size()) {
            debug_message_fp = fopen(deb_buf.substr(i).c_str(), "w");
        }
        if (!debug_message_fp) {
            std::cerr << "While trying to open log file \"" << deb_buf << "\": " << strerror(errno) <<
                "\t=> Using stderr as default.\n";
            /* use stderr by default. */
            debug_message_fp = stderr;
        }
    }

    try {
        deb_buf = fmt::vformat(fmt, args);
    }
    catch(const std::exception &e) {
        deb_buf = std::string("BUG! driver[debug_message_internal]: ") + e.what() + "\nFormatstring: \"" + fmt + "\"\n";
        std::cerr << deb_buf;
    }

    fputs(deb_buf.c_str(), debug_message_fp);
    fflush(debug_message_fp);

    if (debug_message_fp != stderr) {
        fputs(deb_buf.c_str(), stderr);
        fflush(stderr);
    }
}

int debug_level = 0;

#define NELEM(x) (sizeof(x) / sizeof(x[0]))

void debug_level_set(const std::string level) {
  unsigned int i;

  for (i = 0; i < NELEM(levels); i++) {
    if (level == levels[i].name) {
      debug_level |= levels[i].bit;
      return;
    }
  }
}

void debug_level_clear(const std::string level) {
  unsigned int i;

  for (i = 0; i < NELEM(levels); i++) {
    if (level == levels[i].name) {
      debug_level &= ~levels[i].bit;
      return;
    }
  }
}
