/* buffer.h by John Garnett, 1993/11/07 */

struct buffer {
	/* first two elements of struct must be 'size' followed by 'ref' */
    unsigned int size;
    short ref;
    unsigned char item[1];
};
