#include "spec.h"

void async_read(string, function);
void async_write(string, string, int, function);
#ifdef linux
void async_getdir(string, function);
#endif
