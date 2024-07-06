void do_tests() {
  object ob, *all;
  string path;

  // Loading an object and making sure it is correctly loaded
  path = base_name() + "-aux.c";
  ob = load_object(path);
  all = children(path);
  ASSERT_EQ(sizeof(all), 1);

  // Using test_load and making sure children() is not corrupted
  test_load(path);
  all = children(path);
  ASSERT_EQ(sizeof(all), 1);

  // Destroying an object and doubling down on the children() check
  destruct(ob);
  all = children(path);
  ASSERT_EQ(sizeof(all), 0);

  // Loading it again to finally check that's the case
  load_object(path);
  all = children(path);
  ASSERT_EQ(sizeof(all), 1);

  // Make sure no lingering orphaned entries in the children list
  ASSERT_EQ(member_array(0, all), -1);
}
