#define BS_LEAVE_MY_LFS         1
#define BS_SINGLE_SPACE         2
#define BS_BLOCK                4
#define BS_NO_PARINDENT         8
#define BS_INDENT_ONCE         16
#define BS_PREPEND_INDENT      32

void do_tests() {
  ASSERT_EQ("This is a longer text.\n"
            "Just as an example.\n",
            break_string("This is a longer text. Just as an example.", 27));

  ASSERT_EQ("Wargon says: It looks like\n"
            "Wargon says: this with indent\n",
            break_string("It looks like this with indent", 30, "Wargon says: "));

  ASSERT_EQ("Wargon says: It looks like\n"
            "             this with indent\n",
      break_string("It looks like this with indent", 30, "Wargon says: ", BS_INDENT_ONCE));

  ASSERT_EQ("  It looks like this with\n"
            "  spaces\n",
      break_string("It looks like this with spaces", 30, 2));

  ASSERT_EQ("Wargon says: I want it\n"
            "Wargon says: but pre-formatted!\n",
      break_string ("I want it\nbut pre-formatted!", 60,
          "Wargon says: ", BS_LEAVE_MY_LFS));

  ASSERT_EQ("Wargon says: \n"
            " I want it\n"
            " but pre-formatted!\n",
      break_string ("I want it\nbut pre-formatted!", 30,
          "Wargon says: ", BS_LEAVE_MY_LFS | BS_PREPEND_INDENT));
}
