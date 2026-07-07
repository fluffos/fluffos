// Pins #include with trailing comments (real-mud ftpdconf.h shape): the
// include-name parse must stop at the CLOSING delimiter, ignoring any
// trailing block or line comment. Regression: the parser once assumed
// the closing delimiter was the line's last character and swallowed the
// comment into the filename, breaking classic mudlibs.
#include <tests.h>
// #include <uid.h>        /* a commented-out include must stay inert */
#include <trailing_comment_pin.h> /* gets mud info define from The.Master.Of.Hero */
#include "/include/trailing_comment_pin.h" // quoted form with line-comment tail

void do_tests() {
    ASSERT_EQ(42, TRAILING_PIN_VALUE);
    ASSERT_EQ("pinned", TRAILING_PIN_NAME);
}
