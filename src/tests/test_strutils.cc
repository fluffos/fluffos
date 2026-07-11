#include <gtest/gtest.h>
#include "base/std.h"

#include "base/internal/strutils.h"

// u8_incomplete_tail: how many trailing bytes of a streaming chunk form the
// prefix of a not-yet-complete UTF-8 character (and must be held back
// instead of being sanitized into U+FFFD).

TEST(U8IncompleteTail, CompleteInputHasNoTail) {
  EXPECT_EQ(0u, u8_incomplete_tail(""));
  EXPECT_EQ(0u, u8_incomplete_tail("ascii only"));
  EXPECT_EQ(0u, u8_incomplete_tail("wide \xe4\xbd\xa0"));          // 你 complete
  EXPECT_EQ(0u, u8_incomplete_tail("\xc3\xa9"));                   // é complete
  EXPECT_EQ(0u, u8_incomplete_tail("\xf0\x9f\x98\x80"));           // 😀 complete
}

TEST(U8IncompleteTail, HoldsBackPartialSequences) {
  EXPECT_EQ(1u, u8_incomplete_tail("abc\xc3"));                    // é lead only
  EXPECT_EQ(1u, u8_incomplete_tail("abc\xe4"));                    // 你 lead only
  EXPECT_EQ(2u, u8_incomplete_tail("abc\xe4\xbd"));                // 你 2 of 3
  EXPECT_EQ(1u, u8_incomplete_tail("abc\xf0"));                    // 😀 1 of 4
  EXPECT_EQ(2u, u8_incomplete_tail("abc\xf0\x9f"));                // 😀 2 of 4
  EXPECT_EQ(3u, u8_incomplete_tail("abc\xf0\x9f\x98"));            // 😀 3 of 4
  // partial sequence is the whole chunk
  EXPECT_EQ(1u, u8_incomplete_tail("\xe4"));
  EXPECT_EQ(2u, u8_incomplete_tail("\xe4\xbd"));
}

TEST(U8IncompleteTail, MalformedInputIsNotHeld) {
  // invalid lead bytes: sanitize now, nothing will complete them
  EXPECT_EQ(0u, u8_incomplete_tail("abc\xff"));
  EXPECT_EQ(0u, u8_incomplete_tail("abc\xc0"));                    // overlong lead
  EXPECT_EQ(0u, u8_incomplete_tail("abc\xf5"));                    // > U+10FFFF
  // stray continuation bytes with no lead in reach
  EXPECT_EQ(0u, u8_incomplete_tail("abc\x80"));
  EXPECT_EQ(0u, u8_incomplete_tail("abc\x80\x80\x80"));
  // complete-but-invalid pairs are left for the sanitizer
  EXPECT_EQ(0u, u8_incomplete_tail("abc\xc3\xa9\xa9"));
  // ASCII directly before the end
  EXPECT_EQ(0u, u8_incomplete_tail("abc\x7f"));
}
