#include <gtest/gtest.h>
#include "base/std.h"

#include "base/internal/file.h"
#include "base/internal/port.h"
#include "base/internal/strput.h"

#include <ctime>
#include <set>
#include <string>

// legal_path() is the driver's path sanitizer: everything reachable from LPC
// file efuns goes through it, but only its happy path was exercised.

TEST(LegalPath, AcceptsPlainRelativePaths) {
  EXPECT_EQ(1, legal_path(""));
  EXPECT_EQ(1, legal_path("foo"));
  EXPECT_EQ(1, legal_path("foo/bar.c"));
  EXPECT_EQ(1, legal_path("foo/bar/baz"));
  EXPECT_EQ(1, legal_path(".hidden"));
  EXPECT_EQ(1, legal_path("foo/.hidden"));
  EXPECT_EQ(1, legal_path("foo/..bar"));  // '..' only as a whole component is bad
}

TEST(LegalPath, RejectsNullAndAbsolutePaths) {
  EXPECT_EQ(0, legal_path(nullptr));
  EXPECT_EQ(0, legal_path("/"));
  EXPECT_EQ(0, legal_path("/foo"));
}

TEST(LegalPath, RejectsParentDirectoryTraversal) {
  EXPECT_EQ(0, legal_path(".."));
  EXPECT_EQ(0, legal_path("../foo"));
  EXPECT_EQ(0, legal_path("foo/.."));
  EXPECT_EQ(0, legal_path("foo/../bar"));
  EXPECT_EQ(0, legal_path("foo/bar/../../etc"));
  EXPECT_EQ(0, legal_path("./foo"));
  EXPECT_EQ(0, legal_path("foo/./bar"));
}

// A trailing '.' is deliberately allowed, and everything after the first '#'
// must be a clone id (digits only).
TEST(LegalPath, AllowsTrailingDotAndCloneIds) {
  EXPECT_EQ(1, legal_path("foo/."));
  EXPECT_EQ(1, legal_path("foo#123"));
  EXPECT_EQ(1, legal_path("foo#"));  // nothing after '#' to validate
}

TEST(LegalPath, RejectsNonCloneHashes) {
  EXPECT_EQ(0, legal_path("foo#bar"));
  EXPECT_EQ(0, legal_path("foo#123abc"));
  EXPECT_EQ(0, legal_path("foo#12#34"));  // a second '#' is not a digit
}

// strput()/strput_int(): bounded string appending used all over the driver's
// message formatting. The truncation branch is the interesting one.

TEST(StrPut, CopiesAndReturnsEndOfString) {
  char buf[16] = {};
  char* end = buf + sizeof(buf);

  char* p = strput(buf, end, "abc");
  EXPECT_STREQ("abc", buf);
  EXPECT_EQ(buf + 3, p);

  p = strput(p, end, "de");
  EXPECT_STREQ("abcde", buf);
  EXPECT_EQ(buf + 5, p);

  p = strput(p, end, "");
  EXPECT_STREQ("abcde", buf);
  EXPECT_EQ(buf + 5, p);
}

TEST(StrPut, TruncatesAtLimitAndStaysNulTerminated) {
  char buf[8] = {};
  char* end = buf + sizeof(buf);

  char* p = strput(buf, end, "abc");
  p = strput(p, end, "defghijkl");

  EXPECT_STREQ("abcdefg", buf);
  EXPECT_EQ(buf + 7, p);
  EXPECT_EQ('\0', buf[7]);

  // appending at the limit keeps the buffer terminated
  p = strput(p, end, "xyz");
  EXPECT_STREQ("abcdefg", buf);
  EXPECT_EQ(buf + 7, p);
}

TEST(StrPut, IntFormatsSignedValues) {
  char buf[32] = {};
  char* end = buf + sizeof(buf);

  char* p = strput_int(buf, end, 0);
  EXPECT_STREQ("0", buf);
  EXPECT_EQ(buf + 1, p);

  p = strput_int(buf, end, -12345);
  EXPECT_STREQ("-12345", buf);
  EXPECT_EQ(buf + 6, p);

  strput_int(buf, end, 2147483647);
  EXPECT_STREQ("2147483647", buf);
}

TEST(StrPut, IntTruncatesAtLimit) {
  char buf[4] = {};
  char* p = strput_int(buf, buf + sizeof(buf), 123456);

  EXPECT_STREQ("123", buf);
  EXPECT_EQ(buf + 3, p);
}

// port.cc: random_number()/secure_random_number() guard against the
// non-positive range that would be UB in uniform_int_distribution.

TEST(RandomNumber, NonPositiveRangeReturnsZero) {
  EXPECT_EQ(0, random_number(0));
  EXPECT_EQ(0, random_number(-1));
  EXPECT_EQ(0, random_number(-1000000));
  EXPECT_EQ(0, secure_random_number(0));
  EXPECT_EQ(0, secure_random_number(-42));
}

TEST(RandomNumber, StaysInRangeAndCoversIt) {
  EXPECT_EQ(0, random_number(1));
  EXPECT_EQ(0, secure_random_number(1));

  std::set<int64_t> seen;
  for (int i = 0; i < 2000; i++) {
    auto n = random_number(4);
    ASSERT_GE(n, 0);
    ASSERT_LT(n, 4);
    seen.insert(n);

    auto s = secure_random_number(4);
    ASSERT_GE(s, 0);
    ASSERT_LT(s, 4);
  }
  // 2000 draws from 4 buckets: every value is expected (P(miss) ~ 4*0.75^2000).
  EXPECT_EQ(4u, seen.size());
}

TEST(PortClocks, CurrentTimeAndUsecClockAgree) {
  auto now = get_current_time();
  EXPECT_NEAR(static_cast<double>(now), static_cast<double>(::time(nullptr)), 5.0);

  long sec = 0, usec = -1;
  get_usec_clock(&sec, &usec);
  EXPECT_GE(usec, 0);
  EXPECT_LT(usec, 1000000);
  EXPECT_NEAR(static_cast<double>(sec), static_cast<double>(now), 5.0);
}

TEST(PortClocks, CpuTimesAreMonotonic) {
  unsigned long secs = 0, usecs = 0;
  ASSERT_EQ(1, get_cpu_times(&secs, &usecs));

  // burn a little cpu
  volatile double x = 0;
  for (int i = 0; i < 1000000; i++) {
    x += i;
  }

  unsigned long secs2 = 0, usecs2 = 0;
  ASSERT_EQ(1, get_cpu_times(&secs2, &usecs2));
  EXPECT_GE(secs2 * 1000000UL + usecs2, secs * 1000000UL + usecs);
}

TEST(PortDir, CurrentDirIsAnAbsolutePath) {
  char buf[1024] = {};
  ASSERT_EQ(buf, get_current_dir(buf, sizeof(buf)));
  EXPECT_FALSE(std::string(buf).empty());

  // a buffer that cannot hold the path fails instead of overflowing
  char tiny[2] = {};
  EXPECT_EQ(nullptr, get_current_dir(tiny, sizeof(tiny)));
}
