#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/utime.h>

int main(int argc, char *argv[]) {
	int i, handle;

	for (i = 1; i <= argc; i++) {
		if (access(argv[i], 0) == -1) {
			handle = open(argv[i], _O_RDWR | _O_CREAT, _S_IREAD | _S_IWRITE);
			close(handle);
		} else {
			utime(argv[i], NULL);
		}
	}
	return 0;
}
