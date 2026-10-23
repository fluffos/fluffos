#ifndef FILE_H
#define FILE_H
/*
 * file.c
 */

#include "vm/internal/base/value.h"

/* Validates path via master valid_read/valid_write. Returns an owned string
 * Value (leading '/' stripped; empty becomes ".") on allow, or a non-string
 * Value on deny. The returned Value keeps the path alive for its lifetime --
 * callers that need the C string across another apply hold the Value, not a
 * pointer into a shared slot. */
vm::Value check_valid_path(const char*, object_t*, const char* const, int);
void dump_file_descriptors(outbuffer_t*);

char* read_file(const char*, LPC_INT, LPC_INT);
char* read_bytes(const char*, LPC_INT, LPC_INT, int*);
int write_file(const char*, const char*, int);
int write_bytes(const char*, LPC_INT, const char*, int);
array_t* get_dir(const char*, int);
int tail(char*);
int file_size(const char*);
int copy_file(const char*, const char*);
int do_rename(const char*, const char*, int);
int remove_file(const char*);

#endif
