#ifndef FILE_H
#define FILE_H

#include "lpc_incl.h"
#include "file_incl.h"

/*
 * Credits for some of the code below goes to Free Software Foundation
 * Copyright (C) 1990 Free Software Foundation, Inc.
 * See the GNU General Public License for more details.
 */
#ifndef S_ISDIR
#define S_ISDIR(m)      (((m)&S_IFMT) == S_IFDIR)
#endif

#ifndef S_ISREG
#define S_ISREG(m)      (((m)&S_IFMT) == S_IFREG)
#endif

#ifndef S_ISCHR
#define S_ISCHR(m)      (((m)&S_IFMT) == S_IFCHR)
#endif

#ifndef S_ISBLK
#define S_ISBLK(m)      (((m)&S_IFMT) == S_IFBLK)
#endif

/*
 * file.c
 */

int legal_path (const char *);
const char *check_valid_path (const char *, object_t *, const char * const, int);
void smart_log (const char *, int, const char *, int);
void dump_file_descriptors (outbuffer_t *);

char *read_file (const char *, int, int);
char *read_bytes (const char *, int, int, int *);
int write_file (const char *, const char *, int);
int write_bytes (const char *, int, const char *, int);
array_t *get_dir (const char *, int);
int tail (char *);
int file_size (const char *);
int copy_file (const char *, const char *);
int do_rename (const char *, const char *, int);
int remove_file (const char *);

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_file_sv (void);
#endif

#endif
