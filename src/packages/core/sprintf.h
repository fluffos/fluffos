#ifndef SPRINTF_H
#define SPRINTF_H

void svalue_to_string(struct svalue_t *, struct outbuffer_t *, int, int, int);
char *string_print_formatted(const char *, int, struct svalue_t *);

#endif
