#include "config.h"

#include <stdio.h>
#ifdef LATTICE
#include <stdlib.h>
#endif
#include <string.h>

#include "lint.h"
#include "interpret.h"
#include "object.h"
#include "exec.h"

static struct function *simul_efunp = 0;
static int num_simul_efun;
struct object *simul_efun_ob;

#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
/* root of simul_efun function table tree */
static unsigned short simul_efunr = (unsigned short) 0xffff;

#endif

/* Don't release this pointer ever. It is used elsewhere. */
char *simul_efun_file_name = 0;

/*
 * If there is a simul_efun file, then take care of it and extract all
 * information we need.
 */
void set_simul_efun P1(char *, file)
{
    char buf[512];

    if (!file) {
	fprintf(stderr, "No simul_efun\n");
	return;
    }
    while (file[0] == '/')
	file++;
    strcpy(buf, file);
    if (file[strlen(file) - 2] != '.')
	strcat(buf, ".c");
    simul_efun_file_name = make_shared_string(buf);

    simul_efun_ob = find_object2(simul_efun_file_name);

    if (simul_efun_ob == 0) {
	simul_efun_ob = load_object(simul_efun_file_name, 0);
	if (simul_efun_ob == 0) {
	    fprintf(stderr, "The simul_efun file %s was not loaded.\n",
		    simul_efun_file_name);
	    exit(-1);
	}
    }
    /* epilog() in compiler.y should call get_simul_efuns()   -bobf */
}

void get_simul_efuns P1(struct program *, prog)
{
    struct function *funp;
    int i;

    if (simul_efunp)
	FREE(simul_efunp);
    num_simul_efun = prog->p.i.num_functions;
    if (num_simul_efun == 0)
	return;

    funp = prog->p.i.functions;
    simul_efunp = (struct function *)
	DMALLOC(sizeof(struct function) * num_simul_efun, 96, "get_simul_efuns");
#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
    simul_efunr = prog->p.i.tree_r;
#endif

    for (i = 0; i < (int) prog->p.i.num_functions; i++) {
	simul_efunp[i].name = make_shared_string(funp[i].name);
	simul_efunp[i].flags = funp[i].flags;
	simul_efunp[i].num_arg = funp[i].num_arg;
	simul_efunp[i].type = funp[i].type & TYPE_MOD_MASK;
#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
	simul_efunp[i].tree_l = funp[i].tree_l;
	simul_efunp[i].tree_r = funp[i].tree_r;
	simul_efunp[i].tree_b = funp[i].tree_b;
#endif
    }
}

/*
 * Test if 'name' is a simul_efun. The string pointer MUST be a pointer to
 * a shared string.
 */
struct function *find_simul_efun P1(char *, name)
{
    int i;

#ifdef OPTIMIZE_FUNCTION_TABLE_SEARCH
    i = lookup_function(simul_efunp, simul_efunr, name);
    if (i != -1)
	return &simul_efunp[i];
#else
    for (i = 0; i < num_simul_efun; i++) {
	if (name == simul_efunp[i].name)
	    return &simul_efunp[i];
    }
#endif
    return 0;
}
