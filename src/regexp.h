#ifndef _REGEXP_H_
#define _REGEXP_H_

/*
 * Definitions etc. for regexp(3) routines.
 *
 * Caveat:  this is V8 regexp(3) [actually, a reimplementation thereof],
 * not the System V one.
 */

#define EFUN_REGEXP 1
#define ED_REGEXP   2

#define NSUBEXP  10
typedef struct regexp {
    char *startp[NSUBEXP];
    char *endp[NSUBEXP];
    char regstart;		/* Internal use only. */
    char reganch;		/* Internal use only. */
    char *regmust;		/* Internal use only. */
    int regmlen;		/* Internal use only. */
    char program[1];		/* Unwarranted chumminess with compiler. */
}      regexp;

extern int regnarrate;
extern int regexp_user;
extern char *regexp_error;

void regdump PROT((regexp *));
regexp *regcomp PROT((char *, int));
int regexec PROT((regexp *, char *));
char *regsub PROT((regexp *, char *, char *, int));

#endif
