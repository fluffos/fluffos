void do_tests() {
#if defined(__OPCPROF__) || defined(__OPCPROF_2D__)
    opcprof();
    opcprof("/opc");
#endif
}
