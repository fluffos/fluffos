/*
 * file: file.c
 * description: handle all file based efuns
 */
#include "base/package_api.h"

#include "packages/core/file.h"

#include <iostream>
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
#include <sstream>
#include <unistd.h>
#include <zlib.h>

#include "base/internal/strutils.h"
#include "ghc/fs_std.hpp"

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

#ifdef _WIN32
#define lstat(x, y) stat(x, y)
#define link(x, y) ((-1))
#define OS_mkdir(x, y) mkdir(x)
#else
#define OS_mkdir(x, y) mkdir(x, y)
#endif

static int match_string(char const * /*match*/, char const * /*str*/);
static int pstrcmp(const void * /*p1*/, const void * /*p2*/);
static int parrcmp(const void * /*p1*/, const void * /*p2*/);
static void encode_stat(svalue_t * /*vp*/, int /*flags*/, char * /*str*/, struct stat * /*st*/);

#define MAX_LINES 50

/*
 * These are used by qsort in get_dir().
 */
static int pstrcmp(const void *p1, const void *p2) {
  auto *x = reinterpret_cast<svalue_t const *>(p1);
  auto *y = reinterpret_cast<svalue_t const *>(p2);

  return x->u.string->compare(y->u.string);
}

static int parrcmp(const void *p1, const void *p2) {
  auto *x = reinterpret_cast<svalue_t const *>(p1);
  auto *y = reinterpret_cast<svalue_t const *>(p2);

  return x->u.arr->item[0].u.string->compare(y->u.arr->item[0].u.string);
}

static void encode_stat(svalue_t *vp, int flags, char *str, struct stat *st) {
  if (flags == -1) {
    array_t *v = allocate_empty_array(3);

    v->item[0].type = T_STRING;
    v->item[0].subtype = 0;
    v->item[0].u.string = std::string {str};
    v->item[1].type = T_NUMBER;
    v->item[1].u.number = ((st->st_mode & S_IFDIR) ? -2 : st->st_size);
    v->item[2].type = T_NUMBER;
    v->item[2].u.number = st->st_mtime;
    vp->type = T_ARRAY;
    vp->u.arr = v;
  } else {
    vp->type = T_STRING;
    vp->subtype = 0;
    vp->u.string = std::string {str};
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
array_t *get_dir(const std::string path, int flags) {
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

  if (path.empty()) {
    return nullptr;
  }

  auto i_path {check_valid_path(path, current_object, "stat", 0)};

  if (i_path.empty()) {
    return nullptr;
  }

  if (i_path.size() < 2) {
    temppath[0] = i_path[0] ? i_path[0] : '.';
    temppath[1] = '\000';
    p = temppath;
  } else {
    strncpy(temppath, i_path.c_str(), MAX_FNAME_SIZE + MAX_PATH_LEN + 1);
    temppath[MAX_FNAME_SIZE + MAX_PATH_LEN + 1] = '\0';

    /*
     * If path ends with '/' or "/." remove it
     */
    if ((p = strrchr(temppath, '/')) == nullptr) {
      p = temppath;
    }
    if (p[0] == '/' && ((p[1] == '.' && p[2] == '\0') || p[1] == '\0')) {
      *p = '\0';
    }
  }

  if (stat(temppath, &st) < 0) {
    if (*p == '\0') {
      return nullptr;
    }
    if (p != temppath) {
      strncpy(regexppath, p + 1, MAX_FNAME_SIZE + MAX_PATH_LEN + 1);
      *p = '\0';
    } else {
      strncpy(regexppath, p, MAX_FNAME_SIZE + MAX_PATH_LEN + 1);
      strncpy(temppath, ".", MAX_FNAME_SIZE + MAX_PATH_LEN + 1);
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
  if ((dirp = opendir(temppath)) == nullptr) {
    return nullptr;
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
  size_t size {strlen(temppath)};
  endtemp = temppath + size;

  strncat(endtemp++, "/", MAX_FNAME_SIZE + MAX_PATH_LEN + 1 - size++);

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
      strncpy(endtemp, de->d_name, MAX_FNAME_SIZE + MAX_PATH_LEN + 1 - size);
      stat(temppath, &st); /* We assume it works. */ // Not a good idea....
    }
    encode_stat(&v->item[i], flags, de->d_name, &st);
    i++;
  }
  closedir(dirp);
  /* Sort the names. */
  qsort((void *)v->item, count, sizeof v->item[0], (flags == -1) ? parrcmp : pstrcmp);
  return v;
}

int remove_file(const std::string path) {
  auto i_path {check_valid_path(path, current_object, "remove_file", 1)};

  if (i_path.empty()) {
    return 0;
  }
  if (unlink(i_path.c_str()) == -1) {
    return 0;
  }
  return 1;
}

/*
 * Append string to file. Return 0 for failure, otherwise 1.
 */
int write_file(const std::string file, const std::string str, int flags) {
  FILE *f;
  gzFile gf;

  auto i_file = check_valid_path(file, current_object, "write_file", 1);
  if (i_file.empty()) {
    return 0;
  }
  if (flags & 2) {
    gf = gzopen(i_file.c_str(), (flags & 1) ? "wb" : "ab");
    if (!gf) {
      error("Wrong permissions for opening file /%s for %s.\n\"%s\"\n", file,
            (flags & 1) ? "overwrite" : "append", strerror(errno));
    }
  } else {
    f = fopen(i_file.c_str(), (flags & 1) ? "wb" : "ab");
    if (f == nullptr) {
      error("Wrong permissions for opening file /%s for %s.\n\"%s\"\n", file,
            (flags & 1) ? "overwrite" : "append", strerror(errno));
    }
  }
  if (flags & 2) {
    gzwrite(gf, str.c_str(), str.size());
  } else {
    fwrite(str.c_str(), str.size(), 1, f);
  }

  if (flags & 2) {
    gzclose(gf);
  } else {
    fclose(f);
  }
  return 1;
}

/* Reads file, starting from line of "start", with maximum lines of "lines".
 * Returns a new string.
 */
std::string *read_file(const std::string file, int start, int lines) {
    static char *theBuff {nullptr};
    std::string *ret {nullptr};
    const auto read_file_max_size = CONFIG_INT(__MAX_READ_FILE_SIZE__);

    if (lines < 0) {
        debug(file, "read_file: trying to read negative lines: %d", lines);
        return ret;
    }

    auto real_file {check_valid_path(file, current_object, "read_file", 0)};
    if (real_file.empty()) {
        return ret;
    }

    auto fs_real_file = fs::path(real_file);

    /*
    * file doesn't exist, or is really a directory
    */
    if (!fs::exists(fs_real_file) || fs::is_directory(fs_real_file)) {
        return ret;
    }

    if (fs::is_empty(fs_real_file)) {
    /* zero length file */
        ret = new std::string();
        return ret;
    }

    gzFile f = gzopen(real_file.c_str(), "rb");

    if (f == nullptr) {
        debug(file, "read_file: fail to open: {}.\n", file);
        return ret;
    }

    if (!theBuff) {
        theBuff = reinterpret_cast<char *>(DMALLOC(2 * read_file_max_size + 1, TAG_PERMANENT, "read_file: theBuff"));
    }

    int total_bytes_read = gzread(f, (void *)theBuff, 2 * read_file_max_size);
    gzclose(f);

    if (total_bytes_read <= 0) {
        debug(file, "read_file: read error: {}.\n", file);
        return ret;
    }
    theBuff[total_bytes_read] = '\0';

    // skip forward until the "start"-th line
    char *ptr_start = theBuff;
    while (start > 1 && ptr_start < theBuff + total_bytes_read) {
        if (*ptr_start == '\0') {
            debug(file, "read_file: file contains '\\0': {}.\n", file);
            return ret;
        }
        if (*ptr_start == '\n') {
            start--;
        }
        ptr_start++;
    }

    // not found
    if (start > 1) {
        debug(file, "read_file: reached EOF searching for start: {}.\n", file);
        return ret;
    }

    char *ptr_end {nullptr};
    // search forward for "lines" of '\n' for the end
    if (lines == 0) {
        ptr_end = ptr_start + read_file_max_size;
        if (ptr_end > theBuff + total_bytes_read) {
            ptr_end = theBuff + total_bytes_read + 1;
        }
    } else {
        ptr_end = ptr_start;
        while (lines > 0 && ptr_end < theBuff + total_bytes_read) {
            if (*ptr_end++ == '\n') {
                lines--;
            }
        }
        // not enough lines, directly go to the end.
        if (lines > 0) {
            ptr_end = theBuff + total_bytes_read + 1;
        }
    }

    *ptr_end = '\0';
    // result is too big.
    if (strlen(ptr_start) > read_file_max_size) {
        debug(file, "read_file: result too big: {}.\n", file);
        return ret;
    }

    ret = new std::string(ptr_start);
    if (ret->find('\r') != std::string::npos) {;
        // Deal with CRLF.
        ReplaceStringInPlace(*ret, "\r\n", "\n");
    }
    return ret;
}

std::string *read_bytes(const std::string file, int start, int len, int *rlen) {
  const auto max_byte_transfer = CONFIG_INT(__MAX_BYTE_TRANSFER__);

  struct stat st;
  FILE *fptr;
  char *str;
  int size;
  std::string *ret {nullptr};

  if (len < 0) {
    return ret;
  }
  auto i_file {check_valid_path(file, current_object, "read_bytes", 0)};
  if (i_file.empty()) {
    return ret;
  }
  fptr = fopen(i_file.c_str(), "rb");
  if (fptr == nullptr) {
    return ret;
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
    return ret;
  }
  if (start >= size) {
    fclose(fptr);
    return ret;
  }
  if ((start + len) > size) {
    len = (size - start);
  }

  if ((size = fseek(fptr, start, 0)) < 0) {
    fclose(fptr);
    return ret;
  }

  str = new char[len];

  size = fread(str, 1, len, fptr);

  fclose(fptr);

  if (size <= 0) {
    delete[] str;
    return ret;
  }
  /*
   * The string has to end to '\0'!!!
   */
  str[size] = '\0';
  ret = new std::string(str);
  delete[] str;

  *rlen = size;
  return ret;
}

int write_bytes(const std::string file, int start, const std::string str) {
    return write_bytes(file, start, str.c_str(), str.size());
}

int write_bytes(const std::string file, int start, const char *str, size_t len) {
  const auto max_byte_transfer = CONFIG_INT(__MAX_BYTE_TRANSFER__);

  struct stat st;
  int size;
  FILE *fptr;

  auto i_file {check_valid_path(file, current_object, "write_bytes", 1)};

  if (i_file.empty()) {
    return 0;
  }
  if (len > max_byte_transfer) {
    return 0;
  }
  /* Under system V, it isn't possible change existing data in a file
   * opened for append, so it can't be opened for append.
   * opening for r+ won't create the file if it doesn't exist.
   * opening for w or w+ will truncate it if it does exist.  So we
   * have to check if it exists first.
   */
  if (stat(i_file.c_str(), &st) == -1) {
    fptr = fopen(i_file.c_str(), "wb");
  } else {
    fptr = fopen(i_file.c_str(), "r+b");
  }
  if (fptr == nullptr) {
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
  size = fwrite(str, 1, len, fptr);

  fclose(fptr);

  if (size <= 0) {
    return 0;
  }
  return 1;
}

int file_size(const std::string file) {
  struct stat st;
  long ret;

  auto i_file {check_valid_path(file, current_object, "file_size", 0)};
  if (i_file.empty()) {
    return -1;
  }

  if (stat(i_file.c_str(), &st) == -1) {
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
const std::string check_valid_path(const std::string path, object_t *call_object, const char *const call_fun, int writeflg) {
  svalue_t *v;
  std::string ret {};

  if (!master_ob && !call_object) {
    // early startup, ignore security
    free_svalue(&apply_ret_value, "check_valid_path");
    apply_ret_value.type = T_STRING;
    apply_ret_value.subtype = 0;
    apply_ret_value.u.string = path;
    return path;
  }

  if (call_object == nullptr || call_object->flags & O_DESTRUCTED) {
    return ret;
  }

  push_string(path);
  push_object(call_object);
  push_string(call_fun);
  if (writeflg) {
    v = apply_master_ob(APPLY_VALID_WRITE, 3);
  } else {
    v = apply_master_ob(APPLY_VALID_READ, 3);
  }

  if (v == (svalue_t *)-1) {
    v = nullptr;
  }

  if (v && v->type == T_NUMBER && v->u.number == 0) {
    return ret;
  }
  if (v && v->type == T_STRING) {
    ret = v->u.string;
  } else {
    extern svalue_t apply_ret_value;

    free_svalue(&apply_ret_value, "check_valid_path");
    apply_ret_value.type = T_STRING;
    apply_ret_value.subtype = STRING_MALLOC;
    ret = apply_ret_value.u.string = path;
  }

  if (ret[0] == '/') {
    ret = ret.substr(1, std::string::npos);
  }
  if (ret.empty()) {
    ret = ".";
  }
  if (legal_path(ret)) {
    return ret;
  }

  return nullptr;
}

static int match_string(char const *match, char const *str) {
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
      for (i = 0; str[i] != '\0'; i++) {
        if (match_string(match, str + i)) {
          return 1;
        }
      }
      return 0;
    case '\0':
      return 0;
    case '\\':
      match++;
      if (*match == '\0') {
        return 0;
      }
      [[gnu::fallthrough]];
    default:
      if (*match == *str) {
        match++;
        str++;
        goto again;
      }
      return 0;
  }
}

void debug_perror(const char *what, const char *file) {
  if (file) {
    debug_message("System Error: {}:{}:{}\n", what, file, strerror(errno));
  } else {
    debug_message("System Error: {}:{}\n", what, strerror(errno));
  }
}

static svalue_t from_sv;
static svalue_t to_sv;

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_file_sv() {
  mark_svalue(&from_sv);
  mark_svalue(&to_sv);
}
#endif

/* Move FROM onto TO.  Handles cross-filesystem moves.
   If TO is a directory, FROM must be also.
   Return 0 if successful, 1 if an error occurred.  */

#ifdef F_RENAME
static int do_move(const std::string from, const std::string to, int flag) {
    struct stat to_stats, from_stats;

  if (lstat(from.c_str(), &from_stats) != 0) {
    error("/{}: lstat failed\n", from);
    return 1;
  }
  if (lstat(to.c_str(), &to_stats) == 0) {
#ifdef __WIN32
    if (from == to) {
#else
    if (from_stats.st_dev == to_stats.st_dev && from_stats.st_ino == to_stats.st_ino) {
#endif
      error("`/{}' and `/{}' are the same file", from, to);
      return 1;
    }
    if (S_ISDIR(to_stats.st_mode)) {
      error("/{}: cannot overwrite directory", to);
      return 1;
    }
  } else if (errno != ENOENT) {
    error("/{}: unknown error\n", to);
    return 1;
  }
  if (flag == F_RENAME) {
    std::error_code errorCode;
    fs::rename(from, to, errorCode);
    if (!errorCode) {
      return 0;
    }
  }
#ifdef F_LINK
  else if (flag == F_LINK) {
    if (link(from.c_str(), to.c_str()) == 0) {
      return 0;
    }
  }
#endif

  if (errno != EXDEV) {
    if (flag == F_RENAME) {
      error("cannot move `/{}' to `/{}'\n", from, to);
    } else {
      error("cannot link `/{}' to `/{}'\n", from, to);
    }
    return 1;
  }
  /* rename failed on cross-filesystem link.  Copy the file instead. */
  if (flag == F_RENAME) {
    if (copy_file(from, to)) {
      return 1;
    }
    if (unlink(from.c_str())) {
      error("cannot remove `/{}'", from);
      return 1;
    }
  }
#ifdef F_LINK
  else if (flag == F_LINK) {
    if (symlink(from.c_str(), to.c_str()) == 0) { /* symbolic link */
      return 0;
    }
  }
#endif
  return 0;
}

/*
 * do_rename is used by the efun rename. It is basically a combination
 * of the unix system call rename and the unix command mv.
 */

int do_rename(const std::string fr, const std::string t, int flag) {
  extern svalue_t apply_ret_value;

  /*
   * important that the same write access checks are done for link() as are
   * done for rename().  Otherwise all kinds of security problems would
   * arise (e.g. creating links to files in protected directories and then
   * modifying the protected file by modifying the linked file). The idea
   * is prevent linking to a file unless the person doing the linking has
   * permission to move the file.
   */
  std::string from {check_valid_path(fr, current_object, "rename", 1)};
  if (from.empty()) {
    return 1;
  }

  assign_svalue(&from_sv, &apply_ret_value);

  auto to {check_valid_path(t, current_object, "rename", 1)};
  if (to.empty()) {
    return 1;
  }

  assign_svalue(&to_sv, &apply_ret_value);
  if (to.empty() && !t.compare("/")) {
    to = "./";
  }

  /* Strip trailing slashes */
  from = rtrim(from, "/");

  if (file_size(to) == -2) {
    /* Target is a directory; build full target filename. */
    const char *cp;
    std::string newto;

    cp = strrchr(from.c_str(), '/');
    if (cp) {
      cp++;
    } else {
      cp = from.c_str();
    }

    newto = to + '/' + cp;
    return do_move(from.c_str(), newto.c_str(), flag);
  } else {
    return do_move(from.c_str(), to.c_str(), flag);
  }
}
#endif /* F_RENAME */

int copy_file(const std::string from, const std::string to) {
    struct stat to_stats, from_stats;

  extern svalue_t apply_ret_value;

  auto i_from = check_valid_path(from, current_object, "move_file", 0);
  assign_svalue(&from_sv, &apply_ret_value);

  auto i_to = check_valid_path(to, current_object, "move_file", 1);
  assign_svalue(&to_sv, &apply_ret_value);

  if (i_from.empty()) {
    return -1;
  }
  if (i_to.empty()) {
    return -2;
  }

  if (lstat(i_from.c_str(), &from_stats) != 0) {
    error("/{}: lstat failed\n", from);
    return 1;
  }
  if (lstat(i_to.c_str(), &to_stats) == 0) {
#ifdef __WIN32
    if (!from.compare(to)) {
#else
    if (from_stats.st_dev == to_stats.st_dev && from_stats.st_ino == to_stats.st_ino) {
#endif
      error("`/{}' and `/{}' are the same file", from, to);
      return 1;
    }
  } else if (errno != ENOENT) {
    error("/{}: unknown error\n", to);
    return 1;
  }

  if (file_size(to) == -2) {
    /* Target is a directory; build full target filename. */
    const char *cp;
    std::string newto;

    cp = strrchr(from.c_str(), '/');
    if (cp) {
      cp++;
    } else {
      cp = from.c_str();
    }
    newto = to + '/' + cp;
    return copy_file(from, newto);
  }

  std::error_code error_code;
  auto base = fs::current_path();
  fs::copy_file(base / from, base / to, fs::copy_options::overwrite_existing, error_code);

  if (error_code) {
    debug_message("Error copying file from /%s to /%s, Error: %s", from, to,
                  error_code.message().c_str());
    return -1;
  }

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
    *sp = 0;
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
  const std::string path {check_valid_path(sp->u.string, current_object, "mkdir", 1)};
  if (path.empty() || OS_mkdir(path.c_str(), 0770) == -1) {
    free_string_svalue(sp);
    *sp = 0;
  } else {
    free_string_svalue(sp);
    *sp = 1;
  }
}
#endif
