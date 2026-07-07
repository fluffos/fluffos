#ifdef __PACKAGE_DB__
int calledDB;
#endif

int conn = 0;
void do_tests() {
#ifndef __PACKAGE_DB__
    write("PACKAGE_DB is not enabled, skipping DB tests...\n");
    return;
#else
#ifndef __USE_SQLITE3__
    write("USE_SQLITE3 is not enabled, skipping SQL tests...\n");
    return;
#else
    // test db functions through sqlite
    int rows = 0;
    mixed res;

    // Open & Close
    conn = db_connect("", "/test.sqlite", "", __USE_SQLITE3__);
    ASSERT_NE(0, conn);
    db_close(conn);

    // Full tests
    conn = db_connect("", "/test.sqlite", "", __USE_SQLITE3__);
    ASSERT_NE(0, conn);

    ASSERT_NE("", db_status());
    write("db_status: " + db_status() + "\n");

    // https://sqlite.org/cli.html
    rows = db_exec(conn, "DROP TABLE IF EXISTS tbl1");
    rows = db_exec(conn, "create table IF NOT EXISTS tbl1(one varchar(10), two bigint);");
    ASSERT_EQ(0, rows);
    rows = db_exec(conn, "insert into tbl1 values('hello!',10);");
    ASSERT_EQ(0, rows);
    rows = db_exec(conn, "insert into tbl1 values('goodbye', 20);");
    ASSERT_EQ(0, rows);
    rows = db_exec(conn, "insert into tbl1 values('largeint', 9223372036854775807);");
    ASSERT_EQ(0, rows);
    rows = db_exec(conn, "select * from tbl1;");
    ASSERT_EQ(3, rows);

    res = db_fetch(conn, 1); // index start at 1
    ASSERT_EQ(({ "hello!", 10 }), res);

    res = db_fetch(conn, 2);
    ASSERT_EQ(({ "goodbye", 20 }), res);

    res = db_fetch(conn, 3);
    ASSERT_EQ(({ "largeint", MAX_INT }), res);

    rows = db_exec(conn, "drop table tbl1;");
    ASSERT_EQ(0, rows);

    db_close(conn);

#ifndef __PACKAGE_ASYNC__
    write("PACKAGE_ASYNC is not enabled, skipping async db tests...\n");
    return;
#else
    conn = db_connect("", "/test.sqlite", "", __USE_SQLITE3__);
    ASSERT_NE(0, conn);
    db_exec(conn, "DROP TABLE IF EXISTS tbl1");
    db_exec(conn, "create table IF NOT EXISTS tbl1(one varchar(10), two smallint);");
    async_db_exec(conn, "insert into tbl1 values('hello!',10);", function(int rows) {
        mixed res;

        calledDB = 1;

        write("ASYNC: async_db_exec callback: " + conn + " matched " + rows + " rows\n");

        rows = db_exec(conn, "select * from tbl1;");
        ASSERT_EQ(1, rows);

        res = db_fetch(conn, 1); // index starts at 1
        ASSERT_EQ(({ "hello!", 10 }), res);

        db_close(conn);
    });

    call_out(function() {
        ASSERT_EQ(1, calledDB);
    }, 1);
#endif // __PACKAGE_ASYNC__
#endif // __USE_SQLITE3__
#endif // __PACKAGE_DB__
}
