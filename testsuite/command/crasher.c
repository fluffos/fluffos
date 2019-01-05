// crahser, calling random efun with random argument hope to crash the driver.

nosave mixed *_efuns = ({
    ({ "abs", (: abs :) }),
    ({ "acos", (: acos :) }),
    ({ "act_mxp", (: act_mxp :) }),
    ({ "add_action", (: add_action :) }),
    ({ "all_inventory", (: all_inventory :) }),
    ({ "all_previous_objects", (: all_previous_objects :) }),
    ({ "allocate", (: allocate :) }),
    ({ "allocate_buffer", (: allocate_buffer :) }),
    ({ "allocate_mapping", (: allocate_mapping :) }),
    ({ "angle", (: angle :) }),
    ({ "arr_to_str", (: arr_to_str :) }),
    ({ "arrayp", (: arrayp :) }),
    ({ "asin", (: asin :) }),
    ({ "assemble_class", (: assemble_class :) }),
    ({ "async_read", (: async_read :) }),
    ({ "async_write", (: async_write :) }),
    ({ "atan", (: atan :) }),
    ({ "author_stats", (: author_stats :) }),
    ({ "base_name", (: base_name :) }),
    ({ "bind", (: bind :) }),
    ({ "bufferp", (: bufferp :) }),
    ({ "cache_stats", (: cache_stats :) }),
    ({ "call_other", (: call_other :) }),
    ({ "call_out", (: call_out :) }),
    ({ "call_out_info", (: call_out_info :) }),
    ({ "call_stack", (: call_stack :) }),
    ({ "capitalize", (: capitalize :) }),
    ({ "ceil", (: ceil :) }),
    ({ "children", (: children :) }),
    ({ "classes", (: classes :) }),
    ({ "classp", (: classp :) }),
    ({ "clear_bit", (: clear_bit :) }),
#ifdef HAS_DEBUG_LEVEL
    ({ "clear_debug_level", (: clear_debug_level :) }),
#endif
    ({ "clone_object", (: clone_object :) }),
    ({ "clonep", (: clonep :) }),
    ({ "command", (: command :) }),
    ({ "commands", (: commands :) }),
    ({ "compress", (: compress :) }),
    ({ "compress_file", (: compress_file :) }),
    ({ "compressedp", (: compressedp :) }),
    ({ "copy", (: copy :) }),
    ({ "cos", (: cos :) }),
    ({ "cp", (: cp :) }),
    ({ "crc32", (: crc32 :) }),
    ({ "crypt", (: crypt :) }),
    ({ "ctime", (: ctime :) }),
    ({ "debug_info", (: debug_info :) }),
#ifdef HAS_DEBUG_LEVEL
    ({ "debug_levels", (: debug_levels :) }),
#endif
    ({ "debug_message", (: debug_message :) }),
    ({ "deep_inherit_list", (: deep_inherit_list :) }),
    ({ "deep_inventory", (: deep_inventory :) }),
    ({ "defer", (: defer :) }),
    ({ "destruct", (: destruct :) }),
#ifdef __PACKAGE_DEVELOP__
    ({ "destructed_objects", (: destructed_objects :) }),
#endif
    //    ({ "disable_commands", (: disable_commands :) }),
    ({ "disable_wizard", (: disable_wizard :) }),
    ({ "disassemble_class", (: disassemble_class :) }),
    ({ "distance", (: distance :) }),
    ({ "domain_stats", (: domain_stats :) }),
    ({ "dotprod", (: dotprod :) }),
    ({ "dump_file_descriptors", (: dump_file_descriptors :) }),
    ({ "dump_prog", (: dump_prog :) }),
    ({ "dumpallobj", (: dumpallobj :) }),
    ({ "ed", (: ed :) }),
    ({ "element_of", (: element_of :) }),
    //    ({ "enable_commands", (: enable_commands :) }),
    ({ "enable_wizard", (: enable_wizard :) }),
    ({ "environment", (: environment :) }),
    ({ "error", (: error :) }),
    ({ "eval_cost", (: eval_cost :) }),
    ({ "evaluate", (: evaluate :) }),
    ({ "event", (: event :) }),
    ({ "exec", (: exec :) }),
    ({ "exp", (: exp :) }),
    ({ "explode", (: explode :) }),
    ({ "export_uid", (: export_uid :) }),
    ({ "external_start", (: external_start :) }),
    ({ "fetch_class_member", (: fetch_class_member :) }),
    ({ "fetch_variable", (: fetch_variable :) }),
    ({ "file_length", (: file_length :) }),
    ({ "file_name", (: file_name :) }),
    ({ "file_size", (: file_size :) }),
    ({ "filter", (: filter :) }),
    ({ "filter_array", (: filter_array :) }),
    ({ "filter_mapping", (: filter_mapping :) }),
    ({ "find_call_out", (: find_call_out :) }),
    ({ "find_living", (: find_living :) }),
    ({ "find_object", (: find_object :) }),
    ({ "find_player", (: find_player :) }),
    ({ "first_inventory", (: first_inventory :) }),
    ({ "floatp", (: floatp :) }),
    ({ "floor", (: floor :) }),
    ({ "flush_messages", (: flush_messages :) }),
    ({ "function_exists", (: function_exists :) }),
    ({ "function_owner", (: function_owner :) }),
    ({ "functionp", (: functionp :) }),
    ({ "functions", (: functions :) }),
    ({ "get_char", (: get_char :) }),
    ({ "get_config", (: get_config :) }),
    ({ "get_dir", (: get_dir :) }),
    ({ "get_garbage", (: get_garbage :) }),
    ({ "geteuid", (: geteuid :) }),
    ({ "getuid", (: getuid :) }),
    ({ "has_gmcp", (: has_gmcp :) }),
    ({ "has_mxp", (: has_mxp :) }),
    ({ "has_zmp", (: has_zmp :) }),
    ({ "heart_beats", (: heart_beats :) }),
#ifdef __PACKAGE_MATRIX__
    ({ "id_matrix", (: id_matrix :) }),
#endif
    ({ "implode", (: implode :) }),
    ({ "in_edit", (: in_edit :) }),
    ({ "in_input", (: in_input :) }),
    ({ "inherit_list", (: inherit_list :) }),
    ({ "inherits", (: inherits :) }),
    ({ "input_to", (: input_to :) }),
    ({ "interactive", (: interactive :) }),
    ({ "intp", (: intp :) }),
    ({ "is_daylight_savings_time", (: is_daylight_savings_time :) }),
    ({ "keys", (: keys :) }),
    ({ "link", (: link :) }),
    ({ "living", (: living :) }),
    ({ "livings", (: livings :) }),
    ({ "load_object", (: load_object :) }),
    ({ "localtime", (: localtime :) }),
    ({ "log", (: log :) }),
    ({ "log10", (: log10 :) }),
    ({ "log2", (: log2 :) }),
    ({ "lookat_rotate", (: lookat_rotate :) }),
    ({ "lower_case", (: lower_case :) }),
    ({ "malloc_status", (: malloc_status :) }),
    ({ "map", (: map :) }),
    ({ "map_array", (: map_array :) }),
    ({ "map_delete", (: map_delete :) }),
    ({ "map_mapping", (: map_mapping :) }),
    ({ "mapp", (: mapp :) }),
    ({ "master", (: master :) }),
    ({ "match_path", (: match_path :) }),
    ({ "max", (: max :) }),
    ({ "max_eval_cost", (: max_eval_cost :) }),
    ({ "member_array", (: member_array :) }),
    ({ "memory_info", (: memory_info :) }),
    ({ "memory_summary", (: memory_summary :) }),
    ({ "message", (: message :) }),
    ({ "min", (: min :) }),
    ({ "mkdir", (: mkdir :) }),
    ({ "move_object", (: move_object :) }),
    ({ "mud_status", (: mud_status :) }),
    ({ "named_livings", (: named_livings :) }),
    ({ "network_stats", (: network_stats :) }),
    ({ "next_bit", (: next_bit :) }),
    ({ "next_inventory", (: next_inventory :) }),
    ({ "norm", (: norm :) }),
    ({ "notify_fail", (: notify_fail :) }),
    ({ "nullp", (: nullp :) }),
    ({ "num_classes", (: num_classes :) }),
    ({ "objectp", (: objectp :) }),
    ({ "objects", (: objects :) }),
    ({ "oldcrypt", (: oldcrypt :) }),
    ({ "origin", (: origin :) }),
    ({ "pluralize", (: pluralize :) }),
    ({ "pointerp", (: pointerp :) }),
    ({ "pow", (: pow :) }),
    ({ "present", (: present :) }),
    ({ "previous_object", (: previous_object :) }),
    ({ "printf", (: printf :) }),
    ({ "program_info", (: program_info :) }),
    ({ "query_charmode", (: query_charmode :) }),
    ({ "query_heart_beat", (: query_heart_beat :) }),
    ({ "query_host_name", (: query_host_name :) }),
    ({ "query_idle", (: query_idle :) }),
    ({ "query_ip_name", (: query_ip_name :) }),
    ({ "query_ip_number", (: query_ip_number :) }),
    ({ "query_ip_port", (: query_ip_port :) }),
    ({ "query_load_average", (: query_load_average :) }),
    ({ "query_notify_fail", (: query_notify_fail :) }),
    ({ "query_num", (: query_num :) }),
    ({ "query_replaced_program", (: query_replaced_program :) }),
    ({ "query_shadowing", (: query_shadowing :) }),
    ({ "query_snoop", (: query_snoop :) }),
    ({ "query_snooping", (: query_snooping :) }),
    ({ "query_verb", (: query_verb :) }),
    ({ "random", (: random :) }),
    ({ "read_buffer", (: read_buffer :) }),
    ({ "read_bytes", (: read_bytes :) }),
    ({ "read_file", (: read_file :) }),
    ({ "real_time", (: real_time :) }),
    ({ "receive", (: receive :) }),
    ({ "reclaim_objects", (: reclaim_objects :) }),
    ({ "refs", (: refs :) }),
    ({ "reg_assoc", (: reg_assoc :) }),
    ({ "regexp", (: regexp :) }),
    ({ "reload_object", (: reload_object :) }),
    ({ "remove_action", (: remove_action :) }),
    ({ "remove_call_out", (: remove_call_out :) }),
    ({ "remove_charmode", (: remove_charmode :) }),
    ({ "remove_get_char", (: remove_get_char :) }),
    ({ "remove_interactive", (: remove_interactive :) }),
    ({ "remove_shadow", (: remove_shadow :) }),
    ({ "rename", (: rename :) }),
    ({ "repeat_string", (: repeat_string :) }),
    ({ "replace_program", (: replace_program :) }),
    ({ "replace_string", (: replace_string :) }),
    ({ "replaceable", (: replaceable :) }),
    ({ "request_term_size", (: request_term_size :) }),
    ({ "request_term_type", (: request_term_type :) }),
    ({ "reset_eval_cost", (: reset_eval_cost :) }),
    ({ "resolve", (: resolve :) }),
    ({ "restore_from_string", (: restore_from_string :) }),
    ({ "restore_object", (: restore_object :) }),
    ({ "restore_variable", (: restore_variable :) }),
    ({ "rm", (: rm :) }),
    ({ "rmdir", (: rmdir :) }),
#ifdef __PACKAGE_MATRIX__
    ({ "rotate_x", (: rotate_x :) }),
    ({ "rotate_y", (: rotate_y :) }),
    ({ "rotate_z", (: rotate_z :) }),
#endif
    ({ "round", (: round :) }),
    ({ "rusage", (: rusage :) }),
    ({ "save_object", (: save_object :) }),
    ({ "save_variable", (: save_variable :) }),
    ({ "say", (: say :) }),
    ({ "scale", (: scale :) }),
    ({ "send_gmcp", (: send_gmcp :) }),
    ({ "send_nullbyte", (: send_nullbyte :) }),
    ({ "send_zmp", (: send_zmp :) }),
    ({ "set_author", (: set_author :) }),
    ({ "set_bit", (: set_bit :) }),
    ({ "set_debug_level", (: set_debug_level :) }),
    ({ "set_encoding", (: set_encoding :) }),
    ({ "set_eval_limit", (: set_eval_limit :) }),
    ({ "set_heart_beat", (: set_heart_beat :) }),
    ({ "set_hide", (: set_hide :) }),
    ({ "set_living_name", (: set_living_name :) }),
    ({ "set_reset", (: set_reset :) }),
    ({ "seteuid", (: seteuid :) }),
    ({ "shadow", (: shadow :) }),
    ({ "shallow_inherit_list", (: shallow_inherit_list :) }),
    ({ "shout", (: shout :) }),
    ({ "shuffle", (: shuffle :) }),
    //    ({ "shutdown", (: shutdown :) }),
    ({ "sin", (: sin :) }),
    ({ "sizeof", (: sizeof :) }),
    ({ "snoop", (: snoop :) }),
    ({ "socket_accept", (: socket_accept :) }),
    ({ "socket_acquire", (: socket_acquire :) }),
    ({ "socket_address", (: socket_address :) }),
    ({ "socket_bind", (: socket_bind :) }),
    ({ "socket_close", (: socket_close :) }),
    ({ "socket_connect", (: socket_connect :) }),
    ({ "socket_create", (: socket_create :) }),
    ({ "socket_error", (: socket_error :) }),
    ({ "socket_listen", (: socket_listen :) }),
    ({ "socket_release", (: socket_release :) }),
    ({ "socket_status", (: socket_status :) }),
    ({ "socket_write", (: socket_write :) }),
    ({ "sort_array", (: sort_array :) }),
    ({ "sprintf", (: sprintf :) }),
    ({ "sqrt", (: sqrt :) }),
    ({ "start_request_term_type", (: start_request_term_type :) }),
    ({ "stat", (: stat :) }),
    ({ "store_class_member", (: store_class_member :) }),
    ({ "store_variable", (: store_variable :) }),
    ({ "str_to_arr", (: str_to_arr :) }),
    ({ "strcmp", (: strcmp :) }),
    ({ "string_difference", (: string_difference :) }),
    ({ "stringp", (: stringp :) }),
    ({ "strlen", (: strlen :) }),
    ({ "strsrch", (: strsrch :) }),
    ({ "strwidth", (: strwidth :) }),
    ({ "tan", (: tan :) }),
    ({ "tell_object", (: tell_object :) }),
    ({ "tell_room", (: tell_room :) }),
    ({ "terminal_colour", (: terminal_colour :) }),
    ({ "test_bit", (: test_bit :) }),
    ({ "this_interactive", (: this_interactive :) }),
    ({ "this_object", (: this_object :) }),
    ({ "this_player", (: this_player :) }),
    ({ "this_user", (: this_user :) }),
    ({ "throw", (: throw :) }),
    ({ "time", (: time :) }),
    ({ "to_float", (: to_float :) }),
    ({ "to_int", (: to_int :) }),
    ({ "to_utf8", (: to_utf8 :) }),
    ({ "trace", (: trace :) }),
    ({ "traceprefix", (: traceprefix :) }),
    ({ "translate", (: translate :) }),
    ({ "typeof", (: typeof :) }),
    ({ "uncompress", (: uncompress :) }),
    ({ "uncompress_file", (: uncompress_file :) }),
    ({ "undefinedp", (: undefinedp :) }),
    ({ "unique_array", (: unique_array :) }),
    ({ "unique_mapping", (: unique_mapping :) }),
    ({ "upper_case", (: upper_case :) }),
    ({ "uptime", (: uptime :) }),
    ({ "userp", (: userp :) }),
    ({ "users", (: users :) }),
    ({ "utf8_to", (: utf8_to :) }),
    ({ "values", (: values :) }),
    ({ "variables", (: variables :) }),
    ({ "virtualp", (: virtualp :) }),
    ({ "wizardp", (: wizardp :) }),
    ({ "write", (: write :) }),
    ({ "write_buffer", (: write_buffer :) }),
    ({ "write_bytes", (: write_bytes :) }),
    ({ "write_file", (: write_file :) }),
    ({ "zonetime", (: zonetime :) }),

    // operators
    ({ "+", (: $1 + $2 :) }),
    ({ "-", (: $1 - $2 :) }),
    ({ "*", (: $1 * $2 :) }),
    ({ "/", (: $1 / $2 :) }),
    ({ "^", (: $1 ^ $2 :) }),
    ({ "^", (: $1 ^ $2 :) }),

    ({ ">", (: $1 > $2 :) }),
    ({ "<", (: $1 < $2 :) }),
    ({ ">=", (: $1 >= $2 :) }),
    ({ "<=", (: $1 <= $2 :) }),
    ({ "==", (: $1 == $2 :) }),

    ({ "&&", (: $1 && $2 :) }),
    ({ "||", (: $1 || $2 :) }),
    ({ "!", (: !($1) :) }),
    ({ "~", (: ~($1) :) }),

    ({ ">>", (: $1 >> $2 :) }),
    ({ "<<", (: $1 << $2 :) }),

    ({ "?", (: $1 ? 0 : 1 :) }),
});

nosave mixed *_values = ({
    /* --- Integers: --- */
    0,
    1,
    -1,
    32,
    -32,
    40,
    -40,
    29292929,
    -5050211,
    __INT_MAX__,
    __INT_MIN__,
    /* --- Floats: --- */
    0.0,
    0.5,
    -0.5,
    55.5,
    -55.5,
    999999999999999999999999999999999999999999999.99,
    -999999999999999999999999999999999999999999999.99,
    /* --- Strings: --- */
    "",
    "foo bar",
    "%d %q %T",
    "0",
    "",
    " ",
    "_",
    "^^^^^^",
    "#^#         #^#",
    "                ^",
    "^                      ",
    "  -   -   -                    - ",
    "? ?    ?    ?  ? ?  ?????   ????  ??",
    "°°°°°°°°°°°°°°°°°°°°°",
    "\\/ ",
    "    !",

    /* --- Objekte: --- */
    clone_object("/clone/login.c"),
    this_object(),
    previous_object(),
    find_object("/single/master.c"),

    /* --- Closures: --- */
    (: 0 :),
    (: (: 1 :) :),
    (: sizeof :),
    (: efun::input_to :),
    (: error("123") :),
    (: clone_object("/clone/login.c") :),

    /* --- Arrays: --- */
    ({ }),
    ({ 0 }),
    ({ "" }),
    ({ ({ }) }),
    ({ ([ ]) }),
    ({ (: 1 :) }),
    (: clone_object("/clone/login.c") :),

    //allocate(3000),

    /* --- Mappings: --- */
    ([ ]),
    ([ 1 : (: 1 :) ]),
    ([ clone_object("/clone/login.c"): (: sizeof :) ]),

    //allocate_mapping(200),
});

int main(string arg) {
  mixed ob;

  write("Ok, let's try to crash the driver...\n");

  ob = clone_object(this_object());
  ob->move(this_player());
}

mixed do_eval(mixed func, mixed* args) {
  switch(sizeof(args)) {
    case 0:
      return evaluate(func);
    case 1:
      return evaluate(func, args[0]);
    case 2:
      return evaluate(func, args[0], args[1]);
    case 3:
      return evaluate(func, args[0], args[1], args[2]);
    case 4:
      return evaluate(func, args[0], args[1], args[2], args[3]);
  }
}

void create() {
  mixed *args = ({ });
  mixed func, arg, result;
  string func_name, log;
  int i,j;

  set_living_name("crasher");

  for(i=0; i<50; i++) {

    args = ({ });
    for(j=0; j<4; j++) {
      if(random(2)) {
        if(random(5)) {
          args += ({ _values[random(sizeof(_values))] });
        } else {
          args += ({ _efuns[random(sizeof(_efuns))][1] });
        }
      }
    }

    j = random(sizeof(_efuns));
    func_name = _efuns[j][0];
    func = _efuns[j][1];

    log = sprintf("calling %s with ({ \n", func_name);
    foreach( arg in args) {
      log += sprintf("  %s: %O \n", typeof(arg), arg);
    }

    log += "}) \n";

    write_file("/log/crasher", log);

    catch(write("test... " + i + " " + log));

    set_eval_limit(0x7fffffff);

    catch(do_eval(func, args));
  }

  call_out("create", 1);
}
