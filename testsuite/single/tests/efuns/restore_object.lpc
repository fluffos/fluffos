int var1;
#ifndef __SENSIBLE_MODIFIERS__
static int var2;
#else
nosave int var2;
#endif
int var3;
int var4;

void setup() {
    var1 = 1;
    var2 = 2;
    var3 = 3;
    var4 = 4;
}

void do_tests() {
    write_file("/sf.o", "#empty\n", 1);
    setup();
    restore_object("/sf");
    ASSERT(!var1);
    ASSERT(var2 == 2);
    ASSERT(!var3);
    ASSERT(!var4);

    setup();
    restore_object("/sf", 1);
    ASSERT(var1 == 1);
    ASSERT(var2 == 2);
    ASSERT(var3 == 3);
    ASSERT(var4 == 4);

    write_file("/sf.o", "var1 2\nvar2 4\nvar3 6\nvar4 8\n", 1);
    setup();
    restore_object("/sf", 1);
    ASSERT(var1 == 2);
    ASSERT(var2 == 2);
    ASSERT(var3 == 6);
    ASSERT(var4 == 8);

    write_file("/sf.o", "var4 1\nvar2 9\nvar1 3\nvar3 4\n", 1);
    setup();
    restore_object("/sf", 1);
    ASSERT(var1 == 3);
    ASSERT(var2 == 2);
    ASSERT(var3 == 4);
    ASSERT(var4 == 1);
}

