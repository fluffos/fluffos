---
layout: default
title: db / db_commit
---

### NAME

    db_commit() - commits the last transaction

### SYNOPSIS

    int db_commit(int handle );

### DESCRIPTION

    For transactional databases this will commit the last set of actions.

    Returns 1 on success, 0 otherwise

### NOTES

    Not yet implemented!

### SEE ALSO

    db_exec(3), db_rollback(3), valid_database(4)

