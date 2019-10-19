---
layout: default
title: db / db_connect
---

### NAME

    db_connect() - close the database connection

### SYNOPSIS

    int  db_connect( string host, string db ); int db_connect( string host,
    string db, string user );  int  db_connect(  string  host,  string  db,
    string user, int type );

### DESCRIPTION

    Creates  a  new  connection  to the database db on the given host.  The
    connection uses either the given user or a compile time value as  login
    id.  type may be used to choose the type of database server. Valid val‐
    ues depend on compile time settings should be made available through  a
    corresponding header file within the mudlib.

    Returns a handle to the new connection on success, 0 otherwise

### SEE ALSO

    db_close(3), db_status(3), valid_database(4)

