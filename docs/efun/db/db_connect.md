---
layout: default
title: db / db_connect
---

### NAME

    db_connect() - close the database connection

### SYNOPSIS

    int db_connect( string host, string db );
    int db_connect( string host, string db, string user );
    int db_connect( string  host, string db, string user, int type );

### DESCRIPTION

    Creates  a  new  connection  to the database db on the given host.  The
    connection uses either the given user or a compile time value as  login
    id.  type may be used to choose the type of database server. Valid val‐
    ues depend on compile time settings should be made available through  a
    corresponding header file within the mudlib.

    Returns a handle to the new connection on success, 0 otherwise

    Driver will call master object's valid_database function to reterive
    password for this database (string) or approvals (positive intgeter).

    FlufFOS supports MYSQL, SQLITE3 and PostgreSQL.

    When compiling driver, you need to pass -DPACAKGE_DB=ON and
        -DPACKAGE_MYSQL=X or
        -DPAKCAGE_SQLITE=X or
        -DPACAKGE_POSTGRESQL=X
     and you should also pass -DPACAKGE_DB_DEFAULT_DB=X which should be one
     of the value ablve. if one value is empty string, the driver support for
     that DB is disabled.

     X here means an integer representing the "type" parameter in db_connect().

     Driver prvoides following pre-defines constant to lib for DB types.

    __USE_MYSQL__   is what -DPACAKGE_MYSQL=<value> is, default to be 1
    __USE_SQLITE3__ is what -DPACKAGE_SQLITE=<value> is, default to be not defined.
    __USE_POSTGRE__ is what -DPACKAGE_POSTGRESQL=<value> is, default to be not defined.
    __DEFAULT_DB__  is what -DPACKAGE_DEFAULT_DB=<value> is, default to be 1

### SEE ALSO

    db_close(3), db_status(3), valid_database(4)

