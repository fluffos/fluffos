#if !defined(STD_INCL_H) && !defined(_FUNC_SPEC_)
#define STD_INCL_H

#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <math.h>
#include <ctype.h>
#include <errno.h>
#include <setjmp.h>

#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif
#ifdef HAVE_FCNTL_H
#  include <fcntl.h>
#endif
#ifdef HAVE_TIME_H
#  include <time.h>
#endif
#ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
#endif
#ifdef HAVE_SYS_WAIT_H
#  include <sys/wait.h>
#endif
#ifdef HAVE_SYS_CRYPT_H
#  include <sys/crypt.h>
#endif
#ifdef HAVE_CRYPT_H
#  include <crypt.h>
#endif
#ifdef HAVE_VALUES_H
#  include <values.h>
#endif
#ifdef HAVE_LIMITS_H
#  include <limits.h>
#endif
#ifdef HAVE_INTTYPES_H
#  include <inttypes.h>
#endif
#ifdef HAVE_STDINT_H
#  include <stdint.h>
#endif
#ifdef HAVE_FLOAT_H
#  include <float.h>
#endif
#ifdef HAVE_MALLOC_H
#  include <malloc.h>
#endif
#ifdef HAVE_LOCALE_H
#  include <locale.h>
#endif
#ifdef HAVE_DLFCN_H
#  include <dlfcn.h>
#endif
#ifdef HAVE_NETINET_IN_H
#  include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#ifdef HAVE_ALLOCA_H
#  include <alloca.h>
#endif
#ifdef HAVE_EXECINFO_H
#include <execinfo.h>
#endif
#ifdef HAVE_SYS_RUSAGE_H
#include <sys/rusage.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#endif /* STD_INCL_H */
