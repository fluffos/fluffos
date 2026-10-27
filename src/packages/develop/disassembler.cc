/*
 * Dump information about a program, optionally disassembling it.
 */

#include "base/package_api.h"

#include "packages/core/file.h"

#ifdef F_DUMP_PROG
void f_dump_prog(vm::EfunArgs&, object_t* ob, std::optional<LPC_INT> flags,
                 std::optional<vm::StringArg> file_arg) {
  program_t* prog;
  int const d = static_cast<int>(flags.value_or(0));
  const char* where = file_arg ? file_arg->c_str() : "/PROG_DUMP";

  if (!(prog = ob->prog)) {
    error("No program for object.\n");
  } else {
    const char* fname;
    FILE* file;

    vm::Value fnamev = check_valid_path(where, current_object, "dumpallobj", 1);

    if (!fnamev.is(T_STRING)) {
      error("Invalid path '%s' for writing.\n", where);
      return;
    }
    fname = fnamev->u.string;
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
