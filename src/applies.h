/*
 * applies.h
 *
 * Note: The QGET_* applies from parse.c are not included here.
 */

#ifndef APPLIES_H
#define APPLIES_H

/* the folowing must be the first character of __INIT */
#define APPLY___INIT_SPECIAL_CHAR           '#'
#define APPLY___INIT                        "#global_init#"
#define APPLY_AUTHOR_FILE                   "author_file"
#define APPLY_CATCH_TELL                    "catch_tell"
#define APPLY_CLEAN_UP                      "clean_up"
#define APPLY_COMPILE_OBJECT                "compile_object"
#define APPLY_CONNECT                       "connect"
#define APPLY_CRASH                         "crash"
#define APPLY_CREATE                        "create"
#define APPLY_CREATOR_FILE                  "creator_file"
#define APPLY_DOMAIN_FILE                   "domain_file"
#define APPLY_EPILOG                        "epilog"
#define APPLY_ERROR_HANDLER                 "error_handler"
#define APPLY_GET_BACKBONE_UID              "get_bb_uid"
#define APPLY_GET_ED_BUFFER_SAVE_FILE_NAME  "get_save_file_name"
#define APPLY_GET_ROOT_UID                  "get_root_uid"
#define APPLY_ID                            "id"
#define APPLY_INIT                          "init"
#define APPLY_LOGON                         "logon"
#define APPLY_LOG_ERROR                     "log_error"
#define APPLY_MAKE_PATH_ABSOLUTE            "make_path_absolute"
#define APPLY_FLAG                          "flag"
#define APPLY_MOVE                          "move_or_destruct"
#define APPLY_NET_DEAD                      "net_dead"
#define APPLY_OBJECT_NAME                   "object_name"
#define APPLY_PARSER_ERROR_MESSAGE	    "parser_error_message"
#define APPLY_PRELOAD                       "preload"
#define APPLY_PRIVS_FILE                    "privs_file"
#define APPLY_PROCESS_INPUT                 "process_input"
#define APPLY_RECEIVE_MESSAGE               "receive_message"
#define APPLY_RECEIVE_SNOOP                 "receive_snoop"
#define APPLY_RESET                         "reset"
#define APPLY_RETRIEVE_ED_SETUP             "retrieve_ed_setup"
#define APPLY_SAVE_ED_SETUP                 "save_ed_setup"
#define APPLY_SLOW_SHUTDOWN                 "slow_shutdown"
#define APPLY_TELNET_SUBOPTION              "telnet_suboption"
#define APPLY_TERMINAL_TYPE                 "terminal_type"
#define APPLY_VALID_ASM                     "valid_asm"
#define APPLY_VALID_BIND                    "valid_bind"
#define APPLY_VALID_COMPILE_TO_C	    "valid_compile_to_c"
#define APPLY_VALID_HIDE                    "valid_hide"
#define APPLY_VALID_LINK                    "valid_link"
#define APPLY_VALID_OBJECT                  "valid_object"
#define APPLY_VALID_OVERRIDE                "valid_override"
#define APPLY_VALID_READ                    "valid_read"
#define APPLY_VALID_SAVE_BINARY             "valid_save_binary"
#define APPLY_VALID_SETEUID                 "valid_seteuid"
#define APPLY_VALID_SHADOW                  "valid_shadow"
#define APPLY_VALID_SOCKET                  "valid_socket"
#define APPLY_VALID_WRITE                   "valid_write"
#define APPLY_WINDOW_SIZE                   "window_size"
#define APPLY_WRITE_PROMPT                  "write_prompt"

#endif				/* APPLIES_H */
