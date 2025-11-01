#ifndef COMPILER_INTERNAL_DISASSEMBLER_H
#define COMPILER_INTERNAL_DISASSEMBLER_H

#include <iosfwd>

struct program_t;

// Add the declaration for the function you want to use.
void dump_prog(program_t *prog, FILE *f, int options);

#endif /* COMPILER_INTERNAL_DISASSEMBLER_H */
