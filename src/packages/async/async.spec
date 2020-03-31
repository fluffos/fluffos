void async_read(string, function);
void async_write(string, string, int, function);
void async_getdir(string, function);
#ifdef PACKAGE_DB
void async_db_exec(int, string, string | function, ...);
#endif
