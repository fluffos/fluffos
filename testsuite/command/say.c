#include <command.h>

#ifdef __NO_ENVIRONMENT__
#define say(x) shout(x)
#endif

int main(string arg)
{
	say((string)previous_object()->query_name()
		+ " says: " +  arg + "\n");
	return 1;
}
