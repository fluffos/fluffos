/*
 * Dump information about a program, optionally disassembling it.
 */

#include "base/package_api.h"

#include "packages/core/file.h"

#ifdef F_DUMP_PROG
void f_dump_prog(int num_arg) {
  LPCFrame f = LPCFrame::args(num_arg);
  program_t* prog;
  const char* where;
  int d;
  object_t* ob;

  if (num_arg == 2) {
    if (f.type(0) != T_OBJECT) {
      bad_argument(f.argp(0), T_OBJECT, 1, F_DUMP_PROG);
    }

    ob = f.object(0);
    d = static_cast<int>(f.number(1));
    where = nullptr;
  } else if (num_arg == 3) {
    if (f.type(0) != T_OBJECT) {
      bad_argument(f.argp(0), T_OBJECT, 1, F_DUMP_PROG);
    }

    ob = f.object(0);
    d = static_cast<int>(f.number(1));
    where = (f.type(2) == T_STRING) ? f.string(2) : nullptr;
  } else {
    if (f.type(0) != T_OBJECT) {
      bad_argument(f.argp(0), T_OBJECT, 1, F_DUMP_PROG);
    }

    ob = f.object(0);
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
  f.ret_void();
}

#endif
