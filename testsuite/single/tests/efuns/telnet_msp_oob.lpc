void do_tests() {
  string msg = "!!SOUND(cow.wav L=2 V=100)";
  telnet_msp_oob(msg);
  if (this_player()) {
    this_player()->msp_oob(msg);
  }
}
