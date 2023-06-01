void do_tests() {
    mixed *ports = sys_network_ports();
    int *port;

    ASSERT_EQ(1, arrayp(ports));
    ASSERT_EQ(4, sizeof(ports));

    port = filter(ports, (: $1[1] == 4000 :))[0];
    ASSERT_EQ("telnet", port[0]);
    ASSERT_EQ(4000, port[1]);
    ASSERT_EQ(0, port[2]); // no tls

    port = filter(ports, (: $1[1] == 4001 :))[0];
    ASSERT_EQ("websocket", port[0]);
    ASSERT_EQ(4001, port[1]);
    ASSERT_EQ(0, port[2]); // no tls

    port = filter(ports, (: $1[1] == 4002 :))[0];
    ASSERT_EQ("websocket", port[0]);
    ASSERT_EQ(4002, port[1]);
    ASSERT_EQ(1, port[2]); // tls

    port = filter(ports, (: $1[1] == 4003 :))[0];
    ASSERT_EQ("telnet", port[0]);
    ASSERT_EQ(4003, port[1]);
    ASSERT_EQ(1, port[2]); // tls
}