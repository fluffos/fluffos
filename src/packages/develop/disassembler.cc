/*
 * Dump information about a program, optionally disassembling it.
 */

#include "base/package_api.h"

#include "packages/core/file.h"

#ifdef F_DUMP_PROG
void f_dump_prog(void) {
  program_t *prog;
  const char *where;
  int d;
  object_t *ob;
  int narg = st_num_arg;

  if (st_num_arg == 2) {
    if ((sp - 1)->type != T_OBJECT) {
      bad_argument(sp - 1, T_OBJECT, 1, F_DUMP_PROG);
    }

    ob = sp[-1].u.ob;
    d = sp->u.number;
    where = nullptr;
  } else if (st_num_arg == 3) {
    if ((sp - 2)->type != T_OBJECT) {
      bad_argument(sp - 2, T_OBJECT, 1, F_DUMP_PROG);
    }

    ob = sp[-2].u.ob;
    d = sp[-1].u.number;
    where = (sp->type == T_STRING) ? sp->u.string : nullptr;
  } else {
    if (sp->type != T_OBJECT) {
      bad_argument(sp, T_OBJECT, 1, F_DUMP_PROG);
    }

    ob = sp->u.ob;
    d = 0;
    where = nullptr;
  }

  if (!(prog = ob->prog)) {
    error("No program for object.\n");
  } else {
    if (!where) {
      where = "/PROG_DUMP";
    }

    const char *fname;
    FILE *f;

    fname = check_valid_path(where, current_object, "dumpallobj", 1);

    if (!fname) {
      error("Invalid path '%s' for writing.\n", where);
      return;
    }
    f = fopen(fname, "w");
    if (!f) {
      error("Unable to open '/%s' for writing.\n", fname);
      return;
    }
    dump_prog(prog, f, d);
  }
  pop_n_elems(narg);
}

#endif
