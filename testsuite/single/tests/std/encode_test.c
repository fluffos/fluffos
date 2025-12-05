void do_tests() {
    string encoded = json_encode("\e你好");
    write("Encoded: " + encoded + "\n");
    write("Encoded length: " + strlen(encoded) + "\n");
    write("Encoded bytes: ");
    for(int i = 0; i < strlen(encoded); i++) {
        write(sprintf("%02x ", encoded[i]));
    }
    write("\n");
}
