#include "spec.h"

void async_read(string, function);
void async_write(string, string, int, function);
#ifdef __linux__
void async_getdir(string, function);
#endif
#ifdef PACKAGE_DB
void async_db_exec(int, string, function);
#endif
