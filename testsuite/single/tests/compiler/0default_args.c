inherit __DIR__"0default_args_1.c";

int id (string id: (: "" :)) {
  return member_array(id, query_id()) > -1;
}

void do_tests() {
  object ob = new(__FILE__);
  ASSERT_EQ(0, ob->id());
}
