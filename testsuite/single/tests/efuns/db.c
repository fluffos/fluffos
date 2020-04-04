#ifdef __PACKAGE_DB__
int CALLED = 0;
void on_async_db_exec(int conn) {
  int rows = 0;

  CALLED = 1;

  write("ASYNC on_async_db_exec called: " + conn + "\n");

  rows = db_exec(conn,  "select * from tbl1;");
  ASSERT_EQ(1, rows);

  {
    mixed res;

    // index start at 1
    res = db_fetch(conn, 1);
    ASSERT_EQ(({ "hello!", 10 }), res);
  }

  db_close(conn);
}
void check_result() {
  ASSERT_EQ(1, CALLED);
}
#endif

void do_tests() {
#ifndef __PACKAGE_DB__
  write("PACKAGE_DB is not enabled, skipping...\n");
#else
#ifndef __USE_SQLITE3__
  write("USE_SQLITE3 is not enabled, skipping tests...\n");
#else
  // test db functions through sqlite
  int conn = 0, rows = 0;

  // Open & Close
  {
    conn = db_connect("", "/test.sqlite", "", __USE_SQLITE3__);
    ASSERT_NE(0, conn);
    db_close(conn);
  }

  // Full tests
  conn = db_connect("", "/test.sqlite", "", __USE_SQLITE3__);
  ASSERT_NE(0, conn);

  ASSERT_NE("", db_status());
  write(db_status() + "\n");

  // https://sqlite.org/cli.html
  rows = db_exec(conn,  "DROP TABLE IF EXISTS tbl1");
  rows = db_exec(conn,  "create table IF NOT EXISTS tbl1(one varchar(10), two smallint);");
  ASSERT_EQ(0, rows);
  rows = db_exec(conn,  "insert into tbl1 values('hello!',10);");
  ASSERT_EQ(0, rows);
  rows = db_exec(conn,  "insert into tbl1 values('goodbye', 20);");
  ASSERT_EQ(0, rows);
  rows = db_exec(conn,  "select * from tbl1;");
  ASSERT_EQ(2, rows);

  {
    mixed res;

    // index start at 1
    res = db_fetch(conn, 1);
    ASSERT_EQ(({ "hello!", 10 }), res);

    res = db_fetch(conn, 2);
    ASSERT_EQ(({ "goodbye", 20 }), res);
  }

  rows = db_exec(conn,  "drop table tbl1;");
  ASSERT_EQ(0, rows);

  db_close(conn);

#ifndef __PACKAGE_ASYNC__
  write("PACKAGE_ASYNC not defined, skipping async DB tests..");
#else
  conn = db_connect("", "/test.sqlite", "", __USE_SQLITE3__);
  ASSERT_NE(0, conn);
  db_exec(conn,  "DROP TABLE IF EXISTS tbl1");
  db_exec(conn,  "create table IF NOT EXISTS tbl1(one varchar(10), two smallint);");
  async_db_exec(conn, "insert into tbl1 values('hello!',10);", (: on_async_db_exec, conn :) );
  return ;
#endif // __PACKAGE_ASYNC__
#endif // __USE_SQLITE3__
#endif // __PACKAGE_DB__
}
