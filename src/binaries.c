/*
  binaries.c: code to save loaded LPC objects to binary files (in order
  to speed loading during subsequent runs of the driver).

  This is mostly original code by Darin Johnson.  Ideas came from CD,
  including crdir_fopen().  Feel free to use this code but please keep
  credits intact.
*/

#define SUPPRESS_COMPILER_INLINES
#include "std.h"
#include "config.h"
#include "lpc_incl.h"
#include "file_incl.h"
#include "compiler.h"
#include "binaries.h"
#include "lex.h"
#include "backend.h"
#include "swap.h"
#include "qsort.h"
#include "compile_file.h"
#include "hash.h"

/* This should be a configure check.  What the heck is it needed for, anyway?*/
#ifdef WIN32
#include <direct.h>
#endif

#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
#include "functab_tree.h"
#endif

#ifdef BINARIES

static char *magic_id = "MUDB";
static time_t driver_id;
static time_t config_id;

char driver_name[512];

static void patch_out PROT((program_t *, short *, int));
static void patch_in PROT((program_t *, short *, int));
static int str_case_cmp PROT((char *, char *));
static int check_times PROT((time_t, char *));
static int do_stat PROT((char *, struct stat *, char *));

/*
 * stats fname or CONFIG_FILE_DIR/fname (for finding config files) and/or
 * BIN_DIR/fname (for finding config files and/or driver), whichever exists.
 */
static int
do_stat P3(char *, fname, struct stat *, st, char *, pathname)
{
    int i;
    char buf[256];

    if ((i = stat(fname, st)) != -1) {
	if (pathname)
	    strcpy(pathname, fname);
	return i;
    }
    /* look in CONFIG_FILE_DIR */
    if (!pathname) {
#ifdef LATTICE
	if (strchr(CONFIG_FILE_DIR, ':'))
	    sprintf(buf, "%s%s", CONFIG_FILE_DIR, fname);
	else
#endif
	    sprintf(buf, "%s/%s", CONFIG_FILE_DIR, fname);
	if ((i = stat(buf, st)) != -1)
	    return i;
    }
    /* look in BIN_DIR */
#ifdef LATTICE
    if (strchr(BIN_DIR, ':'))
	sprintf(buf, "%s%s", BIN_DIR, fname);
    else
#endif
	sprintf(buf, "%s/%s", BIN_DIR, fname);
	
    if ((i = stat(buf, st)) != -1) {
	if (pathname)
	    strcpy(pathname, buf);
	return i;
    }
    return -1;
}				/* do_stat() */

void save_binary P3(program_t *, prog, mem_block_t *, includes, mem_block_t *, patches)
{
    char file_name_buf[200];
    char *file_name = file_name_buf;
    FILE *f;
    int i, tmp;
    short len;
    program_t *p;
    struct stat st;

    svalue_t *ret;
    char *nm;

    nm = add_slash(prog->name);
    push_malloced_string(nm);
    ret = safe_apply_master_ob(APPLY_VALID_SAVE_BINARY, 1);
    if (!MASTER_APPROVED(ret))
	return;
    if (prog->total_size > (int) USHRT_MAX ||
	includes->current_size > (int) USHRT_MAX)
	/* assume all other sizes ok */
	return;

    strcpy(file_name, SAVE_BINARIES);
    if (file_name[0] == '/')
	file_name++;
    if (stat(file_name, &st) == -1)
	return;
    strcat(file_name, "/");
    strcat(file_name, prog->name);
    len = strlen(file_name);
#ifdef LPC_TO_C
    if (compile_to_c)
	file_name[len - 1] = 'B';	/* change .c ending to .B */
    else
#endif
	file_name[len - 1] = 'b';	/* change .c ending to .b */

    if (!(f = crdir_fopen(file_name)))
	return;

    if (comp_flag) {
	debug_message(" saving binary ... ");
#ifdef LATTICE
	fflush(stderr);
#endif
    }
    /*
     * Write out preamble.  Includes magic number, etc, all of which must
     * match while loading.
     */
    if (fwrite(magic_id, strlen(magic_id), 1, f) != 1 ||
	fwrite((char *) &driver_id, sizeof driver_id, 1, f) != 1 ||
	fwrite((char *) &config_id, sizeof config_id, 1, f) != 1) {
	debug_message("I/O error in save_binary.\n");
	fclose(f);
	unlink(file_name);
	return;
    }
    /*
     * Write out list of include files.
     */
    len = includes->current_size;
    fwrite((char *) &len, sizeof len, 1, f);
    fwrite(includes->block, includes->current_size, 1, f);

    /*
     * copy and patch program
     */
    p = (program_t *) DXALLOC(prog->total_size, TAG_TEMPORARY, "save_binary");
    /* convert to relative pointers, copy, then convert back */
    locate_out(prog);
    memcpy(p, prog, prog->total_size);
    locate_in(prog);
    if (patches->current_size) {
	locate_in(p);
	patch_out(p, (short *) patches->block,
		  patches->current_size / sizeof(short));
	locate_out(p);
    }
    /*
     * write out prog.  The prog structure is mostly setup, but strings will
     * have to be stored specially.
     */
    len = SHARED_STRLEN(p->name);
    fwrite((char *) &len, sizeof len, 1, f);
    fwrite(p->name, sizeof(char), len, f);

    fwrite((char *) &p->total_size, sizeof p->total_size, 1, f);
    fwrite((char *) p, p->total_size, 1, f);
    FREE(p);
    p = prog;

    /* inherit names */
    for (i = 0; i < (int) p->num_inherited; i++) {
	len = SHARED_STRLEN(p->inherit[i].prog->name);
	fwrite((char *) &len, sizeof len, 1, f);
	fwrite(p->inherit[i].prog->name, sizeof(char), len, f);
    }

    /* string table */
    for (i = 0; i < (int) p->num_strings; i++) {
	tmp = SHARED_STRLEN(p->strings[i]);
	if (tmp > (int) USHRT_MAX) {	/* possible? */
	    fclose(f);
	    unlink(file_name);
	    error("String to long for save_binary.\n");
	    return;
	}
	len = tmp;
	fwrite((char *) &len, sizeof len, 1, f);
	fwrite(p->strings[i], sizeof(char), len, f);
    }

    /* var names */
    for (i = 0; i < (int) p->num_variables; i++) {
	len = SHARED_STRLEN(p->variable_names[i].name);
	fwrite((char *) &len, sizeof len, 1, f);
	fwrite(p->variable_names[i].name, sizeof(char), len, f);
    }

    /* function names */
    for (i = 0; i < (int) p->num_functions; i++) {
	len = SHARED_STRLEN(p->functions[i].name);
	fwrite((char *) &len, sizeof len, 1, f);
	fwrite(p->functions[i].name, sizeof(char), len, f);
    }

    /* line_numbers */
    if (p->line_info)
	len = p->file_info[0];
    else
	len = 0;
    fwrite((char *) &len, sizeof len, 1, f);
    fwrite((char *) p->file_info, len, 1, f);

    /*
     * patches
     */
    len = patches->current_size;
    fwrite((char *) &len, sizeof len, 1, f);
    fwrite(patches->block, patches->current_size, 1, f);

    fclose(f);
}				/* save_binary() */

#define ALLOC_BUF(size) \
    if ((size) > buf_size) { FREE(buf); buf = DXALLOC(buf_size = size, TAG_TEMPORARY, "ALLOC_BUF"); }

/* crude hack to check both .B and .b */
#define OUT_OF_DATE (lpc_obj ? load_binary(name, 0) : 0)

int load_binary P2(char *, name, lpc_object_t *, lpc_obj)
{
    char file_name_buf[400];
    char *buf, *iname, *file_name = file_name_buf, *file_name_two = &file_name_buf[200];
    FILE *f;
    int i, buf_size, ilen;
    time_t mtime;
    short len;
    program_t *p;
    object_t *ob;
    struct stat st;

    /* stuff from prolog() */
    prog = 0;
    num_parse_error = 0;

    sprintf(file_name, "%s/%s", SAVE_BINARIES, name);
    if (file_name[0] == '/')
	file_name++;
    len = strlen(file_name);
    file_name[len - 1] = (lpc_obj ? 'B' : 'b');	/* change .c ending to .b */

    if (stat(file_name, &st) != -1)
	mtime = st.st_mtime;
    else 
	return OUT_OF_DATE;

    if (!(f = fopen(file_name, "r")))
	return OUT_OF_DATE;

    if (comp_flag) {
	debug_message(" loading binary %s ... ", name);
#ifdef LATTICE
	fflush(stderr);
#endif
    }
    /* see if we're out of date with source */
    if (check_times(mtime, name) <= 0) {
	if (comp_flag)
	    debug_message("out of date (source file newer).\n");
	fclose(f);
	return OUT_OF_DATE;
    }
    buf = DXALLOC(buf_size = SMALL_STRING_SIZE, TAG_TEMPORARY, "ALLOC_BUF");

    /*
     * Read preamble.  This must match, or we assume a different driver or
     * configuration.
     */
    if (fread(buf, strlen(magic_id), 1, f) != 1 ||
	strncmp(buf, magic_id, strlen(magic_id)) != 0) {
	if (comp_flag)
	    debug_message("out of date. (bad magic number)\n");
	fclose(f);
	FREE(buf);
	return OUT_OF_DATE;
    }
    if ((fread((char *) &i, sizeof i, 1, f) != 1 || driver_id != i)
	&& (!lpc_obj)) {
	if (comp_flag)
	    debug_message("out of date. (driver changed)\n");
	fclose(f);
	FREE(buf);
	return OUT_OF_DATE;
    }
    if (fread((char *) &i, sizeof i, 1, f) != 1 || config_id != i) {
	if (comp_flag)
	    debug_message("out of date. (config file changed)\n");
	fclose(f);
	FREE(buf);
	return OUT_OF_DATE;
    }
    /*
     * read list of includes and check times
     */
    fread((char *) &len, sizeof len, 1, f);
    ALLOC_BUF(len);
    fread(buf, sizeof(char), len, f);
    for (iname = buf; iname < buf + len; iname += strlen(iname) + 1) {
	if (check_times(mtime, iname) <= 0) {
	    if (comp_flag)
		debug_message("out of date (include file newer).\n");
	    fclose(f);
	    FREE(buf);
	    return OUT_OF_DATE;
	}
    }

    /* check program name */
    fread((char *) &len, sizeof len, 1, f);
    ALLOC_BUF(len + 1);
    fread(buf, sizeof(char), len, f);
    buf[len] = '\0';
    if (strcmp(name, buf) != 0) {
	if (comp_flag)
	    debug_message("binary name inconsistent with file.\n");
	fclose(f);
	FREE(buf);
	return OUT_OF_DATE;
    }
    /*
     * Read program structure.
     */
    fread((char *) &ilen, sizeof ilen, 1, f);
    p = (program_t *) DXALLOC(ilen, TAG_PROGRAM, "load_binary");
    fread((char *) p, ilen, 1, f);
    locate_in(p);		/* from swap.c */
    p->name = make_shared_string(name);

    /* Read inherit names and find prog.  Check mod times also. */
    for (i = 0; i < (int) p->num_inherited; i++) {
	fread((char *) &len, sizeof len, 1, f);
	ALLOC_BUF(len + 1);
	fread(buf, sizeof(char), len, f);
	buf[len] = '\0';

	/*
	 * Check times against inherited source.  If saved binary of
	 * inherited prog exists, check against it also.
	 */
	sprintf(file_name_two, "%s/%s", SAVE_BINARIES, buf);
	if (file_name_two[0] == '/')
	    file_name_two++;
	len = strlen(file_name_two);
	file_name_two[len - 1] = 'b';
	if (check_times(mtime, buf) <= 0 ||
	    check_times(mtime, file_name_two) == 0) {	/* ok if -1 */

	    if (comp_flag)
		debug_message("out of date (inherited source newer).\n");
	    fclose(f);
	    free_string(p->name);
	    FREE(p);
	    FREE(buf);
	    return OUT_OF_DATE;
	}
	/* find inherited program (maybe load it here?) */
	ob = find_object2(buf);
	if (!ob) {
	    if (comp_flag)
		debug_message("missing inherited prog.\n");
	    fclose(f);
	    free_string(p->name);
	    FREE(p);
	    inherit_file = buf;	/* freed elsewhere */
	    return 1;		/* not 0 */
	}
	p->inherit[i].prog = ob->prog;
    }

    /* Read string table */
    for (i = 0; i < (int) p->num_strings; i++) {
	fread((char *) &len, sizeof len, 1, f);
	ALLOC_BUF(len + 1);
	fread(buf, sizeof(char), len, f);
	buf[len] = '\0';
	p->strings[i] = make_shared_string(buf);
    }

    /* var names */
    for (i = 0; i < (int) p->num_variables; i++) {
	fread((char *) &len, sizeof len, 1, f);
	ALLOC_BUF(len + 1);
	fread(buf, sizeof(char), len, f);
	buf[len] = '\0';
	p->variable_names[i].name = make_shared_string(buf);
    }

#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
    /* function table tree's root */
    p->tree_r = (unsigned short) 0xffff;
#endif

    /* function names */
    for (i = 0; i < (int) p->num_functions; i++) {
	fread((char *) &len, sizeof len, 1, f);
	ALLOC_BUF(len + 1);
	fread(buf, sizeof(char), len, f);
	buf[len] = '\0';
	p->functions[i].name = make_shared_string(buf);

#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
	/* rebuild function table tree */
	if (!(p->functions[i].flags & NAME_ALIAS))
	    add_function(p->functions, &p->tree_r, i);
#endif
    }

    /* line numbers */
    fread((char *) &len, sizeof len, 1, f);
    p->file_info = (unsigned short *) DXALLOC(len, TAG_LINENUMBERS, "load binary");
    fread((char *) p->file_info, len, 1, f);
    p->line_info = (unsigned char *)&p->file_info[p->file_info[1]];

    /* patches */
    fread((char *) &len, sizeof len, 1, f);
    ALLOC_BUF(len);
    fread(buf, len, 1, f);
    /* fix up some stuff */
    patch_in(p, (short *) buf, len / sizeof(short));

    fclose(f);
    FREE(buf);

    /*
     * Now finish everything up.  (stuff from epilog())
     */
    prog = p;
    prog->id_number = get_id_number();

    total_prog_block_size += prog->total_size;
    total_num_prog_blocks += 1;

    swap_line_numbers(prog);
    reference_prog(prog, "load_binary");
    for (i = 0; (unsigned) i < prog->num_inherited; i++) {
	reference_prog(prog->inherit[i].prog, "inheritance");
    }

#ifdef LPC_TO_C
    if (prog->program_size == 0) {
	if (lpc_obj) {
	    if (comp_flag)
		debug_message("linking jump table ...\n");
	    link_jump_table(prog, (void **)lpc_obj->jump_table);
	} else {
	    if (prog)
		free_prog(prog, 1);
	    return OUT_OF_DATE;
	}
    }
#endif

    if (comp_flag)
	debug_message("done.\n");
    return 1;
}				/* load_binary() */

void init_binaries P2(int, argc, char **, argv)
{
    struct stat st;
    int arg_id, i;

    if (do_stat(argv[0], &st, driver_name) != -1) {
	driver_id = st.st_mtime;
    } else {
	driver_id = current_time;
	debug_message("Can't find \"%s\", ignoring old binaries.\n", argv[0]);
	debug_message("Try invoking the driver using the full pathname.\n");
    }

    arg_id = 0;
    for (i = 1; i < argc; i++) {
	if (argv[i][0] != '-') {
	    if (config_id)
		continue;
	    if (do_stat(argv[i], &st, 0) != -1)
		config_id = st.st_mtime;
	    else
		config_id = current_time;
	} else if (argv[i][1] == 'D' || argv[i][1] == 'm') {
	    arg_id = (arg_id << 1) ^ whashstr(argv[i] + 2, 32);
	}
    }

    config_id ^= (arg_id << (sizeof(short) * 8));
}				/* init_binaries() */

/*
 * Test against modification times.  -1 if file doesn't exist,
 * 0 if out of date, and 1 if it's ok.
 */
static int check_times P2(time_t, mtime, char *, nm)
{
    struct stat st;

#ifdef LATTICE
    if (*nm == '/')
	nm++;
#endif
    if (stat(nm, &st) == -1)
	return -1;
    if (st.st_mtime > mtime) {
	return 0;
    }
    return 1;
}				/* check_times() */

/*
 * Routines to do some hacking on the program being saved/loaded.
 * Basically to fix up string switch tables, since the alternative
 * would probably need a linear search in f_switch().
 * I set things up so these routines can be used with other things
 * that might need patching.
 */
static void patch_out P3(program_t *, prog, short *, patches, int, len)
{
    int i;
    char *p;

    p = prog->program;
    while (len > 0) {
	i = patches[--len];
	if (p[i] == F_SWITCH && p[i + 1] >> 4 != 0xf) {	/* string switch */
	    short offset, break_addr;
	    char *s;

	    /* replace strings in table with string table indices */
	    COPY_SHORT(&offset, p + i + 2);
	    COPY_SHORT(&break_addr, p + i + 4);

	    while (offset < break_addr) {
		COPY_PTR(&s, p + offset);
		/*
		 * take advantage of fact that s is in strings table to find
		 * it's index.
		 */
		if (s == 0)
		    s = (char *)(POINTER_INT)-1;
		else
		    s = (char *)(POINTER_INT)store_prog_string(s);
		COPY_PTR(p + offset, &s);
		offset += SWITCH_CASE_SIZE;
	    }
	}
    }
}				/* patch_out() */

static int str_case_cmp P2(char *, a, char *, b)
{
    char *s1, *s2;

    COPY_PTR(&s1, a);
    COPY_PTR(&s2, b);

    return s1 - s2;
}				/* str_case_cmp() */

static void patch_in P3(program_t *, prog, short *, patches, int, len)
{
    int i;
    char *p;

    p = prog->program;
    while (len > 0) {
	i = patches[--len];
	if (p[i] == F_SWITCH && p[i + 1] >> 4 != 0xf) {	/* string switch */
	    short offset, start, break_addr;
	    char *s;

	    /* replace string indices with string pointers */
	    COPY_SHORT(&offset, p + i + 2);
	    COPY_SHORT(&break_addr, p + i + 4);

	    start = offset;
	    while (offset < break_addr) {
		COPY_PTR(&s, p + offset);
		/*
		 * get real pointer from strings table
		 */
		if (s == (char *)-1)
		    s = 0;
		else
		    s = prog->strings[(POINTER_INT)s];
		COPY_PTR(p + offset, &s);
		offset += SWITCH_CASE_SIZE;
	    }
	    /* sort so binary search still works */
	    quickSort(&p[start], (break_addr - start) / SWITCH_CASE_SIZE, 
		      SWITCH_CASE_SIZE, str_case_cmp);
	}
    }
}				/* patch_in() */

#endif

/*
 * open file for writing, creating intermediate directories if needed.
 */
FILE *crdir_fopen P1(char *, file_name)
{
    char *p;
    struct stat st;
    FILE *ret;

    /*
     * Beek - These directories probably exist most of the time, so let's
     * optimize by trying the fopen first
     */
    if ((ret = fopen(file_name, "wb")) != NULL) {
	return ret;
    }
    p = file_name;

    while (*p && (p = (char *) strchr(p, '/'))) {
	*p = '\0';
	if (stat(file_name, &st) == -1) {
	    /* make this dir */
	    if (OS_mkdir(file_name, 0770) == -1) {
		*p = '/';
		return (FILE *) 0;
	    }
	}
	*p = '/';
	p++;
    }

    return fopen(file_name, "wb");
}				/* crdir_fopen() */
