#ifndef MACROS_H
#define MACROS_H

#ifndef _FUNC_SPEC_

/*
 * Some useful macros...
 */
#ifdef BUFSIZ
#define PROT_STDIO(x) PROT(x)
#else				/* BUFSIZ */
#define PROT_STDIO(x) ()
#endif				/* BUFSIZ */

/* ANSI/K&R compatibility stuff;
 *
 * The correct way to prototype a function now is:
 *
 * foobar PROT((int, char *));
 *
 * foobar P2(int, x, char *, y) { ... }
 */
/* xlc can't handle an ANSI protoype followed by a K&R def, and varargs
 * functions must be done K&R (b/c va_dcl is K&R style) so don't prototype
 * vararg function arguments under AIX
 */
#ifdef __STDC__
#define PROT(x) x
#define P1(t1, v1) (t1 v1)
#define P2(t1, v1, t2, v2) (t1 v1, t2 v2)
#define P3(t1, v1, t2, v2, t3, v3) (t1 v1, t2 v2, t3 v3)
#define P4(t1, v1, t2, v2, t3, v3, t4, v4) (t1 v1, t2 v2, t3 v3, t4 v4)
#define P5(t1, v1, t2, v2, t3, v3, t4, v4, t5, v5) (t1 v1, t2 v2, t3 v3, t4 v4, t5 v5)
#define P6(t1, v1, t2, v2, t3, v3, t4, v4, t5, v5, t6, v6) (t1 v1, t2 v2, t3 v3, t4 v4, t5 v5, t6 v6)
#define P7(t1, v1, t2, v2, t3, v3, t4, v4, t5, v5, t6, v6, t7, v7) (t1 v1, t2 v2, t3 v3, t4 v4, t5 v5, t6 v6, t7 v7)
#define VOLATILE volatile
#define SIGNED signed
#else				/* __STDC__ */
#define PROT(x) ()
#define P1(t1, v1) (v1) t1 v1;
#define P2(t1, v1, t2, v2) (v1, v2) t1 v1; t2 v2;
#define P3(t1, v1, t2, v2, t3, v3) (v1, v2, v3) t1 v1; t2 v2; t3 v3;
#define P4(t1, v1, t2, v2, t3, v3, t4, v4) (v1, v2, v3, v4) t1 v1; t2 v2; t3 v3; t4 v4;
#define P5(t1, v1, t2, v2, t3, v3, t4, v4, t5, v5) (v1, v2, v3, v4, v5) t1 v1; t2 v2; t3 v3; t4 v4; t5 v5;
#define P6(t1, v1, t2, v2, t3, v3, t4, v4, t5, v5, t6, v6) (v1, v2, v3, v4, v5, v6) t1 v1; t2 v2; t3 v3; t4 v4; t5 v5; t6 v6;
#define P7(t1, v1, t2, v2, t3, v3, t4, v4, t5, v5, t6, v6, t7, v7) (v1, v2, v3, v4, v5, v6, v7) t1 v1; t2 v2; t3 v3; t4 v4; t5 v5; t6 v6; t7 v7;
#define VOLATILE
#define SIGNED
#endif				/* __STDC__ */

/*
 * varargs can't be prototyped portably ... *sigh*
 */
#if defined(LATTICE)
#define PROTVARGS(x) (char *, ...)
#define PVARGS(v) (char *v, ...)
#else
#define PROTVARGS(x) ()
#define PVARGS(v) (v) va_dcl
#endif
#endif

#ifndef INLINE
#  if defined(__GNUC__) && !defined(__STRICT_ANSI__) && !defined(lint)
#    define INLINE inline
#  else
#    ifdef LATTICE
#      define INLINE __inline
#    else
#      define INLINE
#    endif
#  endif
#endif

/*
   define MALLOC, FREE, REALLOC, and CALLOC depending upon what malloc
   package is is used.  This technique is used because overlaying system malloc
   with another function also named malloc doesn't work on most machines
   that have shared libraries.  It will also let us keep malloc stats even
   when system malloc is used.
*/

#ifdef WRAPPEDMALLOC
#define MALLOC(x)  wrappedmalloc(x)
#define FREE(x)    wrappedfree(x)
#define REALLOC(x,y) wrappedrealloc(x,y)
#define CALLOC(x,y)   wrappedcalloc(x,y)
#endif

#ifdef SYSMALLOC
#define MALLOC(x)  malloc(x)
#define FREE(x)    free(x)
#define REALLOC(x,y) realloc(x,y)
#define CALLOC(x,y)   calloc(x,y)
#endif

#ifdef DEBUGMALLOC
#define MALLOC(x)  debugmalloc(x,0,(char *)0)
#define DMALLOC(x,tag,desc)  debugmalloc(x,tag,desc)
#define XALLOC(x) debugmalloc(x,0,(char *)0)
#define DXALLOC(x,tag,desc) debugmalloc(x,tag,desc)
#define FREE(x)    debugfree(x)
#define REALLOC(x,y) debugrealloc(x,y,0,(char *)0)
#define DREALLOC(x,y,tag,desc) debugrealloc(x,y,tag,desc)
#define CALLOC(x,y)   debugcalloc(x,y,0,(char *)0)
#define DCALLOC(x,y,tag,desc)   debugcalloc(x,y,tag,desc)
#else
#define XALLOC(x) xalloc(x)
#define DXALLOC(x,tag,desc) xalloc(x)
#define DMALLOC(x,tag,desc)  MALLOC(x)
#define DREALLOC(x,y,tag,desc) REALLOC(x,y)
#define DCALLOC(x,y,tag,desc)   CALLOC(x,y)
#endif

#ifndef MALLOC
#define MALLOC(x) puts("You need to specify a malloc package in options.h")
#define FREE(x) puts("You need to specify a malloc package in options.h")
#define REALLOC(x) puts("You need to specify a malloc package in options.h")
#define CALLOC(x) puts("You need to specify a malloc package in options.h")
#endif

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

#ifndef _FUNC_SPEC_
char *string_copy PROT((char *));
char *xalloc PROT((int));
#endif
#endif
