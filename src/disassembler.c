/*
 * Dump information about a program, optionally disassembling it.
 */

#include "std.h"
#include "lpc_incl.h"
#include "efuns_incl.h"
#include "simul_efun.h"
#include "comm.h"
#include "swap.h"
#include "lex.h"

#ifdef F_DUMP_PROG
void dump_prog PROT((struct program *, char *, int));
static void disassemble PROT((FILE *, char *, int, int, struct program *));
static char *disassem_string PROT((char *));
static int short_compare PROT((unsigned short *, unsigned short *));
static void dump_line_numbers PROT((FILE *, struct program *));

void
f_dump_prog P2(int, num_arg, int, instruction)
{
    struct program *prog;
    char *where;
    int d;
    struct object *ob;

    if (num_arg == 2) {
	ob = sp[-1].u.ob;
	d = sp->u.number;
	where = 0;
    } else if (num_arg == 3) {
	ob = sp[-2].u.ob;
	d = sp[-1].u.number;
	where = (sp->type == T_STRING) ? sp->u.string : 0;
    } else {
	ob = sp->u.ob;
	d = 0;
	where = 0;
    }
    pop_n_elems(num_arg);
    if (!(prog = ob->prog)) {
	add_message("No program for object.\n");
    } else {
	if (!where) {
	    where = "/PROG_DUMP";
	}
	dump_prog(prog, where, d);
    }
    push_number(0);
}

/* Current flags:
 * 1 - do disassembly
 * 2 - dump line number table
 */
void
dump_prog P3(struct program *, prog, char *, fn, int, flags)
{
    char *fname;
    FILE *f;
    int i, j;

    fname = check_valid_path(fn, current_object, "dumpallobj", 1);
    if (!fname) {
	add_message("Invalid path '%s' for writing.\n", fn);
	return;
    }
    f = fopen(fname, "w");
    if (!f) {
	add_message("Unable to open '%s' for writing.\n", fname);
	return;
    }
    add_message("Dumping to %s ...", fname);

    fprintf(f, "NAME: %s\n", prog->name);
    fprintf(f, "INHERITS:\n");
    fprintf(f, "\tname                    fio    vio\n");
    fprintf(f, "\t----------------        ---    ---\n");
    for (i = 0; i < (int) prog->p.i.num_inherited; i++)
	fprintf(f, "\t%-20s  %5d  %5d\n",
		prog->p.i.inherit[i].prog->name,
		(int)prog->p.i.inherit[i].function_index_offset,
		(int)prog->p.i.inherit[i].variable_index_offset
	    );
    fprintf(f, "PROGRAM:");
    for (i = 0; i < (int) prog->p.i.program_size; i++) {
	if (i % 16 == 0)
	    fprintf(f, "\n\t%04x: ", (unsigned int) i);
	fprintf(f, "%02d ", (unsigned char) prog->p.i.program[i]);
    }
    fputc('\n', f);
    fprintf(f, "FUNCTIONS:\n");
    fprintf(f, "      name        offset    fio   flags   # locals  # args\n");
    fprintf(f, "      ----------- ------    ---  -------  --------  ------\n");
    for (i = 0; i < (int) prog->p.i.num_functions; i++) {
	char sflags[8];
	int flags;

	flags = prog->p.i.functions[i].flags;
	sflags[6] = '\0';
	sflags[0] = (flags & NAME_INHERITED) ? 'i' : '-';
	sflags[1] = (flags & NAME_UNDEFINED) ? 'u' : '-';
	sflags[2] = (flags & NAME_STRICT_TYPES) ? 's' : '-';
	sflags[3] = (flags & NAME_PROTOTYPE) ? 'p' : '-';
	sflags[4] = (flags & NAME_DEF_BY_INHERIT) ? 'd' : '-';
	sflags[5] = (flags & NAME_ALIAS) ? 'a' : '-';
	fprintf(f, "%4d: %-12s %5d  %5d  %7s  %8d  %6d\n",
		i,
		prog->p.i.functions[i].name,
		(int)prog->p.i.functions[i].offset,
		prog->p.i.functions[i].function_index_offset,
		sflags,
		prog->p.i.functions[i].num_local,
		prog->p.i.functions[i].num_arg
	    );
    }
    fprintf(f, "VARIABLES:\n");
    for (i = 0; i < (int) prog->p.i.num_variables; i++)
	fprintf(f, "%4d: %-12s\n", i,
		prog->p.i.variable_names[i].name);
    fprintf(f, "STRINGS:\n");
    for (i = 0; i < (int) prog->p.i.num_strings; i++) {
	fprintf(f, "%4d: ", i);
	for (j = 0; j < 32; j++) {
	    char c;

	    if (!(c = prog->p.i.strings[i][j]))
		break;
	    else if (c == '\n')
		fprintf(f, "\\n");
	    else
		fputc(c, f);
	}
	fputc('\n', f);
    }

    if (flags & 1) {
	fprintf(f, "\n;;;  *** Disassembly ***\n");
	disassemble(f, prog->p.i.program, 0, prog->p.i.program_size, prog);
    }
    if (flags & 2) {
	fprintf(f, "\n;;;  *** Line Number Info ***\n");
	dump_line_numbers(f, prog);
    }
    add_message("done.\n");
    fclose(f);
}

static char *disassem_string P1(char *, str)
{
    static char buf[30];
    char *b;
    int i;

    if (!str)
	return "0";

    b = buf;
    for (i = 0; i < 29; i++) {
	if (!str[i])
	    break;
	if (str[i] == '\n') {
	    *b++ = '\\';
	    *b++ = 'n';
	} else {
	    *b++ = str[i];
	}
    }
    *b++ = 0;
    return buf;
}

#define FUNS     prog->p.i.functions
#define NUM_FUNS prog->p.i.num_functions
#define VARS     prog->p.i.variable_names
#define NUM_VARS prog->p.i.num_variables
#define STRS     prog->p.i.strings
#define NUM_STRS prog->p.i.num_strings

static int
short_compare P2(unsigned short *, a, unsigned short *, b)
{
    return (int) (*a - *b);
}

static void
disassemble P5(FILE *, f, char *, code, int, start, int, end, struct program *, prog)
{
    int i, instr, iarg, is_efun;
    unsigned short sarg;
    unsigned short offset;
    char *pc, buff[256];
    int next_func;

    short *offsets;

    if (start == 0) {
	/* sort offsets of functions */
	offsets = (short *) malloc(NUM_FUNS * 2 * sizeof(short));
	for (i = 0; i < (int) NUM_FUNS; i++) {
	    if (!FUNS[i].offset ||
		(FUNS[i].flags & (NAME_ALIAS | NAME_INHERITED)))
		offsets[i * 2] = end + 1;
	    else
		offsets[i * 2] = FUNS[i].offset;
	    offsets[i * 2 + 1] = i;
	}
#ifdef _SEQUENT_
	qsort((void *) &offsets[0],
#else
	qsort((char *) &offsets[0],
#endif
	      NUM_FUNS, sizeof(short) * 2, (int (*) ()) short_compare);
	next_func = 0;
    } else {
	offsets = 0;
	next_func = -1;
    }

    pc = code + start;

    while ((pc - code) < end) {

	fprintf(f, "%04x: ", (unsigned) (pc - code));

#ifdef NEEDS_CALL_EXTRA
	if ((instr = EXTRACT_UCHAR(pc)) == F_CALL_EXTRA) {
	    fprintf(f, "call_extra+");
	    pc++;
	    instr = EXTRACT_UCHAR(pc) + 0xff;
	    is_efun = 1;
	} else {
	    is_efun = (instr >= BASE);
	}
#else
	is_efun = (instr = EXTRACT_UCHAR(pc)) >= BASE;
#endif

	pc++;
	buff[0] = 0;
	sarg = 0;

	switch (instr) {
	    /* Single numeric arg */
	case F_BRANCH:
	case F_BRANCH_WHEN_ZERO:
	case F_BRANCH_WHEN_NON_ZERO:
#ifdef F_LOR
	case F_LOR:
	case F_LAND:
#endif
	    ((char *) &sarg)[0] = pc[0];
	    ((char *) &sarg)[1] = pc[1];
	    offset = (pc - code) + (unsigned short) sarg;
	    sprintf(buff, "%04x (%04x)", (unsigned) sarg, (unsigned) offset);
	    pc += 2;
	    break;

	case F_BBRANCH_WHEN_ZERO:
	case F_BBRANCH_WHEN_NON_ZERO:
	case F_BBRANCH:
	    ((char *) &sarg)[0] = pc[0];
	    ((char *) &sarg)[1] = pc[1];
	    offset = (pc - code) - (unsigned short) sarg;
	    sprintf(buff, "%04x (%04x)", (unsigned) sarg, (unsigned) offset);
	    pc += 2;
	    break;

#ifdef F_JUMP
	case F_JUMP:
#endif
#ifdef F_JUMP_WHEN_ZERO
	case F_JUMP_WHEN_ZERO:
	case F_JUMP_WHEN_NON_ZERO:
#endif
	case F_CATCH:
	    ((char *) &sarg)[0] = pc[0];
	    ((char *) &sarg)[1] = pc[1];
	    sprintf(buff, "%04x", (unsigned) sarg);
	    pc += 2;
	    break;

	case F_AGGREGATE:
	case F_AGGREGATE_ASSOC:
	    ((char *) &sarg)[0] = pc[0];
	    ((char *) &sarg)[1] = pc[1];
	    sprintf(buff, "%d", (int)sarg);
	    pc += 2;
	    break;

	case F_CALL_FUNCTION_BY_ADDRESS:
	    ((char *) &sarg)[0] = pc[0];
	    ((char *) &sarg)[1] = pc[1];
	    pc += 2;
	    if (sarg < NUM_FUNS)
		sprintf(buff, "%-12s %5d", FUNS[sarg].name,
			(int)sarg);
	    else
		sprintf(buff, "<out of range %d>", (int)sarg);
	    pc++;
	    break;

	case F_GLOBAL_LVALUE:
	case F_GLOBAL:
	    if ((unsigned) (iarg = EXTRACT_UCHAR(pc)) < NUM_VARS)
		sprintf(buff, "%s", VARS[iarg].name);
	    else
		sprintf(buff, "<out of range %d>", iarg);
	    pc++;
	    break;

	case F_LOOP_INCR:
	    sprintf(buff, "LV%d", EXTRACT_UCHAR(pc));
	    pc++;
	    break;
	case F_WHILE_DEC:
	case F_LOOP_COND:
	case F_LOCAL:
	case F_LOCAL_LVALUE:
	    sprintf(buff, "LV%d", EXTRACT_UCHAR(pc));
	    pc++;
	    break;

	case F_STRING:
	    ((char *) &sarg)[0] = pc[0];
	    ((char *) &sarg)[1] = pc[1];
	    if (sarg < NUM_STRS)
		sprintf(buff, "\"%s\"", disassem_string(STRS[sarg]));
	    else
		sprintf(buff, "<out of range %d>", (int)sarg);
	    pc += 2;
	    break;

	case F_SIMUL_EFUN:
	    ((char *) &sarg)[0] = pc[0];
	    ((char *) &sarg)[1] = pc[1];
	    sprintf(buff, "\"%s\" %d", simuls[sarg]->name, pc[2]);
	    pc += 3;
	    break;

	case F_FUNCTION_CONSTRUCTOR:
	    switch (EXTRACT_UCHAR(pc++)) {
	    case ORIGIN_CALL_OTHER | 1:
		strcpy(buff, "<this_object call_other>");
		break;
	    case ORIGIN_CALL_OTHER:
		strcpy(buff, "<call_other>");
		break;
	    case ORIGIN_SIMUL_EFUN:
		((char *) &sarg)[0] = pc[0];
		((char *) &sarg)[1] = pc[1];
		sprintf(buff, "<simul_efun> \"%s\"", simuls[sarg]->name);
		pc += 2;
		break;
	    case ORIGIN_EFUN:
#ifdef NEEDS_CALL_EXTRA
		if ((sarg = EXTRACT_UCHAR(pc++)) == F_CALL_EXTRA) {
		    sarg = EXTRACT_UCHAR(pc++) + 0xff;
		}
#else
		sarg = EXTRACT_UCHAR(pc++);
#endif
		sprintf(buff, "<efun> %s", instrs[sarg].name);
		break;
	    case ORIGIN_LOCAL:
		((char *) &sarg)[0] = pc[0];
		((char *) &sarg)[1] = pc[1];
		pc += 2;
		if (sarg < NUM_FUNS)
		    sprintf(buff, "<local_fun> %s", FUNS[sarg].name);
		else
		    sprintf(buff, "<local_fun> <out of range %d>", (int)sarg);
		break;
	    case ORIGIN_FUNCTIONAL:
		strcpy(buff, "<functional, code follows>");
		pc+=3;
	    }
	    break;

	case F_NUMBER:
	    ((char *) &iarg)[0] = pc[0];
	    ((char *) &iarg)[1] = pc[1];
	    ((char *) &iarg)[2] = pc[2];
	    ((char *) &iarg)[3] = pc[3];
	    sprintf(buff, "%d", iarg);
	    pc += 4;
	    break;

	case F_REAL:
	    {
		float farg;

		((char *) &farg)[0] = pc[0];
		((char *) &farg)[1] = pc[1];
		((char *) &farg)[2] = pc[2];
		((char *) &farg)[3] = pc[3];
		sprintf(buff, "%f", farg);
		pc += 4;
		break;
	    }

	case F_SSCANF:
	case F_PARSE_COMMAND:
	case F_BYTE:
	case F_POP_BREAK:
	    sprintf(buff, "%d", EXTRACT_UCHAR(pc));
	    pc++;
	    break;

	case F_NBYTE:
	    sprintf(buff, "-%d", EXTRACT_UCHAR(pc));
	    pc++;
	    break;

	case F_SWITCH:
	    {
		unsigned char ttype;
		unsigned short stable, etable, def;

		ttype = EXTRACT_UCHAR(pc);
		((char *) &stable)[0] = pc[1];
		((char *) &stable)[1] = pc[2];
		((char *) &etable)[0] = pc[3];
		((char *) &etable)[1] = pc[4];
		((char *) &def)[0] = pc[5];
		((char *) &def)[1] = pc[6];
		fprintf(f, "switch\n");
		fprintf(f, "      type: %02x table: %04x-%04x deflt: %04x\n",
			(unsigned) ttype, (unsigned) stable,
			(unsigned) etable, (unsigned) def);
		/* recursively disassemble stuff in switch */
		disassemble(f, code, pc - code + 7, stable, prog);

		/* now print out table - ugly... */
		fprintf(f, "      switch table (for %04x)\n",
			(unsigned) (pc - code - 1));
		if (ttype == 0xfe)
		    ttype = 0;	/* direct lookup */
		else if (ttype >> 4 == 0xf)
		    ttype = 1;	/* normal int */
		else
		    ttype = 2;	/* string */

		pc = code + stable;
		if (ttype == 0) {
		    i = 0;
		    while (pc < code + etable - 4) {
			((char *) &sarg)[0] = pc[0];
			((char *) &sarg)[1] = pc[1];
			fprintf(f, "\t%2d: %04x\n", i++, (unsigned) sarg);
			pc += 2;
		    }
		    ((char *) &iarg)[0] = pc[0];
		    ((char *) &iarg)[1] = pc[1];
		    ((char *) &iarg)[2] = pc[2];
		    ((char *) &iarg)[3] = pc[3];
		    fprintf(f, "\tminval = %d\n", iarg);
		    pc += 4;
		} else {
		    while (pc < code + etable) {
			((char *) &iarg)[0] = pc[0];
			((char *) &iarg)[1] = pc[1];
			((char *) &iarg)[2] = pc[2];
			((char *) &iarg)[3] = pc[3];
			((char *) &sarg)[0] = pc[4];
			((char *) &sarg)[1] = pc[5];
			if (ttype == 1 || !iarg) {
			    fprintf(f, "\t%-4d\t%04x\n", iarg, (unsigned) sarg);
			} else {
			    fprintf(f, "\t\"%s\"\t%04x\n",
			    disassem_string((char *) iarg), (unsigned) sarg);
			}
			pc += 6;
		    }
		}
		continue;
	    }
	default:
	    /* Instructions with no args */
	    if (is_efun && (instrs[instr].min_arg != instrs[instr].max_arg)) {
		/* efun w/varargs, next byte is actual number */
		sprintf(buff, "%d", EXTRACT_UCHAR(pc));
		pc++;
	    }
	}
	fprintf(f, "%s %s\n", get_f_name(instr), buff);

	if ((next_func >= 0) && ((pc - code) >= offsets[next_func])) {
	    fprintf(f, "\n;; Function %s\n", FUNS[offsets[next_func + 1]].name);
	    next_func += 2;
	    if (next_func >= ((int) NUM_FUNS * 2))
		next_func = -1;
	}
    }

    if (offsets)
	free(offsets);
}

#define INCLUDE_DEPTH 10

static void
dump_line_numbers P2(FILE *, f, struct program *, prog) {
    unsigned short *fi;
    unsigned char *li_start;
    unsigned char *li_end;
    unsigned char *li;
    int addr;
    int sz;
    short s;

    if (!prog->p.i.line_info) {
	load_line_numbers(prog);
	if (!prog->p.i.line_info) {
	    fprintf(f, "Failed to load line numbers\n");
	    return;
	}
    }

    fi = prog->p.i.file_info;
    li_end = (unsigned char *)(((char *)fi) + fi[0]);
    li_start = (unsigned char *)(fi + fi[1]);
    
    fi += 2;
    fprintf(f, "\nabsolute line -> (file, line) table:\n");
    while (fi < (unsigned short *)li_start) {
	fprintf(f, "%i lines from %i [%s]\n", (int)fi[0], (int)fi[1], 
		prog->p.i.strings[fi[1]-1]);
	fi += 2;
    }

    li = li_start;
    addr = 0;
    fprintf(f,"\naddress -> absolute line table:\n");
    while (li < li_end) {
	sz = *li++;
	((char *)&s)[0] = *li++;
	((char *)&s)[1] = *li++;
	fprintf(f, "%4x-%4x: %i\n", addr, addr + sz - 1, (int)s);
	addr += sz;
    }
}
#endif
