#ifndef MACROS_H
#define MACROS_H

#ifdef __STDC__
#define ANSI_STRING_HACK(x) #x
#define DONT_ASK_WHY(x) (x)
#define WHERE (__FILE__ ":" DONT_ASK_WHY(x))
#else
#define WHERE "non-ansi compilers are a pain"
#endif

/*
 * Some useful macros...
 */
#ifdef BUFSIZ
#  define PROT_STDIO(x) (x ()
#endif                          /* BUFSIZ */

/* ANSI/K&R compatibility stuff;
 *
 * The correct way to prototype a function now is:
 *
 * foobar (int, char *);
 *
 * foobar (int x, char * y) { ... }
 */
/* xlc can't handle an ANSI protoype followed by a K&R def, and varargs
 * functions must be done K&R (b/c va_dcl is K&R style) so don't prototype
 * vararg function arguments under AIX
 */
#if defined(__STDC__) || defined(WIN32)
#  define VOLATILE volatile
#  define SIGNED signed
#else                           /* __STDC__ */
#  define VOLATILE
#  define SIGNED
#endif                          /* __STDC__ */

/* do things both ways ... */
#define V_START(vlist, last_arg) va_start(vlist, last_arg)
#define V_VAR(type, var, vlist)
#define SAFE(x) do { x } while (0)
 

/*
   Define for MALLOC, FREE, REALLOC, and CALLOC depend upon what malloc
   package and optional wrapper is used.  This technique is used because
   overlaying system malloc with another function also named malloc doesn't
   work on most mahines that have shared libraries.  It will also let
   us keep malloc stats even when system malloc is used.

   Please refer to options.h for selecting malloc package and wrapper.
*/
#if (defined(SYSMALLOC) + defined(SMALLOC) + defined(BSDMALLOC) + defined(MMALLOC)) > 1
!Only one malloc package should be defined
#endif

#if (defined(WRAPPEDMALLOC) + defined(DEBUGMALLOC)) > 1
!Only one wrapper (at most) should be defined
#endif

#if defined(DO_MSTATS) && defined(SYSMALLOC)
!'DO_MSTATS' not available with 'SYSMALLOC'
#endif

#if defined (WRAPPEDMALLOC) && !defined(IN_MALLOC_WRAPPER)

#  define MALLOC(x)               wrappedmalloc(x)
#  define FREE(x)                 wrappedfree(x)
#  define REALLOC(x, y)           wrappedrealloc(x, y)
#  define CALLOC(x, y)            wrappedcalloc(x, y)
#  define DXALLOC(x, t, d)        xalloc(x)
#  define DMALLOC(x, t, d)        MALLOC(x)
#  define DREALLOC(x, y, t, d)    REALLOC(x,y)
#  define DCALLOC(x, y, t, d)     CALLOC(x,y)

#else

#  if defined(DEBUGMALLOC) && !defined(IN_MALLOC_WRAPPER)

#    define MALLOC(x)               debugmalloc(x, 0, (char *)0)
#    define DMALLOC(x, t, d)        debugmalloc(x, t, d)
#    define DXALLOC(x, t, d)        debugmalloc(x, t, d)
#    define FREE(x)                 debugfree(x)
#    define REALLOC(x,y)            debugrealloc(x,y,0,(char *)0)
#    define DREALLOC(x,y,tag,desc)  debugrealloc(x,y,tag,desc)
#    define CALLOC(x,y)             debugcalloc(x,y,0,(char *)0)
#    define DCALLOC(x,y,tag,desc)   debugcalloc(x,y,tag,desc)

#  else

#    include "my_malloc.h"

#  endif
#endif

#if !defined(MALLOC) && !defined(EDIT_SOURCE)
!You need to specify a malloc package in local_options/options.h
#endif

#define ALLOCATE(type, tag, desc) ((type *)DXALLOC(sizeof(type), tag, desc))
#define CALLOCATE(num, type, tag, desc) ((type *)DXALLOC(sizeof(type[1]) * (num), tag, desc))
#define RESIZE(ptr, num, type, tag, desc) ((type *)DREALLOC((void *)ptr, sizeof(type) * (num), tag, desc))

#ifdef DEBUG
#  define IF_DEBUG(x) x
#  define DEBUG_CHECK(x, y) if (x) fatal(y)
#  define DEBUG_CHECK1(x, y, a) if (x) fatal(y, a)
#  define DEBUG_CHECK2(x, y, a, b) if (x) fatal(y, a, b)
#else
#  define IF_DEBUG(x) 
#  define DEBUG_CHECK(x, y)
#  define DEBUG_CHECK1(x, y, a)
#  define DEBUG_CHECK2(x, y, a, b)
#endif

#if !defined(EDIT_SOURCE) && !defined(_FUNC_SPEC_)
#define COPY2(x, y)      ((char *)(x))[0] = ((char *)(y))[0]; \
                         ((char *)(x))[1] = ((char *)(y))[1]
#define LOAD2(x, y)      ((char *)&(x))[0] = *y++; \
                         ((char *)&(x))[1] = *y++
#define STORE2(x, y)     *x++ = ((char *)(&(y)))[0]; \
                         *x++ = ((char *)(&(y)))[1]

#define COPY4(x, y)      ((char *)(x))[0] = ((char *)(y))[0]; \
                         ((char *)(x))[1] = ((char *)(y))[1]; \
                         ((char *)(x))[2] = ((char *)(y))[2]; \
                         ((char *)(x))[3] = ((char *)(y))[3]
#define LOAD4(x, y)      ((char *)&(x))[0] = *y++; \
                         ((char *)&(x))[1] = *y++; \
                         ((char *)&(x))[2] = *y++; \
                         ((char *)&(x))[3] = *y++
#define STORE4(x, y)     *x++ = ((char *)(&(y)))[0]; \
                         *x++ = ((char *)(&(y)))[1]; \
                         *x++ = ((char *)(&(y)))[2]; \
                         *x++ = ((char *)(&(y)))[3]

#define COPY8(x, y)      ((char *)(x))[0] = ((char *)(y))[0]; \
                         ((char *)(x))[1] = ((char *)(y))[1]; \
                         ((char *)(x))[2] = ((char *)(y))[2]; \
                         ((char *)(x))[3] = ((char *)(y))[3]; \
                         ((char *)(x))[4] = ((char *)(y))[4]; \
                         ((char *)(x))[5] = ((char *)(y))[5]; \
                         ((char *)(x))[6] = ((char *)(y))[6]; \
                         ((char *)(x))[7] = ((char *)(y))[7]
#define LOAD8(x, y)      ((char *)&(x))[0] = *y++; \
                         ((char *)&(x))[1] = *y++; \
                         ((char *)&(x))[2] = *y++; \
                         ((char *)&(x))[3] = *y++; \
                         ((char *)&(x))[4] = *y++; \
                         ((char *)&(x))[5] = *y++; \
                         ((char *)&(x))[6] = *y++; \
                         ((char *)&(x))[7] = *y++;
#define STORE8(x, y)     *x++ = ((char *)(&(y)))[0]; \
                         *x++ = ((char *)(&(y)))[1]; \
                         *x++ = ((char *)(&(y)))[2]; \
                         *x++ = ((char *)(&(y)))[3]; \
                         *x++ = ((char *)(&(y)))[4]; \
                         *x++ = ((char *)(&(y)))[5]; \
                         *x++ = ((char *)(&(y)))[6]; \
                         *x++ = ((char *)(&(y)))[7]

#if SIZEOF_SHORT == 2
#define COPY_SHORT(x, y) COPY2(x,y)
#define LOAD_SHORT(x, y) LOAD2(x,y)
#define STORE_SHORT(x, y) STORE2(x,y)
#else
!shorts of size other than 2 not implemented
#endif

#if SIZEOF_INT == 4
#define INT_32 int
#endif

#if SIZEOF_LONG == 4
#define COPY_INT(x, y) COPY4(x,y)
#define LOAD_INT(x, y) LOAD4(x,y)
#define STORE_INT(x, y) STORE4(x,y)
#else
#if SIZEOF_LONG == 8
#define COPY_INT(x, y) COPY8(x,y)
#define LOAD_INT(x, y) LOAD8(x,y)
#define STORE_INT(x, y) STORE8(x,y)
#else
!ints of size other than 4 not implemented
#endif
#endif

#if SIZEOF_FLOAT == 8
#define COPY_FLOAT(x, y) COPY8(x,y)
#define LOAD_FLOAT(x, y) LOAD8(x,y)
#define STORE_FLOAT(x, y) STORE8(x,y)
#else
!floats of size other than 8 not implemented
#endif

#if SIZEOF_PTR == 4
#  define COPY_PTR(x, y) COPY4(x,y)
#  define LOAD_PTR(x, y) LOAD4(x,y)
#  define STORE_PTR(x, y) STORE4(x,y)

#  define POINTER_INT int
#  define INS_POINTER ins_int
#else
#  if SIZEOF_PTR == 8
#    define COPY_PTR(x, y) COPY8(x,y)
#    define LOAD_PTR(x, y) LOAD8(x,y)
#    define STORE_PTR(x, y) STORE8(x,y)

#    define POINTER_INT long
#    define INS_POINTER ins_long
#  else
!pointers of size other than 4 or 8 not implemented
#  endif
#endif
#endif

#ifndef _FUNC_SPEC_
   char *xalloc (int);
#  ifdef DEBUGMALLOC
      char *int_string_copy (const char * const, char *);
      char *int_string_unlink (const char *, char *);
      char *int_new_string (int, char *);
      char *int_alloc_cstring (const char *, char *);
#  else
      char *int_string_copy (const char * const);
      char *int_string_unlink (const char *);
      char *int_new_string (int);
      char *int_alloc_cstring (const char *);
#  endif
#endif

#ifdef DEBUGMALLOC
#   define string_copy(x,y) int_string_copy(x, y)
#   define string_unlink(x,y) int_string_unlink(x, y)
#   define new_string(x,y) int_new_string(x, y)
#   define alloc_cstring(x,y) int_alloc_cstring(x, y)
#else
#   define string_copy(x,y) int_string_copy(x)
#   define string_unlink(x,y) int_string_unlink(x)
#   define new_string(x,y) int_new_string(x)
#   define alloc_cstring(x,y) int_alloc_cstring(x)
#endif

#ifndef INLINE
#define INLINE
#endif

#ifndef INLINE_STATIC
#  define INLINE_STATIC static INLINE
#endif

/* The ANSI versions must take an unsigned char, and must work on EOF.  These
 * versions take a (possibly signed) char, and do not work correctly on EOF.
 *
 * Note that calling isdigit(x) where x is a signed char with x < 0 (i.e.
 * high bit set) invokes undefined behavior.
 */
#define uisdigit(x) isdigit((unsigned char)x)
#define uislower(x) islower((unsigned char)x)
#define uisspace(x) isspace((unsigned char)x)
#define uisalnum(x) isalnum((unsigned char)x)
#define uisupper(x) isupper((unsigned char)x)
#define uisalpha(x) isalpha((unsigned char)x)
#define uisxdigit(x) isxdigit((unsigned char)x)
#define uisascii(x) isascii((unsigned char)x)
#define uisprint(x) isprint((unsigned char)x)

#endif
