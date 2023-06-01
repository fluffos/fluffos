#include <string>

// Init everything driver needs.
struct event_base* init_main(std::string config_file);
void setup_signal_handlers();
std::string get_argument(unsigned int pos, int argc, char **argv);
