/* buffer.h by John Garnett, 1993/11/07 */

#ifndef _BUFFER_H_
#define _BUFFER_H_

struct buffer {
    /* first two elements of struct must be 'ref' followed by 'size' */
    unsigned short ref;
    unsigned int size;
    unsigned char item[1];
};

#endif
