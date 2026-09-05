/* Classic form: callbacks, returns the socket fd (int).
   Promise form (issue #1319): omit the callbacks, returns a PROMISE
   fulfilled with ({ stdout/stderr, exit_code }) when the process
   exits, or rejected with the negative socket error the classic form
   would have returned (EESECURITY, EESOCKET, ...) -- or with
   "*external process aborted" if the owner is destructed first.
   `mixed *r = await external_start(1, ({ "-s", url }));` */
mixed external_start(int, string | string *, void | string | function,
                     void | string | function, void | string | function);
