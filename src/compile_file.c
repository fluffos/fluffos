#include "std.h"
#include "file_incl.h"

#if defined(LPC_TO_C) && defined(RUNTIME_LOADING)

#include "cc.h"

#undef DEBUG_LINKER

/* portability stuff, if you have to fool with this, mail me.  -Beek */

#ifndef _AIX
#undef NO_A_OUT_H
#undef ALIGN_CODE

#if defined(sgi) || defined(_AIX)
#define HAS_NLIST
#define NO_STRUCT_EXEC
#endif

/* AIX seems to be missing N_TXTOFF */
#if defined (_AIX)
#define N_TXTOFF(file, aout)  (aout).o_text_start
#endif

#ifdef NO_A_OUT_H
#include <sys/exec.h>
#else
#include <a.out.h>
#endif

#ifdef HAS_NLIST
#include <nlist.h>
#endif

#include <sys/file.h>
#endif

/* END */

link_jump_table(prog, jump_table, code)
    struct program *prog;
    void (**jump_table) (struct svalue *);
    char *code;
{
    int num = prog->p.i.num_functions;
    struct function *funp = prog->p.i.functions;
    int i;

    for (i = 0; i < num; i++, funp++)
	if (jump_table[i])
	    funp->offset = (unsigned long) jump_table[i];
    prog->p.i.program = code;
}

#ifndef _AIX
compile_and_link(input_name, the_funcs, code, error_name, output_name)
    char *input_name;
    char **code;
    char *output_name, *error_name;
    void (**the_funcs) ();
{
    char *p, command[1024];
    char module_name[256];

    /* A quick security check to make the system() calls below safe */
    p = module_name;
    while (*input_name) {
	if ((*input_name >= 'A' && *input_name <= 'Z')
	    || (*input_name >= 'a' && *input_name <= 'z')
	    || (*input_name >= '0' && *input_name <= '9')
	  || *input_name == '/' || *input_name == '.' || *input_name == '_')
	    *p++ = *input_name++;
	else
	    return -1;
    }

    /* Chop off everything from the . on to make the rest easier */
    if (p = (char *) rindex(module_name, '.'))
	*p = '\0';
    else {
	return 1;
    }

    /* Do the compile */
    sprintf(command,
#ifdef sgi
	    "%s %s -I%s -c -G 0 -o %s.B %s.c > %s 2>&1",
#else
	    "%s %s -I%s -c -o %s.B %s.c > %s 2>&1",
#endif
	    COMPILER, CFLAGS2,
	    "lpc2c", module_name, module_name, error_name);

#ifdef DEBUG_LINKER
    printf("compile command: %s\n", command);
#endif

    if (system(command)) {
	return 2;
    }
    return runtime_link(module_name, the_funcs, code, error_name, output_name);
}

runtime_link(module_name, the_funcs, code, error_name, output_name)
    char *module_name;
    char **code;
    char *output_name, *error_name;
    void (**the_funcs) ();
{
    char command[1024], *func, *func1, address[20], *p, symbol_name[256];
    int fd;
    int size_needed;
    char code_name[256];

#ifndef HAS_NLIST
    register struct nlist *nlst, *nlp, *nlstend;
    struct nlist *namelist();
    char *stbl;

#else
    struct nlist nl[3];

#endif

#ifdef NO_STRUCT_EXEC
    struct {
	struct filehdr ex_f;
	struct aouthdr ex_o;
    }      hdr;

#else
    struct exec hdr;

#endif

#ifdef DEBUG_LINKER
    unsigned long c;

#endif

    strcpy(code_name, module_name);
    strcat(code_name, ".B");
    if ((fd = open(code_name, O_RDONLY)) < 0)
	return 4;

#ifndef HAS_NLIST
    nlst = namelist(fd, &hdr, &stbl);
    if (nlst == (struct nlist *) NULL)
#else
    /* Read in exec header from object file */
    lseek(fd, (long) 0, L_SET);
    if ((read(fd, &hdr, sizeof(hdr))) != sizeof(hdr))
#endif
    {
#ifndef HAS_NLIST
	FREE(stbl);
#endif
	return 5;
    }
#ifndef HAS_NLIST
    size_needed = (int) (hdr.a_text + hdr.a_data);
#else
    size_needed = (int) (hdr.ex_o.tsize + hdr.ex_o.dsize);
#endif

#if ALIGN_CODE
    size_needed += 2048;
    func = (char *) DMALLOC(size_needed, 0, "lpc->c code block");
    func1 = (char *) ((((int) func) + 2047) & ~0x7ff);
#else
    func = func1 = (char *) DMALLOC(size_needed, 0, "lpc->c code block");
#endif

    close(fd);

    sprintf(command,
	    "ld -x -N -A %s -T %x -o %s %s.B -lm -lc > %s 2>&1",
	    driver_name, func1, output_name, module_name,
	    error_name);

#ifdef DEBUG_LINKER
    printf("link command: %s\n", command);
#endif

    if (system(command))
	return 3;

    /* Now find the symbol in the symbol table */
    if ((fd = open(output_name, O_RDONLY)) < 0)
	return 4;

#ifndef HAS_NLIST
    nlst = namelist(fd, &hdr, &stbl);
    if (nlst == (struct nlist *) NULL)
#else
    /* Read in exec header from object file */
    lseek(fd, (long) 0, L_SET);
    if ((read(fd, &hdr, sizeof(hdr))) != sizeof(hdr))
#endif

    {
#ifndef HAS_NLIST
	FREE(stbl);
#endif
	return 5;
    }
#ifdef DEBUG_LINKER
#ifndef HAS_NLIST
    printf("Magic number: %o\n", hdr.a_magic);
    printf("Text          %d\n", hdr.a_text);
    printf("Data          %d\n", hdr.a_data);
    printf("BSS           %d\n", hdr.a_bss);
    printf("Offset: %d\n", N_TXTOFF(hdr));
#else
    printf("Magic number: %o\n", hdr.ex_o.magic);
    printf("Text          %d\n", hdr.ex_o.tsize);
    printf("Data          %d\n", hdr.ex_o.dsize);
    printf("BSS           %d\n", hdr.ex_o.bsize);
    printf("Offset: %d\n", N_TXTOFF(hdr.ex_f, hdr.ex_o));
#endif
#endif

#ifndef HAS_NLIST
    lseek(fd, (off_t) N_TXTOFF(hdr), L_SET);
    read(fd, func1, size_needed);
#else
    lseek(fd, (off_t) N_TXTOFF(hdr.ex_f, hdr.ex_o), L_SET);
    read(fd, func1, size_needed);
#endif

#if 0
#ifndef HAS_NLIST
    for (p = func1, c = hdr.a_text + hdr.a_data; c; --c, ++p)
	printf("%x %x\n", p, *p);
    printf("Found it at location %x\n", func1);
#else
    for (p = func1, c = hdr.ex_o.tsize + hdr.ex_o.dsize; c; --c, ++p)
	printf("%x %x\n", p, *p);
    printf("Found it at location %x\n", func1);
#endif
#endif

    /* Come up with the entry symbol for the __FUNCS structure */
    (void) strcpy(symbol_name, "__FUNCS");
#ifndef HAS_NLIST
    nlstend = nlst + (hdr.a_syms / sizeof(struct nlist));
    for (nlp = nlstend - 1; nlp >= nlst; nlp--) {
	if (!strcmp(symbol_name, nlp->n_un.n_name)) {
	    /* Fill in the symbol and code pointers */
	    *the_funcs = (void (*) ()) nlp->n_value;
	    *code = func;
	    FREE(stbl);
	    return 0;
	}
    }
#else
    (void) strcpy(code_name, p);
    (void) strcat(code_name, "_main");
    nl[0].n_name = symbol_name;
    nl[1].n_name = code_name;
    nl[2].n_name = NULL;
    nlist(output_name, nl);
    if (nl[0].n_type != 0 || nl[1].n_type != 0) {
	*the_funcs = (void (*) ()) nl[0].n_value;
	*code = (char *) nl[1].n_value;
	return 0;
    }
#endif
    FREE(func);
#ifndef HAS_NLIST
    FREE(stbl);
#endif
    return 6;
}
#endif

void
compile_file_error P2(int, err, char *, routine)
{
    char *why;
    char buf[1024];

    switch (err) {
    case -1:
	why = "Illegal file name to compile.\n";
	break;
    case 1:
	why = "Error parsing program name.\n";
	break;
    case 2:
	why = "Error in compiled C code.\n   see /errors for compiler error messages.\n";
	break;
    case 3:
	why = "Error linking C code.\n   see /errors for linker error messages.\n";
	break;
    case 4:
	why = "Error opening .o file.\n";
	break;
    case 5:
	why = "Error reading .o file.\n";
	break;
    case 6:
	why = "Failed to find symbol in symbol table.\n";
	break;
    }
    sprintf(buf, "%s failed: %s", routine, why);
    error(buf);
}

#if !defined(HAS_NLIST) && !defined(_AIX)
/*
** Returns the entire symbol table of the given executable.
** If anything goes wrong, NULL is returned.
*/
struct nlist *
      namelist P3(int, fd,	/* (seekable) File descriptor of executable  */
		        struct exec *, hdr,	/* Pointer to exec struct
						 * which is filled in */
		        char **, stbl)
{
    register struct nlist *nlst, *nlstend;	/* Name list */
    register struct nlist *nlp;	/* Pointer into list */
    int size;			/* String table size */

    /* Snarf the header out of the file. */

    lseek(fd, (long) 0, L_SET);
    if ((read(fd, (char *) hdr, sizeof(*hdr))) != sizeof(*hdr))
	return ((struct nlist *) 0);

    /* Allocate a buffer for and read the symbol table */

    nlst = (struct nlist *) DMALLOC((unsigned) hdr->a_syms, "namelist");
    lseek(fd, (long) N_SYMOFF(*hdr), L_SET);
    if ((read(fd, (char *) nlst, (int) hdr->a_syms)) != hdr->a_syms) {
	FREE((char *) nlst);
	return ((struct nlist *) 0);
    }
    /* Now, read the string table size. */

    lseek(fd, (long) N_STROFF(*hdr), L_SET);
    if ((read(fd, (char *) &size, sizeof(int))) != sizeof(int)) {
	FREE((char *) nlst);
	return ((struct nlist *) 0);
    }
    /* Allocate a buffer and read the string table out of the file. */

    *stbl = (char *) DMALLOC((unsigned) size, "namelist: 2");
    lseek(fd, (long) N_STROFF(*hdr), L_SET);
    if ((read(fd, *stbl, size)) != size) {
	FREE((char *) nlst);
	FREE(*stbl);
	return ((struct nlist *) 0);
    }
    /*
     * Rearrange the namelist to point at the character strings rather than
     * have offsets from the start of the string table.
     */
    nlstend = nlst + (hdr->a_syms / sizeof(struct nlist));
    for (nlp = nlst; nlp < nlstend; nlp++)
	nlp->n_un.n_name = nlp->n_un.n_strx + *stbl;

    /* Return the namelist we just constructed. */
    return (nlst);
}
#endif

/* AIX is so odd, we might as well keep it separate */
#ifdef _AIX

/* from opcode.h; will clash otherwise */
#undef F_EXEC
#include <nlist.h>

compile_and_link(input_name, the_funcs, code, error_name, output_name)
    char *input_name;
    char **code;
    char *output_name, *error_name;
    void (**the_funcs) ();
{
    char *p, command[1024];
    char module_name[256];

    /* A quick security check to make the system() calls below safe */
    p = module_name;
    while (*input_name) {
	if ((*input_name >= 'A' && *input_name <= 'Z')
	    || (*input_name >= 'a' && *input_name <= 'z')
	    || (*input_name >= '0' && *input_name <= '9')
	  || *input_name == '/' || *input_name == '.' || *input_name == '_')
	    *p++ = *input_name++;
	else
	    return -1;
    }

    /* Chop off everything from the . on to make the rest easier */
    if (p = (char *) rindex(module_name, '.'))
	*p = '\0';
    else {
	return 1;
    }

    /* Do the compile */
    sprintf(command, "%s %s -I%s -c -o %s.B %s.c > %s 2>&1",
	    COMPILER, CFLAGS2,
	    "lpc2c", module_name, module_name, error_name);

#ifdef DEBUG_LINKER
    printf("compile command: %s\n", command);
#endif

    if (system(command)) {
	return 2;
    }
    return runtime_link(module_name, the_funcs, code, error_name, output_name);
}

runtime_link(module_name, the_funcs, code, error_name, output_name)
    char *module_name;
    char **code;
    char *output_name, *error_name;
    void (**the_funcs) ();
{
    char command[1024];
    char *p;
    struct nlist nl[2];
    char *address;

    sprintf(command,
	    "%s -o %s -bM:SRE -e__nostart -bE:lpc2c/aix.exp %s.B > %s 2>&1",
	    COMPILER, output_name, module_name, error_name);

#ifdef DEBUG_LINKER
    printf("link command: %s\n", command);
#endif

    if (system(command))
	return 3;

    if ((p = (char*)load(output_name, 1, getenv("LIBPATH"))) == NULL)
	return 5;

    memset(nl, 0, 2 * sizeof(struct nlist));
    nl[0]._n._n_name = "__FUNCS";
    nlist(output_name, nl);
    if (nl[0].n_type != 0) {
	address = p + nl[0].n_value;
	*the_funcs = (void (*) ()) address;
	*code = p;
	return 0;
    }
    return 6;
}
#endif
#endif

#ifdef LPC_TO_C
init_lpc_to_c()
{
    struct interface_item *p = interface;
}
#endif

/* F_GENERATE_SOURCE will only be def'd if LPC->C is */
#ifdef F_GENERATE_SOURCE
static void generate_identifier P2(char *, buf, char *, name)
{
   while (*name) {
	if ((*name >= 'a' && *name <= 'z') || (*name >= 'A' && *name <= 'Z'))
	    *buf++ = *name++;
	else {
	    *buf++ = '_';
	    name++;
	}
    }
    *buf = 0;
}

int generate_source P2(char *, lname, char *, out_fname)
{
    FILE *crdir_fopen();
    FILE *f;
    struct stat c_st;
    char real_name[200];
    char name[200];
    char out_name[200];
    char ident[205];
    int done = 0;
    char *p;
    int name_length;

    p = lname;
    while (*p) {
	if (*p == '/' && *(p + 1) == '/') {
	    error("Filenames with consecutive /'s in them aren't allowed.\n");
	    return 0;
	}
	p++;
    }
    while (lname[0] == '/')
	lname++;
    strncpy(name, lname, sizeof(name) - 1);
    name[sizeof name - 1] = '\0';
    name_length = strlen(name);
    if (name_length > sizeof name - 4)
	name_length = sizeof name - 4;
    name[name_length] = '\0';
    if (name[name_length - 2] == '.' &&
	(name[name_length - 1] == 'c' || name[name_length - 1] == 'C')) {
	name_length -= 2;
	name[name_length] = '\0';
    }
    /*
     * First check that the c-file exists.
     */
    (void) strcpy(real_name, name);
    (void) strcat(real_name, ".c");
    if (stat(real_name, &c_st) == -1) {
	real_name[strlen(real_name) - 1] = 'C';
	if (stat(real_name, &c_st) == -1) {
	    return 0;
	}
    }
    if (!legal_path(real_name)) {
	fprintf(stderr, "Illegal pathname: %s\n", real_name);
	error("Illegal path name.\n");
	return 0;
    }
    if (!out_fname) {
	out_fname = out_name;
	strcpy(out_name, SAVE_BINARIES);
	if (*out_fname == '/')
	    out_fname++;
	strcat(out_fname, "/");
	strcat(out_fname, name);
	strcat(out_fname, ".c");
    }
    while (!done) {
	if (comp_flag)
	    fprintf(stderr, " compiling %s ...", real_name);
	f = open(real_name, O_RDONLY);
	if (f == -1) {
	    perror(real_name);
	    error("Could not read the file.\n");
	}
	compilation_output_file = crdir_fopen(out_fname);
	if (compilation_output_file == 0) {
	    perror(out_fname);
	    error("Could not open output file '%s'.\n", out_fname);
	}
	current_file = string_copy(real_name);
	generate_identifier(ident, name);
	compilation_ident = ident;
	compile_file(f, real_name);
	fclose(compilation_output_file);
	if (comp_flag)
	    fprintf(stderr, " done\n");
	update_compile_av(total_lines);
	(void) close(f);
	total_lines = 0;
	FREE(current_file);
	current_file = 0;
	if (inherit_file) {
	    char *tmp = inherit_file;

	    if (prog) {
		free_prog(prog, 1);
		prog = 0;
	    }
	    if (strcmp(inherit_file, name) == 0) {
		FREE(inherit_file);
		inherit_file = 0;
		error("Illegal to inherit self.\n");
	    }
	    inherit_file = 0;
	    load_object(tmp, 0);
	    FREE(tmp);
	} else
	    done = 1;
    }
    return 1;
}
#endif
