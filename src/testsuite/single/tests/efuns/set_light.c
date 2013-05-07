int light = 0;

#define CHK ASSERT(light = set_light(0));

#ifndef __NO_ENVIRONMENT__
void move_or_destruct(object ob) {
    move_object(new("/single/void"));
}
#endif

void do_tests() {
    object ob;
    
#ifndef __NO_LIGHT__
    CHK;
    set_light(2);
    light += 2;
    CHK;

#ifndef __NO_ENVIRONMENT__
    ob = new(__DIR__ "light");
    ob->move(this_object());
    light += 3;
    CHK;

    destruct(ob);
    light -= 3;
    CHK;
    
    ob = new(__DIR__ "light");
    move_object(ob);
    light += 3;
    CHK;
    
    destruct(ob);
    light -= 3;
    CHK;
#endif    
#endif
}
