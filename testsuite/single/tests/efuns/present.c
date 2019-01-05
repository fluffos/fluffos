string id;
object o1, o2, o3, o4, o5;

int id(string name) { return name == id; }

void hide() {
  set_hide(1);
}

void move(object ob) {
  move_object(ob);
}

void create(string arg) {
#ifndef __NO_ENVIRONMENT__
  id = arg;
  if (!arg) {
    o1 = new(__FILE__, "foo");
    o2 = new(__FILE__, "foo");
    o3 = new(__FILE__, "foo1");

    o1->move(this_object());
    o2->move(this_object());
    o3->move(this_object());

    o4 = new(__FILE__, "foo2");
    o4->move(previous_object());
    move_object(previous_object());

    o5 = new(__FILE__, "foo5");
    o5->move(this_object());
    o5->hide();
  }
#endif
}

void do_tests() {
#ifndef __NO_ENVIRONMENT__
  // if first argument is string, second argument is 0/missing,
  // searches for a object that id(name) == 1 in this object inventory
  // then this object's environment's inventory. Returns the found object.
  ASSERT_EQ(o4, present("foo2"));

  // if first argument is string, second argument is object,
  // only searches for traget ion that object's inventory.
  ASSERT_EQ(o4, present("foo2", environment(this_object())));
  ASSERT_EQ(0, present("foo2", this_object()));

  // the first in the inventory is the last object that moved in.
  ASSERT_EQ(o2, present("foo", this_object()));
  ASSERT_EQ(o3, present("foo1", this_object()));

  // no such id
  ASSERT_EQ(0, present("foobar"));

  // "foo 1" means the first "foo" in the inventory.
  ASSERT_EQ(o2, present("foo 1", this_object()));
  ASSERT_EQ(o1, present("foo 2", this_object()));
  ASSERT_EQ(0, present("foo 10", this_object()));

  // not ended with number, so treated as id="foo 10 "
  ASSERT_EQ(0, present("foo 10 ", this_object()));

  // ended with a digit, but second part is not only digits.
  // still use full string to search.
  ASSERT_EQ(0, present("foo a1", this_object()));
  ASSERT_EQ(0, present("foo 1a1", this_object()));

  // If object is hidden (via set_hide()), and current object is not hidable.
  // returns 0. In test case everything is hideable, so we can still find
  // this object regardless.
  // TODO: add a test for above case.
  ASSERT_EQ(o5, present("foo5", this_object()));

  // if first argument is object, second argument is 0/missing
  // check if object is in this object's inventory, or as a sibling in
  // this object's evnironemnt's inventory, returns object's parent.
  // FIXME: this looks like a mis-feature.
  ASSERT_EQ(this_object(), present(o2)); // direct children
  ASSERT_EQ(previous_object(), present(o4)); // sibiling

  // if first argument is object, second argument is object,
  // check if object1 is in object2 inventory, return object1 if true.
  ASSERT_EQ(o1, present(o1, this_object()));
  ASSERT_EQ(o2, present(o2, this_object()));
#endif
}
