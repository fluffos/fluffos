#include "spec.h"

int db_close(int);
int db_commit(int);
int db_connect(string, string);
mixed db_exec(int, string);
mixed *db_fetch(int, int);
int db_rollback(int);

