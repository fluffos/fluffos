// FIXME: This efun is undocumented.
void do_tests() {
    mixed stack, entry;
    int n = sizeof(call_stack(0));
    
    stack = call_stack(0);
    ASSERT(stack[0] == __FILE__);
    ASSERT(sizeof(filter(stack, (: stringp :))) == n);
    stack = call_stack(1);
    ASSERT(stack[0] == this_object());
    ASSERT(sizeof(filter(stack, (: objectp :))) == n);
    stack = call_stack(2);
    ASSERT(stack[0] == "do_tests");
    ASSERT(sizeof(filter(stack, (: stringp :))) == n);
    stack = call_stack(3);
    ASSERT(sizeof(filter(stack, (: stringp :))) == n);

    // returns filename:linenumber
    stack = call_stack(4);
    ASSERT_EQ(1, arrayp(stack));
    ASSERT(sizeof(filter(stack, (: stringp :))) == n);

    ASSERT(catch(call_stack(5)));
    ASSERT(catch(call_stack(-100)));
}

	
