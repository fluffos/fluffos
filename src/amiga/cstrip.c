#include <stdio.h>
#include <stdlib.h>

main(int argc, char *argv[])
{
    FILE *FIn;
    unsigned int c;
    int mode = 0;

    if (argc != 2) {
	fprintf(stderr, "Error: no input filename given.\n");
	exit(0);
    }
    if (!(FIn = fopen(argv[1], "rb"))) {
	fprintf(stderr, "Error opening %s\n", argv[1]);
	exit(0);
    }
    c = fgetc(FIn);
    while (!feof(FIn)) {
	switch (mode) {
	case 0:
	    if (c == '/')
		mode = 1;
	    else
		fputc(c, stdout);
	    break;
	case 1:
	    if (c == '*')
		mode = 2;
	    else {
		mode = 0;
		fputc('/', stdout);
		fputc(c, stdout);
	    }
	    break;
	case 2:
	    if (c == '*')
		mode = 3;
	    break;
	case 3:
	    if (c == '/')
		mode = 0;
	    else
		mode = 2;
	    break;
	}
	c = fgetc(FIn);
    }
    fclose(FIn);
}
