#include "std.h"
#include "config.h"
#include "lpc_incl.h"
#include "file_incl.h"
#include "otable.h"
#include "backend.h"
#include "debug.h"
#include "comm.h"
#include "swap.h"
#include "socket_efuns.h"
#include "call_out.h"
#include "port.h"

#define too_deep_save_error() \
    error("Mappings and/or arrays nested too deep (%d) for save_object\n",\
	  MAX_SAVE_SVALUE_DEPTH);

struct object *previous_ob;
int tot_alloc_object, tot_alloc_object_size;

#ifdef DEALLOCATE_MEMORY_AT_SHUTDOWN
static void remove_all_objects PROT((void));
#endif
char *save_mapping PROT ((struct mapping *m));
INLINE static int restore_array PROT((char **str, struct svalue *));
int restore_hash_string PROT((char **str, int *a, struct svalue *));

INLINE int
valid_hide P1(struct object *, obj)
{
    struct svalue *ret;

    if (!obj) {
	return 0;
    }
    push_object(obj);
    ret = apply_master_ob(APPLY_VALID_HIDE, 1);
    return (!IS_ZERO(ret));
}


int save_svalue_depth = 0, max_depth;
int *sizes = 0;

INLINE int svalue_save_size P1(struct svalue *, v)
{
    switch(v->type){
    case T_STRING:
	{
	    register char *cp = v->u.string;
	    char c;
	    int size = 0;

	    while (c = *cp++){
		if (c == '\\' || c == '\"') size++;
		size++;
	    }
	    return 3 + size;
	}

    case T_POINTER:
	{
	    struct svalue *sv = v->u.vec->item;
	    int i = v->u.vec->size, size = 0;

	    if (++save_svalue_depth > MAX_SAVE_SVALUE_DEPTH){
		too_deep_save_error();
	    }
	    while (i--) size += svalue_save_size(sv++);
	    save_svalue_depth--;
	    return size + 5;
	}

    case T_MAPPING:
	{
	    struct node **a = v->u.map->table, *elt;
	    int j = v->u.map->table_size, size = 0;

	    if (++save_svalue_depth > MAX_SAVE_SVALUE_DEPTH){
                too_deep_save_error();
	    }
	    do {
		for (elt = a[j]; elt; elt = elt->next){
		    size += svalue_save_size(elt->values) +
			    svalue_save_size(elt->values+1);
		}
	    } while (j--);
	    save_svalue_depth--;
	    return size + 5;
	}

    case T_NUMBER:
	{
	    int res = v->u.number, len;
	    len = res < 0 ? (res = (-res) & 0x7fffffff,3) : 2;
	    while (res>9) { res /= 10; len++; }
	    return len;
	}

    case T_REAL:
	{
	    char buf[256];
	    sprintf(buf, "%g", v->u.real);
	    return (int)(strlen(buf)+1);
	}

    default:
	{
	    return 2;
	}
    }
}

INLINE void save_svalue P2(struct svalue *, v, char **, buf)
{
    switch(v->type){
    case T_STRING:
	{
	    register char *cp = *buf, *str = v->u.string;
	    char c;

	    *cp++ = '"';
	    while (c = *str++){
		if (c == '"' || c == '\\'){
		    *cp++ = '\\';
		    *cp++ = c;
		}
#ifndef MSDOS
		else *cp++ = (c == '\n') ? '\r' : c;
#else
		else *cp++ = (c == '\n') ? 30 : c;
#endif
	    }

	    *cp++ = '"';
	    *(*buf = cp) = '\0';
	    return;
	}

    case T_POINTER:
	{
	    int i = v->u.vec->size;
	    struct svalue *sv = v->u.vec->item;

	    *(*buf)++ = '(';
	    *(*buf)++ = '{';
	    while (i--){
		save_svalue(sv++, buf);
		*(*buf)++ = ',';
	    }
	    *(*buf)++ = '}';
	    *(*buf)++ = ')';
	    *(*buf) = '\0';
	    return;
	}

    case T_NUMBER:
	{
	    int res = v->u.number, fact, len = 1, neg = 0;
	    register char *cp;

	    if (res < 0) { len++, neg = 1, res = (-res) & 0x7fffffff; }
	    fact = res;
	    while (fact > 9) { fact /= 10; len++; }
	    *(cp = (*buf += len)) = '\0';
	    do {
		*--cp = res % 10 + '0';
		res /= 10;
	    } while (res);
	    if (neg) *(cp-1) = '-';
	    return;
	}

    case T_REAL:
	{
	    sprintf(*buf, "%g", v->u.real);
	    (*buf) += strlen(*buf);
	    return;
	}

    case T_MAPPING:
	{
	    int j = v->u.map->table_size;
	    struct node **a = v->u.map->table, *elt;

	    *(*buf)++ = '(';
	    *(*buf)++ = '[';
	    do {
		for (elt = a[j]; elt; elt = elt = elt->next){
		    save_svalue(elt->values, buf);
		    *(*buf)++ = ':';
		    save_svalue(elt->values + 1, buf);
		    *(*buf)++ = ',';
		}
	    } while (j--);

	    *(*buf)++ = ']';
	    *(*buf)++ = ')';
	    *(*buf) = '\0';
	    return;
	}
    }
}

INLINE static int
restore_internal_size P3(char **, str, int, is_mapping, int, depth)
{
    register char *cp = *str;
    int size = 0;
    char c, delim, index = 0;

    delim = is_mapping ? ':' : ',';
    while (c = *cp++){
	switch(c){
	case '\"':
	    {
		while ((c = *cp++) != '"')
		    if ((c == '\0') || (c == '\\' && !*cp++)){
			return 0;
		    }
		if (*cp++ != delim) return 0;
		size++;
		break;
	    }

	case '(':
	    {
		if (*cp == '{'){
	            *str = ++cp;
		    if (!restore_internal_size(str, 0, save_svalue_depth++)){
			return 0;
		    }
		}
		else if (*cp == '['){
		    *str = ++cp;
		    if (!restore_internal_size(str, 1, save_svalue_depth++)){ return 0;}
		}
		else { return 0;}
		
		if (*(cp = *str) != delim){ return 0;}
		cp++;
		size++;
		break;
	    }

	case ']':
	    {
		if (*cp++ == ')' && is_mapping){
		    *str = cp;
		    if (!sizes){
			max_depth = 128;
			while (max_depth <= depth) max_depth <<= 1;
			sizes = CALLOCATE(max_depth, int, TAG_TEMPORARY,
					  "restore_internal_size");
		    }
		    else if (depth >= max_depth){
			while ((max_depth <<= 1) <= depth);
			sizes = RESIZE(sizes, max_depth, int, TAG_TEMPORARY,
				       "restore_internal_size");
		    }
		    sizes[depth] = size;
		    return 1;
		}
		else { return 0; }
	    }

	case '}':
	    {
		if (*cp++ == ')' && !is_mapping){
		    *str = cp;
                    if (!sizes){
                        max_depth = 128;
                        while (max_depth <= depth) max_depth <<= 1;
			sizes = CALLOCATE(max_depth, int, TAG_TEMPORARY,
					  "restore_internal_size");
		    }
                    else if (depth >= max_depth){
                        while ((max_depth <<= 1) <= depth);
			sizes = RESIZE(sizes, max_depth, int, TAG_TEMPORARY,
				       "restore_internal_size");
		    }
                    sizes[depth] = size;
		    return 1;
		}
		else { return 0;}
	    }

	case ':':
	case ',':
	    {
		if (c != delim) return 0;
		size++;
		break;
	    }

	default:
	    {
		if (!(cp = strchr(cp, delim))) return 0;
		cp++;
		size++;
	    }
	}
	if (is_mapping) delim = (index ^= 1) ? ',' : ':';
    }
    return 0;
}



INLINE static int
restore_size P2(char **, str, int, is_mapping)
{
    register char *cp = *str;
    int size = 0;
    char c, delim, index = 0;

    delim = is_mapping ? ':' : ',';

    while (c = *cp++){
	switch(c){
	case '\"':
	    {
		while ((c = *cp++) != '"')
		    if ((c == '\0') || (c == '\\' && !*cp++)) return 0;

		if (*cp++ != delim){ return -1; }
		size++;
		break;
	    }

	case '(':
	    {
		if (*cp == '{'){
	            *str = ++cp;
		    if (!restore_internal_size(str, 0, save_svalue_depth++)) return -1;
		}
		else if (*cp == '['){
		    *str = ++cp;
		    if (!restore_internal_size(str, 1, save_svalue_depth++)) return -1;
		}
		else { return -1; }
		
		if (*(cp = *str) != delim) { return -1;}
		cp++;
		size++;
		break;
	    }

	case ']':
	    {
		save_svalue_depth = 0;
		if (*cp++ == ')' && is_mapping){
		    *str = cp;
		    return size;
		}
		else { return -1;}
	    }

	case '}':
	    {
		save_svalue_depth = 0;
		if (*cp++ == ')' && !is_mapping){
		    *str = cp;
		    return size;
		}
		else { return -1;}
	    }

	case ':':
	case ',':
	    {
		if (c != delim) return -1;
		size++;
		break;
	    }

	default:
	    {
		if (!(cp = strchr(cp, delim))) { return -1;}
		cp++;
		size++;
	    }
	}
	if (is_mapping) delim = (index ^= 1) ? ',' : ':';
    }
    return -1;
}

INLINE static int
restore_interior_string P2(char **, val, struct svalue *, sv)
{
    register char *cp = *val;
    char *start = cp;
    char c;
    int len;

    while ((c = *cp++) != '"') {
	switch (c){
#ifndef MSDOS	    
	case '\r':
#else
	case 30:
#endif
	    {
		*(cp-1) = '\n';
		break;
	    }

	case '\\':
	    {
		char *new = cp - 1;

		if (*new++ = *cp++){
		    while ((c = *cp++) != '"'){
			if (c == '\\'){
			    if (!(*new++ = *cp++)) return ROB_STRING_ERROR;
			}
			else {
#ifndef MSDOS
			    if (c == '\r')
#else
			    if (c == 30)
#endif
				*new++ = '\n';
			    else *new++ = c;
			}
		    }
		    if (c == '\0') return ROB_STRING_ERROR;
		    *new = '\0';
		    *val = cp;
		    sv->u.string = DXALLOC((len = (new - start)) + 1, TAG_STRING,
				"restore_string");
		    strcpy(sv->u.string, start);
		    sv->type = T_STRING;
		    sv->subtype = STRING_MALLOC;
		    return 0;
		}
		else return ROB_STRING_ERROR;
	    }

	case '\0':
	    {
		return ROB_STRING_ERROR;
	    }

	}
    }

    *val = cp;
    *--cp = '\0';
    len = cp - start;
    sv->u.string = DXALLOC(len + 1, TAG_STRING, "restore_string");
    strcpy(sv->u.string, start);
    sv->type = T_STRING;
    sv->subtype = STRING_MALLOC;
    return 0;
}

#define PARSE_NUMERIC(X,Y,Z) \
    { \
                int res = c - '0'; \
	\
                while ((c = *cp++) >= '0' && c <= '9'){ \
                    res *= 10, res += c - '0'; \
		} \
                if (c == '.'){ \
                    float f1 = 0.0, f2 = 10.0; \
                    int hh = 0; \
			\
                    while ((c = *cp++) >= '0' && c <= '9' && !(++hh & 8)){ \
                        f1 += (c - '0') / f2; \
                        f2 *= 10.0; \
		    } \
                    if (!hh)  \
			Z; \
		    X; \
		} \
                else { \
		    Y; \
		} \
                break; \
    } \

INLINE static void add_map_stats P2(struct mapping *, m, int, count)
{
    total_mapping_nodes += count;
    total_mapping_size += count * sizeof(struct node);
#ifndef NO_MUDLIB_STATS
    add_array_size(&m->stats, count << 1);
#endif
    m->count = count;
}

int growMap PROT((struct mapping *));

static int
restore_mapping P2(char **,str, struct svalue *, sv)
{
    int size, i, oi, mask, count = 0;
    char c;
    struct mapping *m;
    struct svalue key, value;
    struct node **a, *elt, *elt2;
    register char *cp = *str;
    int err;

    if (save_svalue_depth) size = sizes[save_svalue_depth-1]; 
    else if ((size = restore_size(str, 1)) < 0) return 0;
    
    if (!size) {
	*str += 2;
	sv->u.map = allocate_mapping(0);
	sv->type = T_MAPPING;
	return 0;
    }
    m = allocate_mapping(size >> 1); /* have to clean up after this or */
    a = m->table;                    /* we'll leak */
    mask = m->table_size;
    
    while (1) {
	switch (c = *cp++) {
	case '"':
	    {
		*str = cp;
		if (err = restore_hash_string(str, &oi, &key))
		    goto key_error;
		cp = *str;
		cp++;
		break;
	    }
	    
	case '(':
	    {
		save_svalue_depth++;
		if (*cp == '['){
		    *str = ++cp;
		    if (err = restore_mapping(str, &key))
			goto key_error;
		    oi = (int) key.u.map;
		}
		else if (*cp == '{'){
		    *str = ++cp;
		    if (err = restore_array(str, &key))
			goto key_error;
		    oi = (int) key.u.vec;
		}
		else goto generic_key_error;
		cp = *str;
		cp++;
		break;
	    }
	    
	case ':':
	    {
		oi = key.u.number = 0;
		key.type = T_NUMBER;
		break;
	    }
	    
	case ']':
	    *str = ++cp;
	    add_map_stats(m, count);
	    sv->type = T_MAPPING;
	    sv->u.map = m;
	    return 0;

	case '-':
	    if ((c = *cp++) < '0' || c > '9') {
		goto key_numeral_error;
	    }
	    PARSE_NUMERIC(oi = 0;key.u.real = -(f1+res); key.type = T_REAL,
			  key.type = T_NUMBER; key.u.number = -(oi = res),
			  goto key_numeral_error)
		
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	    {
		PARSE_NUMERIC(oi = 0; key.u.real = f1+res;key.type = T_REAL,
			      key.type = T_NUMBER; oi = key.u.number = res,
			      goto key_numeral_error)
	    }
	    
	default:
	    goto generic_key_error;
	}

	/* At this point, key is a valid, referenced svalue and we're
	   responsible for it */
	
	switch (c = *cp++){
	case '"':
	    {
		*str = cp;
		if (err = restore_interior_string(str, &value))
		    goto value_error;
		cp = *str;
		cp++;
		break;
	    }
	    
	case '(':
	    {
		save_svalue_depth++;
		if (*cp == '['){
		    *str = ++cp;
		    if (err = restore_mapping(str, &value))
			goto value_error;
		}
		else if (*cp == '{'){
		    *str = ++cp;
		    if (err = restore_array(str, &value))
			goto value_error;
		}
		else goto generic_value_error;
		cp = *str;
		cp++;
		break;
	    }
	    
	case '-':
	    {
		if ((c = *cp++) < '0' || c > '9') {
		    goto value_numeral_error;
		}
		PARSE_NUMERIC((value.u.real = -(f1+res), value.type = T_REAL),
			      (value.type = T_NUMBER, value.u.number = -res),
			      goto value_numeral_error)
	    }
	    
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	    {
		PARSE_NUMERIC((value.u.real = (f1+res),value.type = T_REAL),
			      (value.type = T_NUMBER,value.u.number = res),
			      goto value_numeral_error)
	    }
	    
	case ',':
	    {
		value.u.number = 0;
		value.type = T_NUMBER;
		break;
	    }
	    
	default:
	    goto generic_value_error;
	}

	/* both key and value are valid, referenced svalues */

	i = oi & mask;
	if (elt2 = elt = a[i]){
	    do {
		if (sameval(&key, elt->values)){
		    free_svalue(&key, "restore_mapping: duplicate key");
		    free_svalue(elt->values+1, "restore_mapping: replaced value");
		    *(elt->values+1) = value;
		    break;
		}
	    } while (elt = elt->next);
	    if (elt)
		continue;
	} else if (!(--m->unfilled)){
	    if (growMap(m)){
		a = m->table;
		if (oi & ++mask) elt2 = a[i |= mask];
		mask <<= 1;
		mask--;
	    } else {
		add_map_stats(m, count);
		free_mapping(m);
		free_svalue(&key, "restore_mapping: out of memory");
		free_svalue(&value, "restore_mapping: out of memory");
		error("Out of memory\n");
	    }
	}
	
	if (++count > MAX_MAPPING_SIZE) {
	    add_map_stats(m, count -1);
	    free_mapping(m);
	    free_svalue(&key, "restore_mapping: mapping too large");
	    free_svalue(&value, "restore_mapping: mapping too large");
	    mapping_too_large();
	}
	
	elt = ALLOCATE(struct node, TAG_MAP_NODE, "restore_mapping");
	*elt->values = key;
	*(elt->values + 1) = value;
	elt->hashval = oi;
	(a[i] = elt)->next = elt2;
    }

    /* something went wrong */
 value_numeral_error:
    free_svalue(&key, "restore_mapping: numeral value error");
 key_numeral_error:
    add_map_stats(m, count);
    free_mapping(m);
    return ROB_NUMERAL_ERROR;
 generic_value_error:
    free_svalue(&key, "restore_mapping: generic value error");
 generic_key_error:
    add_map_stats(m, count);
    free_mapping(m);
    return ROB_MAPPING_ERROR;
 value_error:
    free_svalue(&key, "restore_mapping: value error");
 key_error:
    add_map_stats(m, count);
    free_mapping(m);
    return err;
}


INLINE static int
restore_array P2(char **, str, struct svalue *, ret)
{   
    int size;
    char c;
    struct vector *v;
    struct svalue *sv;
    register char *cp = *str;
    int err;

    if (save_svalue_depth) size = sizes[save_svalue_depth-1];
    else if ((size = restore_size(str,0)) < 0) return ROB_ARRAY_ERROR; 

    v = allocate_array(size); /* after this point we have to clean up
				 or we'll leak */
    sv = v->item;

    while (size--) {
	switch (c = *cp++) {
	case '"':
	    *str = cp;
	    if (err = restore_interior_string(str, sv))
		goto generic_error;
	    cp = *str;
	    cp++;
	    sv++;
	    break;

	case ',':
	    sv++;
	    break;

	case '(':
	    {
		save_svalue_depth++;
		if (*cp == '['){
		    *str = ++cp;
		    if (err = restore_mapping(str, sv))
			goto error;
		}
		else if (*cp == '{'){
		    *str = ++cp;
		    if (err = restore_array(str, sv))
			goto error;
		}
		else goto generic_error;
		sv++;
		cp = *str;
		cp++;
		break;
	    }

	case '-':
	    {
		if ((c = *cp++) < '0' || c > '9') {
		    err = ROB_NUMERAL_ERROR;
		    goto error;
		}
		PARSE_NUMERIC((sv->u.real = -(f1+res), (sv++)->type = T_REAL),
			      ((sv++)->u.number = -res),
			      goto numeral_error)
	    }

	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	    {
		PARSE_NUMERIC((sv->u.real = (f1+res), (sv++)->type = T_REAL),
			      ((sv++)->u.number = res),
			      goto numeral_error)
	    }

	default:
	    goto generic_error;
	}
    }

    cp += 2;
    *str = cp;
    ret->u.vec = v;
    ret->type = T_POINTER;
    return 0;
    /* something went wrong */
 numeral_error:
    err = ROB_NUMERAL_ERROR;
    goto error;
 generic_error:
    err = ROB_ARRAY_ERROR;
 error:
    free_vector(v);
    return err;
}

INLINE int
restore_string P2(char *, val, struct svalue *, sv)
{
    register char *cp = val;
    char *start = cp;
    char c;
    int len;

    while ((c = *cp++) != '"') {
        switch (c){
#ifndef MSDOS
	case '\r':
#else
	case 30:
#endif
            {
                *(cp-1) = '\n';
                break;
	    }

	case '\\':
            {
                char *new = cp - 1;

                if (*new++ = *cp++){
                    while ((c = *cp++) != '"'){
                        if (c == '\\'){
                            if (!(*new++ = *cp++)) return ROB_STRING_ERROR;
			}
                        else {
#ifndef MSDOS
                            if (c == '\r')
#else
                            if (c == 30)
#endif
                                *new++ = '\n';
                            else *new++ = c;
			}
		    }
                    if ((c == '\0') || (*cp != '\0')) return ROB_STRING_ERROR;
                    *new = '\0';
                    sv->u.string = DXALLOC(new - start + 1, TAG_STRING,
					   "restore_string");
                    strcpy(sv->u.string, start);
		    sv->type = T_STRING;
		    sv->subtype = STRING_MALLOC;
                    return 0;
		}
                else return ROB_STRING_ERROR;
	    }

	case '\0':
            {
                return ROB_STRING_ERROR;
	    }

	}
    }

    if (*cp--) return ROB_STRING_ERROR;
    *cp = '\0';
    len = cp - start;
    sv->u.string = DXALLOC(len + 1, TAG_STRING, "restore_string");
    strcpy(sv->u.string, start);
    sv->type = T_STRING;
    sv->subtype = STRING_MALLOC;
    return 0;
}

/* for this case, the variable in question has been set to zero already,
   and we don't have to worry about preserving it */
INLINE int
restore_svalue P2(char *, cp, struct svalue *, v)
{
    switch(*cp++) {
    case '\"':
	return restore_string(cp, v);
    case '(':
	{
	    if (*cp == '{'){
		cp++;
		return restore_array(&cp, v);
	    } else if (*cp++ == '[') {
		return restore_mapping(&cp, v);
	    }
	    else return ROB_GENERAL_ERROR;
	}

    case '-':
	{
	    char c;

	    if ((c = *cp++) < '0' || (c > '9')) return ROB_NUMERAL_ERROR;
	    PARSE_NUMERIC((v->type = T_REAL, v->u.real = -f1-res),
			  (v->type = T_NUMBER, v->u.number = -res),
			  return ROB_NUMERAL_ERROR);
	}

    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        {
	    char c = *(cp-1);
	    PARSE_NUMERIC((v->type = T_REAL, v->u.real = f1+res),
			  (v->type = T_NUMBER, v->u.number = res),
			  return ROB_NUMERAL_ERROR);
	}

    default: {
	    v->type = T_NUMBER;
	    v->u.number = 0;
	}
    }
    return 0;
}

/* for this case, we're being careful and want to leave the value alone on
   an error */
INLINE int
safe_restore_svalue P2(char *, cp, struct svalue *, v)
{
    int ret;
    struct svalue val;

    val.type = T_NUMBER;
    switch(*cp++) {
    case '\"':
	if (ret = restore_string(cp, &val)) return ret;
	break;
    case '(':
	{
	    if (*cp == '{'){
		cp++;
		if (ret = restore_array(&cp, &val)) return ret;
	    } else if (*cp++ == '[') {
		if (ret = restore_mapping(&cp, &val)) return ret;
	    }
	    else return ROB_GENERAL_ERROR;
	    break;
	}
	
    case '-':
	{
	    char c;
	    
	    if ((c = *cp++) < '0' || (c > '9')) return ROB_NUMERAL_ERROR;
	    PARSE_NUMERIC((val.type = T_REAL, val.u.real = -f1-res),
			  (val.type = T_NUMBER, val.u.number = -res),
			  return ROB_NUMERAL_ERROR);
	}
	
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        {
	    char c = *(cp-1);
	    PARSE_NUMERIC((val.type = T_REAL, val.u.real = f1+res),
			  (val.type = T_NUMBER, val.u.number = res),
			  return ROB_NUMERAL_ERROR);
	}

    default:
	val.type = T_NUMBER;
	val.u.number = 0;
    }
    free_svalue(v, "safe_restore_svalue");
    *v = val;
    return 0;
}

static int var_index = 0;

static struct variable *find_status P1(char *, str) {
    int i;
    struct variable *vars = current_object->prog->p.i.variable_names;
    int n = current_object->prog->p.i.num_variables;
    
    if (str = findstring(str)) {
	for (i=var_index; i < n; i++) {
	    if (vars[i].name == str) {
		var_index = i+1;
		return &vars[i];
	    }
	}
	for (i=0; i < var_index; i++) {
	    if (vars[i].name == str) {
		var_index = i+1;
		return &vars[i];
	    }
	}
    }
    /* leave var_index alone; they might have just deleted this var */
    return 0;
}

void
restore_object_from_buff P4(struct object *, ob, char *, theBuff, char *, name,
			    int, noclear)
{
    char *buff, *nextBuff, *tmp,  *space;
    char var[100];
    struct variable *p, *var_start = ob->prog->p.i.variable_names;
    struct svalue *sv = ob->variables;
    int rc;
 
    var_index = 0;

    nextBuff = theBuff;
    while ((buff = nextBuff) && *buff) {
        struct svalue *v;
 
        if ((tmp = strchr(buff, '\n'))) {
            *tmp = '\0';
            nextBuff = tmp + 1;
        } else {
            nextBuff = 0;
        }
        if (buff[0] == '#') /* ignore 'comments' in savefiles */
            continue;
        space = strchr(buff, ' ');
        if (!space || ((space - buff) >= sizeof(var))) {
            FREE(name);
            FREE(theBuff);
            error("restore_object(): Illegal file format.\n");
        }
        (void)strncpy(var, buff, space - buff);
        var[space - buff] = '\0';
        p = find_status(var);
        if (!p || (p->type & TYPE_MOD_STATIC))
            continue;

        v = &sv[p - var_start];
	if (noclear)
	    rc = safe_restore_svalue(space+1, v);
	else
	    rc = restore_svalue(space+1, v);
	if (save_svalue_depth){
	    save_svalue_depth = max_depth = 0;
	    FREE((char *) sizes);
	    sizes = (int *) 0;
	}
        if (rc & ROB_ERROR) {
            FREE(name);
            FREE(theBuff);

	    if (rc & ROB_GENERAL_ERROR)
		error("restore_object(): Illegal general format while restoring %s.\n", var);
	    else if (rc & ROB_NUMERAL_ERROR)
		error("restore_object(): Illegal numeric format while restoring %s.\n", var);
	    else if (rc & ROB_ARRAY_ERROR)
                error("restore_object(): Illegal array format while restoring %s.\n", var);
            else if (rc & ROB_MAPPING_ERROR)
                error("restore_object(): Illegal mapping format while restoring %s.\n", var);
	    else if (rc & ROB_STRING_ERROR)
		error("restore_object(): Illegal string format while restoring %s.\n", var);
        }
    }
}

/*
 * Save an object to a file.
 * The routine checks with the function "valid_write()" in /obj/master.c
 * to assertain that the write is legal.
 * If 'save_zeros' is set, 0 valued variables will be saved
 */
int
save_object P3(struct object *, ob, char *, file, int, save_zeros)
{
    char *name;
    static char tmp_name[80];
    int len, i;
    FILE *f;
    int failed = 0;
    char *use_name, *new_string, *p;
    int free_use_name = 0, theSize;
    struct variable *var = ob->prog->p.i.variable_names;
    struct svalue *v = ob->variables;

    if (ob->flags & O_DESTRUCTED)
        return 0;
    if (file[0] != '/')
    {
	use_name = DXALLOC((len = strlen(file)) + 2, TAG_TEMPORARY, "save_object: 1");
	strcpy(use_name, "/");
	strcpy(use_name+1, file);
	free_use_name = 1;
    } else {
	use_name = file;
	len = 0;
    }

    file = check_valid_path(use_name, ob, "save_object", 1);

    if (free_use_name)
      FREE(use_name);

    if (file == 0)
        error("Denied write permission in save_object().\n");
    if (!len) len = strlen(file);
    name = DXALLOC(len + strlen(SAVE_EXTENSION) + 1, TAG_TEMPORARY, "save_object: 1");
    (void)strcpy(name, file);
#ifndef MSDOS
    (void)strcat(name + len, SAVE_EXTENSION);
#endif
    /*
     * Write the save-files to different directories, just in case
     * they are on different file systems.
     */
    sprintf(tmp_name, "%s.tmp", name);
#ifdef MSDOS
    (void)strcat(name, SAVE_EXTENSION);
#endif
    if (!(f = fopen(tmp_name, "w"))){
	FREE(name);
        error("Could not open %s for a save.\n", tmp_name);
    }
    fprintf(f, "#%s\n", ob->prog->name);

    i = ob->prog->p.i.num_variables;
    while (i--){
	if (var->type & TYPE_MOD_STATIC) { v++; var++; continue; }

	save_svalue_depth = 0;
	theSize = svalue_save_size(v);
	new_string = (char *)DXALLOC(theSize, TAG_TEMPORARY, "save_object: 2");
	*new_string = '\0';
	p = new_string;	
	save_svalue(v++, &p);
	if (save_zeros || strcmp(new_string,"0")) /* Armidale */
	    if (fprintf(f, "%s %s\n", var->name, new_string) == EOF) failed = 1;
	FREE(new_string);
	var++;
    }
    if (failed) add_message("Failed to completely save file. Disk could be full.\n");
    else {
	(void) fclose(f);
#ifdef OS2
        /* Need to erase it to write over it. */
        unlink(name);
#endif
	if (rename(tmp_name, name) < 0)
	{
#ifdef LATTICE
                /* AmigaDOS won't overwrite when renaming */
                if (errno == EEXIST) {
                    unlink(name);
                    if (rename(tmp_name, name) >= 0) {
                        FREE(name);
                        return 1;
                    }
                }
#endif
		perror(name);
		printf("Failed to rename %s to %s\n", tmp_name, name);
		add_message("Failed to save object!\n");
	}
    }
    FREE(name);
    if (failed) {   
	add_message("Failed to save to file. Disk could be full.\n");
	return 0;
    }
    return 1;
}


/*
 * return a string representing an svalue in the form that save_object()
 * would write it.
 */
char *
save_variable P1(struct svalue *, var)
{
    int theSize;
    char *new_string, *p;
    
    save_svalue_depth = 0;
    theSize = svalue_save_size(var);
    new_string = (char *)DXALLOC(theSize, TAG_STRING, "save_object: 2");
    *new_string = '\0';
    p = new_string;
    save_svalue(var, &p);
    return new_string;
}


int restore_object P3(struct object *, ob, char *, file, int, noclear)
{
    char *name, *theBuff;
    int len, i;
    FILE *f;
    struct object *save = current_object;
    struct stat st;

    if (ob->flags & O_DESTRUCTED)
        return 0;

    file = check_valid_path(file, ob, "restore_object", 0);
    if (!file) error("Denied read permission in restore_object().\n");

    len = strlen(file);
    name = DXALLOC(len + strlen(SAVE_EXTENSION) + 1, TAG_TEMPORARY, "restore_object: 2");
    (void)strcpy(name, file);
    if (name[len-2] == '.' &&
#ifndef LPC_TO_C
        name[len - 1] == 'c')
#else
        (name[len - 1] == 'c' || name[len - 1] == 'C'))
#endif
        name[len-2] = 0;
    (void)strcat(name, SAVE_EXTENSION);
#ifdef LATTICE
    f = NULL;
    if ((stat(name, &st) == -1) || !(f = fopen(name, "r"))) {
#else
    f = fopen(name, "r");
    if (!f || fstat(fileno(f), &st) == -1) {
#endif
        FREE(name);
        if (f) 
            (void)fclose(f);
        return 0;
    }

    if (!(i = st.st_size)) {
        (void)fclose(f);
        FREE(name);
        return 0;
    }
    theBuff = DXALLOC(i + 1, TAG_TEMPORARY, "restore_object: 4");
    fread(theBuff, 1, i, f);
    fclose(f);
    theBuff[i] = '\0';
    current_object = ob;
    
    /* This next bit added by Armidale@Cyberworld 1/1/93
     * If 'noclear' flag is not set, all non-static variables will be
     * initialized to 0 when restored.
     */
    if (!noclear) {
	struct variable *v = ob->prog->p.i.variable_names;
	struct svalue *sv = ob->variables;

	i = ob->prog->p.i.num_variables; 
	while (i--) {
	    if (!((v++)->type & TYPE_MOD_STATIC))
		assign_svalue(sv++, &const0n);
	    else sv++;
	}
    }
    
    restore_object_from_buff(ob, theBuff, name, noclear);
    current_object = save;
#ifdef DEBUg
    if (d_flag > 1)
        debug_message("Object %s restored from %s.\n", ob->name, name);
#endif
    FREE(name);
    FREE(theBuff);
    return 1;
}

void restore_variable P2(struct svalue *, var, char *, str)
{
    int rc;

    rc = restore_svalue(str, var);
    if (save_svalue_depth){
	save_svalue_depth = max_depth = 0;
	FREE((char *) sizes);
	sizes = (int *) 0;
    }
    if (rc & ROB_ERROR) {
	*var = const0; /* clean up */
	if (rc & ROB_GENERAL_ERROR)
	    error("restore_object(): Illegal general format.\n");
	else if (rc & ROB_NUMERAL_ERROR)
	    error("restore_object(): Illegal numeric format.\n");
	else if (rc & ROB_ARRAY_ERROR)
	    error("restore_object(): Illegal array format.\n");
	else if (rc & ROB_MAPPING_ERROR)
	    error("restore_object(): Illegal mapping format.\n");
	else if (rc & ROB_STRING_ERROR)
	    error("restore_object(): Illegal string format.\n");
    }
}

void tell_npc P2(struct object *, ob, char *, str)
{
    push_constant_string(str);
    (void) apply(APPLY_CATCH_TELL, ob, 1, ORIGIN_DRIVER);
}

 /* save some space snoop */
#define ALM_BREAK LARGEST_PRINTABLE_STRING - 10

static void add_long_message P1(char *, s) {
    char save;
    int len;

    len = strlen(s);
    while (len > ALM_BREAK) {
	save = s[ALM_BREAK];
	s[ALM_BREAK] = 0;
	add_message(s);
	s[ALM_BREAK] = save;
	s += ALM_BREAK;
	len -= ALM_BREAK;
    }
    add_message(s);
}

/*
 * tell_object: Send a message to an object.
 * If it is an interactive object, it will go to his
 * screen. Otherwise, it will go to a local function
 * catch_tell() in that object. This enables communications
 * between users and NPC's, and between other NPC's.
 * If INTERACTIVE_CATCH_TELL is defined then the message always
 * goes to catch_tell unless the target of tell_object is interactive
 * and is the current_object in which case it is written via add_message().
 */
void tell_object P2(struct object *, ob, char *, str)
{
    struct object *save_command_giver;
    
    save_command_giver = command_giver;
    if (!ob || (ob->flags & O_DESTRUCTED)) {
	command_giver = 0;
	add_long_message(str);
	command_giver = save_command_giver;
	return;
    }
#ifdef INTERACTIVE_CATCH_TELL
    tell_npc(ob, str);
    return;
#else
    if (ob->interactive) {
	command_giver = ob;
	add_long_message(str);
	command_giver = save_command_giver;
	return;
    }
    tell_npc(ob, str);
#endif
}

void free_object P2(struct object *, ob, char *, from)
{
#ifndef NO_ADD_ACTION
    struct sentence *s;
#endif

    ob->ref--;
#ifdef DEBUG
    if (d_flag > 1)
	printf("Subtr ref to ob %s: %u (%s)\n", ob->name,
	       ob->ref, from);
#endif
    debug(16384, ("subtr ref to ob %s: %d (%s)\n", ob->name, ob->ref, from));
    if (ob->ref > 0)
	return;
#ifdef DEBUG
    if (d_flag)
	printf("free_object: %s.\n", ob->name);
#endif
    if (!(ob->flags & O_DESTRUCTED)) {
	/* This is fatal, and should never happen. */
	fatal("FATAL: Object 0x%x %s ref count 0, but not destructed (from %s).\n",
	      ob, ob->name, from);
    }
    DEBUG_CHECK(ob->interactive,
		"Tried to free an interactive object.\n");
    /*
     * If the program is freed, then we can also free the variable
     * declarations.
     */
    if (ob->swap_num != -1)
	remove_swap_file(ob);	/* do this before prog is freed */
    if (ob->prog) {
	tot_alloc_object_size -=
	    (ob->prog->p.i.num_variables - 1) * sizeof(struct svalue) +
	    sizeof(struct object);
	free_prog(ob->prog, 1);
	ob->prog = 0;
    }
#ifndef NO_ADD_ACTION
    for (s = ob->sent; s;) {
	struct sentence *next;

	next = s->next;
	free_sentence(s);
	s = next;
    }
#endif
#ifdef PRIVS
    if (ob->privs)
	free_string(ob->privs);
#endif
    if (ob->name) {
#ifdef DEBUG
	if (d_flag > 1)
	    debug_message("Free object %s\n", ob->name);
#endif
	DEBUG_CHECK1(lookup_object_hash(ob->name) == ob,
		     "Freeing object %s but name still in name table", ob->name);
	FREE(ob->name);
	ob->name = 0;
    }
    tot_alloc_object--;
    FREE((char *) ob);
}

/*
 * Allocate an empty object, and set all variables to 0. Note that a
 * 'struct object' already has space for one variable. So, if no variables
 * are needed, we allocate a space that is smaller than 'struct object'. This
 * unused (last) part must of course (and will not) be referenced.
 */
struct object *get_empty_object P1(int, num_var)
{
    static struct object NULL_object;
    struct object *ob;
    int size = sizeof(struct object) +
    (num_var - !!num_var) * sizeof(struct svalue);
    int i;

    tot_alloc_object++;
    tot_alloc_object_size += size;
    ob = (struct object *) DXALLOC(size, TAG_OBJECT, "get_empty_object");
    /*
     * marion Don't initialize via memset, this is incorrect. E.g. the bull
     * machines have a (char *)0 which is not zero. We have structure
     * assignment, so use it.
     */
    *ob = NULL_object;
    ob->ref = 1;
    ob->swap_num = -1;
    for (i = 0; i < num_var; i++)
	ob->variables[i] = const0n;
    return ob;
}

#ifdef DEALLOCATE_MEMORY_AT_SHUTDOWN
static void remove_all_objects()
{
    struct object *ob;
    struct svalue v;

    v.type = T_OBJECT;
    while (1) {
	if (obj_list == 0)
	    break;
	ob = obj_list;
	v.u.ob = ob;
	destruct_object(&v);
	if (ob == obj_list)
	    break;
    }
    remove_destructed_objects();
}
#endif

#ifdef CHECK_OB_REF
/*
 * For debugging purposes.
 */
void check_ob_ref P2(struct object *, ob, char *, from)
{
    struct object *o;
    int i;

    for (o = obj_list, i = 0; o; o = o->next_all) {
	if (o->inherit == ob)
	    i++;
    }
    if (i + 1 > ob->ref) {
	fatal("FATAL too many references to inherited object %s (%d) from %s.\n",
	      ob->name, ob->ref, from);
	if (current_object)
	    fprintf(stderr, "current_object: %s\n", current_object->name);
	for (o = obj_list; o; o = o->next_all) {
	    if (o->inherit != ob)
		continue;
	    fprintf(stderr, "  %s\n", ob->name);
	}
    }
}
#endif				/* CHECK_OB_REF */

#ifndef NO_ADD_ACTION
static struct object *hashed_living[LIVING_HASH_SIZE];

static int num_living_names, num_searches = 1, search_length = 1;

static INLINE int hash_living_name P1(char *, str)
{
    return whashstr(str, 20) & (LIVING_HASH_SIZE - 1);
}

struct object *find_living_object P2(char *, str, int, user)
{
    struct object **obp, *tmp;
    struct object **hl;

    if (!str)
	return 0;
    num_searches++;
    hl = &hashed_living[hash_living_name(str)];
    for (obp = hl; *obp; obp = &(*obp)->next_hashed_living) {
	search_length++;
	if ((*obp)->flags & O_HIDDEN) {
	    if (!valid_hide(current_object))
		continue;
	}
	if (user && !((*obp)->flags & O_ONCE_INTERACTIVE))
	    continue;
	if (!((*obp)->flags & O_ENABLE_COMMANDS))
	    continue;
	if (strcmp((*obp)->living_name, str) == 0)
	    break;
    }
    if (*obp == 0)
	return 0;
    /* Move the found ob first. */
    if (obp == hl)
	return *obp;
    tmp = *obp;
    *obp = tmp->next_hashed_living;
    tmp->next_hashed_living = *hl;
    *hl = tmp;
    return tmp;
}

void set_living_name P2(struct object *, ob, char *, str)
{
    struct object **hl;

    if (ob->flags & O_DESTRUCTED)
	return;
    if (ob->living_name) {
	remove_living_name(ob);
    }
    num_living_names++;
    hl = &hashed_living[hash_living_name(str)];
    ob->next_hashed_living = *hl;
    *hl = ob;
    ob->living_name = make_shared_string(str);
    return;
}

void remove_living_name P1(struct object *, ob)
{
    struct object **hl;

    num_living_names--;
    if (!ob->living_name)
	fatal("remove_living_name: no living name set.\n");
    hl = &hashed_living[hash_living_name(ob->living_name)];
    while (*hl) {
	if (*hl == ob)
	    break;
	hl = &(*hl)->next_hashed_living;
    }
    if (*hl == 0)
	fatal("remove_living_name: Object named %s no in hash list.\n",
	      ob->living_name);
    *hl = ob->next_hashed_living;
    free_string(ob->living_name);
    ob->next_hashed_living = 0;
    ob->living_name = 0;
}

void stat_living_objects()
{
    add_message("Hash table of living objects:\n");
    add_message("-----------------------------\n");
    add_vmessage("%d living named objects, average search length: %4.2f\n",
		num_living_names, (double) search_length / num_searches);
}
#endif /* NO_ADD_ACTION */

void reset_object P1(struct object *, ob)
{
    struct object *save_command_giver;

    /* Be sure to update time first ! */
    ob->next_reset = current_time + TIME_TO_RESET / 2 +
	random_number(TIME_TO_RESET / 2);

    save_command_giver = command_giver;
    command_giver = (struct object *) 0;
    if (!apply(APPLY_RESET, ob, 0, ORIGIN_DRIVER)) {
	/* no reset() in the object */
	ob->flags &= ~O_WILL_RESET;	/* don't call it next time */
    }
    command_giver = save_command_giver;
    ob->flags |= O_RESET_STATE;
}

void call_create P2(struct object *, ob, int, num_arg)
{
    /* Be sure to update time first ! */
    ob->next_reset = current_time + TIME_TO_RESET / 2 +
	random_number(TIME_TO_RESET / 2);

    call___INIT(ob);

    if (ob->flags & O_DESTRUCTED) {
	pop_n_elems(num_arg);
	return; /* sigh */
    }

    apply(APPLY_CREATE, ob, num_arg, ORIGIN_DRIVER);

    ob->flags |= O_RESET_STATE;
}

/*
 * If there is a shadow for this object, then the message should be
 * sent to it. But only if catch_tell() is defined. Beware that one of the
 * shadows may be the originator of the message, which means that we must
 * not send the message to that shadow, or any shadows in the linked list
 * before that shadow.
 */
#ifndef NO_SHADOWS
int shadow_catch_message P2(struct object *, ob, char *, str)
{
    if (!ob->shadowed)
	return 0;
    while (ob->shadowed != 0 && ob->shadowed != current_object)
	ob = ob->shadowed;
    while (ob->shadowing) {
	if (function_exists(APPLY_CATCH_TELL, ob)) {
	    push_constant_string(str);
	    if (apply(APPLY_CATCH_TELL, ob, 1, ORIGIN_DRIVER))	
		/* this will work, since we know the */
		/* function is defined */
		return 1;
	}
	ob = ob->shadowing;
    }
    return 0;
}
#endif

INLINE int object_visible P1(struct object *, ob)
{
    if (ob->flags & O_HIDDEN) {
	if (current_object->flags & O_HIDDEN) {
	    return 1;
	}
	return valid_hide(current_object);
    } else {
	return 1;
    }
}

void reload_object P1(struct object *, obj)
{
    int i;

    if (!obj->prog)
	return;
    for (i = 0; i < (int) obj->prog->p.i.num_variables; i++) {
	free_svalue(&obj->variables[i], "reload_object");
	obj->variables[i] = const0n;
    }
#ifdef SOCKET_EFUNS
    if (obj->flags & O_EFUN_SOCKET) {
	close_referencing_sockets(obj);
    }
#endif
    /*
     * If this is the first object being shadowed by another object, then
     * destruct the whole list of shadows.
     */
#ifndef NO_SHADOWS
    if (obj->shadowed && !obj->shadowing) {
	struct svalue svp;
	struct object *ob2;

	svp.type = T_OBJECT;
	for (ob2 = obj->shadowed; ob2;) {
	    svp.u.ob = ob2;
	    ob2 = ob2->shadowed;
	    svp.u.ob->shadowed = 0;
	    svp.u.ob->shadowing = 0;
	    destruct_object(&svp);
	}
    }
    /*
     * The chain of shadows is a double linked list. Take care to update it
     * correctly.
     */
    if (obj->shadowing)
	obj->shadowing->shadowed = obj->shadowed;
    if (obj->shadowed)
	obj->shadowed->shadowing = obj->shadowing;
    obj->shadowing = 0;
    obj->shadowed = 0;
#endif
#ifndef NO_ADD_ACTION
    if (obj->living_name)
	remove_living_name(obj);
    obj->flags &= ~O_ENABLE_COMMANDS;
#endif
    set_heart_beat(obj, 0);
    remove_all_call_out(obj);
#ifndef NO_LIGHT
    add_light(obj, -(obj->total_light));
#endif
#ifndef NO_UIDS
#ifdef AUTO_SETEUID
    obj->euid = obj->uid;
#else
    obj->euid = NULL;
#endif
#endif
    call_create(obj, 0);
}
