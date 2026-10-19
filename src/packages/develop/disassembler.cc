/*
 * Dump information about a program, optionally disassembling it.
 */

#include "base/package_api.h"

#include "packages/core/file.h"

#ifdef F_DUMP_PROG
void f_dump_prog(vm::EfunArgs& f, object_t* ob) {
  int num_arg = f.argc();
  program_t* prog;
  const char* where;
  int d;

  /* the flags are an untyped `...` argument, read unchecked as before */
  if (num_arg == 2) {
    d = static_cast<int>(f[1].sv()->u.number);
    where = nullptr;
  } else if (num_arg == 3) {
    d = static_cast<int>(f[1].sv()->u.number);
    where = f[2].is(T_STRING) ? f[2].string().c_str() : nullptr;
  } else {
    d = 0;
    where = nullptr;
  }

  if (!(prog = ob->prog)) {
    error("No program for object.\n");
  } else {
    if (!where) {
      where = "/PROG_DUMP";
    }

    const char* fname;
    FILE* file;

    fname = check_valid_path(where, current_object, "dumpallobj", 1);

    if (!fname) {
      error("Invalid path '%s' for writing.\n", where);
      return;
    }
    file = fopen(fname, "w");
    if (!file) {
      error("Unable to open '/%s' for writing.\n", fname);
      return;
    }
    dump_prog(prog, file, d);
    fclose(file);
  }
}

#endif
