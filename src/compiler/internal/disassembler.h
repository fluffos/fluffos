#ifndef COMPILER_INTERNAL_DISASSEMBLER_H
#define COMPILER_INTERNAL_DISASSEMBLER_H

#include <cstdio>  // for FILE
void dump_prog(struct program_t* prog, FILE* f, int flags);

#endif  // COMPILER_INTERNAL_DISASSEMBLER_H
