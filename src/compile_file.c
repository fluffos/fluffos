#include "std.h"
#ifdef LPC_TO_C
#define SUPRESS_COMPILER_INLINES
#include "file_incl.h"
#include "lpc_incl.h"
#include "interface.h"
#include "lex.h"
#include "compiler.h"
#include "md.h"
#include "otable.h"
#include "cfuns.h"
#include "file.h"
#include "backend.h"
#include "binaries.h"

void
link_jump_table P2(program_t *, prog, void **, jump_table)
{
    int num = prog->num_functions;
    function_t *funcs = prog->functions;
    int i;
    int j;

    for (i = 0, j = 0; i < num; i++) {
	if (funcs[i].flags & (NAME_NO_CODE | NAME_INHERITED)) continue;
	if (jump_table[j])
	    funcs[i].offset = (unsigned long) jump_table[j];
	else
	    funcs[i].offset = 0;
	j++;
    }
}

void
init_lpc_to_c()
{
    interface_t **p = interface;
    lpc_object_t *ob;

    while (*p) {
	ob = ALLOCATE(lpc_object_t, TAG_LPC_OBJECT, "init_lpc_to_c");
	ob->name = alloc_cstring((*p)->fname, "init_lpc_to_c");
	SET_TAG(ob->name, TAG_OBJ_NAME);
	enter_object_hash((object_t *)ob);
	ob->flags = O_COMPILED_PROGRAM;
	ob->jump_table = (*p)->jump_table;
	if ((ob->string_switch_tables = (*p)->string_switch_tables))
	    fix_switches(ob->string_switch_tables);
	p++;
    }    
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

static char *rest_of_makefile = "interface.c\n\
\n\
%%ifdef GNU\n\
OBJ=$(addprefix $(OBJDIR)/,$(subst .c,.o,$(SRC)))\n\
\n\
$(OBJDIR)/%%.o: %%.c $(OBJDIR)\n\
\t$(CC) -I$(OBJDIR) -I.. $(CFLAGS) -o $@ -c $<\n\
%%else\n\
.c.o:\n\
\t$(CC) $(CFLAGS) -I.. -c $*.c\n\
%%endif\n\
\n\
all: $(OBJ)\n\
\tar rlcu mudlib.a $(OBJ)\n\
\n\
clean:\n\
\t-rm -f *.o\n\
\t-rm -f mudlib.a\n\
\n";

int generate_source P2(svalue_t *, arg1, char *, out_fname)
{
    FILE *makefile;

    struct stat c_st;
    char real_name[200];
    char name[200];
    char out_name[200];
    char ident[205];
    int done;
    char *outp;
    char *string_needs_free;
    int index;
    array_t tmp_arr, *arr;
    int f;
    int single;
    error_context_t econ;

    compilation_output_file = 0;
    string_needs_free = 0;

    save_context(&econ);
    if (SETJMP(econ.context)) {
	restore_context(&econ);
	pop_context(&econ);
	if (compilation_output_file)
	    fclose(compilation_output_file);
	if (string_needs_free)
	    FREE(string_needs_free);
	return 0;
    }

    if (arg1->type != T_ARRAY) {
	tmp_arr.size = 1;
	tmp_arr.item[0] = *arg1;
	arr = &tmp_arr;
	
	single = 1;
	if (!out_fname) {
	    out_fname = out_name;
	    strcpy(out_name, SAVE_BINARIES);
	    strcat(out_fname, "lpc_to_c.c");
	}
    } else {
	arr = arg1->u.arr;

	single = 0;
	if (!out_fname) {
	    out_fname = out_name;
	    strcpy(out_name, SAVE_BINARIES);
	    strcat(out_fname, "/mudlib/");
	}
    }
    outp = out_fname + strlen(out_fname);
    while (*out_fname == '/')
	out_fname++;
	
    for (index = 0; index < arr->size; index++) {
	*outp = 0; /* go back to the base name */

	if (arr->item[index].type != T_STRING)
	    error("Bad type for filename in generate_source()\n");

	if (!strip_name(arr->item[index].u.string, name, sizeof name))
	    error("Filenames with consecutive /'s in them aren't allowed.\n");
	
	/*
	 * First check that the c-file exists.
	 */
	(void) strcpy(real_name, name);
	(void) strcat(real_name, ".c");
	if (stat(real_name, &c_st) == -1) 
	    error("Could not find '%s' to compile.\n", real_name);
	
	if (!legal_path(real_name)) {
	    debug_message("Illegal pathname: %s\n", real_name);
	    error("Illegal path name.\n");
	}

	if (!single) {
	    generate_identifier(ident, name);
	    strcat(out_fname, ident);
	    strcat(out_fname, ".c");
	}

	compilation_output_file = crdir_fopen(out_fname);
	if (compilation_output_file == 0) {
	    debug_perror("generate_source: fopen", out_fname);
	    error("Could not open output file '/%s'.\n", out_fname);
	}
	fprintf(compilation_output_file, "#include \"std.h\"\n#include \"interface.h\"\n#include \"lpc_to_c.h\"\n\n");
	
	done = 0;
	while (!done) {
	    if (comp_flag)
		debug_message(" compiling /%s ...", real_name);
	    f = open(real_name, O_RDONLY);
	    if (f == -1) {
		fclose(compilation_output_file);
		compilation_output_file = 0;
		debug_perror("generate_source", real_name);
		error("Could not read the file '/%s'.\n", real_name);
	    }
	    generate_identifier(ident, name);
	    compilation_ident = ident;
	    compile_to_c = 1;
	    compile_file(f, real_name);
	    compile_to_c = 0;
	    if (comp_flag)
		debug_message(" done\n");
	    update_compile_av(total_lines);
	    close(f);
	    total_lines = 0;
	    
	    if (inherit_file == 0 && (num_parse_error > 0 || !prog)) {
		fclose(compilation_output_file);
		compilation_output_file = 0;
		if (prog)
		    free_prog(prog, 1);
		pop_context(&econ);
		return 0;
	    }
	    
	    if (inherit_file) {
		string_needs_free = inherit_file;
		
		if (prog) {
		    free_prog(prog, 1);
		    prog = 0;
		}
		if (strcmp(inherit_file, name) == 0) {
		    fclose(compilation_output_file);
		    compilation_output_file = 0;
		    FREE(inherit_file);
		    string_needs_free = inherit_file = 0;
		    error("Illegal to inherit self.\n");
		}
		inherit_file = 0;
		if (!load_object(string_needs_free, 0))
		    error("Attempted to inherit a non-existent file.");

		FREE(string_needs_free);
		string_needs_free = 0;
	    } else
		done = 1;
	}
	if (prog) {
	    free_prog(prog, 1);
	    prog = 0;
	}
	fclose(compilation_output_file);
	compilation_output_file = 0;
    }
    if (!single) {
	*outp = 0;
	strcat(out_fname, "interface.c");
	compilation_output_file = crdir_fopen(out_fname);
	if (compilation_output_file == 0) {
	    debug_perror("generate_source: fopen", out_fname);
	    error("Could not open output file '/%s'.\n", out_fname);
	}
	*outp = 0;
	strcat(out_fname, "Makefile.master");
	makefile = crdir_fopen(out_fname);
	if (makefile == 0) {
	    fclose(compilation_output_file);
	    compilation_output_file = 0;
	    debug_perror("generate_source: fclose", out_fname);
	    error("Could not open output file '/%s'.\n", out_fname);
	}
	
	fprintf(compilation_output_file, "#include \"std.h\"\n#include \"interface.h\"\n#include \"lpc_to_c.h\"\n\n");
	fprintf(makefile, "OBJ=");
	for (index = 0; index < arr->size; index++) {
	    strip_name(arr->item[index].u.string, name, sizeof name);
	    generate_identifier(ident, name);
	    fprintf(compilation_output_file, "extern interface_t LPCINFO_%s;\n", ident);
	}
	fprintf(compilation_output_file, "\n\ninterface_t *interface[] = {\n");
	for (index = 0; index < arr->size; index++) {
	    strip_name(arr->item[index].u.string, name, sizeof name);
	    generate_identifier(ident, name);
	    fprintf(compilation_output_file, "    &LPCINFO_%s,\n", ident);
	    fprintf(makefile, "%s.o ", ident);
	}
	fprintf(makefile, "interface.o\nSRC=");
	for (index = 0; index < arr->size; index++) {
	    strip_name(arr->item[index].u.string, name, sizeof name);
	    generate_identifier(ident, name);
	    fprintf(makefile, "%s.c ", ident);
	}
	fprintf(compilation_output_file, "    0\n};\n");
	fclose(compilation_output_file);
	compilation_output_file = 0;
	fprintf(makefile, rest_of_makefile);
	fclose(makefile);

	*outp = 0;
	strcat(out_fname, "Makefile.pre");
	makefile = crdir_fopen(out_fname);
	if (makefile == 0) {
	    debug_perror("generate_source: fclose", out_fname);
	    error("Could not open output file '/%s'.\n", out_fname);
	}
	fprintf(makefile, "%%define NORMAL\n\n%%include \"mudlib/Makefile.master\"\n\n");
	fclose(makefile);

	*outp = 0;
	strcat(out_fname, "GNUmakefile.pre");
	makefile = crdir_fopen(out_fname);
	if (makefile == 0) {
	    debug_perror("generate_source: fclose", out_fname);
	    error("Could not open output file '/%s'.\n", out_fname);
	}
	fprintf(makefile, "%%define GNU\n\n%%include \"mudlib/Makefile.master\"\n\n");
	fclose(makefile);
    }
    pop_context(&econ);
    return 1;
}
#endif
