#ifndef MAKE_FUNC_H
#define MAKE_FUNC_H

#define MAX_FUNC        2048  /* If we need more than this we're in trouble! */

#define VOID 		1
#define INT		2
#define STRING		3
#define OBJECT		4
#define MAPPING		5
#define MIXED		6
#define UNKNOWN		7
#define FLOAT		8
#define FUNCTION	9
#define BUFFER         10

extern int num_buff;
extern int op_code, efun_code;
extern char *oper_codes[MAX_FUNC];
extern char *efun_codes[MAX_FUNC];
extern char *efun_names[MAX_FUNC];
extern const char *key[MAX_FUNC], *buf[MAX_FUNC];

extern int arg_types[1000], last_current_type;

const char *ctype (int);
const char *etype (int);

#endif
