int x = 0;
#ifndef __SENSIBLE_MODIFIERS__
static int z = 5;
#else
nosave int z = 5;
#endif
int y = 0x7fffffffffffffff;

void do_tests() {
    ASSERT_EQ(24, save_object("/sf"));
    ASSERT_EQ(read_file("/sf.o") , "#" + __FILE__ + "\ny " + MAX_INT + "\n");
    save_object("/sf", 1);
    ASSERT_EQ(read_file("/sf.o"),  "#" + __FILE__ + "\nx 0\ny " + MAX_INT + "\n");

    // Fluffos new behavior.
    ASSERT_EQ(save_object(0), "#" + __FILE__ + "\ny " + MAX_INT + "\n");
    ASSERT_EQ(save_object(1), "#" + __FILE__ + "\nx 0\ny " + MAX_INT + "\n");
}
