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
#define	S_ISDIR(m)	(((m)&S_IFMT) == S_IFDIR)
#endif

#ifndef S_ISREG
#define	S_ISREG(m)	(((m)&S_IFMT) == S_IFREG)
#endif

#ifndef S_ISCHR
#define	S_ISCHR(m)	(((m)&S_IFMT) == S_IFCHR)
#endif

#ifndef S_ISBLK
#define	S_ISBLK(m)	(((m)&S_IFMT) == S_IFBLK)
#endif

/*
 * file.c
 */

int legal_path PROT((char *));
char *check_valid_path PROT((char *, object_t *, char *, int));
void smart_log PROT((char *, int, char *, int));
void dump_file_descriptors PROT((outbuffer_t *));

char *read_file PROT((char *, int, int));
char *read_bytes PROT((char *, int, int, int *));
int write_file PROT((char *, char *, int));
int write_bytes PROT((char *, int, char *, int));
array_t *get_dir PROT((char *, int));
int tail PROT((char *));
int file_size PROT((char *));
int copy_file PROT((char *, char *));
int do_rename PROT((char *, char *, int));
int remove_file PROT((char *));

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_file_sv PROT((void));
#endif

#endif
