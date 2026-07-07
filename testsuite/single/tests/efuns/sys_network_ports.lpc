void do_tests() {
    mixed *ports = sys_network_ports();
    int *port;

    ASSERT_EQ(1, arrayp(ports));
    ASSERT_EQ(4, sizeof(ports));

    port = filter(ports, (: $1[2] == 4000 :))[0];
    ASSERT_EQ(1, port[0]);
    ASSERT_EQ("telnet", port[1]);
    ASSERT_EQ(4000, port[2]);
    ASSERT_EQ(0, port[3]); // no tls

    port = filter(ports, (: $1[2] == 4001 :))[0];
    ASSERT_EQ(2, port[0]);
    ASSERT_EQ("websocket", port[1]);
    ASSERT_EQ(4001, port[2]);
    ASSERT_EQ(0, port[3]); // no tls

    port = filter(ports, (: $1[2] == 4002 :))[0];
    ASSERT_EQ(3, port[0]);
    ASSERT_EQ("websocket", port[1]);
    ASSERT_EQ(4002, port[2]);
    ASSERT_EQ(1, port[3]); // tls

    port = filter(ports, (: $1[2] == 4003 :))[0];
    ASSERT_EQ(4, port[0]);
    ASSERT_EQ("telnet", port[1]);
    ASSERT_EQ(4003, port[2]);
    ASSERT_EQ(1, port[3]); // tls
}