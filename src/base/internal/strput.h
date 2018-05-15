#ifndef SRC_BASE_STRPUT_H_
#define SRC_BASE_STRPUT_H_

/* equivalent to strcpy(x, y); return x + strlen(y), but faster and safer */
/* Code like:
 *
 * char buf[256];
 * strcpy(buf, ...);
 * strcat(buf, ...);
 * strcat(buf, ...);
 *
 * Should be replaced with:
 *
 * char buf[256];
 * char *p, *end = EndOf(buf);
 * p = strput(buf, end, ...);
 * p = strput(p, end, ...);
 * p = strput(p, end, ...);
 */
char *strput(char *, char *, const char *);
char *strput_int(char *, char *, int);

#endif /* SRC_BASE_STRPUT_H_ */
