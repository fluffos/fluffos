#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN	1024

int main(int argc, char *argv[]) {
	char command[MAX_LEN];
	int i;

	if (strcmp(argv[1], "rcu"))
		return -1;
	if (!strstr(argv[2], ".lib"))
		return -1;
	strcpy(command, "lib ");
	for (i = 3; i < argc; i++) {
		if (!strstr(argv[i], ".obj"))
			return -1;
		strcat(command, argv[i]);
		strcat(command, " ");
	}
	strcat(command, "/OUT:");
	strcat(command, argv[2]);
	system(command);
	return 0;
}
