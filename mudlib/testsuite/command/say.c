#include <command.h>

main(string arg)
{
	say((string)previous_object()->query_name()
		+ " says: " +  arg + "\n");
	return 1;
}
