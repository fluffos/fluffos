#ifndef _INSTRS_H_
#define _INSTRS_H_

/*
 * Information about all instructions. This is not really needed as the
 * automatically generated efun_arg_types[] should be used.
 */

struct instr {
    short max_arg, min_arg;	/* Can't use char to represent -1 */
    short type[2];		/* need a short to hold the biggest type flag */
    short Default;
    short ret_type;
    char *name;
    int arg_index;
};

extern struct instr instrs[512];

#endif
