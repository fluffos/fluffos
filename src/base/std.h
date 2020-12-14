#ifndef STD_H
#define STD_H

// IWYU pragma: begin_exports

/* This stuff should be included EVERYWHERE */
#ifndef _REENTRANT
#define _REENTRANT
#endif
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

/* the definition of ARCH */
#include "base/internal/arch.h"

/* CYGWIN build is working. - sunyc@2013-11-12 */
#ifdef __CYGWIN__
#undef WINNT
#undef WIN95
#undef WIN98
#undef WINSOCK
#undef WIN32
/* CYGWIN needs this to proper setup POSIX env. */
#undef __STRICT_ANSI__
#endif

/* all options and configuration */
#include "base/internal/options_incl.h"

// These shall be moved.
#define EXTRACT_UCHAR(p) (*(unsigned char *)(p))
#define READ_USHORT(p) (*(unsigned short *)(p++))

#define COPY2(x, y)                    \
  ((char *)(x))[0] = ((char *)(y))[0]; \
  ((char *)(x))[1] = ((char *)(y))[1]
#define LOAD2(x, y)         \
  ((char *)&(x))[0] = *y++; \
  ((char *)&(x))[1] = *y++
#define STORE2(x, y)          \
  *x++ = ((char *)(&(y)))[0]; \
  *x++ = ((char *)(&(y)))[1]

#define COPY4(x, y)                    \
  ((char *)(x))[0] = ((char *)(y))[0]; \
  ((char *)(x))[1] = ((char *)(y))[1]; \
  ((char *)(x))[2] = ((char *)(y))[2]; \
  ((char *)(x))[3] = ((char *)(y))[3]
#define LOAD4(x, y)         \
  ((char *)&(x))[0] = *y++; \
  ((char *)&(x))[1] = *y++; \
  ((char *)&(x))[2] = *y++; \
  ((char *)&(x))[3] = *y++
#define STORE4(x, y)          \
  *x++ = ((char *)(&(y)))[0]; \
  *x++ = ((char *)(&(y)))[1]; \
  *x++ = ((char *)(&(y)))[2]; \
  *x++ = ((char *)(&(y)))[3]

#define COPY8(x, y)                    \
  ((char *)(x))[0] = ((char *)(y))[0]; \
  ((char *)(x))[1] = ((char *)(y))[1]; \
  ((char *)(x))[2] = ((char *)(y))[2]; \
  ((char *)(x))[3] = ((char *)(y))[3]; \
  ((char *)(x))[4] = ((char *)(y))[4]; \
  ((char *)(x))[5] = ((char *)(y))[5]; \
  ((char *)(x))[6] = ((char *)(y))[6]; \
  ((char *)(x))[7] = ((char *)(y))[7]
#define LOAD8(x, y)         \
  ((char *)&(x))[0] = *y++; \
  ((char *)&(x))[1] = *y++; \
  ((char *)&(x))[2] = *y++; \
  ((char *)&(x))[3] = *y++; \
  ((char *)&(x))[4] = *y++; \
  ((char *)&(x))[5] = *y++; \
  ((char *)&(x))[6] = *y++; \
  ((char *)&(x))[7] = *y++;
#define STORE8(x, y)          \
  *x++ = ((char *)(&(y)))[0]; \
  *x++ = ((char *)(&(y)))[1]; \
  *x++ = ((char *)(&(y)))[2]; \
  *x++ = ((char *)(&(y)))[3]; \
  *x++ = ((char *)(&(y)))[4]; \
  *x++ = ((char *)(&(y)))[5]; \
  *x++ = ((char *)(&(y)))[6]; \
  *x++ = ((char *)(&(y)))[7]

#define COPY_SHORT(x, y) COPY2(x, y)
#define LOAD_SHORT(x, y) LOAD2(x, y)
#define STORE_SHORT(x, y) STORE2(x, y)

/* LPC INT */
#define COPY_INT(x, y) COPY8(x, y)
#define LOAD_INT(x, y) LOAD8(x, y)
#define STORE_INT(x, y) STORE8(x, y)

/* LPC FLOAT */
#define COPY_FLOAT(x, y) COPY8(x, y)
#define LOAD_FLOAT(x, y) LOAD8(x, y)
#define STORE_FLOAT(x, y) STORE8(x, y)

#include <cstdint>
#if UINTPTR_MAX == UINT32_MAX
#define COPY_PTR(x, y) COPY4(x, y)
#define LOAD_PTR(x, y) LOAD4(x, y)
#define STORE_PTR(x, y) STORE4(x, y)
#elif UINTPTR_MAX == UINT64_MAX
#define COPY_PTR(x, y) COPY8(x, y)
#define LOAD_PTR(x, y) LOAD8(x, y)
#define STORE_PTR(x, y) STORE8(x, y)
#else
#error pointers of size other than 4 or 8 not implemented
#endif

#define POINTER_INT intptr_t
#define INS_POINTER ins_pointer

/* The ANSI versions must take an unsigned char, and must work on EOF.  These
 * versions take a (possibly signed) char, and do not work correctly on EOF.
 *
 * Note that calling isdigit(x) where x is a signed char with x < 0 (i.e.
 * high bit set) invokes undefined behavior.
 */
#include <ctype.h>
#define uisdigit(x) isdigit((unsigned char)x)
#define uislower(x) islower((unsigned char)x)
#define uisspace(x) isspace((unsigned char)x)
#define uisalnum(x) isalnum((unsigned char)x)
#define uisupper(x) isupper((unsigned char)x)
#define uisalpha(x) isalpha((unsigned char)x)
#define uisxdigit(x) isxdigit((unsigned char)x)
#define uisascii(x) isascii((unsigned char)x)
#define uisprint(x) isprint((unsigned char)x)

#define __STRINGIFY(x) #x
#define __TOSTRING(x) __STRINGIFY(x)
#define __CURRENT_FILE_LINE__ __FILE__ ":" __TOSTRING(__LINE__)

/* Compare two number */
#define COMPARE_NUMS(x, y) (((x) > (y) ? 1 : ((x) < (y) ? -1 : 0)))

#include "base/internal/port.h"  // must be first

#include "base/internal/debugmalloc.h"
#include "base/internal/md.h"

#include "base/internal/external_port.h"

#include "base/internal/file.h"

#include "base/internal/hash.h"

#include "base/internal/log.h"

#include "base/internal/outbuf.h"

#include "base/internal/rusage.h"

#include "base/internal/rc.h"

#include "base/internal/stats.h"

#include "base/internal/stralloc.h"

#include "base/internal/strput.h"

#include "base/internal/strutils.h"

#include "thirdparty/scope_guard/scope_guard.hpp"

// IWYU pragma: end_exports
#endif  // STD_H
