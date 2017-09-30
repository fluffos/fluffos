#define TESTING
#include "otable.cc"
#include <gtest/gtest.h>

TEST (BasenameTest, Strings) { 
    auto t = ObjectTable::get();
    EXPECT_EQ ("realms/silenus", t->basename("/realms/silenus.c"));
    EXPECT_EQ ("realms/silenus", t->basename("//realms/silenus.c"));
    EXPECT_EQ ("realms/silenus", t->basename("/////realms/silenus.c"));
    EXPECT_EQ ("realms/silenus", t->basename("/realms/silenus.c.c"));
    EXPECT_EQ ("realms/silenus", t->basename("///realms/silenus.c.c.c"));
    EXPECT_EQ ("realms/silenus", t->basename("/realms/silenus#123"));
    EXPECT_EQ ("realms/beek", t->basename("/realms/beek#4"));
}

TEST(OTableTest, Operations) {
    auto t = ObjectTable::get();
    t->insert("/realms/silenus#1", new object_t("/realms/silenus#1"));
    t->insert("/realms/silenus#13", new object_t("/realms/silenus#13"));
    t->insert("/realms/silenus#2", new object_t("/realms/silenus#2"));
    t->insert("/realms/beek#123", new object_t("/realms/beek#123"));
    EXPECT_EQ(3, t->children("realms/silenus").size() );
    EXPECT_EQ(1, t->children("realms/beek").size() );
    EXPECT_EQ("/realms/silenus#2",t->find("/realms/silenus#2")->obname);
    EXPECT_EQ(0, t->remove("realms/descartes") );
    EXPECT_EQ(0, t->remove("/realms/silenus#3") );
    EXPECT_EQ(1, t->remove("/realms/silenus#2") );
    EXPECT_EQ(2, t->children("realms/silenus").size() );

}

int main(int argc, char **argv) {
    /*
    auto t = ObjectTable::get();

    std::cout << ObjectTable::get()->basename("/realms/silenus.c") << std::endl;
    std::cout << ObjectTable::get()->basename("//realms/silenus.c") << std::endl;
    std::cout << ObjectTable::get()->basename("/////realms/silenus.c") << std::endl;
    std::cout << ObjectTable::get()->basename("/realms/silenus.c.c") << std::endl;
    std::cout << ObjectTable::get()->basename("///realms/silenus.c.c.c") << std::endl;
    std::cout << ObjectTable::get()->basename("/realms/silenus#123") << std::endl;
    std::cout << ObjectTable::get()->basename("/realms/beek#4") << std::endl;
    */
    
/*
    t->insert("/realms/silenus#1", new object_t("/realms/silenus#1"));
    t->insert("/realms/silenus#13", new object_t("/realms/silenus#13"));
    t->insert("/realms/silenus#2", new object_t("/realms/silenus#2"));
    t->insert("/realms/beek#123", new object_t("/realms/beek#123"));
    std::cout << t->children("realms/silenus").size() << std::endl;
    std::cout << t->children("realms/beek").size() << std::endl;
    
    std::cout << t->find("/realms/silenus#2")->obname << std::endl;
    std::cout << t->remove("realms/descartes") << std::endl;
    std::cout << t->remove("/realms/silenus#3") << std::endl;
    std::cout << t->remove("/realms/silenus#2") << std::endl;
    std::cout << t->children("realms/silenus").size() << std::endl;
*/
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

