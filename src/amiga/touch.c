/*
 * touch.c:
 *   touch file's timestamp, setting it to the current system time
 *
 *   Requires OS 2.04 (v37) or better
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <exec/libraries.h>
#include <dos/dos.h>
#include <clib/dos_protos.h>

int main(int argc, char *argv[]) {
    if (argc > 1) {
        extern struct Library *DOSBase;
        struct DateStamp ds;

        if (DOSBase->lib_Version >= 37) {
            while (--argc) {
                DateStamp((struct DateStamp *)&ds);
                SetFileDate(argv[argc], (struct DateStamp *)&ds);
            }
        }
    }
}
