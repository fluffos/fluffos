---
layout: default
title: db / db_fetch
---

### NAME

    db_fetch() - fetches a result set

### SYNOPSIS

    mixed *db_fetch( int handle, int row );

### DESCRIPTION

    Fetches  the result set for the given row of the last executeted sql on
    the passed database handle.

    Returns an array of columns of the named  row  upon  success  an  error
    string otherwise.

### EXAMPLE

    string *res;
    mixed rows;
    int dbconn, i;

    dbconn = db_connect("db.server", "db_mud");
    if(dbconn < 1)
        return  0;
    rows  =  db_exec(dbconn,  "SELECT player_name, exp FROM t_player");
    if(!rows)
        write("No rows return3d.");
    else
        if(stringp(rows))  /* error */
            write(rows);
        else
            for(i = 1; i <= rows; i++)
            {
                res = db_fetch(dbconn, i);
                write(res[0]);
                write(res[1]);
            }

    db_close(dbconn);

### SEE ALSO

    db_exec(3), valid_database(4)
