inherit "/std/object";

void setup() {
  set_short("red rose");
  set_name("rose");
  add_alias("note");
  add_plural("notes");
  add_adjective("red");
  add_adjective("lovely");
  set_long(
    "A small perfectly made rose.  There is a small note attched to the stalk.\n");
  set_read_mess("\"Happy birthday Eskarina!\"\n");
}

mixed do_read() {
  ::do_read();
  return "note";
}
