// Test bitmap_font sefun

void do_tests() {
  if(find_object("/std/bitmap_font")) {
    ASSERT(bitmap_font("??"));
    ASSERT(bitmap_font("Abc"));
    ASSERT(bitmap_font("??Abc"));
    ASSERT(bitmap_font("??Abc", 16));
    ASSERT(bitmap_font("??Abc", 16, "88", "  "));
    // TODO: add more test cases
    // ASSERT(bitmap_font("??Abc", 16, "88", "  ", HIR, CYN));
  }
}
