#ifndef l_db_h
#define l_db_h

#ifdef INCL_LOCAL_MSQL_H
#include "/usr/local/include/msql.h"
#endif
#ifdef INCL_LOCAL_MSQL_MSQL_H
#include "/usr/local/msql/include/msql.h"
#endif
#ifdef INCL_LOCAL_MINERVA_MSQL_H
#include "/usr/local/Minerva/include/msql.h"
#endif
#ifdef MY_SQL
#include <mysql.h>
#endif

typedef struct _db {
    struct _db *next;
    struct _db *prior;
    int handle;
    char errmsg[255];
#ifdef MSQL
    m_result *result_set;
#endif
#ifdef MY_SQL
    MYSQL *my_handle;
    MYSQL_RES *result_set;
#endif
} db_t;

#ifdef MSQL
char *db_error P1(int, hdl);
#endif
#ifdef MY_SQL
char *mysql_db_error P1(MYSQL *, hdl);
#endif
db_t *create_db_conn P1(int, sock);
void free_db_conn P1(int, sock);
db_t *valid_db_conn P1(int, hdl);

#endif /* l_db_h */
