#include "std.h"
#include "file_incl.h"
#include "lpc_incl.h"
#include "interface.h"
#include "lex.h"
#include "compiler.h"
#include "md.h"

#ifdef LPC_TO_C
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
    interface_item_t *p = interface;
    lpc_object_t *ob;

    while (p->fname) {
	ob = ALLOCATE(lpc_object_t, TAG_LPC_OBJECT, "init_lpc_to_c");
	ob->name = string_copy(p->fname, "init_lpc_to_c");
	SET_TAG(ob->name, TAG_OBJ_NAME);
	enter_object_hash(ob);
	ob->flags = O_COMPILED_PROGRAM;
	ob->jump_table = p->jump_table;
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

int generate_source P2(svalue_t *, arg1, char *, out_fname)
{
    FILE *crdir_fopen();
    FILE *specfile;
    int len;

    struct stat c_st;
    char real_name[200];
    char name[200];
    char out_name[200];
    char ident[205];
    int done;
    char *p;
    int index;
    array_t tmp_arr, *arr;
    int f;

    if (out_fname)
	while (*out_fname == '/')
	    out_fname++;
    compilation_output_file = 0;

    if (arg1->type != T_ARRAY) {
	tmp_arr.size = 1;
	tmp_arr.item[0] = arg1->u.arr->item[0];
	arr = &tmp_arr;
    } else {
	arr = arg1->u.arr;
	
	if (!out_fname) {
	    out_fname = out_name;
	    strcpy(out_name, SAVE_BINARIES);
	    strcat(out_fname, "/interface.c");
	    while (*out_fname == '/')
		out_fname++;
	}
	
	compilation_output_file = crdir_fopen(out_fname);
	fprintf(compilation_output_file, "#include \"std.h\"\n\n#include \"interface.h\"\n\n");
    }

    for (index = 0; index < arr->size; index++) {
	if (arr->item[index].type != T_STRING) {
	    if (arg1->type == T_ARRAY)
		fclose(compilation_output_file);
	    error("Bad type for filename in generate_source()\n");
	}
	if (!strip_name(arr->item[index].u.string, name, sizeof name)) {
	    if (arg1->type == T_ARRAY)
		fclose(compilation_output_file);
	    error("Filenames with consecutive /'s in them aren't allowed.\n");
	}
	/*
	 * First check that the c-file exists.
	 */
	(void) strcpy(real_name, name);
	(void) strcat(real_name, ".c");
	if (stat(real_name, &c_st) == -1) {
	    if (arg1->type == T_ARRAY)
		fclose(compilation_output_file);
	    return 0;
	}
	if (!legal_path(real_name)) {
	    fprintf(stderr, "Illegal pathname: %s\n", real_name);
	    if (arg1->type == T_ARRAY)
		fclose(compilation_output_file);
	    error("Illegal path name.\n");
	    return 0;
	}
	if (!out_fname && arg1->type != T_ARRAY) {
	    out_fname = out_name;
	    strcpy(out_name, SAVE_BINARIES);
	    strcat(out_fname, "/");
	    strcat(out_fname, name);
	    strcat(out_fname, ".c");
	    while (*out_fname == '/')
		out_fname++;
	}
	
	done = 0;
	while (!done) {
	    if (comp_flag)
		fprintf(stderr, " compiling %s ...", real_name);
	    f = open(real_name, O_RDONLY);
	    if (f == -1) {
		if (arg1->type == T_ARRAY)
		    fclose(compilation_output_file);
		perror(real_name);
		error("Could not read the file '%s'.\n", real_name);
	    }
	    if (arg1->type == T_STRING) {
		compilation_output_file = crdir_fopen(out_fname);
		if (compilation_output_file == 0) {
		    perror(out_fname);
		    error("Could not open output file '%s'.\n", out_fname);
		}
	    }
	    generate_identifier(ident, name);
	    compilation_ident = ident;
	    compile_to_c = 1;
	    compile_file(f, real_name);
	    compile_to_c = 0;
	    if (arg1->type == T_STRING)
		fclose(compilation_output_file);
	    if (comp_flag)
		fprintf(stderr, " done\n");
	    update_compile_av(total_lines);
	    close(f);
	    total_lines = 0;
	    
	    if (inherit_file == 0 && (num_parse_error > 0 || !prog)) {
		if (arg1->type == T_ARRAY)
		    fclose(compilation_output_file);
		if (prog)
		    free_prog(prog, 1);
		return 0;
	    }
	    
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
	    } else {
		done = 1;
		if (pragmas & PRAGMA_EFUN) {
		    strcpy(out_name, out_fname);
		    len = strlen(out_name);
		    if (out_name[len-1] == 'c' && out_name[len-2] == '.') {
			len -= 2;
			out_name[len] = '\0';
		    }
		    strcat(out_name, ".spec");
		    specfile = crdir_fopen(out_name);
		    if (specfile == 0) {
			if (arg1->type == T_ARRAY)
			    fclose(compilation_output_file);
			perror(out_fname);
			error("Could not open output file '%s.'\n", out_fname);
		    }
		    out_name[len] = '\0';
		    fprintf(specfile, "package %s;\n\n", out_name);
		    if (prog) {
			int n = prog->num_functions;
			function_t *functions = prog->functions;
			unsigned short *types;
			while (n--) {
			    int i = 0;
			    
			    if (prog->type_start && prog->type_start[n] != INDEX_START_NONE)
				types = &prog->argument_types[prog->type_start[n]];
			    else 
				types = 0;
			    
			    if (functions[n].flags & 
				(NAME_NO_CODE | NAME_INHERITED)) continue;
			    
			    fprintf(specfile, "%s%s( ", 
				    get_type_name(functions[n].type & TYPE_MOD_MASK),
				    functions[n].name);
			    if (functions[n].num_arg)
				while (1) {
				    fprintf(specfile, "%s%s",
					    (functions[n].type & TYPE_MOD_VARARGS) ?
					    "void | " : "", get_type_name(types[i]));
				    if (++i != functions[n].num_arg)
					fprintf(specfile, ", ");
				    else break;
				}
			    fprintf(specfile, ");\n");
		    }
		    }
		    fclose(specfile);
		}
	    }
	}
	if (prog) {
	    free_prog(prog, 1);
	    prog = 0;
	}
    }
    if (arg1->type == T_ARRAY) {
	fprintf(compilation_output_file, "\n\ninterface_item_t interface[] = {\n");
	for (index = 0; index < arr->size; index++) {
	    strip_name(arr->item[index].u.string, name, sizeof name);
	    generate_identifier(ident, name);
	    fprintf(compilation_output_file, "    { \"%s\", LPCFUNCS_%s },\n",
		    name, ident);
	}
	fprintf(compilation_output_file, "    { 0, 0 }\n};\n");
	fclose(compilation_output_file);
    }
    return 1;
}
#endif
