#ifndef FILE_H
#define FILE_H

/*
 * file.c
 */
int legal_path PROT((char *));
char *check_valid_path PROT((char *, struct object *, char *, int));
void smart_log PROT((char *, int, char *, int));
void dump_file_descriptors PROT((void));

char *read_file PROT((char *, int, int));
char *read_bytes PROT((char *, int, int, int *));
int write_file PROT((char *, char *, int));
int write_bytes PROT((char *, int, char *, int));
struct vector *get_dir PROT((char *, int));
int tail PROT((char *));
int file_size PROT((char *));
int copy_file PROT((char *, char *));
int do_rename PROT((char *, char *, int));
int remove_file PROT((char *));

#endif
