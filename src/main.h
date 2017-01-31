#ifndef MAIN_H
#define MAIN_H

/*
 * main.c
 */

#include <time.h>  // for time_t

// TODO: These should all be deleted, or moved away soon.
extern time_t boot_time;

#ifdef __cplusplus
extern "C" {
#endif
  void init_env();
  void init_md();
  void init_config(const char* file);
  void print_version_and_time();
  const char* get_mudlib_dir();
  int get_is_shutdown();
  void* init_libevent();
  void init_vm();
  void call_master_flag(const char * flag);
  void real_main(void*);
#ifdef __cplusplus
}
#endif

#endif
