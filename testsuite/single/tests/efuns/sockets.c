#ifdef __PACKAGE_SOCKETS__

// temporial hack
#define DATAGRAM 2
#define EESUCCESS 1

void read_callback(int s) {
    // We didn't listen/bind our socket, so should not receive callback.
    // This is by design.
    ASSERT_EQ(1, 0);
}

void close_callback(int s)
{
    // socket_close on an UDP socket should not call callback.
    // This is by design.
    ASSERT_EQ(1, 0);
}

void do_tests() {
    int s;
    int error;
    string address;

    s = socket_create(DATAGRAM, "read_callback", "close_callback");

    ASSERT(s >= 0);

    write("Created socket descriptor " + s + "\n");

    address = "127.0.0.1 4000";
	error = socket_write(s, "test test", address);
	ASSERT_EQ(EESUCCESS, error);
#ifdef __IPV6__
    address = "::1 4000";
	error = socket_write(s, "test test", address);
	ASSERT_EQ(EESUCCESS, error);
#endif
    ASSERT_EQ(EESUCCESS, socket_close(s));
}


#endif