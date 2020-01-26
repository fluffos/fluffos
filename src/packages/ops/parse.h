#ifndef PARSE_H
#define PARSE_H

int parse(const std::string, svalue_t *, const std::string, svalue_t *, int);
char *process_string(char *);
svalue_t *process_value(char *);
char *break_string(char *, int, svalue_t *);

#endif
