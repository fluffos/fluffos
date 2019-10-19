---
layout: default
title: master / valid_database
---

### NAME

    valid_database - protects the database efunctions.

### SYNOPSIS

    mixed valid_database( object caller, string function, mixed *info );

### DESCRIPTION

    Each of the database efunctions calls valid_database() prior to execut‐
    ing.  If valid_database returns 0, then the database  efunction  fails.
    If  valid_database  returns  1, then the database efunction attempts to
    succeed.  The first argument 'caller' is the  object  that  called  the
    database  efunction.   The  second argument is the name of the database
    efunction that is being called (e.g. db_connect() or db_commit()).  The
    third  argument,  if applicable, is an array of additional information.
    Currently this is used for efun::db_connect with  the  form  ({  string
    database,  string  host, string user }) and for efund::dh_exec with the
    form ({ string sql_query })

    Returns 0 if the use of the efun is to be disallowed, for efun::db_con‐
    nect  it  should  return  the password to be used for the connect and 1
    otherwise

### SEE ALSO

    db_connect(3), db_exec(3)

