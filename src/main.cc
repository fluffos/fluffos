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

time_t boot_time;

static void sig_cld(int /*sig*/);

static void sig_usr1(int /*sig*/);
static void sig_usr2(int /*sig*/);

static void attempt_shutdown(int sig);
static void setup_signal_handlers();

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

int get_is_shutdown() {
  return MudOS_is_being_shut_down;
}

void real_main(void* base) {
  backend(reinterpret_cast<event_base *>(base));
}

static void setup_signal_handlers() {
  signal(SIGFPE, attempt_shutdown);
  signal(SIGTERM, attempt_shutdown);
  // signal(SIGINT, attempt_shutdown);
  signal(SIGABRT, attempt_shutdown);
  signal(SIGBUS, attempt_shutdown);
  signal(SIGSEGV, attempt_shutdown);
  signal(SIGILL, attempt_shutdown);

  // User signal
  signal(SIGUSR1, sig_usr1);
  signal(SIGUSR2, sig_usr2);

  // shutdown
  signal(SIGHUP, startshutdownMudOS);

  // for external events?
  signal(SIGCHLD, sig_cld);

  /*
   * we use nonblocking socket, must ignore SIGPIPE.
   */
  if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
    debug_perror("can't ignore signal SIGPIPE", 0);
    exit(5);
  }
}

static void try_dump_stacktrace() {
#if !defined(__CYGWIN__) && __GNUC__ > 2
  static void *bt[100];
  size_t bt_size;
  bt_size = backtrace(bt, 100);
  backtrace_symbols_fd(bt, bt_size, STDERR_FILENO);
#else
  debug_message("Not able to generate backtrace, please use core.\n");
#endif
}

static void sig_cld(int sig) {
  /*FIXME: restore this
   int status;
   while (wait3(&status, WNOHANG, NULL) > 0) {
   ;
   }*/
}

/* send this signal when the machine is about to reboot.  The script
 which restarts the MUD should take an exit code of 1 to mean don't
 restart
 */
static void sig_usr1(int sig) {
  push_constant_string("Host machine shutting down");
  push_undefined();
  push_undefined();
  apply_master_ob(APPLY_CRASH, 3);
  debug_message("Received SIGUSR1, calling exit(-1)\n");
  exit(-1);
}

/* Abort evaluation */
static void sig_usr2(int sig) {
  debug_message("Received SIGUSR2, current eval aborted.\n");
  outoftime = 1;
}

/*
 * Actually, doing all this stuff from a signal is probably illegal
 * -Beek
 */
static void attempt_shutdown(int sig) {
  const char *msg = "Unkonwn signal!";
  switch (sig) {
    case SIGABRT:
      msg = "SIGABRT: Aborted";
      break;
    case SIGTERM:
      msg = "SIGTERM: Process terminated";
      break;
    case SIGINT:
      msg = "SIGINT: Process interrupted";
      break;
    case SIGSEGV:
      msg = "SIGSEGV: Segmentation fault";
      break;
    case SIGFPE:
      msg = "SIGFPE: Floating point exception";
      break;
    case SIGBUS:
      msg = "SIGBUS: Bus error";
      break;
    case SIGILL:
      msg = "SIGILL: Illegal instruction";
      break;
  }

  // Reverse all traps
  signal(SIGFPE, SIG_DFL);
  signal(SIGTERM, SIG_DFL);
  signal(SIGINT, SIG_DFL);
  signal(SIGABRT, SIG_DFL);
  signal(SIGBUS, SIG_DFL);
  signal(SIGSEGV, SIG_DFL);
  signal(SIGILL, SIG_DFL);

  try_dump_stacktrace();
  fatal(msg);
}

/*int main(int argc, char **argv) {
  return real_main(argc, argv);
}*/
