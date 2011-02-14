#ifndef PACKAGES_DB_H
#define PACKAGES_DB_H

#ifdef PACKAGE_DB

#ifdef USE_POSTGRES
#include "/usr/include/postgresql/libpq-fe.h"
#endif

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
#define USE_OLD_FUNCTIONS
#ifndef TCC
#ifdef INCL_MYSQL_INCLUDE_MYSQL_H
#include "/usr/mysql/include/mysql/mysql.h"
#endif
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
#undef USE_OLD_FUNCTIONS
#endif

#ifdef USE_SQLITE2
#include "/usr/include/sqlite.h"
#endif

#ifdef USE_SQLITE3
#include "/usr/include/sqlite3.h"
#endif


typedef union dbconn_u {
#ifdef USE_POSTGRES
	struct tmp_postgres {
		PGconn      *conn;
		PGresult    *res;
	} postgres;
#endif
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
#ifdef USE_SQLITE2
	struct tmp_sqlite2 {
		sqlite *handle;
		sqlite_vm *vm;
		int nrows;
		int ncolumns;
		int last_row;
		int step_res;
		const char **values;
		const char **col_names;
		char *sql;
		char *errormsg;
	} SQLite2;
#endif
#ifdef USE_SQLITE3
	struct tmp_sqlite3 {
		sqlite3 *handle;
		sqlite3_stmt *results;
		int nrows;
		int ncolumns;
		int last_row;
		int step_res;
		char errormsg[256];
	} SQLite3;
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
svalue_t *valid_database (const char * action, array_t * info);
db_t *find_db_conn (int handle);
#endif  /* PACKAGES_DB */

#endif  /* PACKAGES_DB_H */
