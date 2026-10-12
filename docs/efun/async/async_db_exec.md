---
title: async / async_db_exec
---
# async_db_exec

### NAME

    async_db_exec() - execute an sql statement, then call back or return a promise

### SYNOPSIS

    void async_db_exec( int handle, string sql_query, function callback, ... );
    promise async_db_exec( int handle, string sql_query );

### DESCRIPTION

    This function will execute the passed sql statement for the given
    database handle.

    With a callback, the call returns 0 and the callback is invoked later
    with one argument: the number of matched rows on success, or the error
    string `db_exec()` would have returned on failure. Arguments after the
    callback are passed through to it, after that result.

    ```c
    async_db_exec(
        handle,
        sql_query,
        function (int rows) { // number of matched rows
            mixed *results = db_fetch(handle, 1);
            db_close(handle);
        }
    );
    ```

    With the callback omitted, the call returns a promise instead: fulfilled
    with that row count, or rejected with the error string. Inside an async
    function:

    ```c
    int rows = await async_db_exec(handle, sql_query);
    mixed *results = db_fetch(handle, 1);
    ```

    A statement the backend rejects is the same failure in both forms. On
    success an INSERT, UPDATE or DELETE fulfills with 0, because those
    statements have no result rows.

### NOTE

    When the 'this_player in call_out' driver setting is enabled,
    this_player() inside the callback is preserved from the time the
    request was made, like call_out().

### SEE ALSO

    db_commit(3), db_exec(3), db_fetch(3), db_rollback(3), valid_database(4)

