#include "base/std.h"

#include "vm/internal/base/svalue.h"
#include "vm/internal/base/machine.h"
#include "vm/internal/base/interpret.h"
#include "compiler/internal/disassembler.h"
#include "vm/internal/trace.h"

#include <string>
#include <iostream>
#include <sstream>
#include <nlohmann/json.hpp>
#include <fmt/format.h>

std::string framekind_name(int framekind) {
  std::string result = "";
  switch (framekind & FRAME_MASK) {
    case FRAME_FUNCTION:
      result = "FRAME_FUNCTION";
      break;
    case FRAME_FUNP:
      result = "FRAME_FUNP";
      break;
    case FRAME_CATCH:
      result = "FRAME_CATCH";
      break;
    case FRAME_FAKE:
      result = "FRAME_FAKE";
      break;
    default:
      result = "FRAME_UNKNOWN";
      break;
  }
  if (framekind & FRAME_EXTERNAL) {
    result += "| FRAME_EXTERNAL";
  }
  if (framekind & FRAME_OB_CHANGE) {
    result += "| FRAME_OB_CHANGE";
  }
  if (framekind & FRAME_RETURNED_FROM_CATCH) {
      result += "| FRAME_RETURNED_FROM_CATCH";
  }
  return result;
}

std::string print_object_ptr(object_t *ob) {
  if (!ob) {
    return "null";
  }
  std::stringstream ss;
  ss << (void*)ob << " (" << ob->obname << ")";
  return ss.str();
}

std::string print_program_ptr(program_t *prog) {
  if (!prog) {
    return "null";
  }
  std::stringstream ss;
  ss << (void*)prog << " (" << prog->filename << ")";
  return ss.str();
}

std::string print_pc(program_t *prog, char *pc) {
  if (!prog || !pc) {
    return "null";
  }
  std::stringstream ss;
  ss << (void*)pc << " (addr: " << fmt::format(FMT_STRING("{:04x}"), (pc - prog->program)) << ")";
  return ss.str();
}

bool dump_vm_state() {
  const auto *prefix = "  ";
  std::cout << "VM state:\n";
  std::cout << prefix << "current_object = " << print_object_ptr(current_object) << ")\n";
  std::cout << prefix << "current_interactive = " << current_interactive << "\n";
  std::cout << prefix << "current_prog = " << print_program_ptr(current_prog) << "\n";
  std::cout << prefix << "caller_type = " << caller_type << "\n";
  std::cout << prefix << "pc = " << print_pc(current_prog, pc) << "\n";
  std::cout << prefix << "fp = " << (void *)fp << " (sp - " << (sp - fp) << ")" << "\n";
  std::cout << prefix << "sp = " << (void *)sp << "\n";
  std::cout << prefix << "st_num_arg = " << st_num_arg << "\n";

  // Dump current stack
  std::cout << "current stack:\n";
  for(auto *sv = csp->fp; sv < sp; sv++) {
    if (sv == nullptr) break;
    std::cout << "sv " << (sv - csp->fp) << ":\n";
    std::cout << prefix << "type = " << type_name(sv->type) << "\n";
    std::cout << prefix << svalue_to_json_summary(sv, 0).dump(2) << "\n";
  }

  // Dump control stack
  auto *p = csp;
  int depth = 1;
  while (p != control_stack && p->prog != nullptr) {
    std::cout << "control stack: "<< -depth << "\n";
    std::cout << prefix << "framekind = " << framekind_name(p->framekind) << "(" << p->framekind << ")\n";
    std::cout << prefix << "ob = " << print_object_ptr(p->ob) << "\n";
    std::cout << prefix << "prev_ob = " << print_object_ptr(p->prev_ob) << "\n";
    std::cout << prefix << "prog = " << print_program_ptr(p->prog) << "\n";
    std::cout << prefix << "pc = " << print_pc(p->prog, p->pc) << "\n";
    std::cout << prefix << "fp = " << p->fp << "\n";
    std::cout << prefix << "num_local_variables = " << p->num_local_variables << "\n";
    std::cout << prefix << "function_index_offset = " << p->function_index_offset << "\n";
    std::cout << prefix << "variable_index_offset = " << p->variable_index_offset << "\n";
    std::cout << prefix << "caller_type = " << p->caller_type << "\n";
    p--;
    depth++;
  }

  // Dump the current program
  if (current_prog != nullptr) {
    std::cout << "current program:\n";
    dump_prog(current_prog, stdout, 1 | 2);

    // Dump current trace
    dump_trace(1);
  }

  return true; // so we can use it in DEBUG_CHECK
}
