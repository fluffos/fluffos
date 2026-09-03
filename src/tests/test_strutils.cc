#include <gtest/gtest.h>
#include <string>
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

// ---------------------------------------------------------------------------
// EGCSmartIterator pure-ASCII fast path.
//
// For a string with no high-bit byte, EGCIterator skips ICU entirely and
// EGCSmartIterator answers arithmetically (EGC index == byte offset). These
// tests pin that the fast path is observationally identical to driving ICU,
// including the DONE / out-of-range edges -- they are the reason the fast path
// can be trusted rather than reasoned about.
//
// The reference side forces the ICU path by using a *separate* iterator object
// and driving the raw icu::BreakIterator through operator->(), which calls
// ensure_icu(). Two objects, so the two cursors never interfere.
namespace {

// Reference implementations, computed with ICU on the very same bytes.
int32_t icu_count(const char* s, int32_t len) {
  EGCIterator ref(s, len);
  int32_t n = 0;
  ref->first();
  while (ref->next() != icu::BreakIterator::DONE) ++n;
  return n;
}

// Mirror of EGCSmartIterator::index_to_offset()'s ICU branch, run on a fresh
// iterator so it is unaffected by the fast path under test.
int32_t icu_index_to_offset(const char* s, int32_t len, int32_t index) {
  EGCIterator ref(s, len);
  if (index == 0) return ref->first();
  if (index == -1) {
    ref->last();
    return ref->previous();
  }
  int32_t current_idx;
  if (index > 0) {
    current_idx = 0;
    ref->first();
  } else {
    current_idx = -1;
    ref->last();
    ref->previous();
  }
  auto oldpos = ref->current();
  auto pos = ref->next(index - current_idx);
  if (pos == icu::BreakIterator::DONE) ref->isBoundary(oldpos);
  return pos;
}

}  // namespace

TEST(EGCAsciiFastPath, DetectsAsciiAndNonAscii) {
  EGCSmartIterator ascii("hello world", 11);
  EXPECT_TRUE(ascii.ok());
  EXPECT_TRUE(ascii.is_ascii());

  const char* wide = "a\xe4\xbd\xa0z";  // a 你 z
  EGCSmartIterator non_ascii(wide, 5);
  EXPECT_TRUE(non_ascii.ok());
  EXPECT_FALSE(non_ascii.is_ascii());

  EGCSmartIterator empty("", 0);
  EXPECT_TRUE(empty.ok());
  EXPECT_TRUE(empty.is_ascii());
}

// count() walks the ICU iterator to the end as a side effect, so the ASCII
// path has to leave its cursor there too -- otherwise a count() followed by
// next() means one thing on ASCII input and another on non-ASCII. Compared
// against ICU rather than against an assumed answer.
TEST(EGCAsciiFastPath, CountLeavesCursorAtEndLikeIcu) {
  const char* ascii = "hello";
  const char* wide = "a\xe4\xbd\xa0z";  // a 你 z -- 3 clusters, 5 bytes

  EGCSmartIterator a(ascii, 5);
  ASSERT_TRUE(a.is_ascii());
  EXPECT_EQ(5u, a.count());
  EXPECT_EQ(icu::BreakIterator::DONE, a.next()) << "cursor must be at the end";

  EGCSmartIterator w(wide, 5);
  ASSERT_FALSE(w.is_ascii());
  EXPECT_EQ(3u, w.count());
  EXPECT_EQ(icu::BreakIterator::DONE, w.next()) << "ICU path, same contract";
}

TEST(EGCAsciiFastPath, CountMatchesIcu) {
  const char* cases[] = {"",   "a",         "hello world",
                         "\t\n multi line \n text", "0123456789",
                         "a string long enough to cross a few cache lines "
                         "and keep the break iterator busy for a while"};
  for (const char* s : cases) {
    auto len = static_cast<int32_t>(strlen(s));
    EGCSmartIterator it(s, len);
    ASSERT_TRUE(it.is_ascii()) << s;
    EXPECT_EQ(static_cast<size_t>(len), it.count()) << s;
    EXPECT_EQ(icu_count(s, len), static_cast<int32_t>(it.count())) << s;
  }
}

TEST(EGCAsciiFastPath, IndexToOffsetMatchesIcuIncludingOutOfRange) {
  const char* cases[] = {"", "a", "hello", "0123456789abcdef"};
  for (const char* s : cases) {
    auto len = static_cast<int32_t>(strlen(s));
    // Sweep well past both ends so the DONE edges are covered.
    for (int32_t i = -len - 3; i <= len + 3; i++) {
      EGCSmartIterator it(s, len);
      ASSERT_TRUE(it.is_ascii()) << s;
      EXPECT_EQ(icu_index_to_offset(s, len, i), it.index_to_offset(i))
          << "s='" << s << "' index=" << i;
    }
  }
}

TEST(EGCAsciiFastPath, PostIndexToOffsetMatchesIcu) {
  const char* cases[] = {"", "a", "hello", "0123456789"};
  for (const char* s : cases) {
    auto len = static_cast<int32_t>(strlen(s));
    for (int32_t i = -len - 2; i <= len + 2; i++) {
      // Reference: drive the ICU branch via a non-ASCII-free clone is not
      // possible, so replicate post_index_to_offset()'s ICU steps directly.
      EGCIterator ref(s, len);
      int32_t expect;
      int32_t pos = icu_index_to_offset(s, len, i);
      if (pos < 0) {
        expect = pos;
      } else {
        // Re-seat the reference cursor at pos, then next()/previous().
        ref->isBoundary(pos);
        expect = ref->next();
        ref->previous();
      }
      EGCSmartIterator it(s, len);
      ASSERT_TRUE(it.is_ascii()) << s;
      EXPECT_EQ(expect, it.post_index_to_offset(i)) << "s='" << s << "' index=" << i;
    }
  }
}

TEST(EGCAsciiFastPath, WalkMatchesIcu) {
  const char* s = "walk every boundary";
  auto len = static_cast<int32_t>(strlen(s));

  EGCSmartIterator it(s, len);
  ASSERT_TRUE(it.is_ascii());
  EXPECT_EQ(0, it.first());
  for (int32_t i = 1; i <= len; i++) {
    EXPECT_EQ(i, it.next()) << "boundary " << i;
  }
  EXPECT_EQ(icu::BreakIterator::DONE, it.next());  // off the end
  EXPECT_EQ(icu::BreakIterator::DONE, it.next());  // still off the end
  EXPECT_EQ(len, it.last());
}

TEST(EGCAsciiFastPath, NonAsciiStillUsesIcu) {
  // a 你 😀 z -- 4 clusters, 1 + 3 + 4 + 1 bytes.
  const char* s = "a\xe4\xbd\xa0\xf0\x9f\x98\x80z";
  auto len = static_cast<int32_t>(strlen(s));
  EGCSmartIterator it(s, len);
  ASSERT_TRUE(it.ok());
  ASSERT_FALSE(it.is_ascii());
  EXPECT_EQ(4u, it.count());
  EXPECT_EQ(icu_count(s, len), static_cast<int32_t>(it.count()));
  EXPECT_EQ(0, it.index_to_offset(0));
  EXPECT_EQ(1, it.index_to_offset(1));
  EXPECT_EQ(4, it.index_to_offset(2));
  EXPECT_EQ(8, it.index_to_offset(3));
  EXPECT_EQ(8, it.index_to_offset(-1));
}

// The whole fast path rests on one claim: among ASCII bytes, CR-LF is the ONLY
// pair that forms a single grapheme cluster. Prove it against ICU rather than
// against a reading of UAX #29 -- an extra joining pair would silently corrupt
// every sizeof()/index on strings containing it.
TEST(EGCAsciiFastPath, CrLfIsTheOnlyAsciiJoin) {
  for (int a = 0; a < 128; a++) {
    for (int b = 0; b < 128; b++) {
      if (a == 0 || b == 0) continue;  // NUL would terminate the buffer early
      char buf[2] = {static_cast<char>(a), static_cast<char>(b)};
      int32_t clusters = icu_count(buf, 2);
      bool is_crlf = (a == '\r' && b == '\n');
      EXPECT_EQ(is_crlf ? 1 : 2, clusters)
          << "ASCII pair (" << a << "," << b << ") clustered unexpectedly";
    }
  }
}

// explode() reset()s the iterator to a shrinking suffix once per token.
// A subrange of a known-ASCII string must stay on the ASCII path without
// a fresh all_ascii() scan (issue #1366). Resetting to a different buffer
// still has to rescan — including the ASCII → non-ASCII direction.
TEST(EGCAsciiFastPath, ResetToSuffixStaysAscii) {
  std::string s(2048, 'x');
  for (size_t i = 10; i < s.size(); i += 11) s[i] = ' ';
  EGCIterator it(s.data(), static_cast<int32_t>(s.size()));
  ASSERT_TRUE(it.is_ascii());
  for (int32_t off = 0; off + 11 <= static_cast<int32_t>(s.size()); off += 11) {
    it.reset(s.data() + off, static_cast<int32_t>(s.size()) - off);
    EXPECT_TRUE(it.ok()) << "offset " << off;
    EXPECT_TRUE(it.is_ascii()) << "offset " << off;
    EXPECT_EQ(s.data() + off, it.data());
  }
}

TEST(EGCAsciiFastPath, ResetToUnrelatedStringRescans) {
  EGCIterator it("hello", 5);
  ASSERT_TRUE(it.is_ascii());
  const char* wide = "a\xe4\xbd\xa0z";  // a 你 z
  it.reset(wide, 5);
  EXPECT_TRUE(it.ok());
  EXPECT_FALSE(it.is_ascii());
  it.reset("abc", 3);
  EXPECT_TRUE(it.ok());
  EXPECT_TRUE(it.is_ascii());
}

TEST(U8EgcFind, AsciiForwardAndReverse) {
  const char* s = "ab cd ab";
  EGCIterator it(s, 8);
  ASSERT_TRUE(it.is_ascii());
  EXPECT_EQ(2, u8_egc_find_as_offset(it, " ", 1, false));
  EXPECT_EQ(5, u8_egc_find_as_offset(it, " ", 1, true));
  EXPECT_EQ(0, u8_egc_find_as_offset(it, "ab", 2, false));
  EXPECT_EQ(6, u8_egc_find_as_offset(it, "ab", 2, true));
  EXPECT_EQ(-1, u8_egc_find_as_offset(it, "zz", 2, false));
  EXPECT_EQ(-1, u8_egc_find_as_offset(it, "zz", 2, true));
  // Non-ASCII needle cannot occur in a CR-free ASCII haystack.
  EXPECT_EQ(-1, u8_egc_find_as_offset(it, "\xe4\xbd\xa0", 3, false));
}

// And confirm a CR-bearing string is routed to ICU rather than the fast path.
TEST(EGCAsciiFastPath, CarriageReturnFallsBackToIcu) {
  const char* s = "line one\r\nline two";
  auto len = static_cast<int32_t>(strlen(s));
  EGCSmartIterator it(s, len);
  ASSERT_TRUE(it.ok());
  EXPECT_FALSE(it.is_ascii()) << "CR must disable the fast path";
  EXPECT_EQ(static_cast<size_t>(icu_count(s, len)), it.count());
  EXPECT_EQ(static_cast<size_t>(len - 1), it.count());  // CRLF counts as one
}

// ---------------------------------------------------------------------------
// MSTR_ASCII tag lifecycle on the one string-block allocation that does NOT
// go through int_new_string()/alloc_new_shared_string(): int_string_unlink(),
// which raw-DMALLOCs a fresh header when detaching a multiply-referenced
// string. Before the fix it copied size/ref into the new header but left the
// ascii field as heap garbage -- garbage that happened to equal
// MSTR_ASCII_YES on a non-ASCII string would make sizeof() answer from the
// byte length. This pins the invariant that the unlinked copy's tag is a
// defined value and that the public predicate answers correctly through it.
// (The uninitialized read itself is what MSan would flag pre-fix.)
#include "base/internal/stralloc.h"

TEST(MstrAsciiTag, StringUnlinkProducesADefinedTag) {
  // Non-ASCII case: cache NO on the original, then unlink a second reference.
  char* s = new_string(6, "test: unlink non-ascii");
  memcpy(s, "\xe4\xbd\xa0\xe5\xa5\xbd", 7);  // "你好" + NUL, 6 bytes
  EXPECT_FALSE(u8_string_is_ascii_cached(s, 6, true));
  EXPECT_EQ(MSTR_ASCII_NO, MSTR_ASCII(s));

  MSTR_REF(s)++;  // simulate a second reference, as unlink_string_svalue sees
  char* copy = string_unlink(s, "test: unlink non-ascii");
  ASSERT_NE(s, copy);
  unsigned char tag = MSTR_ASCII(copy);
  EXPECT_TRUE(tag == MSTR_ASCII_UNKNOWN || tag == MSTR_ASCII_NO)
      << "unlinked copy's ascii tag is undefined garbage: " << int(tag);
  EXPECT_FALSE(u8_string_is_ascii_cached(copy, 6, true));
  FREE_MSTR(copy);
  FREE_MSTR(s);

  // ASCII case: the copy must not come out pre-tagged NO-or-garbage either.
  char* a = new_string(3, "test: unlink ascii");
  memcpy(a, "abc", 4);
  EXPECT_TRUE(u8_string_is_ascii_cached(a, 3, true));
  MSTR_REF(a)++;
  char* acopy = string_unlink(a, "test: unlink ascii");
  ASSERT_NE(a, acopy);
  tag = MSTR_ASCII(acopy);
  EXPECT_TRUE(tag == MSTR_ASCII_UNKNOWN || tag == MSTR_ASCII_YES)
      << "unlinked copy's ascii tag is undefined garbage: " << int(tag);
  EXPECT_TRUE(u8_string_is_ascii_cached(acopy, 3, true));
  FREE_MSTR(acopy);
  FREE_MSTR(a);
}
