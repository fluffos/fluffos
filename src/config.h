/*
   config.h: do not change anything in this file.  The user definable
   options have been moved into the options.h file.
*/

#include "port.h"
#include "options.h"

/*
 * runtime config strings.  change these values in the runtime configuration
 * file (config.example)
 */

#define MUD_NAME                get_config_str(0)
#define MUD_LIB                 get_config_str(1)
#define BIN_DIR                 get_config_str(2)
#define SWAP_FILE               get_config_str(3)
#define CONFIG_DIR              get_config_str(4)
#define LOG_DIR                 get_config_str(5)
#define MASTER_FILE             get_config_str(6)
#define ACCESS_FILE             get_config_str(7)
#define ACCESS_LOG              get_config_str(8)
#define INCLUDE_DIRS            get_config_str(9)
#define SIMUL_EFUN              get_config_str(10)
#define ADDR_SERVER_IP          get_config_str(11)
#define DEFAULT_ERROR_MESSAGE   get_config_str(12)
#define DEFAULT_FAIL_MESSAGE   get_config_str(13)
#ifdef BINARIES
#define SAVE_BINARIES           get_config_str(14)
#endif

/*
 * runtime config ints
 */

#define TIME_TO_CLEAN_UP        get_config_int(0)
#define TIME_TO_SWAP            get_config_int(1)
#define TIME_TO_RESET	        get_config_int(2)
#define	ALLOWED_ED_CMDS         get_config_int(3)
#define MAX_BITS                get_config_int(7)
#define MAX_COST                get_config_int(9)
#define MAX_ARRAY_SIZE          get_config_int(10)
#define MAX_MAPPING_SIZE        get_config_int(11)
#define MAX_LOG_SIZE            get_config_int(13)
#define READ_FILE_MAX_SIZE      get_config_int(14)
#define MAX_STRING_LENGTH       get_config_int(15)
#define ADDR_SERVER_PORT        get_config_int(16)
#define MAX_BYTE_TRANSFER       get_config_int(17)
#define PORTNO                  get_config_int(18)
#define RESERVED_SIZE           get_config_int(19)
#define	HTABLE_SIZE             get_config_int(21)
#define OTABLE_SIZE             get_config_int(22)
#define INHERIT_CHAIN_SIZE      get_config_int(23)

#ifdef USE_POSIX_SIGNALS
#define sigblock(m) port_sigblock(m)
#define sigmask(s)  port_sigmask(s)
#define signal(s,f) port_signal(s,f)
#define sigsetmask(m)   port_sigsetmask(m)
#endif

#define SETJMP(x) setjmp(x)
#define LONGJMP(x,y) longjmp(x,y)

#ifndef HAS_UALARM
#define SYSV_HEARTBEAT_INTERVAL  ((HEARTBEAT_INTERVAL+999999)/1000000)
#endif

#ifndef INLINE
#if defined(__GNUC__) && !defined(__STRICT_ANSI__) && !defined(lint)
#define INLINE inline
#else
#define INLINE
#endif
#endif

#ifdef HAS_UNSIGNED_CHAR
#define EXTRACT_UCHAR(p) (*(unsigned char *)p)
#else
#define EXTRACT_UCHAR(p) (*p < 0 ? *p + 0x100 : *p)
#endif /* HAS_UNSIGNED_CHAR */

#define APPLY_CACHE_SIZE (1 << APPLY_CACHE_BITS)

#define NUM_CONSTS 5

#define I(x) (x)

/*
   define MALLOC, FREE, REALLOC, and CALLOC depending upon what malloc
   package is is used.  This technique is used because overlaying system malloc
   with another function also named malloc doesn't work on most machines
   that have shared libraries.  It will also let us keep malloc stats even 
   when system malloc is used.
*/

#ifdef GMALLOC
#define MALLOC(x)  gmalloc(x)
#define FREE(x)    gfree(x)
#define REALLOC(x,y) grealloc(x,y)
#define CALLOC(x,y)   gcalloc(x,y)
#endif

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
