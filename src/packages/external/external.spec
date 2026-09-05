/* Handle API (issue #1319): create, then start (awaitable), then read
   stdout/stderr/exit_code from the handle.
   `int h = external_create(1, ({ "-s", url }));
    await external_start(h);
    string out = external_stdout(h);`
   Classic form: callbacks, returns the socket fd (int). */
int external_create(int, string | string *);
mixed external_start(int, void | string | string *, void | string | function,
                     void | string | function, void | string | function);
string external_stdout(int);
string external_stderr(int);
int external_exit_code(int);
void external_close(int);
