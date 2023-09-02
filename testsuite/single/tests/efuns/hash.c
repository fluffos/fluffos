void do_tests() {
#ifndef __PACKAGE_CRYPTO__
    write("PACKAGE_CRYPTO not defined: test not run.\n");
#else
    ASSERT_EQ(hash("md5", "11111"), "b0baee9d279d34fa1dfd71aadb908c3f");
    ASSERT_EQ(hash("md5", "abc"), "900150983cd24fb0d6963f7d28e17f72");
    ASSERT_EQ(hash("md5", "something"), "437b930db84b8079c2dd804a71936b5f");

    ASSERT_EQ(hash("md4", "12345"), "23580e2a459f7ea40f9efa148b63cafb");
    ASSERT_EQ(sizeof(hash("md4", "12345")), 32);
    ASSERT_EQ(hash("md5", "12345"), "827ccb0eea8a706c4c34a16891f84e7b");
    ASSERT_EQ(sizeof(hash("md5", "12345")), 32);

    ASSERT_EQ(hash("ripemd160", "12345"), "e9cbd2ea8015a084ce9cf83a3c65b51f8fa10a39");
    ASSERT_EQ(sizeof(hash("ripemd160", "12345")), 40);

    ASSERT_EQ(hash("sha1", "12345"), "8cb2237d0679ca88db6464eac60da96345513964");
    ASSERT_EQ(sizeof(hash("sha1", "12345")), 40);
    ASSERT_EQ(hash("sha224", "12345"), "a7470858e79c282bc2f6adfd831b132672dfd1224c1e78cbf5bcd057");
    ASSERT_EQ(sizeof(hash("sha224", "12345")), 56);
    ASSERT_EQ(hash("sha256", "12345"), "5994471abb01112afcc18159f6cc74b4f511b99806da59b3caf5a9c173cacfc5");
    ASSERT_EQ(sizeof(hash("sha256", "12345")), 64);
    ASSERT_EQ(hash("sha384", "12345"), "0fa76955abfa9dafd83facca8343a92aa09497f98101086611b0bfa95dbc0dcc661d62e9568a5a032ba81960f3e55d4a");
    ASSERT_EQ(sizeof(hash("sha384", "12345")), 96);
    ASSERT_EQ(hash("sha512", "12345"), "3627909a29c31381a071ec27f7c9ca97726182aed29a7ddd2e54353322cfb30abb9e3a6df2ac2c20fe23436311d678564d0c8d305930575f60e2d3d048184d79");
    ASSERT_EQ(sizeof(hash("sha512", "12345")), 128);
#endif
}
