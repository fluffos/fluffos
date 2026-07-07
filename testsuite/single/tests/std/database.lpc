// Test database lfun

void do_tests()
{
#ifndef __PACKAGE_DB__
  write("PACKAGE_DB is not enabled, skipping...\n");
#else
#ifndef __USE_SQLITE3__
  write("USE_SQLITE3 is not enabled, skipping tests...\n");
#else
    object db = new("/std/database","","/sqlite.db","",__USE_SQLITE3__);
    mixed res;
    // 保持连接
    db->setAutoClose(0);
    db->sql("DROP TABLE IF EXISTS `users`")->exec();
    res = db->sql("CREATE TABLE IF NOT EXISTS `users` (`id` INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,`name` varchar(25) NOT NULL,`bio` TEXT(255) DEFAULT NULL,`activated_at` timestamp DEFAULT NULL)")->exec();
    ASSERT_EQ(0, res);
    res = db->table("users")->insert((["name":"mudos"]));
    ASSERT_EQ(1, res);
    res = db->table("users")->insert((["name":"fluffos","bio":"Actively maintained LPMUD driver (LPC interpreter, MudOS fork)"]));
    ASSERT_EQ(1, res);
    res = db->table("users")->insert((["name":"小泥巴","bio":"大家好才是真的好~","activated_at":"2021-05-01 11:11:11"]));
    ASSERT_EQ(1, res);
    res = db->table("users")->insert((["name":"mud.ren"]));
    ASSERT_EQ(1, res);
    res = db->table("users")->get();
    ASSERT_EQ(({1,"mudos",0,0}), res[0]);
    res = db->table("users")->first();
    ASSERT_EQ(({1,"mudos",0,0}), res);
    res = db->table("users")->find(4);
    ASSERT_EQ(({4,"mud.ren",0,0}), res);
    res = db->table("users")->pluck("name");
    ASSERT_EQ(({"mudos","fluffos","小泥巴","mud.ren"}), res);
    res = db->table("users")->where("id",2)->value("name");
    ASSERT_EQ("fluffos", res);
    res = db->table("users")->whereBetween("id",({2,3}))->pluck("name");
    ASSERT_EQ(({"fluffos","小泥巴"}), res);
    res = db->table("users")->count();
    ASSERT_EQ(4, res);
    res = db->table("users")->where("name","mudos")->update((["bio":"LPMUD驱动"]));
    ASSERT_EQ(1, res);
    res = db->table("users")->where("name","like","mud%")->delete();
    ASSERT_EQ(1, res);
    // 关闭连接
    db->close(1);
#endif
#endif
}
