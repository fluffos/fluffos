#ifndef __db_h
#define __db_h

int db_exists P2( char *, db, char *, keystr);
int db_query P3( svalue_t *, ret, char *, db, char *, keystr);
int db_store P3( char *, db, char *, keystr, svalue_t *, val);
int db_delete P2( char *, db, char *, keystr);
array_t *db_keys P2( char *, db, char *, pattern);

#endif /* __db_h */
