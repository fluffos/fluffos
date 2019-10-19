---
layout: default
title: db / db_rollback
---

### NAME

    db_rollback() - rollback the last transaction

### SYNOPSIS

    int db_rollback( int );

### DESCRIPTION

    For transactional databases this will rollback the last set of actions.

    Returns 1 on success, 0 otherwise

### NOTES

    Not yet implemented!

### SEE ALSO

    db_commit(3), db_exec(3), valid_database(4)

