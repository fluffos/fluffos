#ifndef FILE_H
#define FILE_H
/*
 * file.c
 */

// const char *check_valid_path(const char *, object_t *, const char *const, int);
const std::string check_valid_path(const std::string, object_t *, const std::string, int);
void dump_file_descriptors(outbuffer_t *);

std::string *read_file(const std::string, int, int);
std::string *read_bytes(const std::string, int, int, int *);
int write_file(const std::string, const std::string, int);
int write_bytes(const std::string, int, const std::string);
int write_bytes(const std::string, int, const char *, size_t);
array_t *get_dir(const std::string, int);
int tail(char *);
int file_size(const char *);
int copy_file(const char *, const char *);
#ifdef F_RENAME
int do_rename(const std::string, const std::string, int);
#endif
int remove_file(const std::string);

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_file_sv(void);
#endif

#endif
