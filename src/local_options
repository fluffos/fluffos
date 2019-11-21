/*
 * local_options.h: compile-time configuration of the driver
 */

/****************************************************************************
 * EVERY time you change ANYTHING in this file, RECOMPILE from scratch.     *
 * (type "make clean" then "make" on a UNIX system) Failure to do so may    *
 * cause the driver to behave oddly.                                        *
 ****************************************************************************/

/* NOTES: Many of the configurable options are now set in the runtime config.
 * 
 * This file only contain a list of default settings, for quick diffing .
 * See detailed explaination from 'local_options.README'
 */

#ifndef _LOCAL_OPTIONS_H_
#define _LOCAL_OPTIONS_H_

/****************************************************************************
 *                              COMPAT                                      *
 ****************************************************************************/
#undef NO_ADD_ACTION
#undef NO_SNOOP
#undef NO_ENVIRONMENT
#undef NO_WIZARDS
#undef NO_LIGHT
#define OLD_ED
#undef ED_INDENT_CASE
#define ED_INDENT_SPACES 4
#undef ED_USE_TABS
#define ED_TAB_WIDTH 8
#undef RECEIVE_ED
#define RESTRICTED_ED
#define SENSIBLE_MODIFIERS

/****************************************************************************
 *                           MISCELLANEOUS                                  *
 ****************************************************************************/
#define CUSTOM_CRYPT
#undef COMPAT_32
#define DEFAULT_PRAGMAS PRAGMA_WARNINGS + PRAGMA_SAVE_TYPES + PRAGMA_ERROR_CONTEXT + PRAGMA_OPTIMIZE
#define SAVE_EXTENSION ".o"
#undef PRIVS
#undef NO_SHADOWS
#undef USE_ICONV
#undef IPV6
#undef DTRACE

/****************************************************************************
 *                              PACKAGES                                    *
 ****************************************************************************/
#define PACKAGE_OPS
#define PACKAGE_CORE

#define PACKAGE_CONTRIB
#define PACKAGE_DEVELOP
#define PACKAGE_MATH
#define PACKAGE_MATRIX
#define PACKAGE_MUDLIB_STATS
#define PACKAGE_SOCKETS
#undef PACKAGE_PARSER
#define PACKAGE_EXTERNAL
#undef PACKAGE_DB
#ifdef PACKAGE_DB
#define USE_MYSQL 1
#undef USE_MSQL
#undef USE_POSTGRES
#undef USE_SQLITE3
#define DEFAULT_DB  USE_MYSQL
#endif
#define PACKAGE_ASYNC
#define PACKAGE_SHA1
#undef PACKAGE_CRYPTO
#define PACKAGE_TRIM
#define PACKAGE_PCRE
#undef PACKAGE_DWLIB
#define PACKAGE_UIDS
#ifdef PACKAGE_UIDS
#define AUTO_SETEUID
#undef AUTO_TRUST_BACKBONE
#endif
#define PACKAGE_COMPRESS
#define SAVE_GZ_EXTENSION ".o.gz"
#undef PACKAGE_THREAD

#endif /* _LOCAL_OPTIONS_H_ */
