#include <gtest/gtest.h>
#include "base/std.h"

#include "base/internal/file.h"

// legal_path: the last line of defense behind master's valid_read()/
// valid_write() -- every mudlib-supplied path reaches the filesystem through
// it, so a component it fails to reject escapes the mudlib directory.

TEST(LegalPath, AcceptsOrdinaryPaths) {
  EXPECT_EQ(1, legal_path("obj/monster.c"));
  EXPECT_EQ(1, legal_path("."));         // trailing "." is allowed
  EXPECT_EQ(1, legal_path("obj/."));     // ... including as last component
  EXPECT_EQ(1, legal_path(".dotfile"));  // leading dot in a name is not ".."
  EXPECT_EQ(1, legal_path("obj/..dotdot"));
  EXPECT_EQ(1, legal_path("obj/monster#123"));  // '#' introducing an OID
}

TEST(LegalPath, RejectsTraversalAndAbsolutePaths) {
  EXPECT_EQ(0, legal_path(nullptr));
  EXPECT_EQ(0, legal_path("/etc/passwd"));
  EXPECT_EQ(0, legal_path(".."));
  EXPECT_EQ(0, legal_path("../secret"));
  EXPECT_EQ(0, legal_path("./secret"));
  EXPECT_EQ(0, legal_path("obj/../../secret"));
  EXPECT_EQ(0, legal_path("obj/.."));
  EXPECT_EQ(0, legal_path("obj/monster#abc"));  // '#' not introducing an OID
}

TEST(LegalPath, RejectsWindowsSeparatorTraversal) {
  // No-ops on POSIX (where '\\' and ':' are ordinary filename bytes), but on
  // Windows both forms are honored by the OS and would escape the mudlib.
#ifdef _WIN32
  EXPECT_EQ(0, legal_path("..\\secret"));
  EXPECT_EQ(0, legal_path("obj\\..\\..\\secret"));
  EXPECT_EQ(0, legal_path("\\windows\\system32"));
  EXPECT_EQ(0, legal_path("C:\\windows\\system32"));
  EXPECT_EQ(0, legal_path("c:/windows/system32"));
#else
  EXPECT_EQ(1, legal_path("..\\secret"));
#endif
}
