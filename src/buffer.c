/* buffer.c for MudOS 0.9.x by John Garnett, 1993/11/07 */

#include <stdio.h>
#include "config.h"
#include "buffer.h"
#include "crctab.h"
#include "lint.h"

extern int max_buffer_size;

struct buffer null_buf = {
	0,  /* size */
	1   /* Ref count, which will ensure that it will never be deallocated */
};

INLINE struct buffer *
null_buffer()
{
	null_buf.ref++;
	return &null_buf;
}

INLINE void
free_buffer(b)
struct buffer *b;
{
	b->ref--;
	/* don't try to free the null_buffer (ref count might overflow) */
	if ((b->ref > 0) || (b == &null_buf)) {
		return;
	}
	FREE((char *)b);
}

struct buffer *
allocate_buffer(size)
int size;
{
	struct buffer *buf;

#ifndef DISALLOW_BUFFER_TYPE
	if ((size < 0) || (size > max_buffer_size)) {
		error("Illegal buffer size.\n");
	}
	if (size == 0) {
		return null_buffer();
	}
	/* using calloc() so that memory will be zero'd out when allocated */
	buf = (struct buffer *)CALLOC(sizeof(struct buffer) + size - 1, 1);
	buf->size = size;
	buf->ref = 1;
	return buf;
#else
	return NULL;
#endif
}

int
write_buffer(buf, start, str, theLength)
struct buffer *buf;
char *str;
int start, theLength;
{
	int size;

	size = buf->size;
	if (start < 0) {
		start = size + start;
		if (start < 0) {
			return 0;
		}
	}
	/* can't write past the end of the buffer since we can't reallocate the
	   buffer here (no easy way to propagate back the changes to the caller
	*/
	if ((start + theLength) > size) {
		return 0;
	}
	memcpy(buf->item + start, str, theLength);
	return 1;
}

char *
read_buffer(b,start,len,rlen)
struct buffer *b;
int start,len;
int *rlen;
{
	char *str;
	int size;

	if (len < 0)
		return 0;

	size = b->size;
	if (start < 0) {
		start = size + start;
		if (start < 0) {
			return 0;
		}
	}

    if (len == 0) len = size;

	if (start >= size) {
		return 0;
    }
	if ((start + len) > size) {
		len = (size - start);
	}

    str = (char *)DXALLOC(len + 1, 42, "read_buffer: str");

	memcpy(str, b->item + start, len);

    /*
     * The string has to end to '\0'!!!
     */
    str[len] = '\0';
	if ((*rlen = strlen(str)) != len) {
		char *p;

		p = str;
		str = string_copy(p);
		*rlen = strlen(str);
		FREE(p);
	}

    return str;
}
