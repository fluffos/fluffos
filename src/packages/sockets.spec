package sockets;

/*
 * socket efuns
 */
    int socket_create(int, string, string | void);
    int socket_bind(int, int);
    int socket_listen(int, string);
    int socket_accept(int, string, string);
    int socket_connect(int, string, string, string);
    int socket_write(int, mixed, string | void);
    int socket_close(int);
    int socket_release(int, object, string);
    int socket_acquire(int, string, string, string);
    string socket_error(int);
    string socket_address(int | object);
    void dump_socket_status();
