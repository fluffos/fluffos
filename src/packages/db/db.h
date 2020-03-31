#ifndef PACKAGES_DB_H
#define PACKAGES_DB_H

#ifdef USE_POSTGRES
#include <libpq-fe.h>
#endif

#ifdef USE_MYSQL
#define USE_OLD_FUNCTIONS
#include <mysql.h>
#undef USE_OLD_FUNCTIONS
#endif

#ifdef USE_SQLITE3
#include <sqlite3.h>
#endif

union dbconn_t {
#ifdef USE_POSTGRES
  struct tmp_postgres {
    PGconn *conn;
    PGresult *res;
  } postgres;
#endif
#ifdef USE_MYSQL
  struct tmp_mysql {
    char errormsg[256];
    MYSQL *handle;
    MYSQL_RES *results;
  } mysql;
#endif
#ifdef USE_SQLITE3
  struct tmp_sqlite3 {
    struct sqlite3 *handle;
    struct sqlite3_stmt *results;
    int nrows;
    int ncolumns;
    int last_row;
    int step_res;
    char errormsg[256];
  } SQLite3;
#endif
};

/*
 * Structure so we can have a lookup table for the specific database
 */
struct db_defn_t {
  const char *name;
  int (*connect)(dbconn_t *, const char *, const char *, const char *, const char *);
  int (*close)(dbconn_t *);
  int (*execute)(dbconn_t *, const char *);
  array_t *(*fetch)(dbconn_t *, int);
  int (*commit)(dbconn_t *);
  int (*rollback)(dbconn_t *);
  void (*cleanup)(dbconn_t *);
  void (*status)(dbconn_t *, outbuffer_t *);
  char *(*error)(dbconn_t *);
};

#define DB_FLAG_EMPTY 0x1

struct db_t {
  int flags;
  db_defn_t *type;
  union dbconn_t c;
};

void db_cleanup(void);
svalue_t *valid_database(const char *action, array_t *info);
db_t *find_db_conn(int handle);

void mark_db_conn();
#endif /* PACKAGES_DB_H */
