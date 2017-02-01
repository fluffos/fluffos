#ifndef MAIN_H
#define MAIN_H

/*
 * main.c
 */

#ifdef __cplusplus
extern "C" {
#endif
  void init_env();
  void init_md();
  void init_config(const char*);
  void print_version_and_time();
  const char* get_mudlib_dir();
  int get_is_shutdown();
  void* init_libevent();
  void init_vm();
  void call_master_flag(const char *);
  void real_main(void*);
  const char* get_mud_ip();

  void wrap_backend_once();
  int get_gametick_ms();

  void wrap_new_user_handler(int, int, char*, int);



#ifdef __cplusplus
}
#endif


#endif
