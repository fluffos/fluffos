#include <stdio.h>
#include <string.h>

#include "config.h"
#include "lint.h"
#include "interpret.h"
#include "object.h"
#include "exec.h"

static struct function *simul_efunp = 0;
static int num_simul_efun;

/* Don't release this pointer ever. It is used elsewhere. */
static char *simul_efun_file_name;

/*
 * If there is a simul_efun file, then take care of it and extract all
 * information we need.
 */
void set_simul_efun(file)
     char *file;
{
  struct object *ob;
  struct function *funp;
  int i;
  
  if (!file)
    {
      fprintf(stderr, "No simul_efun\n");
      return;
    }
  simul_efun_file_name = make_shared_string(file);
  
  ob = find_object2(simul_efun_file_name);
  
  if (ob == 0) 
    {
      ob = load_object (simul_efun_file_name, 0);
      if (ob == 0)
	{
	  fprintf(stderr, "The simul_efun file %s was not loaded.\n",
		  simul_efun_file_name);
	  exit(-1);
	}
    }
  num_simul_efun = ob->prog->p.i.num_functions;
  if (num_simul_efun == 0)
    return;
  funp = ob->prog->p.i.functions;
  simul_efunp = (struct function *)
    DMALLOC(sizeof (struct function) * num_simul_efun,96,"set_simul_efun");
  for (i=0; i < (int)ob->prog->p.i.num_functions; i++) {
    simul_efunp[i].name = make_shared_string(funp[i].name);
    simul_efunp[i].flags = funp[i].flags;
    simul_efunp[i].num_arg = funp[i].num_arg;
    simul_efunp[i].type = funp[i].type & TYPE_MOD_MASK;
  }
}

/*
 * Test if 'name' is a simul_efun. The string pointer MUST be a pointer to
 * a shared string.
 */
struct function *find_simul_efun(name)
    char *name;
{
    int i;
    for (i=0; i < num_simul_efun; i++) {
	if (name == simul_efunp[i].name)
	    return &simul_efunp[i];
    }
    return 0;
}

char *query_simul_efun_file_name() {
#ifdef DEBUG
    if (simul_efunp == 0)
	fatal("query_simul_efun_file_name called when none exists!\n");
#endif
    return simul_efun_file_name;
}
