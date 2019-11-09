// In mainlib.cc
extern "C" {
int driver_main(int argc, char **argv);
}
int main(int argc, char **argv) { driver_main(argc, argv); }