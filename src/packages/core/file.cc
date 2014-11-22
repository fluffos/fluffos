/*
 * file: file.c
 * description: handle all file based efuns
 */
#include "base/package_api.h"

#include "packages/core/file.h"

#include <errno.h>
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
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
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
#include <fcntl.h>

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

#ifdef PACKAGE_COMPRESS
#include <zlib.h>
#endif

static int match_string(char *, char *);
static int copy(const char *from, const char *to);
static int do_move(const char *from, const char *to, int flag);
static int pstrcmp(const void *, const void *);
static int parrcmp(const void *, const void *);
static void encode_stat(svalue_t *, int, char *, struct stat *);

#define MAX_LINES 50

/*
 * These are used by qsort in get_dir().
 */
static int pstrcmp(const void *p1, const void *p2) {
  svalue_t *x = (svalue_t *)p1;
  svalue_t *y = (svalue_t *)p2;

  return strcmp(x->u.string, y->u.string);
}

static int parrcmp(const void *p1, const void *p2) {
  svalue_t *x = (svalue_t *)p1;
  svalue_t *y = (svalue_t *)p2;

  return strcmp(x->u.arr->item[0].u.string, y->u.arr->item[0].u.string);
}

static void encode_stat(svalue_t *vp, int flags, char *str, struct stat *st) {
  if (flags == -1) {
    array_t *v = allocate_empty_array(3);

    v->item[0].type = T_STRING;
    v->item[0].subtype = STRING_MALLOC;
    v->item[0].u.string = string_copy(str, "encode_stat");
    v->item[1].type = T_NUMBER;
    v->item[1].u.number = ((st->st_mode & S_IFDIR) ? -2 : st->st_size);
    v->item[2].type = T_NUMBER;
    v->item[2].u.number = st->st_mtime;
    vp->type = T_ARRAY;
    vp->u.arr = v;
  } else {
    vp->type = T_STRING;
    vp->subtype = STRING_MALLOC;
    vp->u.string = string_copy(str, "encode_stat");
  }
}

/*
 * List files in directory. This function do same as standard list_files did,
 * but instead writing files right away to user this returns an array
 * containing those files. Actually most of code is copied from list_files()
 * function.
 * Differences with list_files:
 *
 *   - file_list("/w"); returns ({ "w" })
 *
 *   - file_list("/w/"); and file_list("/w/."); return contents of directory
 *     "/w"
 *
 *   - file_list("/");, file_list("."); and file_list("/."); return contents
 *     of directory "/"
 *
 * With second argument equal to non-zero, instead of returning an array
 * of strings, the function will return an array of arrays about files.
 * The information in each array is supplied in the order:
 *    name of file,
 *    size of file,
 *    last update of file.
 */
/* WIN32 should be fixed to do this correctly (i.e. no ifdefs for it) */
#define MAX_FNAME_SIZE 255
#define MAX_PATH_LEN 1024
array_t *get_dir(const char *path, int flags) {
  auto max_array_size = CONFIG_INT(__MAX_ARRAY_SIZE__);

  array_t *v;
  int i, count = 0;
  DIR *dirp;
  int namelen, do_match = 0;

  struct dirent *de;
  struct stat st;
  char *endtemp;
  char temppath[MAX_FNAME_SIZE + MAX_PATH_LEN + 2];
  char regexppath[MAX_FNAME_SIZE + MAX_PATH_LEN + 2];
  char *p;

  if (!path) {
    return 0;
  }

  path = check_valid_path(path, current_object, "stat", 0);

  if (path == 0) {
    return 0;
  }

  if (strlen(path) < 2) {
    temppath[0] = path[0] ? path[0] : '.';
    temppath[1] = '\000';
    p = temppath;
  } else {
    strncpy(temppath, path, MAX_FNAME_SIZE + MAX_PATH_LEN + 1);
    temppath[MAX_FNAME_SIZE + MAX_PATH_LEN + 1] = '\0';

    /*
     * If path ends with '/' or "/." remove it
     */
    if ((p = strrchr(temppath, '/')) == 0) {
      p = temppath;
    }
    if (p[0] == '/' && ((p[1] == '.' && p[2] == '\0') || p[1] == '\0')) {
      *p = '\0';
    }
  }

  if (stat(temppath, &st) < 0) {
    if (*p == '\0') {
      return 0;
    }
    if (p != temppath) {
      strcpy(regexppath, p + 1);
      *p = '\0';
    } else {
      strcpy(regexppath, p);
      strcpy(temppath, ".");
    }
    do_match = 1;
  } else if (*p != '\0' && strcmp(temppath, ".")) {
    if (*p == '/' && *(p + 1) != '\0') {
      p++;
    }
    v = allocate_empty_array(1);
    encode_stat(&v->item[0], flags, p, &st);
    return v;
  }
  if ((dirp = opendir(temppath)) == 0) {
    return 0;
  }
  /*
   * Count files
   */
  for (de = readdir(dirp); de; de = readdir(dirp)) {
    namelen = strlen(de->d_name);
    if (!do_match && (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)) {
      continue;
    }
    if (do_match && !match_string(regexppath, de->d_name)) {
      continue;
    }
    count++;
    if (count >= max_array_size) {
      break;
    }
  }

  /*
   * Make array and put files on it.
   */
  v = allocate_empty_array(count);
  if (count == 0) {
    /* This is the easy case :-) */
    closedir(dirp);
    return v;
  }
  rewinddir(dirp);
  endtemp = temppath + strlen(temppath);

  strcat(endtemp++, "/");

  for (i = 0, de = readdir(dirp); i < count; de = readdir(dirp)) {
    namelen = strlen(de->d_name);
    if (!do_match && (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)) {
      continue;
    }
    if (do_match && !match_string(regexppath, de->d_name)) {
      continue;
    }
    de->d_name[namelen] = '\0';
    if (flags == -1) {
      /*
       * We'll have to .... sigh.... stat() the file to get some add'tl
       * info.
       */
      strcpy(endtemp, de->d_name);
      stat(temppath, &st); /* We assume it works. */
    }
    encode_stat(&v->item[i], flags, de->d_name, &st);
    i++;
  }
  closedir(dirp);
  /* Sort the names. */
  qsort((void *)v->item, count, sizeof v->item[0], (flags == -1) ? parrcmp : pstrcmp);
  return v;
}

int remove_file(const char *path) {
  path = check_valid_path(path, current_object, "remove_file", 1);

  if (path == 0) {
    return 0;
  }
  if (unlink(path) == -1) {
    return 0;
  }
  return 1;
}

/*
 * Append string to file. Return 0 for failure, otherwise 1.
 */
int write_file(const char *file, const char *str, int flags) {
  FILE *f;
#ifdef PACKAGE_COMPRESS
  gzFile gf;
#endif

  file = check_valid_path(file, current_object, "write_file", 1);
  if (!file) {
    return 0;
  }
#ifdef PACKAGE_COMPRESS
  if (flags & 2) {
    gf = gzopen(file, (flags & 1) ? "w" : "a");
    if (!gf)
      error("Wrong permissions for opening file /%s for %s.\n\"%s\"\n", file,
            (flags & 1) ? "overwrite" : "append", strerror(errno));
  } else {
#endif
    f = fopen(file, (flags & 1) ? "w" : "a");
    if (f == 0) {
      error("Wrong permissions for opening file /%s for %s.\n\"%s\"\n", file,
            (flags & 1) ? "overwrite" : "append", strerror(errno));
    }
#ifdef PACKAGE_COMPRESS
  }
  if (flags & 2) {
    gzwrite(gf, str, strlen(str));
  } else
#endif
    fwrite(str, strlen(str), 1, f);
#ifdef PACKAGE_COMPRESS
  if (flags & 2) {
    gzclose(gf);
  } else
#endif
    fclose(f);
  return 1;
}

/* Reads file, starting from line of "start", with maximum lines of "lines".
 * Returns a malloced_string.
 */
char *read_file(const char *file, int start, int lines) {
  const auto read_file_max_size = CONFIG_INT(__MAX_READ_FILE_SIZE__);

  struct stat st;
  // Try and keep one buffer for droping all reads into.
  static char *theBuff = NULL;
  char *result = NULL;
#ifndef PACKAGE_COMPRESS
  FILE *f = NULL;
#else
  gzFile f = NULL;
#endif
  int chunk;
  char *ptr_start, *ptr_end;
  const char *real_file;

  if (lines < 0) {
    debug(file, "read_file: trying to read negative lines: %d", lines);
    return 0;
  }

  real_file = check_valid_path(file, current_object, "read_file", 0);

  if (!real_file) {
    return 0;
  }
  /*
   * file doesn't exist, or is really a directory
   */
  if (stat(real_file, &st) == -1 || (st.st_mode & S_IFDIR)) {
    return 0;
  }

  if (st.st_size == 0) {
    /* zero length file */
    result = new_string(0, "read_file: empty");
    result[0] = '\0';
    return result;
  }

#ifndef PACKAGE_COMPRESS
  f = fopen(real_file, FOPEN_READ);
#else
  f = gzopen(real_file, "rb");
#endif

  if (f == 0) {
    debug(file, "read_file: fail to open: %s.\n", file);
    return 0;
  }

  if (!theBuff) {
    theBuff = (char *)DMALLOC(2 * read_file_max_size + 1, TAG_PERMANENT, "read_file: theBuff");
  }

#ifndef PACKAGE_COMPRESS
  chunk = fread(theBuff, 1, 2 * READ_FILE_MAX_SIZE, f);
  fclose(f);
#else
  chunk = gzread(f, theBuff, 2 * read_file_max_size);
  gzclose(f);
#endif

  if (chunk == 0) {
    debug(file, "read_file: read error: %s.\n", file);
    return 0;
  }

  if (memchr(theBuff, '\0', chunk)) {
    debug(file, "read_file: file contains '\\0': %s.\n", file);
    return 0;
  }
  theBuff[chunk] = '\0';

  // skip forward until the "start"-th line
  ptr_start = theBuff;
  while (start > 1 && ptr_start < theBuff + chunk) {
    if (*ptr_start++ == '\n') {
      start--;
    }
  }

  // not found
  if (start > 1) {
    debug(file, "read_file: reached EOF searching for start: %s.\n", file);
    return 0;
  }

  // search forward for "lines" of '\n' for the end
  if (lines == 0) {
    ptr_end = ptr_start + read_file_max_size;
    if (ptr_end > theBuff + chunk) {
      ptr_end = theBuff + chunk + 1;
    }
  } else {
    ptr_end = ptr_start;
    while (lines > 0 && ptr_end < theBuff + chunk) {
      if (*ptr_end++ == '\n') {
        lines--;
      }
    }
    // not enough lines, directly go to the end.
    if (lines > 0) {
      ptr_end = theBuff + chunk + 1;
    }
  }

  *ptr_end = '\0';
  // result is too big.
  if (strlen(ptr_start) > read_file_max_size) {
    debug(file, "read_file: result too big: %s.\n", file);
    return 0;
  }

  result = string_copy(ptr_start, "read_file: result");
  return result;
}

char *read_bytes(const char *file, int start, int len, int *rlen) {
  const auto max_byte_transfer = CONFIG_INT(__MAX_BYTE_TRANSFER__);

  struct stat st;
  FILE *fptr;
  char *str;
  int size;

  if (len < 0) {
    return 0;
  }
  file = check_valid_path(file, current_object, "read_bytes", 0);
  if (!file) {
    return 0;
  }
  fptr = fopen(file, "rb");
  if (fptr == NULL) {
    return 0;
  }
  if (fstat(fileno(fptr), &st) == -1) {
    fatal("Could not stat an open file.\n");
  }
  size = st.st_size;
  if (start < 0) {
    start = size + start;
  }

  if (len == 0) {
    len = size;
  }
  if (len > max_byte_transfer) {
    fclose(fptr);
    error("Transfer exceeded maximum allowed number of bytes.\n");
    return 0;
  }
  if (start >= size) {
    fclose(fptr);
    return 0;
  }
  if ((start + len) > size) {
    len = (size - start);
  }

  if ((size = fseek(fptr, start, 0)) < 0) {
    fclose(fptr);
    return 0;
  }

  str = new_string(len, "read_bytes: str");

  size = fread(str, 1, len, fptr);

  fclose(fptr);

  if (size <= 0) {
    FREE_MSTR(str);
    return 0;
  }
  /*
   * The string has to end to '\0'!!!
   */
  str[size] = '\0';

  *rlen = size;
  return str;
}

int write_bytes(const char *file, int start, const char *str, int theLength) {
  const auto max_byte_transfer = CONFIG_INT(__MAX_BYTE_TRANSFER__);

  struct stat st;
  int size;
  FILE *fptr;

  file = check_valid_path(file, current_object, "write_bytes", 1);

  if (!file) {
    return 0;
  }
  if (theLength > max_byte_transfer) {
    return 0;
  }
  /* Under system V, it isn't possible change existing data in a file
   * opened for append, so it can't be opened for append.
   * opening for r+ won't create the file if it doesn't exist.
   * opening for w or w+ will truncate it if it does exist.  So we
   * have to check if it exists first.
   */
  if (stat(file, &st) == -1) {
    fptr = fopen(file, "wb");
  } else {
    fptr = fopen(file, "r+b");
  }
  if (fptr == NULL) {
    return 0;
  }
  if (fstat(fileno(fptr), &st) == -1) {
    fatal("Could not stat an open file.\n");
  }
  size = st.st_size;
  if (start < 0) {
    start = size + start;
  }
  if (start < 0 || start > size) {
    fclose(fptr);
    return 0;
  }
  if ((size = fseek(fptr, start, 0)) < 0) {
    fclose(fptr);
    return 0;
  }
  size = fwrite(str, 1, theLength, fptr);

  fclose(fptr);

  if (size <= 0) {
    return 0;
  }
  return 1;
}

int file_size(const char *file) {
  struct stat st;
  long ret;

  file = check_valid_path(file, current_object, "file_size", 0);
  if (!file) {
    return -1;
  }

  if (stat(file, &st) == -1) {
    ret = -1;
  } else if (S_IFDIR & st.st_mode) {
    ret = -2;
  } else {
    ret = st.st_size;
  }

  return ret;
}

/*
 * Check that a path to a file is valid for read or write.
 * This is done by functions in the master object.
 * The path is always treated as an absolute path, and is returned without
 * a leading '/'.
 * If the path was '/', then '.' is returned.
 * Otherwise, the returned path is temporarily allocated by apply(), which
 * means it will be deallocated at next apply().
 */
const char *check_valid_path(const char *path, object_t *call_object, const char *const call_fun,
                             int writeflg) {
  svalue_t *v;

  if (!master_ob && !call_object) {
    // early startup, ignore security
    free_svalue(&apply_ret_value, "check_valid_path");
    apply_ret_value.type = T_STRING;
    apply_ret_value.subtype = STRING_MALLOC;
    path = apply_ret_value.u.string = string_copy(path, "check_valid_path");
    return path;
  }

  if (call_object == 0 || call_object->flags & O_DESTRUCTED) {
    return 0;
  }

  copy_and_push_string(path);
  push_object(call_object);
  push_constant_string(call_fun);
  if (writeflg) {
    v = apply_master_ob(APPLY_VALID_WRITE, 3);
  } else {
    v = apply_master_ob(APPLY_VALID_READ, 3);
  }

  if (v == (svalue_t *)-1) {
    v = 0;
  }

  if (v && v->type == T_NUMBER && v->u.number == 0) {
    return 0;
  }
  if (v && v->type == T_STRING) {
    path = v->u.string;
  } else {
    extern svalue_t apply_ret_value;

    free_svalue(&apply_ret_value, "check_valid_path");
    apply_ret_value.type = T_STRING;
    apply_ret_value.subtype = STRING_MALLOC;
    path = apply_ret_value.u.string = string_copy(path, "check_valid_path");
  }

  if (path[0] == '/') {
    path++;
  }
  if (path[0] == '\0') {
    path = ".";
  }
  if (legal_path(path)) {
    return path;
  }

  return 0;
}

static int match_string(char *match, char *str) {
  int i;

again:
  if (*str == '\0' && *match == '\0') {
    return 1;
  }
  switch (*match) {
    case '?':
      if (*str == '\0') {
        return 0;
      }
      str++;
      match++;
      goto again;
    case '*':
      match++;
      if (*match == '\0') {
        return 1;
      }
      for (i = 0; str[i] != '\0'; i++)
        if (match_string(match, str + i)) {
          return 1;
        }
      return 0;
    case '\0':
      return 0;
    case '\\':
      match++;
      if (*match == '\0') {
        return 0;
      }
    /* Fall through ! */
    default:
      if (*match == *str) {
        match++;
        str++;
        goto again;
      }
      return 0;
  }
}

static struct stat to_stats, from_stats;

static int copy(const char *from, const char *to) {
  int ifd;
  int ofd;
  char buf[1024 * 8];
  int len; /* Number of bytes read into `buf'. */

  if (!S_ISREG(from_stats.st_mode)) {
    return 1;
  }
  if (unlink(to) && errno != ENOENT) {
    return 1;
  }
  ifd = open(from, O_RDONLY);
  if (ifd < 0) {
    return errno;
  }
  ofd = open(to, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (ofd < 0) {
    close(ifd);
    return 1;
  }
  if (fchmod(ofd, from_stats.st_mode & 0777)) {
    close(ifd);
    close(ofd);
    unlink(to);
    return 1;
  }
  while ((len = read(ifd, buf, sizeof(buf))) > 0) {
    int wrote = 0;
    char *bp = buf;

    do {
      wrote = write(ofd, bp, len);
      if (wrote < 0) {
        close(ifd);
        close(ofd);
        unlink(to);
        return 1;
      }
      bp += wrote;
      len -= wrote;
    } while (len > 0);
  }
  if (len < 0) {
    close(ifd);
    close(ofd);
    unlink(to);
    return 1;
  }
  if (close(ifd) < 0) {
    close(ofd);
    return 1;
  }
  if (close(ofd) < 0) {
    return 1;
  }
#ifdef FCHMOD_MISSING
  if (chmod(to, from_stats.st_mode & 0777)) {
    return 1;
  }
#endif

  return 0;
}

/* Move FROM onto TO.  Handles cross-filesystem moves.
   If TO is a directory, FROM must be also.
   Return 0 if successful, 1 if an error occurred.  */

#ifdef F_RENAME
static int do_move(const char *from, const char *to, int flag) {
  if (lstat(from, &from_stats) != 0) {
    error("/%s: lstat failed\n", from);
    return 1;
  }
  if (lstat(to, &to_stats) == 0) {
    if (from_stats.st_dev == to_stats.st_dev && from_stats.st_ino == to_stats.st_ino) {
      error("`/%s' and `/%s' are the same file", from, to);
      return 1;
    }
    if (S_ISDIR(to_stats.st_mode)) {
      error("/%s: cannot overwrite directory", to);
      return 1;
    }
  } else if (errno != ENOENT) {
    error("/%s: unknown error\n", to);
    return 1;
  }
  if ((flag == F_RENAME) && (rename(from, to) == 0)) {
    return 0;
  }
#ifdef F_LINK
  else if (flag == F_LINK) {
    if (link(from, to) == 0) {
      return 0;
    }
  }
#endif

  if (errno != EXDEV) {
    if (flag == F_RENAME) {
      error("cannot move `/%s' to `/%s'\n", from, to);
    } else {
      error("cannot link `/%s' to `/%s'\n", from, to);
    }
    return 1;
  }
  /* rename failed on cross-filesystem link.  Copy the file instead. */

  if (flag == F_RENAME) {
    if (copy(from, to)) {
      return 1;
    }
    if (unlink(from)) {
      error("cannot remove `/%s'", from);
      return 1;
    }
  }
#ifdef F_LINK
  else if (flag == F_LINK) {
    if (symlink(from, to) == 0) { /* symbolic link */
      return 0;
    }
  }
#endif
  return 0;
}
#endif

void debug_perror(const char *what, const char *file) {
  if (file) {
    debug_message("System Error: %s:%s:%s\n", what, file, strerror(errno));
  } else {
    debug_message("System Error: %s:%s\n", what, strerror(errno));
  }
}

/*
 * do_rename is used by the efun rename. It is basically a combination
 * of the unix system call rename and the unix command mv.
 */

static svalue_t from_sv = {T_NUMBER};
static svalue_t to_sv = {T_NUMBER};

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_file_sv() {
  mark_svalue(&from_sv);
  mark_svalue(&to_sv);
}
#endif

#ifdef F_RENAME
int do_rename(const char *fr, const char *t, int flag) {
  const char *from;
  const char *to;
  char newfrom[MAX_FNAME_SIZE + MAX_PATH_LEN + 2];
  int flen;
  extern svalue_t apply_ret_value;

  /*
   * important that the same write access checks are done for link() as are
   * done for rename().  Otherwise all kinds of security problems would
   * arise (e.g. creating links to files in protected directories and then
   * modifying the protected file by modifying the linked file). The idea
   * is prevent linking to a file unless the person doing the linking has
   * permission to move the file.
   */
  from = check_valid_path(fr, current_object, "rename", 1);
  if (!from) {
    return 1;
  }

  assign_svalue(&from_sv, &apply_ret_value);

  to = check_valid_path(t, current_object, "rename", 1);
  if (!to) {
    return 1;
  }

  assign_svalue(&to_sv, &apply_ret_value);
  if (!strlen(to) && !strcmp(t, "/")) {
    to = "./";
  }

  /* Strip trailing slashes */
  flen = strlen(from);
  if (flen > 1 && from[flen - 1] == '/') {
    const char *p = from + flen - 2;
    int n;

    while (*p == '/' && (p > from)) {
      p--;
    }
    n = p - from + 1;
    memcpy(newfrom, from, n);
    newfrom[n] = 0;
    from = newfrom;
  }

  if (file_size(to) == -2) {
    /* Target is a directory; build full target filename. */
    const char *cp;
    char newto[MAX_FNAME_SIZE + MAX_PATH_LEN + 2];

    cp = strrchr(from, '/');
    if (cp) {
      cp++;
    } else {
      cp = from;
    }

    sprintf(newto, "%s/%s", to, cp);
    return do_move(from, newto, flag);
  } else {
    return do_move(from, to, flag);
  }
}
#endif /* F_RENAME */

int copy_file(const char *from, const char *to) {
  char buf[128];
  int from_fd, to_fd;
  int num_read, num_written;
  char *write_ptr;
  extern svalue_t apply_ret_value;

  from = check_valid_path(from, current_object, "move_file", 0);
  assign_svalue(&from_sv, &apply_ret_value);

  to = check_valid_path(to, current_object, "move_file", 1);
  assign_svalue(&to_sv, &apply_ret_value);

  if (from == 0) {
    return -1;
  }
  if (to == 0) {
    return -2;
  }

  if (lstat(from, &from_stats) != 0) {
    error("/%s: lstat failed\n", from);
    return 1;
  }
  if (lstat(to, &to_stats) == 0) {
    if (from_stats.st_dev == to_stats.st_dev && from_stats.st_ino == to_stats.st_ino) {
      error("`/%s' and `/%s' are the same file", from, to);
      return 1;
    }
  } else if (errno != ENOENT) {
    error("/%s: unknown error\n", to);
    return 1;
  }

  from_fd = open(from, O_RDONLY);
  if (from_fd < 0) {
    return -1;
  }

  if (file_size(to) == -2) {
    /* Target is a directory; build full target filename. */
    const char *cp;
    char newto[MAX_FNAME_SIZE + MAX_PATH_LEN + 2];

    cp = strrchr(from, '/');
    if (cp) {
      cp++;
    } else {
      cp = from;
    }

    sprintf(newto, "%s/%s", to, cp);
    close(from_fd);
    return copy_file(from, newto);
  }
  to_fd = open(to, O_WRONLY | O_CREAT | O_TRUNC, 0666);
  if (to_fd < 0) {
    close(from_fd);
    return -2;
  }
  while ((num_read = read(from_fd, buf, 128)) != 0) {
    if (num_read < 0) {
      debug_perror("copy_file: read", from);
      close(from_fd);
      close(to_fd);
      return -3;
    }
    write_ptr = buf;
    while (write_ptr != (buf + num_read)) {
      num_written = write(to_fd, write_ptr, num_read);
      if (num_written < 0) {
        debug_perror("copy_file: write", to);
        close(from_fd);
        close(to_fd);
        return -3;
      }
      write_ptr += num_written;
    }
  }
  close(from_fd);
  close(to_fd);
  return 1;
}

#ifdef F_CP
void f_cp(void) {
  int i;

  i = copy_file(sp[-1].u.string, sp[0].u.string);
  free_string_svalue(sp--);
  free_string_svalue(sp);
  put_number(i);
}
#endif

#ifdef F_FILE_SIZE
void f_file_size(void) {
  LPC_INT i = file_size(sp->u.string);
  free_string_svalue(sp);
  put_number(i);
}
#endif

#ifdef F_GET_DIR
void f_get_dir(void) {
  array_t *vec;

  vec = get_dir((sp - 1)->u.string, sp->u.number);
  free_string_svalue(--sp);
  if (vec) {
    put_array(vec);
  } else {
    *sp = const0;
  }
}
#endif

#ifdef F_LINK
void f_link(void) {
  svalue_t *ret, *arg;
  int i;

  arg = sp;
  push_svalue(arg - 1);
  push_svalue(arg);
  ret = apply_master_ob(APPLY_VALID_LINK, 2);
  if (MASTER_APPROVED(ret)) {
    i = do_rename((sp - 1)->u.string, sp->u.string, F_LINK);
  } else {
    i = 0;
  }
  (--sp)->type = T_NUMBER;
  sp->u.number = i;
  sp->subtype = 0;
}
#endif /* F_LINK */

#ifdef F_MKDIR
void f_mkdir(void) {
  const char *path;

  path = check_valid_path(sp->u.string, current_object, "mkdir", 1);
  if (!path || mkdir(path, 0770) == -1) {
    free_string_svalue(sp);
    *sp = const0;
  } else {
    free_string_svalue(sp);
    *sp = const1;
  }
}
#endif
