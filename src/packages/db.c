/*    Database package for the MudOS driver
 *    Designed to allow people to write datbase independent LPC
 *    Currently supports MSQL
 *    created 960124 by George Reese
 */

#include "std.h"
#include "md.h"
#include "../lpc_incl.h"
#include "../mapping.h"
#include "../comm.h"
#include "../file_incl.h"
#include "../file.h"
#include "../object.h"
#include "../eoperators.h"
#include "../backend.h"
#include "../swap.h"
#include "db.h"

db_t *dbConnList = (db_t *)NULL;

/* int db_close(int handle);
 *
 * Closes the connection to the database represented by the named handle
 *
 * Returns 1 on success, 0 on failure
 */
#ifdef F_DB_CLOSE
void f_db_close PROT((void)) {
    int sock;
    
    sock = sp->u.number;
    if( !valid_db_conn(sock) )
      error("Attempt to close an invalid database handle.\n");
#ifdef MSQL
    msqlClose(sock);
#else
    error("No database engine exists.\n");
#endif /* MSQL */
    free_db_conn(sock);
    pop_stack();
    push_number(1);
}
#endif /* F_DB_CLOSE */

/* int db_commit(int handle);
 *
 * Commits the last set of transactions to the database
 * NOTE: MSQL does not have transaction logic, so since
 * MSQL is the only thing supported now, this does nothing
 * I have put it in, however, so people can write properly
 * portable LPC code
 *
 * Returns 1 on success, 0 on failure
 */
#ifdef F_DB_COMMIT
void f_db_commit PROT((void)) {
#ifdef MSQL
    /* do nothing, transaction was committed */
#else
    error("No database engine exists.\n");
#endif    
    pop_stack();
    push_number(1);
}
#endif /* F_DB_COMMIT */

/* int db_connect(string host, string database)
 *
 * Creates a database connection to the database named by the
 * second argument found on the host named by the first argument.
 * Note that this means you can connect to database servers running on
 * machines other than the one on which the mud is running.  It will
 * connect based on settings established at compile time for the
 * user id and password (if required).
 *
 * Returns a new database handle.
 */
#ifdef F_DB_CONNECT
void f_db_connect PROT((void)) {
    char *database;
    char *host;
    int sock;

    host = (sp-1)->u.string;
    database = sp->u.string;
#ifdef MSQL
    sock = msqlConnect(host);
    if( sock < 1 ) error_needs_free(db_error(-1));
    if( msqlSelectDB(sock, database) == -1 ) {
	msqlClose(sock);
	error_needs_free(db_error(-1));
    }
    create_db_conn(sock);
#else
    error("No database engine exists.\n");
#endif /* MSQL */
    pop_n_elems(2);
    push_number(sock);
}
#endif /* F_DB_CONNECT */

/* mixed db_exec(int handle, string sql)
 *
 * Executes the SQL statement passed for the named connection handle.
 * If data needs to be retrieved from this execution, it should be done
 * through db_fetch() after making the call to db_exec()
 *
 * Returns number of rows in result set on success, an error string on failure
 * NOTE: the number of rows on INSERT, UPDATE, and DELETE statements will
 * be zero since there is no result set.
 */
#ifdef F_DB_EXEC
void f_db_exec PROT((void)) {
    char *s;
    db_t *db;
    int sock;
    
    sock = (sp-1)->u.number;
    s = sp->u.string;
    if( (db = valid_db_conn(sock)) == (db_t *)NULL )
      error("Invalid database handle.\n");
#ifdef MSQL
    if( db->result_set != (m_result *)NULL ) {
        msqlFreeResult(db->result_set);
	db->result_set = (m_result *)NULL;
    }
    if( msqlQuery(sock, s) == -1 ) {
        char * tmperr;

	/* ERROR */
	if( (tmperr = db_error(sock)) == (char *)NULL ) {
            pop_n_elems(2);           
	    push_malloced_string(string_copy(db->errmsg, "f_db_exec"));
	}
	else error_needs_free(tmperr);
	return;
    }
    if( (db->result_set = msqlStoreResult()) == (m_result *)NULL ) {
        pop_n_elems(2);
        push_number(0);            /* UPDATE or INSERT or DELETE */
        return;
    }
    pop_n_elems(2);
    push_number(msqlNumRows(db->result_set));
#else 
    error("No database engine exists.\n");
#endif /* MSQL */    
}
#endif /* F_DB_EXEC */

/* array db_fetch(int db_handle, int row);
 *
 * Returns the result set from the last database transaction
 * performed through db_exec() on the db handle in question for the row
 * named.  For example, db_exec(10, "SELECT player_name from t_player") might
 * have returned two rows.  Typical code to extract that data might be:
 *     string *res;
 *     mixed rows;
 *     int dbconn, i;
 *
 *     dbconn = db_connect("nightmare.imaginary.com", "db_mud");
 *     if( dbconn < 1 ) return 0;
 *     rows = db_exec(dbconn, "SELECT player_name from t_player");
 *     if( !rows ) write("No rows returned.");
 *     else if( stringp(rows) ) write(rows); 
 *     for(i=1; i<=rows; i++) {
 *         res = db_fetch(dbconn, i);
 *         write(res[0]);
 *     }
 *     db_close(dbconn);
 *     return 1;
 *
 * Returns an array of columns from the named row on success.
 */
#ifdef F_DB_FETCH
void f_db_fetch PROT((void)) {
#ifdef MSQL
    m_row this_row;
#endif /* MSQL */    
    db_t *db;
    array_t *v;
    int row, num_cols, hdl, i;

    hdl = (sp-1)->u.number;
    row = sp->u.number;
    if( (db = valid_db_conn(hdl)) == (db_t *)NULL )
      error("Invalid database handle.\n");
#ifdef MSQL    
    if( db->result_set == (m_result *)NULL ) {
	pop_n_elems(2);
	push_refed_array(&the_null_array);
	return;
    }
    if( row < 1 || row > msqlNumRows(db->result_set) )
      error("Attempt to fetch an invalid row through db_fetch().\n");
    num_cols = msqlNumFields(db->result_set);
    if( num_cols < 1) {
        pop_n_elems(2);
	push_refed_array(&the_null_array);
	return;
    }
    msqlDataSeek(db->result_set, row-1);
    this_row = msqlFetchRow(db->result_set);
    if( this_row == NULL ) {
        pop_n_elems(2);
	push_refed_array(&the_null_array);
	return;
    }
    v = allocate_empty_array(num_cols);	
    for(i=0; i<num_cols; i++) {
        m_field *this_field;
	    
	this_field = msqlFetchField(db->result_set);
	if( this_field == (m_field *)NULL || this_row[i] == NULL )
	  v->item[i] = const0u;
	else {
	    switch(this_field->type) {
	        case INT_TYPE:
		  v->item[i].type = T_NUMBER;
		  v->item[i].u.number = atoi(this_row[i]);
		  break;
		  
	        case REAL_TYPE:
		  v->item[i].type = T_REAL;
		  v->item[i].u.real = atof(this_row[i]);
		  break;
		
	        case CHAR_TYPE:
		  v->item[i].type = T_STRING;
		  v->item[i].subtype = STRING_MALLOC;
		  v->item[i].u.string = string_copy(this_row[i], "f_db_fetch");
		  break;
		  
                default:
		  v->item[i] = const0u;
		  break;
	    }
	}
    }
    msqlFieldSeek(db->result_set, 0);
#endif /* MSQL */
    pop_n_elems(2);
    push_refed_array(v);
}
#endif /* F_DB_FETCH */

/* int db_rollback(int handle)
 *
 * Rollsback all db_exec() calls back to the last db_commit() call for the
 * named connection handle.
 * NOTE: MSQL does not support rollbacks
 *
 * Returns 1 on success, 0 on failure
 */
#ifdef F_DB_ROLLBACK
void f_db_rollback PROT((void)) {
#ifdef MSQL
    error("MSQL does not support transaction rollbacks.\n");
#else
    error("No database engine exists.\n");
#endif /* MSQL */
    pop_stack();
    push_number(0);
}
#endif /* F_DB_ROLLBACK */

char *db_error P1(int, hdl) {
    db_t *db;
    int len;

    len = strlen(msqlErrMsg);
    if( len > 252 ) len = 252;
    if( hdl == -1 || (db = valid_db_conn(hdl)) == (db_t *)NULL) {
        char *tmp;

	tmp = new_string(len+2, "foo");
	memcpy(tmp, msqlErrMsg, len);
	tmp[len] = '\n';
	tmp[len + 1] = '\0';
	return tmp;
    }
    memcpy(db->errmsg, msqlErrMsg, len);
    db->errmsg[len] = '\n';
    db->errmsg[len + 1] = '\0';
    return (char *)NULL;
}

void create_db_conn P1(int, sock) {
    db_t *db;
    
    db = ALLOCATE(db_t, TAG_DB, "db_connect");
    db->handle = sock;
    db->errmsg[0] = '\0';
#ifdef MSQL
    db->result_set = (m_result *)NULL;
#endif /* MSQL */    
    if( dbConnList == (db_t *)NULL ) {
        db->prior = (db_t *)NULL;
	db->next = (db_t *)NULL;
	dbConnList = db;
    }
    else {
        db_t *tmp;

	tmp = dbConnList;
	while( tmp->next != (db_t *)NULL );
	tmp->next = db;
	db->prior = tmp;
    }
}

void free_db_conn P1(int, sock) {
    db_t *tmp, *nextdb, *priordb;

    tmp = dbConnList;
    if( tmp == NULL ) return;
    while( tmp->handle != sock ) {
        if( tmp->next == (db_t *)NULL ) return;
        tmp = tmp->next;
    }
    nextdb = tmp->next;
    priordb = tmp->prior;
    if( priordb != (db_t *)NULL ) priordb->next = nextdb;
    else dbConnList = nextdb;
    if( nextdb != (db_t *)NULL ) nextdb->prior = priordb;
#ifdef MSQL    
    if( tmp->result_set != (m_result *)NULL )
      msqlFreeResult(tmp->result_set);
#endif /* MSQL */    
    FREE(tmp);
}

db_t *valid_db_conn P1(int, hdl) {
    db_t *tmp;

    tmp = dbConnList;
    if( tmp == (db_t *)NULL ) return (db_t *)NULL;
    while( tmp->handle != hdl ) {
        if( tmp->next == (db_t *)NULL ) return (db_t *)NULL;
	tmp = tmp->next;
    }
    return tmp;
}

