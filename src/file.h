#ifndef FILE_H
#define FILE_H

/*
 * file.c
 */
void debug_perror PROT((char *, char *));

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

#endif
