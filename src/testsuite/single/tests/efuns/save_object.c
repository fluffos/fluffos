int x = 0;
#ifndef __SENSIBLE_MODIFIERS__
static int z = 5;
#else
nosave int z = 5;
#endif
int y = 1;

void do_tests() {
    save_object("/sf");
    ASSERT(read_file("/sf.o") == "#" + __FILE__ + "\ny 1\n");
    save_object("/sf", 1);
    ASSERT(read_file("/sf.o") == "#" + __FILE__ + "\nx 0\ny 1\n");
}
