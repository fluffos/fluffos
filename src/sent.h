#ifndef _SENT_H_
#define _SENT_H_

struct sentence {
    char *verb;
    struct object *ob;
    char *function;
    struct sentence *next;
    int flags;
};

#define V_SHORT     1
#define V_NOSPACE	2

#endif
