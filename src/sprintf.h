#ifndef SPRINTF_H
#define SPRINTF_H

void svalue_to_string PROT((struct svalue *, char **, int, int, int, int));
char *string_print_formatted PROT((char *, int, struct svalue *));

#endif
