#include "spec.h"

#ifndef PACKAGE_SOCKETS
!The external package cannot compile unless PACKAGE_SOCKETS is defined
#endif

int external_start(int, string, string | function, string | function, string | function | void);
