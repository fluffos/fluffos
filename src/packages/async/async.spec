void async_read(string, string | function, ...);
void async_write(string, string, int, string | function, ...);
void async_getdir(string, string | function, ...);
#ifdef PACKAGE_DB
void async_db_exec(int, string, string | function, ...);
#endif
