---
layout: default
title: async / async_db_exec
---

### NAME

    async_db_exec() - executes an sql statement then executes a callback

### SYNOPSIS

    void async_db_exec( int handle, string sql_query, function callback );

### DESCRIPTION

    This function will execute the passed sql statement for the given data‐
    base handle.

    Returns the database handle to the callback function provided.

    The callback should follow this format:

        function (int handle) {
            db_close(handle);
        }

### SEE ALSO

    db_commit(3), db_exec(3), db_fetch(3), db_rollback(3), valid_database(4)

