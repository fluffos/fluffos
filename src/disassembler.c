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
#include "file.h"
#include "program.h"

#ifdef F_DUMP_PROG
void dump_prog PROT((program_t *, char *, int));
static void disassemble PROT((FILE *, char *, int, int, program_t *));
static char *disassem_string PROT((char *));
static int CDECL short_compare PROT((CONST void *, CONST void *));
static void dump_line_numbers PROT((FILE *, program_t *));

void
f_dump_prog PROT((void))
{
    program_t *prog;
    char *where;
    int d;
    object_t *ob;
    int narg = st_num_arg;
    
    if (st_num_arg == 2) {
	ob = sp[-1].u.ob;
	d = sp->u.number;
	where = 0;
    } else if (st_num_arg == 3) {
	ob = sp[-2].u.ob;
	d = sp[-1].u.number;
	where = (sp->type == T_STRING) ? sp->u.string : 0;
    } else {
	ob = sp->u.ob;
	d = 0;
	where = 0;
    }
    if (!(prog = ob->prog)) {
	error("No program for object.\n");
    } else {
	if (!where) {
	    where = "/PROG_DUMP";
	}
	dump_prog(prog, where, d);
    }
    pop_n_elems(narg);
}

/* Current flags:
 * 1 - do disassembly
 * 2 - dump line number table
 */
void
dump_prog P3(program_t *, prog, char *, fn, int, flags)
{
    char *fname;
    FILE *f;
    int i, j;

    fname = check_valid_path(fn, current_object, "dumpallobj", 1);
    if (!fname) {
	error("Invalid path '%s' for writing.\n", fn);
	return;
    }
    f = fopen(fname, "w");
    if (!f) {
	error("Unable to open '/%s' for writing.\n", fname);
	return;
    }
    fprintf(f, "NAME: /%s\n", prog->name);
    fprintf(f, "INHERITS:\n");
    fprintf(f, "\tname                    fio    vio\n");
    fprintf(f, "\t----------------        ---    ---\n");
    for (i = 0; i < (int) prog->num_inherited; i++)
	fprintf(f, "\t%-20s  %5d  %5d\n",
		prog->inherit[i].prog->name,
		(int)prog->inherit[i].function_index_offset,
		(int)prog->inherit[i].variable_index_offset
	    );
    fprintf(f, "PROGRAM:");
    for (i = 0; i < (int) prog->program_size; i++) {
	if (i % 16 == 0)
	    fprintf(f, "\n\t%04x: ", (unsigned int) i);
	fprintf(f, "%02d ", (unsigned char) prog->program[i]);
    }
    fputc('\n', f);
    fprintf(f, "FUNCTIONS:\n");
    fprintf(f, "      name                  offset  flags  fio  # locals  # args\n");
    fprintf(f, "      --------------------- ------ ------- ---  --------  ------\n");
    for (i = 0; i < prog->num_functions_total; i++) {
	char sflags[8];
	int flags;

	flags = prog->function_flags[i];
	sflags[0] = (flags & FUNC_INHERITED) ? 'i' : '-';
	sflags[1] = (flags & FUNC_UNDEFINED) ? 'u' : '-';
	sflags[2] = (flags & FUNC_STRICT_TYPES) ? 's' : '-';
	sflags[3] = (flags & FUNC_PROTOTYPE) ? 'p' : '-';
	sflags[4] = (flags & FUNC_DEF_BY_INHERIT) ? 'd' : '-';
	sflags[5] = (flags & FUNC_ALIAS) ? 'a' : '-';
	sflags[6] = (flags & FUNC_TRUE_VARARGS) ? 'v' : '-';
	sflags[7] = '\0';
	if (flags & FUNC_INHERITED) {
	    runtime_function_u *func_entry = FIND_FUNC_ENTRY(prog, i);
	    fprintf(f, "%4d: %-20s  %5d  %7s %5d\n",
		    i, function_name(prog, i),
		    func_entry->inh.offset,
		    sflags,
		    func_entry->inh.function_index_offset);
	} else {
	    runtime_function_u *func_entry = FIND_FUNC_ENTRY(prog, i);
	    fprintf(f, "%4d: %-20s  %5d  %7s      %7d   %5d\n",
		    i, function_name(prog, i),
		    func_entry->def.f_index,
		    sflags,
		    func_entry->def.num_arg,
		    func_entry->def.num_local);
	}
    }
    fprintf(f, "VARIABLES:\n");
    for (i = 0; i < (int) prog->num_variables_defined; i++)
	fprintf(f, "%4d: %-12s\n", i,
		prog->variable_table[i]);
    fprintf(f, "STRINGS:\n");
    for (i = 0; i < (int) prog->num_strings; i++) {
	fprintf(f, "%4d: ", i);
	for (j = 0; j < 32; j++) {
	    char c;

	    if (!(c = prog->strings[i][j]))
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
	disassemble(f, prog->program, 0, prog->program_size, prog);
    }
    if (flags & 2) {
	fprintf(f, "\n;;;  *** Line Number Info ***\n");
	dump_line_numbers(f, prog);
    }
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

#define NUM_FUNS prog->num_functions_total
#define NUM_FUNS_D prog->num_functions_defined
#define VARS     prog->variable_names
#define NUM_VARS prog->num_variables_total
#define STRS     prog->strings
#define NUM_STRS prog->num_strings
#define CLSS     prog->classes

static int CDECL
short_compare P2(CONST void *, a, CONST void *, b)
{
    int x = *(unsigned short *)a;
    int y = *(unsigned short *)b;
    
    return x - y;
}

static char *pushes[] = { "string", "number", "global", "local" };

static void
disassemble P5(FILE *, f, char *, code, int, start, int, end, program_t *, prog)
{
    int i, j, instr, iarg, is_efun;
    unsigned short sarg;
    unsigned short offset;
    char *pc, buff[256];
    int next_func;

    short *offsets;

    if (start == 0) {
	/* sort offsets of functions */
	offsets = (short *) malloc(NUM_FUNS_D * 2 * sizeof(short));
	for (i = 0; i < (int) NUM_FUNS_D; i++) {
	    if (prog->function_flags[prog->function_table[i].runtime_index]
		& (FUNC_INHERITED | FUNC_NO_CODE))
		offsets[i * 2] = end + 1;
	    else
		offsets[i * 2] = prog->function_table[i].address;
	    offsets[i * 2 + 1] = i;
	}
#ifdef _SEQUENT_
	qsort((void *) &offsets[0],
#else
	qsort((char *) &offsets[0],
#endif
	      NUM_FUNS_D, sizeof(short) * 2, short_compare);
	next_func = 0;
    } else {
	offsets = 0;
	next_func = -1;
    }

    pc = code + start;

    while ((pc - code) < end) {
	if ((next_func >= 0) && ((pc - code) >= offsets[next_func])) {
	    fprintf(f, "\n;; Function %s\n", prog->function_table[offsets[next_func + 1]].name);
	    next_func += 2;
	    if (next_func >= ((int) NUM_FUNS_D * 2))
		next_func = -1;
	}

	fprintf(f, "%04x: ", (unsigned) (pc - code));

	is_efun = (instr = EXTRACT_UCHAR(pc)) >= BASE;

	pc++;
	buff[0] = 0;
	sarg = 0;

	switch (instr) {
	case F_PUSH:
	    fprintf(f, "push ");
	    i = EXTRACT_UCHAR(pc++);
	    while (i--) {
		j = EXTRACT_UCHAR(pc++);
		fprintf(f, "%s %i", pushes[(j & PUSH_WHAT) >> 6], 
			j & PUSH_MASK);
		if (i)
		    fprintf(f, ", ");
		else break;
	    }
	    fprintf(f, "\n");
	    continue;
	    /* Single numeric arg */
	case F_BRANCH_NE:
	case F_BRANCH_GE:
	case F_BRANCH_LE:
	case F_BRANCH_EQ:
	case F_BRANCH:
	case F_BRANCH_WHEN_ZERO:
	case F_BRANCH_WHEN_NON_ZERO:
#ifdef F_LOR
	case F_LOR:
	case F_LAND:
#endif
	    COPY_SHORT(&sarg, pc);
	    offset = (pc - code) + (unsigned short) sarg;
	    sprintf(buff, "%04x (%04x)", (unsigned) sarg, (unsigned) offset);
	    pc += 2;
	    break;

	case F_NEXT_FOREACH:
	case F_BBRANCH_LT:
	    COPY_SHORT(&sarg, pc);
	    offset = (pc - code) - (unsigned short) sarg;
	    sprintf(buff, "%04x (%04x)", (unsigned) sarg, (unsigned) offset);
	    pc += 2;
	    break;

	case F_FOREACH:
	    {
		int flags = EXTRACT_UCHAR(pc++);
		char *left = "local", *right = "local";

		if (flags & FOREACH_LEFT_GLOBAL)
		    left = "global";
		if (flags & FOREACH_RIGHT_GLOBAL)
		    right = "global";
		if (flags & FOREACH_REF) {
		    if (flags & FOREACH_MAPPING)
			right = "ref";
		    else
			left = "ref";
		}

		if (flags & FOREACH_MAPPING) {
		    sprintf(buff, "(mapping) %s %i, %s %i", 
			    left, EXTRACT_UCHAR(pc++), right, EXTRACT_UCHAR(pc++));
		} else {
		    sprintf(buff, "(array) %s %i", left, EXTRACT_UCHAR(pc++));
		}
		break;
	    }

	case F_BBRANCH_WHEN_ZERO:
	case F_BBRANCH_WHEN_NON_ZERO:
	case F_BBRANCH:
	    COPY_SHORT(&sarg, pc);
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
	    COPY_SHORT(&sarg, pc);
	    sprintf(buff, "%04x", (unsigned) sarg);
	    pc += 2;
	    break;

	case F_AGGREGATE:
	case F_AGGREGATE_ASSOC:
	    COPY_SHORT(&sarg, pc);
	    sprintf(buff, "%d", (int)sarg);
	    pc += 2;
	    break;

	case F_MAKE_REF:
	case F_KILL_REFS:
	case F_MEMBER:
	case F_MEMBER_LVALUE:
	    sprintf(buff, "%d", (int)EXTRACT_UCHAR(pc++));
	    break;

	case F_EXPAND_VARARGS:
	    {
		int which = EXTRACT_UCHAR(pc++);
		if (which) {
		    sprintf(buff, "%d from top of stack", which);
		} else {
		    strcpy(buff, "top of stack");
		}
	    }		
	    break;

	case F_NEW_EMPTY_CLASS:
	case F_NEW_CLASS:
	    {
		int which = EXTRACT_UCHAR(pc++);
		
		strcpy(buff, STRS[CLSS[which].name]);
		break;
	    }

	case F_CALL_FUNCTION_BY_ADDRESS:
	    COPY_SHORT(&sarg, pc);
	    pc += 3;
	    if (sarg < NUM_FUNS)
		sprintf(buff, "%-12s %5d", function_name(prog, sarg),
			(int)sarg);
	    else
		sprintf(buff, "<out of range %d>", (int)sarg);
	    break;

	case F_CALL_INHERITED:
	{
	    program_t *newprog;

	    newprog = (prog->inherit + EXTRACT_UCHAR(pc++))->prog;
	    COPY_SHORT(&sarg, pc);
	    pc += 3;
	    if (sarg < newprog->num_functions_total)
		sprintf(buff, "%30s::%-12s %5d", newprog->name,
			function_name(newprog, sarg), (int) sarg);
	    else sprintf(buff, "<out of range in %30s - %d>", newprog->name,
			 (int) sarg);
	    break;
	}
	case F_GLOBAL_LVALUE:
	case F_GLOBAL:
	    if ((unsigned) (iarg = EXTRACT_UCHAR(pc)) < NUM_VARS)
		sprintf(buff, "%s", variable_name(prog, iarg));
	    else
		sprintf(buff, "<out of range %d>", iarg);
	    pc++;
	    break;

	case F_LOOP_INCR:
	    sprintf(buff, "LV%d", EXTRACT_UCHAR(pc));
	    pc++;
	    break;
	case F_WHILE_DEC:
	    COPY_SHORT(&sarg, pc + 1);
	    offset = (pc - code) - (unsigned short) sarg;
	    sprintf(buff, "LV%d--, branch %04x (%04x)", EXTRACT_UCHAR(pc),
		    (unsigned) sarg, (unsigned) offset);
	    pc += 3;
	    break;
	case F_TRANSFER_LOCAL:
	case F_LOCAL:
	case F_LOCAL_LVALUE:
	case F_VOID_ASSIGN_LOCAL:
	case F_REF:
	case F_REF_LVALUE:
	    sprintf(buff, "LV%d", EXTRACT_UCHAR(pc));
	    pc++;
	    break;
	case F_LOOP_COND_NUMBER:
	    i = EXTRACT_UCHAR(pc++);
	    COPY_INT(&iarg, pc);
	    pc += 4;
	    COPY_SHORT(&sarg, pc);
	    offset = (pc - code) - (unsigned short) sarg;
	    pc += 2;
	    sprintf(buff, "LV%d < %d bbranch_when_non_zero %04x (%04x)",
		    i, iarg, sarg, offset);
	    break;
	case F_LOOP_COND_LOCAL:
	    i = EXTRACT_UCHAR(pc++);
	    iarg = *pc++;
	    COPY_SHORT(&sarg, pc);
	    offset = (pc - code) - (unsigned short) sarg;
	    pc += 2;
	    sprintf(buff, "LV%d < LV%d bbranch_when_non_zero %04x (%04x)",
		    i, iarg, sarg, offset);
	    break;
	case F_STRING:
	    COPY_SHORT(&sarg, pc);
	    if (sarg < NUM_STRS)
		sprintf(buff, "\"%s\"", disassem_string(STRS[sarg]));
	    else
		sprintf(buff, "<out of range %d>", (int)sarg);
	    pc += 2;
	    break;
	case F_SHORT_STRING:
	    if (EXTRACT_UCHAR(pc) < NUM_STRS)
	        sprintf(buff, "\"%s\"", disassem_string(STRS[EXTRACT_UCHAR(pc)]));
	    else 
	        sprintf(buff, "<out of range %d>", EXTRACT_UCHAR(pc));
	    pc++;
	    break;
	case F_SIMUL_EFUN:
	    COPY_SHORT(&sarg, pc);
	    sprintf(buff, "\"%s\" %d", simuls[sarg].func->name, pc[2]);
	    pc += 3;
	    break;

	case F_FUNCTION_CONSTRUCTOR:
	    switch (EXTRACT_UCHAR(pc++)) {
	    case FP_SIMUL:
		LOAD_SHORT(sarg, pc);
		sprintf(buff, "<simul_efun> \"%s\"", simuls[sarg].func->name);
		break;
	    case FP_EFUN:
		LOAD_SHORT(sarg, pc);
		sprintf(buff, "<efun> %s", instrs[sarg].name);
		break;
	    case FP_LOCAL:
		LOAD_SHORT(sarg, pc);
		if (sarg < NUM_FUNS)
		    sprintf(buff, "<local_fun> %s", function_name(prog, sarg));
		else
		    sprintf(buff, "<local_fun> <out of range %d>", (int)sarg);
		break;
	    case FP_FUNCTIONAL:
	    case FP_FUNCTIONAL | FP_NOT_BINDABLE:
		sprintf(buff, "<functional, %d args>\nCode:", (int)pc[0]);
		pc += 3;
		break;
	    case FP_ANONYMOUS:
	    case FP_ANONYMOUS | FP_NOT_BINDABLE:
		COPY_SHORT(&sarg, &pc[2]);
		sprintf(buff, "<anonymous function, %d args, %d locals, ends at %04x>\nCode:",
			(int)pc[0], (int)pc[1], (int) (pc + 3 + sarg - code));
		pc += 4;
		break;
	    }
	    break;

	case F_NUMBER:
	    COPY_INT(&iarg, pc);
	    sprintf(buff, "%d", iarg);
	    pc += 4;
	    break;

	case F_REAL:
	    {
		float farg;

		COPY_FLOAT(&farg, pc);
		sprintf(buff, "%f", farg);
		pc += 4;
		break;
	    }

	case F_SSCANF:
	case F_PARSE_COMMAND:
	case F_BYTE:
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
		char *parg;
		
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
			COPY_SHORT(&sarg, pc);
			fprintf(f, "\t%2d: %04x\n", i++, (unsigned) sarg);
			pc += 2;
		    }
		    COPY_INT(&iarg, pc);
		    fprintf(f, "\tminval = %d\n", iarg);
		    pc += 4;
		} else {
		    while (pc < code + etable) {
			COPY_PTR(&parg, pc);
			COPY_SHORT(&sarg, pc + SIZEOF_PTR);
			if (ttype == 1 || !parg) {
			    fprintf(f, "\t%-4d\t%04x\n",(int)parg, (unsigned) sarg);
			} else {
			    fprintf(f, "\t\"%s\"\t%04x\n",
			    disassem_string(parg), (unsigned) sarg);
			}
			pc += 2 + SIZEOF_PTR;
		    }
		}
		continue;
	    }
	case F_EFUNV:
	    sprintf(buff, "%d", EXTRACT_UCHAR(pc++));
	    instr = EXTRACT_UCHAR(pc++) + ONEARG_MAX;
	    break;
	case F_EFUN0:
	case F_EFUN1:
	case F_EFUN2:
	case F_EFUN3:
	    if (instrs[instr].min_arg != instrs[instr].max_arg) {
		sprintf(buff, "%d", instr - F_EFUN0);
	    }
	    instr = EXTRACT_UCHAR(pc++) + ONEARG_MAX;
	    break;
	}
	fprintf(f, "%s %s\n", query_instr_name(instr), buff);
    }

    if (offsets)
	free(offsets);
}

#define INCLUDE_DEPTH 10

static void
dump_line_numbers P2(FILE *, f, program_t *, prog) {
    unsigned short *fi;
    unsigned char *li_start;
    unsigned char *li_end;
    unsigned char *li;
    int addr;
    int sz;
    short s;

    if (!prog->line_info) {
	load_line_numbers(prog);
	if (!prog->line_info) {
	    fprintf(f, "Failed to load line numbers\n");
	    return;
	}
    }

    fi = prog->file_info;
    li_end = (unsigned char *)(((char *)fi) + fi[0]);
    li_start = (unsigned char *)(fi + fi[1]);
    
    fi += 2;
    fprintf(f, "\nabsolute line -> (file, line) table:\n");
    while (fi < (unsigned short *)li_start) {
	fprintf(f, "%i lines from %i [%s]\n", (int)fi[0], (int)fi[1], 
		prog->strings[fi[1]-1]);
	fi += 2;
    }

    li = li_start;
    addr = 0;
    fprintf(f,"\naddress -> absolute line table:\n");
    while (li < li_end) {
	sz = *li++;
	COPY_SHORT(&s, li);
	li += 2;
	fprintf(f, "%4x-%4x: %i\n", addr, addr + sz - 1, (int)s);
	addr += sz;
    }
}
#endif
