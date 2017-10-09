#include "base/std.h"
#include "vm/internal/base/machine.h"
#include "vm/internal/otable.h"

#include <gtest/gtest.h>

TEST (BasenameTest, Strings) { 
    auto t = ObjectTable::get();
    EXPECT_EQ ("realms/silenus", basename("/realms/silenus.c"));
    EXPECT_EQ ("realms/silenus", basename("//realms/silenus.c"));
    EXPECT_EQ ("realms/silenus", basename("/////realms/silenus.c"));
    EXPECT_EQ ("realms/silenus", basename("/realms/silenus.c.c"));
    EXPECT_EQ ("realms/silenus", basename("///realms/silenus.c.c.c"));
    EXPECT_EQ ("realms/silenus", basename("/realms/silenus#123"));
    EXPECT_EQ ("realms/beek", basename("/realms/beek#4"));
}

TEST(OTableTest, Operations) {
    auto t = ObjectTable::get();
    struct object_t ob1;
    struct object_t ob2;
    struct object_t ob3;
    struct object_t ob4;
    
    ob1.obname = "/realms/silenus#1";
    ob2.obname = "/realms/silenus#13";
    ob3.obname = "/realms/silenus#2";
    ob4.obname = "/realms/beek#123";
    
    t->insert("/realms/silenus#1", &ob1);
    t->insert("/realms/silenus#13", &ob2);
    t->insert("/realms/silenus#2", &ob3);
    t->insert("/realms/beek#123", &ob4);
    EXPECT_EQ(3, t->children("realms/silenus").size() );
    EXPECT_EQ(1, t->children("realms/beek").size() );
    EXPECT_EQ("/realms/silenus#2",t->find("/realms/silenus#2")->obname);
    EXPECT_EQ(0, t->remove("realms/descartes") );
    EXPECT_EQ(0, t->remove("/realms/silenus#3") );
    EXPECT_EQ(1, t->remove("/realms/silenus#2") );
    EXPECT_EQ(2, t->children("realms/silenus").size() );

}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

