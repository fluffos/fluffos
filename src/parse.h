#ifndef PARSE_H
#define PARSE_H

int parse PROT((char *, struct svalue *, char *, struct svalue *, int));
char *process_string PROT((char *));
struct svalue *process_value PROT((char *));
char *break_string PROT((char *, int, struct svalue *));

#endif
