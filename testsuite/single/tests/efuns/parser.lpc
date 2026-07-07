string* parse_command_id_list() {
  return ({ "bag" });
}

void create() {
  parse_init();

  if(clonep()) {
    move_object(__FILE__);
  }
}

can_look_wrd_obj() {
  return 1;
}

direct_look_wrd_obj() {
  return 1;
}

do_look_wrd_obj() {
  return 0;
}

void do_tests() {
#ifndef __PACKAGE_PARSER__
  write("no package parser, skipped.\n");
  return;
#else
  clone_object(__FILE__);
  write(sprintf("%O", all_inventory()) + "\n");
  parse_add_rule("look", "WRD OBJ");
  write(parse_dump() + "\n");
  ASSERT_EQ(1, parse_sentence("look in bag", 2, all_inventory()));
#endif
}
