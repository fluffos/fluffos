/*
 * replace.c:
 *   search & replace, from stdin or file, to stdout
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Use a 4K temporary buffer
 */
#define MAXBUFSIZE 4096

/*
 * Boyer-Moore string search algorithm
 */
static char *search(char *spc, int slen, char *pat, int plen)
{
    int skip[256];
    int i, j, t;

    for (i = 0; i < 256; i++)
	skip[i] = plen;
    for (i = 0; i < plen; i++)
	skip[pat[i]] = plen - i - 1;

    for (i = plen - 1, j = plen - 1; j > 0; i--, j--)
	while (spc[i] != pat[j]) {
	    t = skip[spc[i]];
	    i += plen - j > t ? plen - j : t;
	    if (i >= slen)
		return NULL;
	    j = plen - 1;
	}
    return spc + i;
}

int main(int argc, char *argv[])
{
    FILE *FIn;
    int plen, slen, rlen;
    char *buf;
    char *pbuf, *sbuf;
    int r, w;

    /*
     * process command line args
     */
    if (argc < 3 || argc > 4) {
	fprintf(stderr, "Syntax: replace search_str replace_str [src_file]\n");
	exit(0);
    }
    /*
     * get input
     */
    if (argc == 3)
	FIn = stdin;
    else {
	if (!(FIn = fopen(argv[3], "rb"))) {
	    fprintf(stderr, "Error opening %s\n", argv[3]);
	    exit(0);
	}
    }

    /*
     * set up
     */
    plen = strlen(argv[1]);	/* search pattern length */
    rlen = strlen(argv[2]);	/* replacement string length */
    slen = plen + MAXBUFSIZE;
    pbuf = NULL;
    w = 0;
    buf = (char *) malloc(slen + 1);
    if (!buf) {
	fprintf(stderr, "Error allocating memory for buffer.\n");
	exit(0);
    }
    /*
     * use small buffers for lower memory usage
     */
    r = fread(buf, 1, slen, FIn);
    while (r > 0) {
	/*
	 * ensure buffer is null terminated
	 */
	buf[r] = '\0';

	/*
	 * scan buffer for the pattern
	 */
	pbuf = buf;
	while (sbuf = search(pbuf, r, argv[1], plen)) {
	    /*
	     * write characters from pbuf to sbuf -1
	     */
	    w = sbuf - pbuf;
	    if (w) {
		fwrite(pbuf, 1, w, stdout);
		r -= w;
	    }
	    /*
	     * write replacement string
	     */
	    fwrite(argv[2], 1, rlen, stdout);

	    /*
	     * keep searching
	     */
	    pbuf = sbuf + plen;
	    r -= plen;
	}

	/*
	 * Write out remaining chars in excess of plen
	 */
	if (r > plen) {
	    fwrite(pbuf, 1, r - plen, stdout);
	    pbuf += r - plen;
	    w = plen;
	} else {
	    w = r;
	}

	/*
	 * Copy plen chars to beginning of buffer
	 */
	if (w) {
	    strncpy(buf, pbuf, w);
	}
	if (feof(FIn))
	    break;

	r = fread(buf + w, 1, slen - w, FIn);
	if (r >= 0) {
	    w += r;
	    r = w;
	}
    }

    /*
     * Write out remaining chars in buffer
     */
    if (w) {
	fwrite(buf, 1, w, stdout);
    }
    /*
     * Clean up
     */
    free(buf);

    fflush(stdout);

    if (argc == 4)
	fclose(FIn);

    return 0;
}
