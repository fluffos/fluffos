#ifndef FILE_H
#define FILE_H

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_DIR_H
#include <sys/dir.h>
#endif

#if HAVE_DIRENT_H
#include <dirent.h>
#define NAMLEN(dirent) strlen((dirent)->d_name)
#else
#define dirent direct
#define NAMLEN(dirent) (dirent)->d_namlen
#if HAVE_SYS_NDIR_H
#include <sys/ndir.h>
#endif
#if HAVE_SYS_DIR_H
#include <sys/dir.h>
#endif
#if HAVE_NDIR_H
#include <ndir.h>
#endif
#endif

#ifdef HAVE_SYS_FILIO_H
#include <sys/filio.h>
#endif
#ifdef HAVE_SYS_SOCKIO_H
#include <sys/sockio.h>
#endif
#ifdef HAVE_SYS_MKDEV_H
#include <sys/mkdev.h>
#endif

#ifdef HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif
#ifdef HAVE_SYS_RUSAGE_H
#include <sys/rusage.h>
#endif
#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif

/*
 * Credits for some of the code below goes to Free Software Foundation
 * Copyright (C) 1990 Free Software Foundation, Inc.
 * See the GNU General Public License for more details.
 */
#ifndef S_ISDIR
#define S_ISDIR(m) (((m)&S_IFMT) == S_IFDIR)
#endif

#ifndef S_ISREG
#define S_ISREG(m) (((m)&S_IFMT) == S_IFREG)
#endif

#ifndef S_ISCHR
#define S_ISCHR(m) (((m)&S_IFMT) == S_IFCHR)
#endif

#ifndef S_ISBLK
#define S_ISBLK(m) (((m)&S_IFMT) == S_IFBLK)
#endif

/*
 * file.c
 */

const char *check_valid_path(const char *, object_t *, const char *const, int);
void dump_file_descriptors(outbuffer_t *);

char *read_file(const char *, int, int);
char *read_bytes(const char *, int, int, int *);
int write_file(const char *, const char *, int);
int write_bytes(const char *, int, const char *, int);
array_t *get_dir(const char *, int);
int tail(char *);
int file_size(const char *);
int copy_file(const char *, const char *);
int do_rename(const char *, const char *, int);
int remove_file(const char *);

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_file_sv(void);
#endif

#endif
