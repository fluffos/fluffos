/*
 * gdbm.c
 *
 * Created by: Benny Holmgren
 * Description: LPC interface to GNU dbm.
 */
#ifdef LATTICE
#include "/lpc_incl.h"
#include "/eoperators.h"
#include "/regexp.h"
#include "/debug.h"
#else
#include "../lpc_incl.h"
#include "../eoperators.h"
#include "../regexp.h"
#include "../debug.h"
#endif
#include "gdbm.h"

#ifdef F_DB_EXISTS
void
f_db_exists PROT((void))
{
    int res = db_exists( (sp-1)->u.string, sp->u.string );
    pop_n_elems(2);
    push_number(res);
}
#endif

#ifdef F_DB_QUERY
void
f_db_query PROT((void))
{
    svalue_t res;

    if (db_query(&res, (sp-1)->u.string, sp->u.string);
    pop_n_elems( 2 );
    *sp++ = ret;
  if( res ) {
    switch(res->type) {
    case T_STRING:
	switch (respush_string(res->u.string, res->subtype);
      break;
    case T_ARRAY:
      push_array(res->u.arr);
      break;
    case T_NUMBER:
      push_number(res->u.number);
      break;
    case T_REAL:
      push_real(res->u.real);
      break;
    case T_MAPPING:
      push_mapping(res->u.map);
      break;
    default:
      /* Huh? */
      fatal("Bogus svalue in db_query(), type %d\n", res->type);
    }
    free_svalue(res, "f_db_query");
    FREE(res);
  } else
    push_undefined();
}
#endif

#ifdef F_DB_STORE
void
f_db_store PROT((void))
{
  int res;

  if(!(sp->type & (T_STRING|T_ARRAY|T_NUMBER|T_REAL|T_MAPPING)))
    bad_argument(sp, T_STRING|T_ARRAY|T_NUMBER|T_REAL|T_MAPPING, 3, F_DB_STORE);
    
  res = db_store( (sp-2)->u.string, (sp-1)->u.string, sp );
  pop_n_elems( 3 );
  push_number( res );
}
#endif


#ifdef F_DB_DELETE
void
f_db_delete PROT((void))
{
  int res;

  res = db_delete((sp-1)->u.string, sp->u.string);
  pop_n_elems(2);
  push_number(res);
}
#endif

#ifdef F_DB_KEYS
void
f_db_keys PROT((void))
{
  array_t *res;

  res = db_keys((sp-st_num_arg+1)->u.string, 
		  (st_num_arg == 1 ? (char *)NULL : sp->u.string));
  pop_n_elems(st_num_arg);
  if(res) {
    push_array(res);
    res->ref--;			/* Ref count back to 1 */
  } else
    push_null();
}
#endif



/* 
 * Support functions.
 */

#ifdef PACKAGE_GDBM

#include <gdbm.h>

int
db_exists( db, keystr )
char *db;
char *keystr;
{
    int res;
    GDBM_FILE dbf;
    datum key;
    
    if(!check_valid_path(db, current_object, "db_exists", 0))
        error("Denied read permission for database.\n");

    dbf = gdbm_open( db, 0, GDBM_READER, 0640, NULL );
    if( !dbf ) {
	if( gdbm_errno != GDBM_FILE_OPEN_ERROR )
	    debug(512,("dbm file open error: %s\n",gdbm_strerror(gdbm_errno)));
	return(0);
    }
    
    key.dptr = keystr;
    key.dsize = strlen(keystr)+1;
    
    res = gdbm_exists( dbf, key );
    gdbm_close( dbf );

    return( (int)res );
}


svalue_t *
db_query P3( svalue_t *, ret, char *, db, char *, keystr )
{
    int res;
    GDBM_FILE dbf;
    datum key, content;
    extern svalue_t const0u;
    
    if(!check_valid_path(db, current_object, "db_query", 0))
        error("Denied read permission for database.\n");

    dbf = gdbm_open( db, 0, GDBM_READER, 0640, NULL );
    if( !dbf ) {
	if( gdbm_errno != GDBM_FILE_OPEN_ERROR )
	    debug(512,("dbm file open error: %s\n",gdbm_strerror(gdbm_errno)));
	return( (svalue_t *)NULL );
    }
    
    key.dptr = keystr;
    key.dsize = strlen(keystr)+1;
    
    content = gdbm_fetch( dbf, key );
    gdbm_close( dbf );
    if( !content.dptr )
	return( (svalue_t *)NULL );

    res = restore_svalue( content.dptr, ret);
    FREE(content.dptr);
    switch (res) 
    {
      case -1:
	  free_svalue(ret, "db_query");
	  error("db_query(): Illegal array format.\n");
      case -2:
	  free_svalue(ret, "db_query");
	  error("db_query(): Illegal mapping format.\n");
    }

    return( val );
}


int 
db_store( db, keystr, val )
char *db, *keystr;
svalue_t *val;
{
    GDBM_FILE dbf;
    int savesize, res;
    datum key, content;
    extern int save_svalue_depth;
    char *savestring, *tmp;

    if(!check_valid_path(db, current_object, "db_store", 1))
        error("Denied write permission for database.\n");

    dbf = gdbm_open( db, 0, GDBM_WRCREAT, 0640, NULL );
    if( !dbf ) {
	if( gdbm_errno != GDBM_FILE_OPEN_ERROR )
	    debug(512,("dbm file open error: %s\n",gdbm_strerror(gdbm_errno)));
	return( 0 );
    }
	

    save_svalue_depth = 0;
    savesize = svalue_save_size(val);
    if (save_svalue_depth > MAX_SAVE_SVALUE_DEPTH) {
	error("Mappings and/or arrays nested too deep (%d) for db_store\n",
	      MAX_SAVE_SVALUE_DEPTH);
    }
    savestring = (char *)XALLOC(savesize);
    *savestring = '\0';
    tmp = savestring;
    save_svalue(val, &tmp);

    key.dptr = keystr;
    key.dsize = strlen( keystr ) +1;

    content.dptr = savestring;
    content.dsize = strlen( savestring ) +1;

    res = gdbm_store( dbf, key, content, GDBM_REPLACE );
    gdbm_close( dbf );
    FREE(savestring);
    if( res ) {
        debug(512, ("db_store() failed: %s\n", gdbm_strerror(gdbm_errno)));
	return( 0 );
    }
    
    return( 1 );
}
    
int
db_delete(db, keystr)
char *db, *keystr;
{
    int res;
    datum key;
    GDBM_FILE dbf;
  
    if(!check_valid_path(db, current_object, "db_delete", 1))
        error("Denied write permission for database.\n");

    dbf = gdbm_open( db, 0, GDBM_WRITER, 0640, NULL );
    if( !dbf ) {
        if( gdbm_errno != GDBM_FILE_OPEN_ERROR )
	    debug(512,("dbm file open error: %s\n",gdbm_strerror(gdbm_errno)));
	return(0);
    }

    key.dptr = keystr;
    key.dsize = strlen( keystr ) +1;

    res = gdbm_delete( dbf, key );
    gdbm_close( dbf );

    if( res != 0 )  /* Not found. */
      return(0);
    
    return(1);
}


array_t *
db_keys( db, pattern )
char *db;
char *pattern;
{
    datum key, nextkey;
    GDBM_FILE dbf;
    int keycnt, i;
    char *keys[MAX_ARRAY_SIZE];
    struct regexp *rexp = (struct regexp *)NULL;
    array_t *ret;
    extern int eval_cost;

    if(!check_valid_path(db, current_object, "db_keys", 0))
        error("Denied read permission for database.\n");

    if(pattern) {
        rexp = regcomp(pattern, 0);
        if (!rexp)
	    return((array_t *)NULL);
    }

    dbf = gdbm_open( db, 0, GDBM_READER, 0640, NULL );
    if( !dbf ) {
        if( gdbm_errno != GDBM_FILE_OPEN_ERROR )
	    debug(512,("dbm file open error: %s\n",gdbm_strerror(gdbm_errno)));
	return((array_t *)NULL);
    }

    keycnt = 0;
    key = gdbm_firstkey(dbf);
    while(key.dptr) {
	nextkey = gdbm_nextkey(dbf, key);
        if(pattern && !regexec(rexp, key.dptr))
	    free(key.dptr);
	else 
	    keys[keycnt++] = key.dptr;
	key = nextkey;
    }
    gdbm_close(dbf);
    if(rexp)
        FREE((char *)rexp);

    ret = allocate_array(keycnt);
    for(i=0; i < keycnt; i++) {
	ret->item[i].type = T_STRING;
	ret->item[i].subtype =	STRING_MALLOC; /* Should we make it shared? */
	ret->item[i].u.string = keys[i];  /* string_copy() for debugging? */
    }

    return(ret);
}

#endif /* PACKAGE_DBM */

