#ifndef _COMPILER_SHARED_H_
#define _COMPILER_SHARED_H_

#include "config.h"
#define _YACC_
#include "lint.h"
#include <string.h>
#include <stdio.h>
#ifndef LATTICE
#include <memory.h>
#endif
#if defined(__386BSD__) || defined(LATTICE)
#include <stdlib.h>
#endif
#if defined(sun)
#include <alloca.h>
#endif
#if defined(NeXT) || defined(SunOS_5)
#include <stdlib.h>
#endif
#ifdef sun
#include <malloc.h>
#endif

#include "mudlib_stats.h"
#include "interpret.h"
#include "object.h"
#include "exec.h"
#include "instrs.h"
#include "switch.h"
#include "incralloc.h"
#include "applies.h"

#define YYMAXDEPTH    600

/* NUMPAREAS ares are saved with the program code after compilation,
 * the rest are only temporary.
 */
#define A_PROGRAM		0	/* executable code */
#define A_FUNCTIONS		1	/* table of functions */
#define A_STRINGS		2	/* table of strings */
#define A_VARIABLES		3	/* table of variables */
#define A_LINENUMBERS		4	/* linenumber information */
#define A_INHERITS		5	/* table of inherited progs */
#define A_ARGUMENT_TYPES	6	/* */
#define A_ARGUMENT_INDEX	7	/* */
#define NUMPAREAS		8
#define A_CASE_NUMBERS		8	/* case labels for numbers */
#define A_CASE_STRINGS		9	/* case labels for strings */
#define A_CASE_LABELS		10	/* used to build switch tables */
#define A_STRING_NEXT		11	/* next prog string in hash chain */
#define A_STRING_REFS		12	/* reference count of prog string */
#define A_INCLUDES		13	/* list of included files */
#define A_PATCH			14	/* for save_binary() */
#define A_INITIALIZER           15
#define NUMAREAS		16

extern struct mem_block mem_block[NUMAREAS];

#define BREAK_ON_STACK          0x40000
#define BREAK_FROM_CASE         0x80000

#define SWITCH_STACK_SIZE  200
#define EXPR_STACK_SIZE  200

#define BREAK_DELIMITER       -0x200000
#define CONTINUE_DELIMITER    -0x40000000

/* make sure that this struct has a size that is a power of two */
struct case_heap_entry {
    int key;
    short addr;
    short line;
};

#define CASE_HEAP_ENTRY_ALIGN(offset) offset &= -((int)sizeof(struct case_heap_entry))

/*
 * Some good macros to have.
 */

#define BASIC_TYPE(e,t) ((e) == TYPE_ANY ||\
                         (e) == (t) ||\
                         (t) == TYPE_ANY)

#define TYPE(e,t) (BASIC_TYPE((e) & TYPE_MOD_MASK, (t) & TYPE_MOD_MASK) ||\
                (((e) & TYPE_MOD_POINTER) && ((t) & TYPE_MOD_POINTER) &&\
                BASIC_TYPE((e) & (TYPE_MOD_MASK & ~TYPE_MOD_POINTER),\
                (t) & (TYPE_MOD_MASK & ~TYPE_MOD_POINTER))))

#define FUNCTION(n) ((struct function *)mem_block[A_FUNCTIONS].block + (n))
#define VARIABLE(n) ((struct variable *)mem_block[A_VARIABLES].block + (n))

#if !defined(__alpha) && !defined(cray)
#define align(x) (((x) + 3) & ~3)
#else
#define align(x) (((x) + 7) & ~7)
#endif

#define SOME_NUMERIC_CASE_LABELS 0x40000
#define NO_STRING_CASE_LABELS    0x80000

/* inlines - if we're luckly, they'll get honored. */
INLINE static void realloc_mem_block PROT((struct mem_block *, int));
INLINE static void add_to_mem_block PROT((int, char *, int));
INLINE static void insert_in_mem_block PROT((int, int, int));
INLINE static void pop_arg_stack PROT((int));
INLINE static int get_argument_type PROT((int, int));

INLINE
static void realloc_mem_block(m, size)
    struct mem_block *m;
    int size;
{
    while (size > m->max_size) {
	m->max_size <<= 1;
	m->block = (char *)
	    DREALLOC((char *) m->block, m->max_size, 49, "realloc_mem_block");
    }
}

INLINE
static void add_to_mem_block(n, data, size)
    int n, size;
    char *data;
{
    struct mem_block *mbp = &mem_block[n];

    if (mbp->current_size + size > mbp->max_size)
	realloc_mem_block(mbp, mbp->current_size + size);
    if (data)
	memcpy(mbp->block + mbp->current_size, data, size);
    mbp->current_size += size;
}

INLINE
static void insert_in_mem_block(n, where, size)
    int n, where, size;
{
    struct mem_block *mbp = &mem_block[n];
    char *p;

    if (mbp->current_size + size > mbp->max_size)
	realloc_mem_block(mbp, mbp->current_size + size);
    /* memcpy can't handle overlapping blocks on some systems */
    p = mbp->block + mbp->current_size;
    while (p-- > mbp->block + where)
	*(p + size) = *p;
    mbp->current_size += size;
}

/*
 * Pop the argument type stack 'n' elements.
 */
INLINE
static void pop_arg_stack(n)
    int n;
{
    type_of_arguments.current_size -= sizeof(unsigned short) * n;
}

/*
 * Get type of argument number 'arg', where there are
 * 'n' arguments in total in this function call. Argument
 * 0 is the first argument.
 */
INLINE
static int get_argument_type(arg, n)
    int arg, n;
{
    return
	((unsigned short *)
       (type_of_arguments.block + type_of_arguments.current_size))[arg - n];
}

#endif
