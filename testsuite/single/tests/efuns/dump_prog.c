void internal(int flag) {
    string fn;

    switch (flag) {
        case 1:
            fn = "/PROG_DUMP_1";
            break;
        case 2:
            fn = "/PROG_DUMP_2";
            break;
        case 3:
            fn = "/PROG_DUMP_3";
            break;
        default:
            dump_prog(this_object());
            return;
    }
    dump_prog(this_object(), flag, fn);
}

void do_tests() {
    for(int i = 0; i < 4; i++) {
        internal(i);
    }
}
