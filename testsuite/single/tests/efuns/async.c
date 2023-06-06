#ifndef __PACKAGE_ASYNC__
nosave int calledGetDir, calledWrite, calledRead;
nosave string t = "" + time();
#endif

void do_tests() {
#ifndef __PACKAGE_ASYNC__
    write("PACKAGE_ASYNC is not enabled, skipping async tests...\n");
    return;
#else

    // async_getdir
    async_getdir("/nonexistant/", function(mixed res) {
        write("ASYNC: async_getdir callback\n");
        ASSERT_EQ(get_dir("/nonexistant/"), res); // 0
        calledGetDir++;
    });
    async_getdir("/nonexistant", function(mixed res) {
        write("ASYNC: async_getdir callback\n");
        ASSERT_EQ(get_dir("/nonexistant"), res); // ({ })
        calledGetDir++;
    });
    async_getdir("/u/", function(mixed res) {
        write("ASYNC: async_getdir callback\n");
        ASSERT_EQ(get_dir("/u/"), res);
        calledGetDir++;
    });
    call_out((: async_getdir, "/std/", function(mixed res) {
        write("ASYNC: call_out async_getdir callback\n");
        ASSERT_EQ(get_dir("/std/"), res);
        calledGetDir++;
    } :), 1);

    // async_write
    async_write("/log", t, 1, function(int res) {
        write("ASYNC: async_write callback\n");
        ASSERT_EQ(-1, res);
        calledWrite++;
    });
    async_write("/log/testfile", t, 1, function(int res) {
        write("ASYNC: async_write callback\n");
        ASSERT_EQ(0, res);
        calledWrite++;
    });

    // async_read
    async_read("/log", function(mixed res) {
        write("ASYNC: async_read callback\n");
        ASSERT_EQ(-1, res);
        calledRead++;
    });
    async_read("/log/testfile", function(mixed res) {
        write("ASYNC: async_read callback\n");
        ASSERT_EQ(t, res);
        calledRead++;
    });

    call_out(function() {
        rm("/log/testfile");
        ASSERT_EQ(3, calledGetDir);
        ASSERT_EQ(2, calledWrite);
        ASSERT_EQ(2, calledRead);
    }, 2);
#endif
}