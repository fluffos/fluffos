#include "spec.h"

/*
 * socket efuns
 */
    int socket_create(int, string | function, string | function | void);
    int socket_bind(int, int);
    int socket_listen(int, string | function);
    int socket_accept(int, string | function, string | function);
    int socket_connect(int, string, string | function, string | function);
    int socket_write(int, mixed, string | void);
    int socket_close(int);
    int socket_release(int, object, string | function);
    int socket_acquire(int, string | function, string | function, string | function);
    string socket_error(int);
    string socket_address(int | object);
    string dump_socket_status();
