#ifndef COMPILER_INTERNAL_DISASSEMBLER_H
#define COMPILER_INTERNAL_DISASSEMBLER_H

#include <cstdio>  // for FILE

#include <nlohmann/json_fwd.hpp>

void dump_prog(struct program_t* prog, FILE* f, int flags);

// JSON-native form of the same dump (lpcc --json bytecode): the model the
// text format is a rendering of. Shape:
//   { name, inherits:[{name,fio,vio}],
//     functions_table:[{i,name,mods,flags,inherited, then per-kind fields}],
//     programs:[{file, globals:[{i,name}], variables:[{i,decl}],
//                strings:[{i,text}],
//                functions:[{sig,name,instructions:[{a,x,m,o,f,l}...]}],
//                line_files:[{lines,file}], line_ranges:[{from,to,line}]}] }
// (top program first, then each inherited program's dump, matching the text
// recursion order). flags as dump_prog: 1 = disassemble, 2 = line tables.
nlohmann::json dump_prog_json(struct program_t* prog, int flags);

#endif  // COMPILER_INTERNAL_DISASSEMBLER_H
