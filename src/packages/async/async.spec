/* With the trailing callback: classic form, delivery via the callback,
   returns 0. With the callback OMITTED: returns a PROMISE (issue #1319)
   fulfilled with the value the callback would have received, or rejected
   with the failure value the callback would have received (async_read's
   negative int). `string s = await async_read(path);` */
mixed async_read(string, void | function);
mixed async_write(string, string, int, void | function);
mixed async_getdir(string, void | function);
#ifdef PACKAGE_DB
void async_db_exec(int, string, string | function, ...);
#endif
