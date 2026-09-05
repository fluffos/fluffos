/* Classic form (unchanged): callbacks, returns the socket fd (int).
   Promise form (issue #1319): omit the callbacks, returns a PROMISE
   fulfilled with ({ stdout, stderr, exit_code }) when the process
   exits, or rejected with the negative socket error the classic form
   would have returned (EESECURITY, EESOCKET, ...) -- or with
   "*external process aborted" if the owner is destructed first.
   `mixed *r = await external_start(1, ({ "-s", url }));`
   Handle form: external_create() then external_run(handle) fulfills
   with the same ({ stdout, stderr, exit_code }) tuple.
   `int h = external_create(1, ({ "-s", url }));
    mixed *r = await external_run(h);`
   Drive a handle's stdin with external_write() / external_close_stdin(). */
int external_create(int, string | string *);
promise external_run(int);
mixed external_start(int, string | string *, void | string | function,
                     void | string | function, void | string | function);
string external_stdout(int);
string external_stderr(int);
int external_exit_code(int);
void external_write(int, string);
void external_close_stdin(int);
void external_close(int);
