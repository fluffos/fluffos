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
  void init_backend(void*);
  const char* get_mud_ip();

  void wrap_backend_once();
  int get_gametick_ms();

  void wrap_call_crash();
  void wrap_shutdownMudOS(int);
  void wrap_call_fatal(const char*);

  struct new_user_result_t {
      struct interactive_t* user;
      struct telnet_t* telnet;
  };

  struct new_user_result_t wrap_new_user_handler(int, int, char*);
  void wrap_on_user_command(struct interactive_t *, char*); // in comm.cc
  void wrap_on_binary_data(struct interactive_t* ip, const char* buf, int);
  void wrap_on_ascii_data(struct interactive_t* ip, const char* buf);
  void wrap_on_mud_data(struct interactive_t* ip, char* buf);

  int is_no_ansi_and_strip(); // in comm.cc
  int is_single_char(struct interactive_t*); // in comm.cc
  void on_conn_error(struct interactive_t*); // in comm.cc
  void wrap_set_outoftime(int);

#ifdef __cplusplus
}
#endif


#endif
