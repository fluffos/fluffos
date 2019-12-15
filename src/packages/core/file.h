#ifndef FILE_H
#define FILE_H
/*
 * file.c
 */

// const char *check_valid_path(const char *, object_t *, const char *const, int);
const std::string check_valid_path(const std::string, object_t *, const std::string, int);
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
