void nothing() {
}

void move(object ob) {
  move_object(ob);
}

void init() {
  add_action(sprintf("%O", this_object()), "nothing");
}

void do_tests() {
#ifdef __NO_ADD_ACTION__
    write("NO_ADD_ACTION is defined, test not ran.\n");
#else
#ifdef __NO_ENVIRONMENT__
    write("NO_ENVIRONMENT is defined, test not ran.\n");
#else
    object save_tp = this_player();
    object ob;
 
    // 1. Simply enable commands, nothing happens.
    // init() is only called when object moves.
    enable_commands();
    ASSERT_EQ(({}), commands());
    // default clearing out actions.
    disable_commands();
    ASSERT_EQ(({}), commands());

    // 2. Moving a object into inventory, will call init() and
    // that will add an action to this object.
    enable_commands();
    ob = new(__FILE__);
    ob->move(this_object());
    ASSERT_EQ(1, sizeof(commands()));

    // 3. Disabling commands and clearing actions
    disable_commands();
    ASSERT_EQ(({}), commands());

    // 4. Move object into inventory in disabling state will not add action
    destruct(ob);
    ob = new(__FILE__);
    ob->move(this_object());
    ASSERT_EQ(0, sizeof(commands()));

    // 5. Enable commands with 0 after fact don't add action (old behavior).
    enable_commands();
    ASSERT_EQ(0, sizeof(commands()));

    // 6. Enable commands with 1 after fact corectly add action.
    disable_commands();
    enable_commands(1);
    ASSERT_EQ(1, sizeof(commands()));

    destruct(ob);

    if (save_tp)
      evaluate(bind( (: enable_commands :), save_tp));

    destruct(this_object());
#endif
#endif
}
