#ifndef PACKAGES_DB_H
#define PACKAGES_DB_H

#ifdef PACKAGE_DB

#ifdef USE_MSQL
/* MSQL v2 requires this so that it knows the right prototypes */
#define _OS_UNIX

#ifdef INCL_LOCAL_MSQL_H
#include "/usr/local/include/msql.h"
#endif
#ifdef INCL_LOCAL_MSQL_MSQL_H
#include "/usr/local/msql/include/msql.h"
#endif
#ifdef INCL_LOCAL_MINERVA_MSQL_H
#include "/usr/local/Minerva/include/msql.h"
#endif
#ifdef INCL_LIB_HUGHES_MSQL_H
#include "/usr/lib/Hughes/include/msql.h"
#endif
#endif

#ifdef USE_MYSQL
#ifndef TCC
#ifdef INCL_LOCAL_MYSQL_H
#include "/usr/local/include/mysql.h"
#endif
#ifdef INCL_LOCAL_INCLUDE_MYSQL_MYSQL_H
#include "/usr/local/include/mysql/mysql.h"
#endif
#ifdef INCL_LOCAL_MYSQL_MYSQL_H
#include "/usr/local/mysql/include/mysql.h"
#endif
#ifdef INCL_MYSQL_MYSQL_H
#include "/usr/include/mysql/mysql.h"
#endif
#else
#ifdef INCL_LOCAL_MYSQL_H
#include <mysql.h>
#endif
#ifdef INCL_LOCAL_INCLUDE_MYSQL_MYSQL_H
#include <mysql/mysql.h>
#endif
#ifdef INCL_LOCAL_MYSQL_MYSQL_H
#include "/usr/local/mysql/include/mysql.h"
#endif
#ifdef INCL_MYSQL_MYSQL_H
#include <mysql/mysql.h>
#endif
#endif
#endif

typedef union dbconn_u {
#ifdef USE_MSQL
    struct tmp_msql {
        int handle;
        m_result * result_set;
    } msql;
#endif
#ifdef USE_MYSQL
    struct tmp_mysql {
        char errormsg[256];
        MYSQL *handle;
        MYSQL_RES *results;
    } mysql;
#endif
} dbconn_t;

/*
 * Structure so we can have a lookup table for the specific database
 */
typedef struct db_defn_s {
    const char *name;
    int (*connect)(dbconn_t *, const char *, const char *, const char *, const char *);
    int (*close)(dbconn_t *);
    int (*execute)(dbconn_t *, const char *);
    array_t * (*fetch)(dbconn_t *, int);
    int (*commit)(dbconn_t *);
    int (*rollback)(dbconn_t *);
    void (*cleanup)(dbconn_t *);
    void (*status)(dbconn_t *, outbuffer_t *);
    char * (*error)(dbconn_t *);
} db_defn_t;

#define DB_FLAG_EMPTY   0x1

typedef struct _db {
    int flags;
    db_defn_t *type;
    dbconn_t c;
} db_t;

void db_cleanup (void);

#endif  /* PACKAGES_DB */

#endif  /* PACKAGES_DB_H */
