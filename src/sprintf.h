#ifndef SPRINTF_H
#define SPRINTF_H

void svalue_to_string PROT((svalue_t *, outbuffer_t *, int, int, int));
char *string_print_formatted PROT((char *, int, svalue_t *));

#endif
