#include <command.h>

int
main(string arg)
{
	write("Bye.\n");
	previous_object()->remove();
	return 1;
}
