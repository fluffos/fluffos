/*
 * file: file.c
 * description: handle all file based efuns
 */

#include "config.h"
#include <sys/types.h>
#if (defined(_SEQUENT_) || defined(hpux) || defined(sgi) \
	|| defined(_AUX_SOURCE) || defined(linux) || defined(cray) \
	|| defined(SunOS_5))
#include <sys/sysmacros.h>
#endif
#include <sys/stat.h>
#ifndef SunOS_5
#include <sys/dir.h>
#endif
#ifdef __386BSD__
#include <unistd.h>
#endif
#include <fcntl.h>
#include <setjmp.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <memory.h>
#if defined(sun)
#include <alloca.h>
#endif
#if defined(M_UNIX) || defined(OSF) || defined(_SEQUENT_) \
	|| defined(cray) || defined(SunOS_5)
#include <dirent.h>
#endif
#if defined(SVR4)
#include <dirent.h>
#include <sys/filio.h>
#include <sys/sockio.h>
#include <sys/mkdev.h>
#endif
#include <ctype.h>

#include "lint.h"
#include "opcodes.h"
#include "interpret.h"
#include "object.h"
#include "sent.h"
#include "exec.h"
#include "comm.h"

extern int errno;
extern int comp_flag;
extern int max_array_size;

char *inherit_file;

#if !defined(NeXT) && !defined(__386BSD__)
extern int readlink PROT((char *, char *, int));
extern int symlink PROT((char *, char *));
#ifdef MSDOS
#define lstat stat
#else
#if !defined(hpux) && !defined(SVR4) \
	&& !defined(linux) && !defined(SunOS_5) && !defined(sgi) \
	&& !defined(__bsdi__)
extern int lstat PROT((char *, struct stat *));
#endif
#endif
#endif /* !NeXT && !__386BSD__ */

#if !defined(hpux) && !defined(_AIX) && !defined(__386BSD__)  \
	&& !defined(linux) && !defined(SunOS_5) && !defined(SVR4) \
	&& !defined(__bsdi__)
extern int fchmod PROT((int, int));
#endif /* !defined(hpux) && !defined(_AIX) */

int legal_path PROT((char *));

extern int d_flag;

struct object *current_object;      /* The object interpreting a function. */
struct object *command_giver;       /* Where the current command came from. */
struct object *current_interactive; /* The user who caused this execution */

#define MAX_LINES 50

/*
 * These are used by qsort in get_dir().
 */
static int pstrcmp(p1, p2)
    struct svalue *p1, *p2;
{
    return strcmp(p1->u.string, p2->u.string);
}
static int parrcmp(p1, p2)
    struct svalue *p1, *p2;
{
    return strcmp(p1->u.vec->item[0].u.string, p2->u.vec->item[0].u.string);
}

static void encode_stat(vp, flags, str, st)
struct svalue *vp;
int flags;
char *str;
struct stat *st;
{
   if (flags == -1) {
      struct vector *v = allocate_array(3);

      v->item[0].type = T_STRING;
      v->item[0].subtype = STRING_MALLOC;
      v->item[0].u.string = string_copy(str);
      v->item[1].type = T_NUMBER;
      v->item[1].u.number =
        ((st->st_mode & S_IFDIR) ? -2 : st->st_size);
      v->item[2].type = T_NUMBER; 
      v->item[2].u.number = st->st_mtime;
      vp->type = T_POINTER;
      vp->u.vec = v;
    } else {
		vp->type = T_STRING;
		vp->subtype = STRING_MALLOC;
		vp->u.string = string_copy(str);
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
 *    last update of file,
 *    size of file (-2 means file doesn't exist).
 */
#define MAX_FNAME_SIZE 255
#define MAX_PATH_LEN   1024
struct vector *get_dir(path, flags)
    char *path;
    int flags;
{
    struct vector *v;
    int i, count = 0;
    DIR *dirp;
    int namelen, do_match = 0;
#if defined(_AIX) || defined(M_UNIX) || defined(OSF) || defined(_SEQUENT_) \
	|| defined(SVR4) || defined(cray) || defined(SunOS_5)
    struct dirent *de;
#else
    struct direct *de;
#endif
    struct stat st;
    char *endtemp;
	char temppath[MAX_FNAME_SIZE + MAX_PATH_LEN + 2];
	char regexp[MAX_FNAME_SIZE + MAX_PATH_LEN + 2];
    char *p;

    if (!path)
	return 0;

    path = check_valid_path(path, current_object, "stat", 0);

    if (path == 0)
	return 0;

    if (strlen(path)<2) {
	temppath[0]=path[0]?path[0]:'.';
	temppath[1]='\000';
	p = temppath;
    } else {
	strncpy(temppath, path, MAX_FNAME_SIZE + MAX_PATH_LEN + 2);
	/*
	 * If path ends with '/' or "/." remove it
	 */
	if ((p = strrchr(temppath, '/')) == 0)
	    p = temppath;
	if (p[0] == '/' && ((p[1] == '.' && p[2] == '\0') || p[1] == '\0'))
	  *p = '\0';
    }

    if (stat(temppath, &st) < 0) {
	if (*p == '\0')
	    return 0;
	if (p != temppath) {
	    strcpy(regexp, p + 1);
	    *p = '\0';
	} else {
	    strcpy(regexp, p);
	    strcpy(temppath, ".");
	}
	do_match = 1;
    } else if (*p != '\0' && strcmp(temppath, ".")) {
	if (*p == '/' && *(p + 1) != '\0')
	    p++;
	v = allocate_array(1);
        encode_stat(&v->item[0], flags, p, &st);
	return v;
    }

    if ((dirp = opendir(temppath)) == 0)
	return 0;

    /*
     *  Count files
     */
    for (de = readdir(dirp); de; de = readdir(dirp)) {
#if defined(OSF) || defined(M_UNIX) || defined(_SEQUENT_) || defined(SVR4) \
	|| defined(linux) || defined(cray) || defined(SunOS_5)
	namelen = strlen(de->d_name);
#else
	namelen = de->d_namlen;
#endif
	if (!do_match && (strcmp(de->d_name, ".") == 0 ||
			  strcmp(de->d_name, "..") == 0))
	    continue;
	if (do_match && !match_string(regexp, de->d_name))
	    continue;
	count++;
	if (count >= max_array_size)
	    break;
    }
    /*
     * Make array and put files on it.
     */
    v = allocate_array(count);
    if (count == 0) {
	/* This is the easy case :-) */
	closedir(dirp);
	return v;
    }
    rewinddir(dirp);
    endtemp = temppath + strlen(temppath);
    strcat(endtemp++, "/");
    for(i = 0, de = readdir(dirp); i < count; de = readdir(dirp)) {
#if defined(OSF) || defined(_SEQUENT_) || defined(SVR4) || defined(linux) \
     || defined(M_UNIX) || defined(cray) || defined(SunOS_5)
        namelen = strlen(de->d_name);
#else
	namelen = de->d_namlen;
#endif
	if (!do_match && (strcmp(de->d_name, ".") == 0 ||
			  strcmp(de->d_name, "..") == 0))
	    continue;
	if (do_match && !match_string(regexp, de->d_name))
	    continue;
	de->d_name[namelen] = '\0';
        if (flags == -1) {
           /* We'll have to .... sigh.... stat() the file to
              get some add'tl info.  */
           strcpy(endtemp, de->d_name);
           stat(temppath, &st);  /* We assume it works. */
        }
        encode_stat(&v->item[i], flags, de->d_name, &st);
	i++;
    }
    closedir(dirp);
    /* Sort the names. */
#ifdef _SEQUENT_
    qsort((void *)v->item, count, sizeof v->item[0],
          (int (*)())((flags == -1) ? parrcmp : pstrcmp));
#else
    qsort((char *)v->item, count, sizeof v->item[0],
          (flags == -1) ? parrcmp : pstrcmp);
#endif
    return v;
}

int tail(path)
    char *path;
{
    char buff[1000];
    FILE *f;
    struct stat st;
    int offset;
 
    path = check_valid_path(path, current_object, "tail", 0);

    if (path == 0)
        return 0;
    f = fopen(path, "r");
    if (f == 0)
	return 0;
    if (fstat(fileno(f), &st) == -1)
	fatal("Could not stat an open file.\n");
    offset = st.st_size - 54 * 20;
    if (offset < 0)
	offset = 0;
    if (fseek(f, offset, 0) == -1)
	fatal("Could not seek.\n");
    /* Throw away the first incomplete line. */
    if (offset > 0)
	(void)fgets(buff, sizeof buff, f);
    while(fgets(buff, sizeof buff, f)) {
	add_message("%s", buff);
    }
    fclose(f);
    return 1;
}

int print_file(path, start, len)
    char *path;
    int start, len;
{
    char buff[1000];
    FILE *f;
    int i;

    if (len < 0)
	return 0;

    path = check_valid_path(path, current_object, "print_file", 0);

    if (path == 0)
        return 0;
    if (start < 0)
	return 0;
    f = fopen(path, "r");
    if (f == 0)
	return 0;
    if (len == 0)
	len = MAX_LINES;
    if (len > MAX_LINES)
	len = MAX_LINES;
    if (start == 0)
	start = 1;
    for (i=1; i < start + len; i++) {
	if (fgets(buff, sizeof buff, f) == 0)
	    break;
	if (i >= start)
	    add_message("%s", buff);
    }
    fclose(f);
    if (i <= start)
	return 0;
    if (i == MAX_LINES + start)
	add_message("*****TRUNCATED****\n");
    return i-start;
}

int remove_file(path)
    char *path;
{
    path = check_valid_path(path, current_object, "remove_file", 1);

    if (path == 0)
        return 0;
    if (unlink(path) == -1)
        return 0;
    return 1;
}

void log_file(file, str)
    char *file, *str;
{
    FILE *f;
    char the_file_name[100], *file_name;
    struct stat st;

    file_name = the_file_name;
    sprintf(file_name, "%s/%s", LOG_DIR, file);
    file_name = check_valid_path(file_name, current_object, "log_file", 1);
    if (!file_name)
	    return;
    if (file_name[0] == '/')
       file_name++;
    if (stat(file_name, &st) != -1 && st.st_size > MAX_LOG_SIZE) {
      char file_name2[sizeof file_name + 4];
      sprintf(file_name2, "%s.old", file_name);
      rename(file_name, file_name2);	/* No panic if failure */
    }
    f = fopen(file_name, "a");	/* Skip leading '/' */
    if (f == 0)
      return;
    fwrite(str, strlen(str), 1, f);
    fclose(f);
}

/*
 * Check that it is an legal path. No '..' are allowed.
 */
int legal_path(path)
    char *path;
{
    char *p;

    if (path == NULL || strchr(path, ' '))
	return 0;
    if (path[0] == '/')
        return 0;
	/* disallowing # seems the easiest way to solve a bug involving
	   loading files containing that character
	*/
	if (strchr(path, '#')) {
		return 0;
	}
#ifdef MSDOS
    if (!valid_msdos(path)) return(0);
#endif
    for(p = strchr(path, '.'); p; p = strchr(p+1, '.')) {
	if (p[1] == '.')
	    return 0;
	if (p[1] == '/') /* disallow paths like /data/./mail/buddha-mbox.o */
	    return 0;
    }
#ifdef AMIGA /* I don't know what the proper define should be, just leaving
		an appropriate place for the right stuff to happen here 
		- Wayfarer */
    /* fail if there's a ':' since on AmigaDOS this means it's a
       logical device! */
    if (strchr(path, ':'))
      return 0;
#endif
    return 1;
}

/*
 * There is an error in a specific file. Ask the MudOS driver to log the
 * message somewhere.
 */
void smart_log(error_file, line, what, flush)
     char *error_file, *what;
     int line, flush;
{
  static char *buff[6], *files[6];
  static int count = 0;
  int i;
  
  if (count > 5)
    fatal ("Fatal error in smart_log\n");

  if (count == 5)
    flush = 1;

  if (error_file) 
    {
      /* the +30 is more than is needed, but what the heck? */
      buff[count] = (char *)
		DMALLOC(strlen(error_file)+strlen(what)+30, 39, "smart_log: 1");
      files[count] = (char *)
		DMALLOC(strlen(error_file)+1, 40, "smart_log: 2");
      sprintf (buff[count], "%s line %d:%s\n", error_file, line, what);
      strcpy (files[count],error_file);
      count ++;
    }
  if (flush) 
    {
      for (i = 0; i < count; i ++)
	{
	  push_constant_string(files[i]);
	  push_constant_string(buff[i]);
	  safe_apply_master_ob("log_error", 2);
	  FREE(files[i]);
	  FREE(buff[i]);
	}
      count = 0;
    }
}

/*
 * Append string to file. Return 0 for failure, otherwise 1.
 */
int write_file(file, str)
    char *file;
    char *str;
{
    FILE *f;

    file = check_valid_path(file, current_object, "write_file", 1);
    if (!file)
	return 0;
    f = fopen(file, "a");
    if (f == 0)
	error("Wrong permissions for opening file %s for append.\n", file);
    fwrite(str, strlen(str), 1, f);
    fclose(f);
    return 1;
}

char *read_file(file,start,len)
    char *file;
    int start,len;
{
    struct stat st;
    FILE *f;
    char *str,*p,*p2,*end,c;
    int size;

    if (len < 0) return 0;
    file = check_valid_path(file, current_object, "read_file", 0);

    if (!file)
	return 0;
    f = fopen(file, "r");
    if (f == 0)
	return 0;
    if (fstat(fileno(f), &st) == -1)
	fatal("Could not stat an open file.\n");
    size = st.st_size;
    if (size > READ_FILE_MAX_SIZE) {
	if ( start || len ) size = READ_FILE_MAX_SIZE;
	else {
	    fclose(f);
	    return 0;
	}
    }
    if (!start) start = 1;
    if (!len) len = READ_FILE_MAX_SIZE;
    str = DXALLOC(size + 1, 41, "read_file: str");
    str[size] = '\0';
    do {
	if (size > st.st_size)
	    size = st.st_size;
        if (fread(str, size, 1, f) != 1) {
    	    fclose(f);
	    FREE(str);
    	    return 0;
        }
	st.st_size -= size;
	end = str+size;
        for (p=str; ( p2=memchr(p,'\n',end-p) ) && --start; ) p=p2+1;
    } while ( start > 1 );
    for (p2=str; p != end; ) {
        c = *p++;
	if ( !isprint(c) && !isspace(c) ) c=' ';
	*p2++=c;
	if ( c == '\n' )
	    if (!--len) break;
    }
    if ( len && st.st_size ) {
	size -= ( p2-str) ; 
	if (size > st.st_size)
	    size = st.st_size;
        if (fread(p2, size, 1, f) != 1) {
    	    fclose(f);
	    FREE(str);
    	    return 0;
        }
	st.st_size -= size;
	end = p2+size;
        for (; p2 != end; ) {
	    c = *p2;
	    if ( !isprint(c) && !isspace(c) ) *p2=' ';
	    p2++;
	    if ( c == '\n' )
	        if (!--len) break;
	}
	if ( st.st_size && len ) {
	    /* tried to read more than READ_MAX_FILE_SIZE */
	    fclose(f);
	    FREE(str);
	    return 0;
	}
    }
    *p2='\0';
    fclose(f);
    return str;
}


char *read_bytes(file,start,len)
    char *file;
    int start,len;
{
    struct stat st;

    char *str,*p;
    int size, f;

    if (len < 0)
	return 0;
    if(len > MAX_BYTE_TRANSFER)
	return 0;
    file = check_valid_path(file, current_object, 
				"read_bytes", 0);
    if (!file)
	return 0;
    f = open(file, O_RDONLY);
    if (f < 0)
	return 0;

    if (fstat(f, &st) == -1)
	fatal("Could not stat an open file.\n");
    size = st.st_size;
    if(start < 0) 
	start = size + start;

    if (start >= size) {
	close(f);
	return 0;
    }
    if ((start+len) > size) 
	len = (size - start);

    if ((size = lseek(f,start, 0)) < 0)
	return 0;

    str = DXALLOC(len + 1, 42, "read_bytes: str");

    size = read(f, str, len);

    close(f);

    if (size <= 0) {
	FREE(str);
	return 0;
    }

    /* We want to allow all characters to pass untouched!
    for (il=0;il<size;il++) 
	if (!isprint(str[il]) && !isspace(str[il]))
	    str[il] = ' ';

    str[il] = 0;
    */
    /*
     * The string has to end to '\0'!!!
     */
    str[size] = '\0';

    p = string_copy(str);
    FREE(str);

    return p;
}

int write_bytes(file,start,str)
    char *file, *str;
    int start;
{
    struct stat st;

    int size, f;

    file = check_valid_path(file, current_object, 
				"write_bytes", 1);

    if (!file)
	return 0;
    if(strlen(str) > MAX_BYTE_TRANSFER)
	return 0;
    f = open(file, O_WRONLY);
    if (f < 0)
	return 0;

    if (fstat(f, &st) == -1)
	fatal("Could not stat an open file.\n");
    size = st.st_size;
    if(start < 0) 
	start = size + start;

    if (start >= size) {
	close(f);
	return 0;
    }
    if ((start+strlen(str)) > size) {
	close(f);
	return 0;
    }

    if ((size = lseek(f,start, 0)) < 0) {
	close(f);
	return 0;
    }

    size = write(f, str, strlen(str));

    close(f);

    if (size <= 0) {
	return 0;
    }

    return 1;
}

int file_size(file)
    char *file;
{
    struct stat st;

    file = check_valid_path(file, current_object, "file_size", 0);
    if (!file)
	return -1;
    if (stat(file, &st) == -1)
	return -1;
    if (S_IFDIR & st.st_mode)
	return -2;
    return st.st_size;
}


/*
 * Check that a path to a file is valid for read or write.
 * This is done by functions in the master object.
 * The path is always treated as an absolute path, and is returned without
 * a leading '/'.
 * If the path was '/', then '.' is returned.
 * The returned string may or may not be residing inside the argument 'path',
 * so don't deallocate arg 'path' until the returned result is used no longer.
 * Otherwise, the returned path is temporarily allocated by apply(), which
 * means it will be dealocated at next apply().
 */
char *check_valid_path(path, call_object, call_fun, writeflg)
    char *path;
    struct object *call_object;
    char *call_fun;
    int writeflg;
{
    struct svalue *v;

    if (call_object == 0 || call_object->flags & O_DESTRUCTED) return 0;
    push_string(path, STRING_MALLOC);
    push_object(call_object);
    push_string(call_fun, STRING_CONSTANT);
    if (writeflg)
	v = apply_master_ob("valid_write", 3);
    else
	v = apply_master_ob("valid_read", 3);
    if (v && v->type == T_NUMBER && v->u.number == 0)
	return 0;
    if (path[0] == '/')
	path++;
    if (path[0] == '\0')
	path = ".";
    if (legal_path(path))
	return path;
    return 0;
}

int match_string(match, str)
    char *match, *str;
{
    int i;

 again:
    if (*str == '\0' && *match == '\0')
	return 1;
    switch(*match) {
    case '?':
	if (*str == '\0')
	    return 0;
	str++;
	match++;
	goto again;
    case '*':
	match++;
	if (*match == '\0')
	    return 1;
	for (i=0; str[i] != '\0'; i++)
	    if (match_string(match, str+i))
		return 1;
	return 0;
    case '\0':
	return 0;
    case '\\':
	match++;
	if (*match == '\0')
	    return 0;
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

int
isdir (path)
     char *path;
{
  struct stat stats;

  return stat (path, &stats) == 0 && S_ISDIR (stats.st_mode);
}

void
strip_trailing_slashes (path)
     char *path;
{
  int last;

  last = strlen (path) - 1;
  while (last > 0 && path[last] == '/')
    path[last--] = '\0';
}

struct stat to_stats, from_stats;

int
copy (from, to)
     char *from, *to;
{
  int ifd;
  int ofd;
  char buf[1024 * 8];
  int len;			/* Number of bytes read into `buf'. */
  
  if (!S_ISREG (from_stats.st_mode))
    {
      return 1;
    }
  
  if (unlink (to) && errno != ENOENT)
    {
      return 1;
    }

  ifd = open (from, O_RDONLY, 0);
  if (ifd < 0)
    {
      return errno;
    }
  ofd = open (to, O_WRONLY | O_CREAT | O_TRUNC, 0600);
  if (ofd < 0)
    {
      close (ifd);
      return 1;
    }
#ifndef FCHMOD_MISSING
  if (fchmod (ofd, from_stats.st_mode & 0777))
    {
      close (ifd);
      close (ofd);
      unlink (to);
      return 1;
    }
#endif
  
  while ((len = read (ifd, buf, sizeof (buf))) > 0)
    {
      int wrote = 0;
      char *bp = buf;
      
      do
	{
	  wrote = write (ofd, bp, len);
	  if (wrote < 0)
	    {
	      close (ifd);
	      close (ofd);
	      unlink (to);
	      return 1;
	    }
	  bp += wrote;
	  len -= wrote;
	} while (len > 0);
    }
  if (len < 0)
    {
      close (ifd);
      close (ofd);
      unlink (to);
      return 1;
    }

  if (close (ifd) < 0)
    {
      close (ofd);
      return 1;
    }
  if (close (ofd) < 0)
    {
      return 1;
    }
  
#ifdef FCHMOD_MISSING
  if (chmod (to, from_stats.st_mode & 0777))
    {
      return 1;
    }
#endif

  return 0;
}

/* Move FROM onto TO.  Handles cross-filesystem moves.
   If TO is a directory, FROM must be also.
   Return 0 if successful, 1 if an error occurred.  */

#ifdef F_RENAME
int
  do_move (from, to, flag)
char *from;
char *to;
int flag;
{
   if (lstat (from, &from_stats) != 0)
     {
	error ("%s: lstat failed\n", from);
	return 1;
     }
   
   if (lstat (to, &to_stats) == 0)
     {
#ifndef MSDOS
	if (from_stats.st_dev == to_stats.st_dev
	    && from_stats.st_ino == to_stats.st_ino)
#else
	  if (same_file(from,to))
#endif
	    {
	       error ("`%s' and `%s' are the same file", from, to);
	       return 1;
	    }
	
	if (S_ISDIR (to_stats.st_mode))
	  {
	     error ("%s: cannot overwrite directory", to);
	     return 1;
	  }
	
     }
   else if (errno != ENOENT)
     {
	error ("%s: unknown error\n", to);
	return 1;
     }
#ifdef SYSV
   if ((flag == F_RENAME) && isdir(from)) {
      char cmd_buf[100];
      sprintf(cmd_buf, "/usr/lib/mv_dir %s %s", from, to);
      return system(cmd_buf);
   } else
#endif /* SYSV */      
     if ((flag == F_RENAME) && (rename(from, to) == 0))
       return 0;
#ifdef F_LINK
     else if ((flag == F_LINK) && (link(from,to) == 0)) /* hard link */
       return 0;
#endif
   
   if (errno != EXDEV) {
      if (flag == F_RENAME)
	error ("cannot move `%s' to `%s'", from, to);
      else
	error ("cannot link `%s' to `%s'", from, to);
      return 1;
   }
   
   /* rename failed on cross-filesystem link.  Copy the file instead. */
   
   if (flag == F_RENAME) {
      if (copy (from, to))
	return 1;
      if (unlink (from)) {
	 error ("cannot remove `%s'", from);
	 return 1;
      }
   }
#ifdef F_LINK
   else if (flag == F_LINK) {
      if (symlink(from, to) == 0) /* symbolic link */
	return 0;
   }
#endif
   return 0;
}
#endif
    
/*
 * do_rename is used by the efun rename. It is basically a combination
 * of the unix system call rename and the unix command mv.
 */

#ifdef F_RENAME
int
do_rename(fr, t, flag)
    char *fr, *t;
	int flag;
{
   char *from, *to, tbuf[3];
   
   /* important that the same write access checks are done for link()
      as are done for rename().  Otherwise all kinds of security problems
      would arise (e.g. creating links to files in protected directories
      and then modifying the protected file by modifying the linked file).
      The idea is prevent linking to a file unless the person doing the
      linking has permission to move the file.
      */
   from = check_valid_path(fr, current_object, "do_rename", 1);
   if(!from)
     return 1;
   to = check_valid_path(t, current_object, "do_rename", 1);
   if(!to)
     return 1;
   if(!strlen(to) && !strcmp(t, "/")) {
      to = tbuf;
      sprintf(to, "./");
   }
   strip_trailing_slashes (from);
   if (isdir (to))
     {
	/* Target is a directory; build full target filename. */
	char *cp;
	char newto[MAX_FNAME_SIZE + MAX_PATH_LEN + 2];
	
	cp = strrchr (from, '/');
	if (cp)
	  cp++;
	else
	  cp = from;
	
	sprintf (newto, "%s/%s", to, cp);
	return do_move (from, newto, flag);
     }
   else
     return do_move (from, to, flag);
}
#endif /* F_RENAME */

int copy_file (from, to)
     char *from, *to;
{
   char buf[128];
   int from_fd, to_fd;
   int num_read, num_written;
   char *write_ptr;
   
   from = check_valid_path (from, current_object, "move_file", 0);
   to = check_valid_path (to, current_object, "move_file", 1);
   if (from == 0)
     return -1;
   if (to == 0)
     return -2;
   
   from_fd = open (from, O_RDONLY, 0777);
   if (from_fd < 0)
     return (-1);
   
   if (isdir (to))
     {
	/* Target is a directory; build full target filename. */
	char *cp;
	char newto[MAX_FNAME_SIZE + MAX_PATH_LEN + 2];
	
	cp = strrchr (from, '/');
	if (cp)
	  cp++;
	else
	  cp = from;
	
	sprintf (newto, "%s/%s", to, cp);
	close(from_fd);
	return copy_file (from, newto);
     }
   to_fd = open (to, O_WRONLY|O_CREAT|O_TRUNC, 0777);
   if (to_fd < 0)
     {
	close (from_fd);
	return (-2);
     }
   
   while ((num_read = read (from_fd, buf, 128)) != 0) {
      if (num_read < 0) {
	 perror ("error in read in copy_file()");
	 close (from_fd);
	 close (to_fd);
	 return (-3);
      }
      
      write_ptr = buf;
      while (write_ptr != (buf + num_read)) {
	 num_written = write (to_fd, write_ptr, num_read);
	 if (num_written < 0) {
	    perror ("error in write in copy_file()");
	    close (from_fd);
	    close (to_fd);
	    return (-3);
	 }
	 write_ptr += num_written;
      }
   }
   close (from_fd);
   close (to_fd);
   return 1;
}

void
dump_file_descriptors()
{
    int i;
    dev_t dev;
    struct stat stbuf;

    add_message("Fd  Device Number  Inode   Mode    Uid    Gid      Size\n");
    add_message("--  -------------  -----  ------  -----  -----  ----------\n");

    for (i = 0; i < FD_SETSIZE; i++) {
	/* bug in NeXT OS 2.1, st_mode == 0 for sockets */
	if (fstat(i, &stbuf) == -1)
	    continue;

	if (S_ISCHR(stbuf.st_mode) || S_ISBLK(stbuf.st_mode))
	    dev = stbuf.st_rdev;
	else
	    dev = stbuf.st_dev;

	add_message("%2d", i);
	add_message("%6x", major(dev));
	add_message("%7x", minor(dev));
	add_message("%9d", stbuf.st_ino);
	add_message("  ");

	switch (stbuf.st_mode & S_IFMT) {

	case S_IFDIR:
	    add_message("d");
	    break;
	case S_IFCHR:
	    add_message("c");
	    break;
	case S_IFBLK:
	    add_message("b");
	    break;
	case S_IFREG:
	    add_message("f");
	    break;
#ifndef apollo
	case S_IFIFO:
	    add_message("p");
	    break;
#endif
	case S_IFLNK:
	    add_message("l");
	    break;
#ifdef S_IFSOCK
	case S_IFSOCK:
	    add_message("s");
	    break; 
#endif
	default:
	    add_message("?");
	    break;
	}

	add_message("%5o", stbuf.st_mode & ~S_IFMT);
	add_message("%7d", stbuf.st_uid);
	add_message("%7d", stbuf.st_gid);
	add_message("%12d", stbuf.st_size);
	add_message("\n");
    }
}
