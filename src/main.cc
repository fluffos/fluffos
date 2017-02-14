#include "base/std.h"

#include "main.h"

#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif
#include <iostream>  // for cout
#include <locale.h>  // for setlocale, LC_ALL
#include <signal.h>  // for signal, SIG_DFL, SIGABRT, etc
#include <stddef.h>  // for size_t
#include <stdio.h>   // for fprintf, stderr, printf, etc
#include <stdlib.h>  // for exit
#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>  // for getrlimit
#endif
#include <time.h>
#include <unistd.h>

#include "backend.h"  // for backend, init_backend
#include "cc.h"       // for SOURCE_REVISION
#include "comm.h"     // for init_user_conn
#include "console.h"  // for console_init, has_console
#if USE_JEMALLOC == 1
#include "thirdparty/jemalloc/include/jemalloc/jemalloc.h"  // for mallctl
#endif
#include "vm/vm.h"  // for push_constant_string, etc

#include "packages/core/dns.h"  // for init_dns_event_base.
#include "comm.h"

time_t boot_time;

void print_version_and_time() {
  /* Print FluffOS version */
  std::cout << "FluffOS Version: " << PACKAGE_VERSION << "(" << SOURCE_REVISION << ")"
            << "@ (" << ARCH << ")" << std::endl;

#if USE_JEMALLOC == 1
  /* Print jemalloc version */
  {
    const char *ver;
    size_t resultlen = sizeof(ver);
    mallctl("version", &ver, &resultlen, NULL, 0);
    std::cout << "Jemalloc Version: " << ver << std::endl;
  }
#endif
}

void init_env() {
  setlocale(LC_ALL, "C");
  tzset();
  boot_time = get_current_time();
}

void init_md() {
#ifdef DEBUGMALLOC
  MDinit();
#endif
}

void init_config(const char* file) {
  read_config(file);
}

const char* get_mudlib_dir() {
  return CONFIG_STR(__MUD_LIB_DIR__);
}

int get_gametick_ms() {
    return CONFIG_INT(__RC_GAMETICK_MSEC__);
}

void* init_libevent() {
  // Initialize libevent, This should be done before executing LPC.
  auto base = init_backend();
  init_dns_event_base(base);
  return base;
}

void init_vm() {
  // Initialize VM layer
  vm_init();
}

void call_master_flag(const char * flag) {
  debug_message("Calling master::flag(\"%s\")...\n", flag);
  push_constant_string(flag);
  safe_apply_master_ob(APPLY_FLAG, 1);
}

void init_backend(void* base) {
  backend(reinterpret_cast<event_base *>(base));
}

void wrap_backend_once() {
  backend_once();
}

int is_no_ansi_and_strip() {
  return (CONFIG_INT(__RC_NO_ANSI__) && CONFIG_INT(__RC_STRIP_BEFORE_PROCESS_INPUT__));
}

int is_single_char(interactive_t* ip) {
  return (ip->iflags & SINGLE_CHAR) != 0;
}

// On Connection Errors.
void on_conn_error(struct interactive_t* ip) {
  ip->iflags |= NET_DEAD;
  remove_interactive(ip->ob, 0);
}

void wrap_on_user_command(struct interactive_t* ip, char* command) {
  on_user_command(ip, command);
}

void wrap_on_binary_data(struct interactive_t* ip, const char* buf, int len) {
    on_binary_data(ip, buf, len);
}
void wrap_on_ascii_data(struct interactive_t* ip, const char* buf) {
    on_ascii_data(ip, buf);
}
void wrap_on_mud_data(struct interactive_t* ip, char* buf) {
    on_mud_data(ip, buf);
}

struct new_user_result_t wrap_new_user_handler(int idx, int connIdx, char *hostport) {
    struct new_user_result_t res;
    res.user = new_user_handler(idx, connIdx, hostport);
    if (res.user) res.telnet = res.user->telnet;
    return res;
}

void wrap_call_crash() {
  push_constant_string("Host machine shutting down");
  push_undefined();
  push_undefined();
  apply_master_ob(APPLY_CRASH, 3);
  debug_message("Received SIGUSR1, calling exit(-1)\n");
}

void shutdownMudOS(int exit_code); // in simulate.cc
void wrap_shutdownMudOS(int exitCode) {
    shutdownMudOS(exitCode);
}

void wrap_call_fatal(const char *msg) {
    fatal(msg);
}

const char* get_mud_ip() {
    return CONFIG_STR(__MUD_IP__);
}

extern volatile int outoftime; // in interpret.cc
void wrap_set_outoftime(int i) {
    outoftime = i;
}