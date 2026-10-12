/* With the trailing callback: classic form, delivery via the callback,
   returns 0. With the callback OMITTED: returns a PROMISE (issue #1319)
   fulfilled with the value the callback would have received, or rejected
   with the failure value the callback would have received (async_read's
   negative int). `string s = await async_read(path);` */
mixed async_read(string, void | function);
mixed async_write(string, string, int, void | function);
mixed async_getdir(string, void | function);
#ifdef PACKAGE_DB
/* Same split as async_read(): callback present -> classic form, returns 0;
   callback omitted -> PROMISE fulfilled with the row count (the callback's
   argument on success) or rejected with the error string db_exec() would
   return. `int rows = await async_db_exec(handle, sql);` */
mixed async_db_exec(int, string, void | string | function, ...);
#endif
