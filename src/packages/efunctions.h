/*
	This file is automatically generated by make_func.
	do not make any manual changes to this file.
*/

#ifndef __AUTOGEN_efunctions_H
#define __AUTOGEN_efunctions_H


#include "efun_protos.h"


typedef void (*func_t) (void);

func_t efun_table[] = {
	f__call_other,
	f__evaluate,
	f__this_object,
	f__to_int,
	f__to_float,
	f__new,
	f_bind,
	f_this_player,
	f_previous_object,
	f_call_stack,
	f_sizeof,
	f_strwidth,
	f_destruct,
	f_file_name,
	f_capitalize,
	f_explode,
	f_implode,
	f_call_out,
	f_member_array,
	f_input_to,
	f_random,
	f_defer,
	f_environment,
	f_all_inventory,
	f_deep_inventory,
	f_first_inventory,
	f_next_inventory,
	f_say,
	f_tell_room,
	f_present,
	f_move_object,
	f_add_action,
	f_query_verb,
	f_command,
	f_remove_action,
	f_living,
	f_commands,
	f_disable_commands,
	f_enable_commands,
	f_set_living_name,
	f_livings,
	f_find_living,
	f_find_player,
	f_notify_fail,
	f_lower_case,
	f_replace_string,
	f_restore_object,
	f_save_object,
	f_save_variable,
	f_restore_variable,
	f_users,
	f_get_dir,
	f_strsrch,
	f_write,
	f_tell_object,
	f_shout,
	f_receive,
	f_message,
	f_find_object,
	f_find_call_out,
	f_allocate_mapping,
	f_values,
	f_keys,
	f_map_delete,
	f_match_path,
	f_clonep,
	f_intp,
	f_undefinedp,
	f_floatp,
	f_stringp,
	f_virtualp,
	f_functionp,
	f_pointerp,
	f_objectp,
	f_classp,
	f_typeof,
	f_bufferp,
	f_allocate_buffer,
	f_inherits,
	f_replace_program,
	f_regexp,
	f_reg_assoc,
	f_allocate,
	f_call_out_info,
	f_crc32,
	f_read_buffer,
	f_write_buffer,
	f_write_file,
	f_rename,
	f_write_bytes,
	f_file_size,
	f_read_bytes,
	f_read_file,
	f_cp,
	f_link,
	f_mkdir,
	f_rm,
	f_rmdir,
	f_clear_bit,
	f_test_bit,
	f_set_bit,
	f_next_bit,
	f_crypt,
	f_oldcrypt,
	f_ctime,
	f_exec,
	f_localtime,
	f_function_exists,
	f_objects,
	f_query_host_name,
	f_query_idle,
	f_query_ip_name,
	f_query_ip_number,
	f_snoop,
	f_query_snoop,
	f_query_snooping,
	f_remove_call_out,
	f_set_heart_beat,
	f_query_heart_beat,
	f_set_hide,
	f_set_reset,
	f_shadow,
	f_query_shadowing,
	f_sort_array,
	f_throw,
	f_time,
	f_unique_array,
	f_unique_mapping,
	f_deep_inherit_list,
	f_shallow_inherit_list,
	f_printf,
	f_sprintf,
	f_mapp,
	f_stat,
	f_interactive,
	f_has_mxp,
	f_has_zmp,
	f_send_zmp,
	f_has_gmcp,
	f_send_gmcp,
	f_in_edit,
	f_in_input,
	f_userp,
	f_enable_wizard,
	f_disable_wizard,
	f_wizardp,
	f_master,
	f_memory_info,
	f_get_config,
	f_get_char,
	f_children,
	f_reload_object,
	f_error,
	f_uptime,
	f_strcmp,
	f_rusage,
	f_flush_messages,
	f_ed,
	f_cache_stats,
	f_filter,
	f_map,
	f_malloc_status,
	f_mud_status,
	f_dumpallobj,
	f_dump_file_descriptors,
	f_query_load_average,
	f_origin,
	f_reclaim_objects,
	f_set_eval_limit,
	f_set_debug_level,
	f_debug_levels,
	f_clear_debug_level,
	f_resolve,
	f_set_encoding,
	f_to_utf8,
	f_utf8_to,
	f_str_to_arr,
	f_arr_to_str,
	f_act_mxp,
	f_websocket_handshake_done,
	f_request_term_type,
	f_start_request_term_type,
	f_request_term_size,
	f_shutdown,
	f_trim,
	f_ltrim,
	f_rtrim,
	f_export_uid,
	f_geteuid,
	f_getuid,
	f_seteuid,
	f_db_close,
	f_db_commit,
	f_db_connect,
	f_db_exec,
	f_db_fetch,
	f_db_rollback,
	f_db_status,
	f_hash,
	f_compress_file,
	f_uncompress_file,
	f_compress,
	f_uncompress,
	f_pcre_version,
	f_pcre_match,
	f_pcre_assoc,
	f_pcre_extract,
	f_pcre_replace,
	f_pcre_replace_callback,
	f_pcre_cache,
	f_cos,
	f_sin,
	f_tan,
	f_asin,
	f_acos,
	f_atan,
	f_sqrt,
	f_log,
	f_log10,
	f_log2,
	f_pow,
	f_exp,
	f_floor,
	f_ceil,
	f_round,
	f_norm,
	f_dotprod,
	f_distance,
	f_angle,
	f_debug_info,
	f_refs,
	f_destructed_objects,
	f_dump_prog,
	f_debugmalloc,
	f_set_malloc_mask,
	f_check_memory,
	f_traceprefix,
	f_trace,
	f_parse_init,
	f_parse_refresh,
	f_parse_sentence,
	f_parse_add_rule,
	f_parse_remove,
	f_parse_add_synonym,
	f_parse_dump,
	f_parse_my_rules,
	f_external_start,
	f_remove_shadow,
	f_query_notify_fail,
	f_named_livings,
	f_copy,
	f_functions,
	f_variables,
	f_heart_beats,
	f_terminal_colour,
	f_pluralize,
	f_file_length,
	f_upper_case,
	f_replaceable,
	f_program_info,
	f_store_variable,
	f_fetch_variable,
	f_remove_interactive,
	f_query_ip_port,
	f_zonetime,
	f_is_daylight_savings_time,
	f_debug_message,
	f_function_owner,
	f_repeat_string,
	f_memory_summary,
	f_query_replaced_program,
	f_network_stats,
	f_real_time,
	f_compressedp,
	f_event,
	f_query_num,
	f_base_name,
	f_get_garbage,
	f_num_classes,
	f_assemble_class,
	f_disassemble_class,
	f_fetch_class_member,
	f_store_class_member,
	f_shuffle,
	f_element_of,
	f_max,
	f_min,
	f_abs,
	f_string_difference,
	f_query_charmode,
	f_remove_charmode,
	f_remove_get_char,
	f_send_nullbyte,
	f_restore_from_string,
	f_classes,
	f_test_load,
	f_socket_create,
	f_socket_bind,
	f_socket_listen,
	f_socket_accept,
	f_socket_connect,
	f_socket_write,
	f_socket_close,
	f_socket_release,
	f_socket_acquire,
	f_socket_error,
	f_socket_address,
	f_socket_status,
	f_sha1,
	f_async_read,
	f_async_write,
	f_async_getdir,
	f_async_db_exec,
	f_domain_stats,
	f_set_author,
	f_author_stats,
};
#endif
