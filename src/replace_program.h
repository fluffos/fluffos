#ifndef _REPLACE_PROGRAM_H_
#define _REPLACE_PROGRAM_H_

#include "lpc_incl.h"

int replace_program_pending PROT((object_t *));
void replace_programs PROT((void));

typedef struct replace_ob_s {
    object_t *ob;
    program_t *new_prog;
    unsigned short var_offset;
    struct replace_ob_s *next;
} replace_ob_t;

extern replace_ob_t *obj_list_replace;

#endif
