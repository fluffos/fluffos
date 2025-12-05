void do_tests() {
    int code1 = 0x4F60;  // 你
    int code2 = 0x597D;  // 好
    string s1, s2;
    buffer b1, b2;
    string msg;

    // Test sprintf
    s1 = sprintf("%c", code1);
    s2 = sprintf("%c", code2);

    write(sprintf("sprintf('%%c', 0x%04X) len=%d\n", code1, strlen(s1)));
    write(sprintf("sprintf('%%c', 0x%04X) len=%d\n", code2, strlen(s2)));

    // Show hex bytes
    msg = "s1 bytes: ";
    for(int i = 0; i < strlen(s1); i++) msg += sprintf("%02x ", s1[i]);
    write(msg + "\n");

    msg = "s2 bytes: ";
    for(int i = 0; i < strlen(s2); i++) msg += sprintf("%02x ", s2[i]);
    write(msg + "\n");

    // Test string_encode
    b1 = string_encode(s1, "utf-8");
    b2 = string_encode(s2, "utf-8");

    msg = sprintf("buffer1 size=%d bytes: ", sizeof(b1));
    for(int i = 0; i < sizeof(b1); i++) msg += sprintf("%02x ", b1[i]);
    write(msg + "\n");

    msg = sprintf("buffer2 size=%d bytes: ", sizeof(b2));
    for(int i = 0; i < sizeof(b2); i++) msg += sprintf("%02x ", b2[i]);
    write(msg + "\n");
}
