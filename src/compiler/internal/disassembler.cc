#include "base/std.h"

#include "compiler/internal/disassembler.h"

#include "vm/vm.h"
#include "compiler/internal/lexer.h"
#include "compiler/internal/icode.h"

#include <fmt/format.h>
#include <nlohmann/json.hpp>

// --------------------------------------------------------------------------
// JSON-native dumps: the ONE opcode switch in disassemble() emits through a
// DisSink with two backends -- text (the exact legacy dump format) and a
// nlohmann::json model (lpcc --json bytecode envelope). There is no second
// decoder to drift; the text output is a rendering of the same emissions.
// --------------------------------------------------------------------------
struct DisSink {
  FILE* f = nullptr;             // text backend (legacy format) when non-null
  nlohmann::json* fns = nullptr; // json backend: array of function objects
  nlohmann::json* cur = nullptr; // current function's "instructions" array
  const char* src_file = nullptr;  // current source position (json rows
  int src_line = 0;                // carry it natively, no trailing annots)
};

static void disassemble(DisSink& sink, char* code /*code*/, int /*start*/ start, int /*end*/ end,
                        program_t* prog /*prog*/);
static const char* disassem_string(const char* /*str*/);
static int short_compare(const void* /*a*/, const void* /*b*/);
static void dump_line_numbers(FILE* /*f*/, program_t* /*prog*/);

void dump_prog_details(program_t* prog, FILE* f, int flags) {
  int i, j;

  fprintf(f, "\n;;; %s\n\n", prog->filename);

  fprintf(f, "Globals:\n");
  for (i = 0; i < prog->num_variables_total; i++) {
    fprintf(f, "%4d: %s\n", i, variable_name(prog, i));
  }

  int variable_runtime_index = 0;
  if (prog->num_inherited > 0) {
    variable_runtime_index = prog->inherit[prog->num_inherited - 1].variable_index_offset +
                             prog->inherit[prog->num_inherited - 1].prog->num_variables_total;
  }
  fprintf(f, "VARIABLES defined:\n");
  for (i = 0; i < prog->num_variables_defined; i++) {
    char buf[255];
    auto end = &buf[sizeof(buf) - 1];
    get_type_name(&buf[0], end, prog->variable_types[i]);
    fprintf(f, "%4d: %s%s\n", variable_runtime_index + i, buf, prog->variable_table[i]);
  }
  fprintf(f, "STRINGS:\n");
  for (i = 0; i < prog->num_strings; i++) {
    fprintf(f, "%4d: ", i);
    for (j = 0; j < 32; j++) {
      char c;

      if (!(c = prog->strings[i][j])) {
        break;
      } else if (c == '\n') {
        fprintf(f, "\\n");
      } else {
        fputc(c, f);
      }
    }
    fputc('\n', f);
  }

  if (flags & 1) {
    fprintf(f, "DISASSEMBLY:\n");
    DisSink sink;
    sink.f = f;
    disassemble(sink, prog->program, 0, prog->program_size, prog);
  } else {
    fprintf(f, "PROGRAM:");
    for (i = 0; i < prog->program_size; i++) {
      if (i % 16 == 0) {
        fprintf(f, "\n\t%04x: ", static_cast<unsigned int>(i));
      }
      fprintf(f, "%02d ", static_cast<unsigned char>(prog->program[i]));
    }
    fputc('\n', f);
  }
  if (flags & 2) {
    fprintf(f, "\n;;;  *** Line Number Info ***\n");
    dump_line_numbers(f, prog);
  }

  for (int i = 0; i < prog->num_inherited; i++) {
    dump_prog_details(prog->inherit[i].prog, f, flags);
  }
}

/* Current flags:
 * 1 - do disassembly
 * 2 - dump line number table
 */
// One row of the FUNCTIONS table, computed once for the text and json
// renderers (single source for the flag-char encodings).
struct FnTabRow {
  int index;
  const char* name;
  bool inherited;
  char smods[4];
  char sflags[8];
  // inherited kind:
  int inh_low = 0, inh_fio = 0, inh_vio = 0;
  // defined kind:
  int offset = 0, locals = 0, args = 0, def_args = 0;
  std::string defmap;
};

static void fn_table_rows(program_t* prog, std::vector<FnTabRow>& rows) {
  int num_funcs_total = prog->last_inherited + prog->num_functions_defined;
  for (int i = 0; i < num_funcs_total; i++) {
    FnTabRow r;
    int flags;
    int runtime_index;
    function_t* func_entry = find_func_entry(prog, i);
    int low, high, mid;

    r.index = i;
    r.name = func_entry->funcname;

    flags = prog->function_flags[i];
    if (flags & FUNC_ALIAS) {
      runtime_index = flags & ~FUNC_ALIAS;
      r.sflags[4] = 'a';
    } else {
      runtime_index = i;
      r.sflags[4] = '-';
    }

    flags = prog->function_flags[runtime_index];

    r.smods[0] = (flags & DECL_HIDDEN) ? '-' : '-';
    r.smods[0] = (flags & DECL_PRIVATE) ? 'p' : '-';
    r.smods[0] = (flags & DECL_PROTECTED) ? 'P' : '-';
    r.smods[0] = (flags & DECL_PUBLIC) ? '+' : '-';
    r.smods[1] = (flags & DECL_NOMASK) ? 'm' : '-';
    r.smods[2] = (flags & DECL_NOSAVE) ? 's' : '-';
    r.smods[3] = '\0';

    r.sflags[0] = (flags & FUNC_INHERITED) ? 'i' : '-';
    r.sflags[1] = (flags & FUNC_UNDEFINED) ? 'u' : '-';
    r.sflags[2] = (flags & FUNC_STRICT_TYPES) ? 's' : '-';
    r.sflags[3] = (flags & FUNC_PROTOTYPE) ? 'p' : '-';
    r.sflags[5] = (flags & FUNC_TRUE_VARARGS) ? 'V' : '-';
    r.sflags[6] = (flags & FUNC_VARARGS) ? 'v' : '-';
    r.sflags[7] = '\0';

    r.inherited = (flags & FUNC_INHERITED) != 0;
    if (r.inherited) {
      low = 0;
      high = prog->num_inherited - 1;
      while (high > low) {
        mid = (low + high + 1) / 2;
        if (prog->inherit[mid].function_index_offset > runtime_index) {
          high = mid - 1;
        } else {
          low = mid;
        }
      }
      r.inh_low = low;
      r.inh_fio = runtime_index - prog->inherit[low].function_index_offset;
      r.inh_vio = prog->inherit[low].variable_index_offset;
    } else {
      r.offset = runtime_index - prog->last_inherited;
      r.locals = func_entry->num_local;
      r.args = func_entry->num_arg;
      r.def_args = func_entry->num_arg - func_entry->min_arg;
      for (int j = 0; j < func_entry->num_arg; j++) {
        if (func_entry->default_args_findex[j] != 0) {
          r.defmap += fmt::format(FMT_STRING(" {}:{}"), j, func_entry->default_args_findex[j]);
        }
      }
    }
    rows.push_back(std::move(r));
  }
}

void dump_prog(program_t* prog, FILE* f, int flags) {
  int i;

  fprintf(f, "NAME: /%s\n", prog->filename);
  fprintf(f, "INHERITS:\n");
  fprintf(f, "      name                    fio    vio\n");
  fprintf(f, "      ----------------        ---    ---\n");

  for (i = 0; i < prog->num_inherited; i++) {
    fprintf(f, "\t%-20s  %5d  %5d\n", prog->inherit[i].prog->filename,
            prog->inherit[i].function_index_offset, prog->inherit[i].variable_index_offset);
  }
  fprintf(f, "FUNCTIONS:\n");
  fprintf(f,
          "      name                      offset  mods   flags   fio  vio # locals  # args # def "
          "args\n");
  fprintf(f,
          "      ------------------------- ------  ----  -------  ---  --- --------  ------ "
          "----------\n");
  std::vector<FnTabRow> rows;
  fn_table_rows(prog, rows);
  for (const auto& r : rows) {
    if (r.inherited) {
      fprintf(f, "%4d: %-24s  %6d  %4s  %7s  %3d %3d\n", r.index, r.name, r.inh_low, r.smods,
              r.sflags, r.inh_fio, r.inh_vio);
    } else {
      fprintf(f, "%4d: %-24s  %6d  %4s  %7s             %7d   %5d %10d", r.index, r.name, r.offset,
              r.smods, r.sflags, r.locals, r.args, r.def_args);
      fprintf(f, " %s\n", r.defmap.c_str());
    }
  }

  dump_prog_details(prog, f, flags);
}

static const char* disassem_string(const char* str) {
  static char buf[30 * 2 + 1];
  char* b;
  int i;

  if (!str) {
    return "0";
  }

  b = buf;
  for (i = 0; i < 29; i++) {
    if (!str[i]) {
      break;
    }
    if (str[i] == '\n') {
      *b++ = '\\';
      *b++ = 'n';
    } else {
      *b++ = str[i];
    }
  }
  *b++ = 0;
  return buf;
}

#define NUM_FUNS (prog->num_functions_defined + prog->last_inherited)
#define NUM_FUNS_D prog->num_functions_defined
#define VARS prog->variable_names
#define NUM_VARS prog->num_variables_total
#define STRS prog->strings
#define NUM_STRS prog->num_strings
#define CLSS prog->classes

static int short_compare(const void* a, const void* b) {
  int x = *(unsigned short*)a;
  int y = *(unsigned short*)b;

  return x - y;
}

static const char* pushes[] = {"string", "number", "global", "local"};

static std::string function_sig_string(program_t* prog, int idx) {
  char buf[255];
  auto end = &buf[sizeof(buf) - 1];
  std::string out;

  auto funp = prog->function_table[idx];
  auto funflags = prog->function_flags[prog->last_inherited + idx];

  buf[0] = '\0';
  get_type_modifiers(&buf[0], end, funflags);
  out += buf;

  get_type_name(&buf[0], end, funp.type);
  out += buf;
  out += funp.funcname;

  out += "(";
  unsigned short* types;
  if (prog->type_start && prog->type_start[idx] != INDEX_START_NONE) {
    types = &prog->argument_types[prog->type_start[idx]];
  } else {
    types = nullptr;
  }
  if (funp.num_arg > 0) {
    if (types) {
      for (int i = 0; i < funp.num_arg; i++) {
        auto p = get_type_name(buf, end, types[i]);
        *(p - 1) = '\0';  // get rid of last space
        if (i != 0) out += ",";
        out += buf;
      }
    } else {
      out += fmt::format(FMT_STRING("args: {}"), funp.num_arg);
      if (funp.min_arg != funp.num_arg) {
        out += fmt::format(FMT_STRING("min args: {}"), funp.num_arg);
      }
    }
  }
  out += ")";
  return out;
}

static void disassemble(DisSink& sink, char* code, int start, int end, program_t* prog) {
  extern int num_simul_efun;
  short instr;
  int i, j, ri;
  LPC_INT iarg;
  unsigned short sarg;
  unsigned short offset;
  char *pc, buff[2048];
  int next_func;

  short* offsets;

  if (start == 0) {
    /* sort offsets of functions */
    offsets = reinterpret_cast<short*>(malloc(NUM_FUNS_D * 2 * sizeof(short)));
    for (i = 0; i < NUM_FUNS_D; i++) {
      ri = i + prog->last_inherited;

      if (prog->function_flags[ri] & FUNC_NO_CODE) {
        offsets[i << 1] = end + 1;
      } else {
        offsets[i << 1] = prog->function_table[i].address;
      }
      offsets[(i << 1) + 1] = i;
    }
    qsort(reinterpret_cast<char*>(&offsets[0]), NUM_FUNS_D, sizeof(short) * 2, short_compare);
    next_func = 0;
  } else {
    offsets = nullptr;
    next_func = -1;
  }

  pc = code + start;

  const char* last_file = nullptr;
  int last_line = 0;

  while ((pc - code) < end) {
    if ((next_func >= 0) && ((pc - code) >= offsets[next_func])) {
      if (next_func > 0) {
        if (sink.f && last_file && last_line > 0) {
          fprintf(sink.f, "; %s:%d\n", last_file, last_line);
        }
        last_file = nullptr;
        last_line = 0;
      }
      auto func_idx = offsets[next_func + 1];
      std::string sig = function_sig_string(prog, func_idx);
      if (sink.f) {
        fprintf(sink.f, "\n;; Function: %s\n", sig.c_str());
      }
      if (sink.fns) {
        sink.fns->push_back({{"sig", sig},
                             {"name", prog->function_table[func_idx].funcname},
                             {"instructions", nlohmann::json::array()}});
        sink.cur = &sink.fns->back()["instructions"];
      }

      next_func += 2;
      if (next_func >= (NUM_FUNS_D * 2)) {
        next_func = -1;
      }
    }

    auto saved_pc = pc;
    instr = *pc++;
    buff[0] = 0;
    sarg = 0;

    {
      const char* new_file = nullptr;
      int new_line = 0;
      get_explicit_line_number_info(pc, prog, &new_file, &new_line);
      if (last_file != new_file || last_line != new_line) {
        if (sink.f && last_file && last_line > 0) {
          fprintf(sink.f, "; %s:%d\n", last_file, last_line);
        }
        last_file = new_file;
        last_line = new_line;
      }
      sink.src_file = new_file;
      sink.src_line = new_line;
    }

    size_t iaddr = (pc - 1) - code;
    if (sink.f) {
      fflush(sink.f);
      fprintf(sink.f, "%04tx: ", (pc - 1) - code);  // Address
    }

    switch (instr) {
      case F_PUSH: {
        auto p = buff;
        p += sprintf(p, "push ");
        i = EXTRACT_UCHAR(pc++);
        while (i--) {
          j = EXTRACT_UCHAR(pc++);
          p += sprintf(p, "%s %d", pushes[(j & PUSH_WHAT) >> 6], j & PUSH_MASK);
          if (i) {
            p += sprintf(p, ", ");
          } else {
            break;
          }
        }
        break;
      }
        /* Single numeric arg */
      case F_BRANCH_NE:
      case F_BRANCH_GE:
      case F_BRANCH_LE:
      case F_BRANCH_EQ:
      case F_BRANCH:
      case F_BRANCH_WHEN_ZERO:
      case F_BRANCH_WHEN_NON_ZERO:
#ifdef F_LOR
      case F_LOR:
      case F_LAND:
#endif
#ifdef F_NULLISH
      case F_NULLISH:
#endif
      case F_LOR_EQ:
      case F_LAND_EQ:
      case F_NULLISH_EQ:
        COPY_SHORT(&sarg, pc);
        offset = (pc - code) + sarg;
        sprintf(buff, "%04x (%04x)", static_cast<unsigned>(sarg), static_cast<unsigned>(offset));
        pc += 2;
        break;

      case F_NEXT_FOREACH:
      case F_BBRANCH_LT:
        COPY_SHORT(&sarg, pc);
        offset = (pc - code) - sarg;
        sprintf(buff, "%04x (%04x)", static_cast<unsigned>(sarg), static_cast<unsigned>(offset));
        pc += 2;
        break;
      case F_FOREACH: {
        int flags = EXTRACT_UCHAR(pc++);
        const char *left = "local", *right = "local";

        if (flags & FOREACH_LEFT_GLOBAL) {
          left = "global";
        }
        if (flags & FOREACH_RIGHT_GLOBAL) {
          right = "global";
        }
        if (flags & FOREACH_REF) {
          if (flags & FOREACH_MAPPING) {
            right = "ref";
          } else {
            left = "ref";
          }
        }

        if (flags & FOREACH_MAPPING) {
          char* tmp = pc++;
          sprintf(buff, "(mapping) %s %i, %s %i", left, EXTRACT_UCHAR(tmp), right,
                  EXTRACT_UCHAR(pc++));
        } else {
          sprintf(buff, "(array | string) %s %i", left, EXTRACT_UCHAR(pc++));
        }
        break;
      }

      case F_BBRANCH_WHEN_ZERO:
      case F_BBRANCH_WHEN_NON_ZERO:
      case F_BBRANCH:
        COPY_SHORT(&sarg, pc);
        offset = (pc - code) - sarg;
        sprintf(buff, "%04x (%04x)", static_cast<unsigned>(sarg), static_cast<unsigned>(offset));
        pc += 2;
        break;

#ifdef F_JUMP
      case F_JUMP:
#endif
#ifdef F_JUMP_WHEN_ZERO
      case F_JUMP_WHEN_ZERO:
      case F_JUMP_WHEN_NON_ZERO:
#endif
      case F_CATCH:
        COPY_SHORT(&sarg, pc);
        sprintf(buff, "%04x", static_cast<unsigned>(sarg));
        pc += 2;
        break;

      case F_AGGREGATE:
      case F_AGGREGATE_ASSOC:
        COPY_SHORT(&sarg, pc);
        sprintf(buff, "%d", sarg);
        pc += 2;
        break;

      case F_MAKE_REF:
      case F_KILL_REFS:
      case F_MEMBER:
      case F_MEMBER_LVALUE:
        sprintf(buff, "%d", EXTRACT_UCHAR(pc++));
        break;

      case F_MAP_MEMBER:
      case F_MAP_MEMBER_LVALUE:
      case F_MAP_MEMBER_OPTIONAL:
        COPY_SHORT(&sarg, pc);
        sprintf(buff, "%d", sarg);
        pc += 2;
        break;
      case F_MAP_INDEX_OPTIONAL:
        /* no operand */
        break;

      case F_EXPAND_VARARGS: {
        int which = EXTRACT_UCHAR(pc++);
        if (which) {
          sprintf(buff, "%d from top of stack", which);
        } else {
          strcpy(buff, "top of stack");
        }
      } break;

      case F_NEW_EMPTY_CLASS:
      case F_NEW_CLASS: {
        int which = EXTRACT_UCHAR(pc++);
        strcpy(buff, STRS[CLSS[which].classname]);
        break;
      }

      case F_CALL_FUNCTION_BY_ADDRESS: {
        COPY_SHORT(&sarg, pc);
        pc += sizeof(short);
        const uint8_t args = EXTRACT_UCHAR(pc++);
        if (sarg < NUM_FUNS) {
          sprintf(buff, "%s, pushed_args:%d", function_name(prog, sarg), args);
        } else {
          sprintf(buff, "<out of range %d>", sarg);
        }
      } break;

      case F_CALL_INHERITED: {
        program_t* newprog;

        newprog = (prog->inherit + EXTRACT_UCHAR(pc++))->prog;
        COPY_SHORT(&sarg, pc);
        pc += 3;
        if (sarg < (newprog->num_functions_defined + newprog->last_inherited)) {
          sprintf(buff, "%30s::%-12s %5d", newprog->filename, function_name(newprog, sarg), sarg);
        } else {
          sprintf(buff, "<out of range in %30s - %d>", newprog->filename, sarg);
        }
        break;
      }
      case F_GLOBAL_LVALUE:
      case F_GLOBAL: {
        short iarg;
        LOAD2(iarg, pc);
        if (iarg < NUM_VARS) {
          sprintf(buff, "%s(%d)", variable_name(prog, iarg), iarg);
        } else {
          sprintf(buff, "<out of range %d >", iarg);
        }
        break;
      }
      case F_LOOP_INCR:
        sprintf(buff, "LV%d", EXTRACT_UCHAR(pc));
        pc++;
        break;
      case F_WHILE_DEC:
        COPY_SHORT(&sarg, pc + 1);
        offset = (pc - code) - sarg;
        sprintf(buff, "LV%d--, branch %04x (%04x)", EXTRACT_UCHAR(pc), static_cast<unsigned>(sarg),
                static_cast<unsigned>(offset));
        pc += 3;
        break;
      case F_TRANSFER_LOCAL:
      case F_LOCAL:
      case F_LOCAL_LVALUE:
      case F_VOID_ASSIGN_LOCAL:
      case F_REF:
      case F_REF_LVALUE:
        sprintf(buff, "LV%d", EXTRACT_UCHAR(pc));
        pc++;
        break;
      case F_LOOP_COND_NUMBER:
        // Layout per do_loop_cond_number(): 1-byte local, sizeof(LPC_INT)
        // (8-byte) literal, 2-byte BACKWARD branch offset. This used to
        // advance only 4 past the literal, desyncing the address stream --
        // every later instruction in the section decoded as garbage and the
        // next function's listing was swallowed.
        i = EXTRACT_UCHAR(pc++);
        COPY_INT(&iarg, pc);
        pc += sizeof(LPC_INT);
        COPY_SHORT(&sarg, pc);
        offset = (pc - code) - sarg;
        pc += 2;
        sprintf(buff, "LV%d < %" LPC_INT_FMTSTR_P " branch back %04x (%04x)", i, iarg, sarg,
                offset);
        break;
      case F_LOOP_COND_LOCAL:
        i = EXTRACT_UCHAR(pc++);
        iarg = *pc++;
        COPY_SHORT(&sarg, pc);
        offset = (pc - code) - sarg;
        pc += 2;
        sprintf(buff, "LV%d < LV%" LPC_INT_FMTSTR_P " bbranch_when_non_zero %04x (%04x)", i, iarg,
                sarg, offset);
        break;
      case F_STRING:
        COPY_SHORT(&sarg, pc);
        if (sarg < NUM_STRS) {
          sprintf(buff, "\"%s\"", disassem_string(STRS[sarg]));
        } else {
          sprintf(buff, "<out of range %d>", sarg);
        }
        pc += 2;
        break;
      case F_SHORT_STRING:
        if (EXTRACT_UCHAR(pc) < NUM_STRS) {
          sprintf(buff, "\"%s\"", disassem_string(STRS[EXTRACT_UCHAR(pc)]));
        } else {
          sprintf(buff, "<out of range %d>", EXTRACT_UCHAR(pc));
        }
        pc++;
        break;
      case F_SIMUL_EFUN:
        COPY_SHORT(&sarg, pc);
        if (sarg >= num_simul_efun || !simuls[sarg].func) {
          sprintf(buff, "<invalid %d> %d\n", sarg, pc[2]);
        } else {
          sprintf(buff, "\"%s\" args: %d", simuls[sarg].func->funcname, pc[2]);
        }
        pc += 3;
        break;

      case F_FUNCTION_CONSTRUCTOR:
        switch (EXTRACT_UCHAR(pc++)) {
          case FP_SIMUL:
            LOAD_SHORT(sarg, pc);
            sprintf(buff, "<simul_efun> \"%s\"",
                    (sarg < num_simul_efun && simuls[sarg].func) ? simuls[sarg].func->funcname
                                                                 : "<removed>");
            break;
          case FP_EFUN:
            LOAD_SHORT(sarg, pc);
            sprintf(buff, "<efun> %s", instrs[sarg].name);
            break;
          case FP_LOCAL:
            LOAD_SHORT(sarg, pc);
            if (sarg < NUM_FUNS) {
              sprintf(buff, "<local_fun> %s", function_name(prog, sarg));
            } else {
              sprintf(buff, "<local_fun> <out of range %d>", sarg);
            }
            break;
          case FP_FUNCTIONAL:
          case FP_FUNCTIONAL | FP_NOT_BINDABLE: {
            uint8_t num_args = EXTRACT_UCHAR(pc++);
            uint16_t size;
            LOAD_SHORT(size, pc);
            sprintf(buff, "<functional, %d args>: Code size: %d,", num_args, size);
            break;
          }
          case FP_ANONYMOUS:
          case FP_ANONYMOUS | FP_NOT_BINDABLE:
            COPY_SHORT(&sarg, &pc[2]);
            sprintf(buff,
                    "<anonymous function, %d args, %d locals, ends at "
                    "%04tu>\nCode:",
                    pc[0], pc[1], (pc + 3 + sarg - code));
            pc += 4;
            break;
        }
        break;
      case F_SHORT_INT: {
        short iarg_tmp;

        COPY_SHORT(&iarg_tmp, pc);
        sprintf(buff, "short %d", iarg_tmp);
        pc += sizeof(iarg_tmp);
        break;
      };
      case F_NUMBER: {
        LPC_INT iarg_tmp;

        COPY_INT(&iarg_tmp, pc);
        sprintf(buff, "%" LPC_INT_FMTSTR_P, iarg_tmp);
        pc += sizeof(LPC_INT);
        break;
      }
      case F_REAL: {
        LPC_FLOAT farg;

        COPY_FLOAT(&farg, pc);
        sprintf(buff, "%" LPC_FLOAT_FMTSTR_P, farg);
        pc += sizeof(LPC_FLOAT);
        break;
      }
      case F_SSCANF:
      case F_PARSE_COMMAND:
      case F_BYTE:
        sprintf(buff, "%d", EXTRACT_UCHAR(pc));
        pc++;
        break;

      case F_NBYTE:
        sprintf(buff, "-%d", EXTRACT_UCHAR(pc));
        pc++;
        break;

      case F_SWITCH: {
        unsigned char ttype;
        unsigned short stable, etable, def;
        unsigned int addr;
        char* aptr;
        char* parg;

        ttype = EXTRACT_UCHAR(pc);
        COPY_SHORT(&stable, pc + 1);
        COPY_SHORT(&etable, pc + 3);
        COPY_SHORT(&def, pc + 5);
        addr = pc - code;
        aptr = pc;

        nlohmann::json swrow;
        if (sink.fns) {
          swrow = {{"a", iaddr}, {"m", "switch"}, {"cases", nlohmann::json::array()}};
          if (sink.src_line > 0 && sink.src_file != nullptr) {
            swrow["f"] = sink.src_file;
            swrow["l"] = sink.src_line;
          }
        }
        if (sink.f) fprintf(sink.f, "switch\n");
        if (sink.f) {
          fprintf(sink.f, "      type: %02x table: %04x-%04x deflt: %04x\n",
                  static_cast<unsigned>(ttype), addr + stable, addr + etable, addr + def);
        }
        if (sink.fns) {
          swrow["type"] = ttype;
          swrow["tstart"] = addr + stable;
          swrow["tend"] = addr + etable;
          swrow["deflt"] = addr + def;
          sink.cur->push_back(swrow);
        }
        // Body instructions between the header and the table go to the SAME
        // sink (json rows land in the current function like any other).
        /* recursively disassemble stuff in switch */
        disassemble(sink, code, pc - code + 7, addr + stable, prog);

        nlohmann::json* swcases = nullptr;
        if (sink.fns && sink.cur != nullptr) {
          // find our switch row again (the recursion may have appended rows,
          // but our row object was copied in -- locate by address)
          for (auto& r : *sink.cur) {
            if (r.contains("a") && r["a"] == iaddr && r["m"] == "switch") {
              swcases = &r["cases"];
              break;
            }
          }
        }

        /* now print out table - ugly... */
        if (sink.f) {
          fprintf(sink.f, "      switch table (for %04x)\n", static_cast<unsigned>(pc - code - 1));
        }
        if (ttype == 0xfe) {
          ttype = 0; /* direct lookup */
        } else if (ttype >> 4 == 0xf) {
          ttype = 1; /* normal int */
        } else {
          ttype = 2; /* string */
        }

        pc += stable;
        if (ttype == 0) {
          i = 0;
          // The table ends with a sizeof(LPC_INT)-wide minval (LPC_INT is
          // 64-bit); stop the short-offset walk before it, not 4 bytes before.
          while (pc < aptr + etable - (int)sizeof(LPC_INT)) {
            COPY_SHORT(&sarg, pc);
            if (sink.f) fprintf(sink.f, "\t%2d: %04x\n", i, addr + sarg);
            if (swcases) swcases->push_back({{"i", i}, {"to", addr + sarg}});
            i++;
            pc += 2;
          }
          COPY_INT(&iarg, pc);
          if (sink.f) fprintf(sink.f, "\tminval = %" LPC_INT_FMTSTR_P "\n", iarg);
          if (swcases) swcases->push_back({{"minval", iarg}});
          pc += sizeof(LPC_INT);
        } else {
          while (pc < aptr + etable) {
            // Table keys are written by icode.cc as ins_int() -- an LPC_INT
            // (8 bytes) even on 32-bit targets, where string-case pointers
            // are WIDENED into it (the runtime f_switch walks the table the
            // same way, SWITCH_CASE_SIZE = sizeof(LPC_INT) + sizeof(short)).
            // A sizeof(char*) stride here happens to coincide on 64-bit
            // hosts but desyncs on wasm32 and reads wild pointers.
            LPC_INT key;
            COPY_INT(&key, pc);
            parg = reinterpret_cast<char*>(static_cast<POINTER_INT>(key));
            COPY_SHORT(&sarg, pc + sizeof(LPC_INT));
            if (ttype == 1 || !parg) {
              if (sarg == 1) {
                if (sink.f) fprintf(sink.f, "\t%-4p\t<range start>\n", parg);
                if (swcases) swcases->push_back({{"range_start", true}});
              } else {
                if (sink.f) fprintf(sink.f, "\t%-4p\t%04x\n", parg, addr + sarg);
                if (swcases) swcases->push_back({{"to", addr + sarg}});
              }
            } else {
              if (sink.f) fprintf(sink.f, "\t\"%s\"\t%04x\n", disassem_string(parg), addr + sarg);
              if (swcases) {
                swcases->push_back({{"key", disassem_string(parg)}, {"to", addr + sarg}});
              }
            }
            pc += 2 + sizeof(LPC_INT);
          }
        }
        continue;
      }
      case F_EFUNV: {
        short efun;
        LOAD_SHORT(efun, pc);
        auto args = EXTRACT_UCHAR(pc++);
        sprintf(buff, "EFUN_V (ARGS: %d) %s(%d)", args, query_instr_name(efun), efun);
        sprintf(buff, "EFUN: %s(%d)", query_instr_name(efun), efun);
        break;
      }
      case F_EFUN0:
      case F_EFUN1:
      case F_EFUN2:
      case F_EFUN3: {
        short efun;
        LOAD_SHORT(efun, pc);
        sprintf(buff, "EFUN: %s(%d)", query_instr_name(efun), efun);
        break;
      }
      case 0:
        if (sink.f) fprintf(sink.f, "*** zero opcode ***\n");
        if (sink.fns && sink.cur != nullptr) {
          sink.cur->push_back({{"a", iaddr}, {"m", "*** zero opcode ***"}});
        }
        continue;
      default:
        // fprintf(f, "*** %s (%d) ***\n", query_instr_name(instr), instr);
        // continue;
        break;
    }
    {
      char tmp[256 + 1] = {};
      auto p = &tmp[0];
      while (saved_pc != pc) {
        p += sprintf(p, "%02hhX ", *saved_pc++);
      }
      if (sink.f) {
        fprintf(sink.f, " %-25s", tmp);  // byte code in HEX
        fprintf(sink.f, " %-35s; %s\n", query_instr_name(instr), buff);
      }
      if (sink.fns && sink.cur != nullptr) {
        nlohmann::json row = {{"a", iaddr},
                              {"x", tmp},
                              {"m", query_instr_name(instr)},
                              {"o", buff}};
        if (sink.src_line > 0 && sink.src_file != nullptr) {
          row["f"] = sink.src_file;
          row["l"] = sink.src_line;
        }
        sink.cur->push_back(row);
      }
    }
  }

  // print last line
  if (sink.f) fprintf(sink.f, "; %s:%d\n", last_file, last_line);

  if (offsets) {
    free(offsets);
  }
}

#define INCLUDE_DEPTH 10

static void dump_line_numbers(FILE* f, program_t* prog) {
  unsigned short* fi;
  unsigned char* li_start;
  unsigned char* li_end;
  unsigned char* li;
  int addr;
  int sz;
  ADDRESS_TYPE s;

  if (!prog->line_info) {
    fprintf(f, "Failed to load line numbers\n");
    return;
  }

  fi = prog->file_info;
  li_end = reinterpret_cast<unsigned char*>((reinterpret_cast<char*>(fi)) + fi[0]);
  li_start = reinterpret_cast<unsigned char*>(fi + fi[1]);

  fi += 2;
  fprintf(f, "\nabsolute line -> (file, line) table:\n");
  while (fi < reinterpret_cast<unsigned short*>(li_start)) {
    fprintf(f, "%i lines from %i [%s]\n", fi[0], fi[1], prog->strings[fi[1] - 1]);
    fi += 2;
  }

  li = li_start;
  addr = 0;
  fprintf(f, "\naddress -> absolute line table:\n");
  while (li < li_end) {
    sz = *li++;
#if !defined(USE_32BIT_ADDRESSES)
    COPY_SHORT(&s, li);
#else
    COPY4(&s, li);
#endif
    li += sizeof(ADDRESS_TYPE);
    fprintf(f, "%04x-%04x: %i\n", addr, addr + sz - 1, s);
    addr += sz;
  }
}

// ---------------------------------------------------------------------------
// JSON-native dump (lpcc --json bytecode). Same emissions as the text form:
// the instruction stream comes from the SAME disassemble() switch via the
// json sink backend; the tables share fn_table_rows(); the line-table walk
// mirrors dump_line_numbers() (kept adjacent -- change both together).
// ---------------------------------------------------------------------------

static nlohmann::json prog_details_json(program_t* prog, int flags) {
  nlohmann::json p;
  p["file"] = prog->filename;

  p["globals"] = nlohmann::json::array();
  for (int i = 0; i < prog->num_variables_total; i++) {
    p["globals"].push_back({{"i", i}, {"name", variable_name(prog, i)}});
  }

  int variable_runtime_index = 0;
  if (prog->num_inherited > 0) {
    variable_runtime_index = prog->inherit[prog->num_inherited - 1].variable_index_offset +
                             prog->inherit[prog->num_inherited - 1].prog->num_variables_total;
  }
  p["variables"] = nlohmann::json::array();
  for (int i = 0; i < prog->num_variables_defined; i++) {
    char buf[255];
    auto end = &buf[sizeof(buf) - 1];
    get_type_name(&buf[0], end, prog->variable_types[i]);
    p["variables"].push_back({{"i", variable_runtime_index + i},
                              {"decl", std::string(buf) + prog->variable_table[i]}});
  }

  p["strings"] = nlohmann::json::array();
  for (int i = 0; i < prog->num_strings; i++) {
    p["strings"].push_back({{"i", i}, {"text", prog->strings[i]}});
  }

  if (flags & 1) {
    nlohmann::json fns = nlohmann::json::array();
    // Preamble entry catches any rows before the first function header
    // (dropped below if none appear).
    fns.push_back({{"sig", ""}, {"name", ""}, {"instructions", nlohmann::json::array()}});
    DisSink sink;
    sink.fns = &fns;
    sink.cur = &fns.back()["instructions"];
    disassemble(sink, prog->program, 0, prog->program_size, prog);
    if (!fns.empty() && fns[0]["name"] == "" && fns[0]["instructions"].empty()) {
      fns.erase(fns.begin());
    }
    p["functions"] = fns;
  }

  if ((flags & 2) && prog->line_info != nullptr && prog->file_info != nullptr) {
    // Mirrors dump_line_numbers()'s walk.
    unsigned short* fi = prog->file_info;
    auto* li_end = reinterpret_cast<unsigned char*>((reinterpret_cast<char*>(fi)) + fi[0]);
    auto* li_start = reinterpret_cast<unsigned char*>(fi + fi[1]);

    p["line_files"] = nlohmann::json::array();
    for (unsigned short* q = fi + 2; q < reinterpret_cast<unsigned short*>(li_start); q += 2) {
      p["line_files"].push_back({{"lines", q[0]}, {"file", prog->strings[q[1] - 1]}});
    }

    p["line_ranges"] = nlohmann::json::array();
    unsigned char* li = li_start;
    int addr = 0;
    while (li < li_end) {
      int sz = *li++;
      ADDRESS_TYPE s;
#if !defined(USE_32BIT_ADDRESSES)
      COPY_SHORT(&s, li);
#else
      COPY4(&s, li);
#endif
      li += sizeof(ADDRESS_TYPE);
      p["line_ranges"].push_back({{"from", addr}, {"to", addr + sz - 1}, {"line", s}});
      addr += sz;
    }
  }
  return p;
}

static void collect_programs_json(program_t* prog, int flags, nlohmann::json& out) {
  out.push_back(prog_details_json(prog, flags));
  for (int i = 0; i < prog->num_inherited; i++) {
    collect_programs_json(prog->inherit[i].prog, flags, out);
  }
}

nlohmann::json dump_prog_json(program_t* prog, int flags) {
  nlohmann::json j;
  j["name"] = std::string("/") + prog->filename;

  j["inherits"] = nlohmann::json::array();
  for (int i = 0; i < prog->num_inherited; i++) {
    j["inherits"].push_back({{"name", prog->inherit[i].prog->filename},
                             {"fio", prog->inherit[i].function_index_offset},
                             {"vio", prog->inherit[i].variable_index_offset}});
  }

  j["functions_table"] = nlohmann::json::array();
  std::vector<FnTabRow> rows;
  fn_table_rows(prog, rows);
  for (const auto& r : rows) {
    nlohmann::json e = {{"i", r.index}, {"name", r.name}, {"mods", r.smods},
                        {"flags", r.sflags}, {"inherited", r.inherited}};
    if (r.inherited) {
      e["inh"] = r.inh_low;
      e["fio"] = r.inh_fio;
      e["vio"] = r.inh_vio;
    } else {
      e["offset"] = r.offset;
      e["locals"] = r.locals;
      e["args"] = r.args;
      e["def_args"] = r.def_args;
      if (!r.defmap.empty()) e["defmap"] = r.defmap;
    }
    j["functions_table"].push_back(e);
  }

  j["programs"] = nlohmann::json::array();
  collect_programs_json(prog, flags, j["programs"]);
  return j;
}
