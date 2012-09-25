#include "std.h"
#include "lpc_incl.h"
#include "file_incl.h"
#include "otable.h"
#include "backend.h"
#include "comm.h"
#include "socket_efuns.h"
#include "call_out.h"
#include "port.h"
#include "file.h"
#include "hash.h"
#include "master.h"
#include "add_action.h"

#define too_deep_save_error() \
    error("Mappings and/or arrays nested too deep (%d) for save_object\n",\
          MAX_SAVE_SVALUE_DEPTH);

object_t *previous_ob;
int tot_alloc_object, tot_alloc_object_size;

char *save_mapping (mapping_t *m);
INLINE_STATIC int restore_array (char **str, svalue_t *);
INLINE_STATIC int restore_class (char **str, svalue_t *);

#ifdef F_SET_HIDE
int num_hidden = 0;

INLINE int
valid_hide (object_t * obj)
{
    svalue_t *ret;

    if (!obj) {
        return 0;
    }
    push_object(obj);
    ret = apply_master_ob(APPLY_VALID_HIDE, 1);
    return (!IS_ZERO(ret));
}
#endif

int save_svalue_depth = 0, max_depth;
int *sizes = 0;

INLINE int svalue_save_size (svalue_t * v)
{
    switch(v->type) {
    case T_STRING:
        {
            register const char *cp = v->u.string;
            char c;
            int size = 0;

            while ((c = *cp++)) {
                if (c == '\\' || c == '"') size++;
                size++;
            }
            return 3 + size;
        }

    case T_ARRAY:
        {
            svalue_t *sv = v->u.arr->item;
            int i = v->u.arr->size, size = 0;

            if (++save_svalue_depth > MAX_SAVE_SVALUE_DEPTH) {
                save_svalue_depth = 0;
                too_deep_save_error();
            }
            while (i--) size += svalue_save_size(sv++);
            save_svalue_depth--;
            return size + 5;
        }

    case T_CLASS:
        {
            svalue_t *sv = v->u.arr->item;
            int i = v->u.arr->size, size = 0;

            if (++save_svalue_depth > MAX_SAVE_SVALUE_DEPTH) {
                save_svalue_depth = 0;
                too_deep_save_error();
            }
            while (i--) size += svalue_save_size(sv++);
            save_svalue_depth--;
            return size + 5;
        }

    case T_MAPPING:
        {
            mapping_node_t **a = v->u.map->table, *elt;
            int j = v->u.map->table_size, size = 0;

            if (++save_svalue_depth > MAX_SAVE_SVALUE_DEPTH) {
                save_svalue_depth = 0;
                too_deep_save_error();
            }
            do {
                for (elt = a[j]; elt; elt = elt->next) {
                    size += svalue_save_size(elt->values) +
                            svalue_save_size(elt->values+1);
                }
            } while (j--);
            save_svalue_depth--;
            return size + 5;
        }

    case T_NUMBER:
        {
            long res = v->u.number, len;
#if SIZEOF_LONG == 4
	      len = res < 0 ? (res = (-res) & 0x7fffffff,3) : 2;
#else
	      len = res < 0 ? (res = (-res) & 0x7fffffffffffffff,3) : 2;
#endif
            while (res>9) {
		res /= 10;
		len++;
	    }
            return len;
        }

    case T_REAL:
        {
            char buf[256];
            sprintf(buf, "%f", v->u.real);
            return (strlen(buf)+1);
        }

    default:
        {
            return 1;
        }
    }
}

INLINE void save_svalue (svalue_t * v, char ** buf)
{
    switch(v->type) {
    case T_STRING:
        {
            register char *cp = *buf;
            const char *str = v->u.string;
            char c;

            *cp++ = '"';
            while ((c = *str++)) {
                if (c == '"' || c == '\\') {
                    *cp++ = '\\';
                    *cp++ = c;
                }
                else *cp++ = (c == '\n') ? '\r' : c;
            }

            *cp++ = '"';
            *(*buf = cp) = '\0';
            return;
        }

    case T_ARRAY:
        {
            int i = v->u.arr->size;
            svalue_t *sv = v->u.arr->item;

            *(*buf)++ = '(';
            *(*buf)++ = '{';
            while (i--) {
                save_svalue(sv++, buf);
                *(*buf)++ = ',';
            }
            *(*buf)++ = '}';
            *(*buf)++ = ')';
            *(*buf) = '\0';
            return;
        }

    case T_CLASS:
        {
            int i = v->u.arr->size;
            svalue_t *sv = v->u.arr->item;

            *(*buf)++ = '(';
            *(*buf)++ = '/';  /* Why yes, this *is* a kludge! */
            while (i--) {
                save_svalue(sv++, buf);
                *(*buf)++ = ',';
            }
            *(*buf)++ = '/';
            *(*buf)++ = ')';
            *(*buf) = '\0';
            return;
        }

    case T_NUMBER:
        {
	    long res = v->u.number, fact;
	    int len = 1, neg = 0;
            register char *cp;
            if (res < 0) {
		len++;
		neg = 1;
#if SIZEOF_LONG == 4
		res = (-res) & 0x7fffffff;
#else
		res = (-res) & 0x7fffffffffffffff;
#endif
	    }

            fact = res;
            while (fact > 9) {
		fact /= 10;
		len++;
	    }
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
            sprintf(*buf, "%f", v->u.real);
            (*buf) += strlen(*buf);
            return;
        }

    case T_MAPPING:
        {
            int j = v->u.map->table_size;
            mapping_node_t **a = v->u.map->table, *elt;

            *(*buf)++ = '(';
            *(*buf)++ = '[';
            do {
                for (elt = a[j]; elt; elt = elt->next) {
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

INLINE_STATIC int
restore_internal_size (const char ** str, int is_mapping, int depth)
{
    register const char *cp = *str;
    int size = 0;
    char c, delim, toggle = 0;

    delim = is_mapping ? ':' : ',';
    while ((c = *cp++)) {
        switch(c) {
        case '"':
            {
                while ((c = *cp++) != '"')
                    if ((c == '\0') || (c == '\\' && !*cp++)) {
                        return 0;
                    }
                if (*cp++ != delim) return 0;
                size++;
                break;
            }

        case '(':
            {
                if (*cp == '{') {
                    *str = ++cp;
                    if (!restore_internal_size(str, 0, save_svalue_depth++)) {
                        return 0;
                    }
                }
                else if (*cp == '[') {
                    *str = ++cp;
                    if (!restore_internal_size(str, 1, save_svalue_depth++)) { return 0;}
                }
                else if (*cp == '/') {
                    *str = ++cp;
                    if (!restore_internal_size(str, 0, save_svalue_depth++))
                        return 0;
                } else { return 0;}

                if (*(cp = *str) != delim) { return 0;}
                cp++;
                size++;
                break;
            }

        case ']':
            {
                if (*cp++ == ')' && is_mapping) {
                    *str = cp;
                    if (!sizes) {
                        max_depth = 128;
                        while (max_depth <= depth) max_depth <<= 1;
                        sizes = CALLOCATE(max_depth, int, TAG_TEMPORARY,
                                          "restore_internal_size");
                    }
                    else if (depth >= max_depth) {
                        while ((max_depth <<= 1) <= depth);
                        sizes = RESIZE(sizes, max_depth, int, TAG_TEMPORARY,
                                       "restore_internal_size");
                    }
                    sizes[depth] = size;
                    return 1;
                }
                else { return 0; }
            }

        case '/':
        case '}':
            {
                if (*cp++ == ')' && !is_mapping) {
                    *str = cp;
                    if (!sizes) {
                        max_depth = 128;
                        while (max_depth <= depth) max_depth <<= 1;
                        sizes = CALLOCATE(max_depth, int, TAG_TEMPORARY,
                                          "restore_internal_size");
                    }
                    else if (depth >= max_depth) {
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
        if (is_mapping) delim = (toggle ^= 1) ? ',' : ':';
    }
    return 0;
}



INLINE_STATIC int
restore_size (const char ** str, int is_mapping)
{
    register const char *cp = *str;
    int size = 0;
    char c, delim, toggle = 0;

    delim = is_mapping ? ':' : ',';

    while ((c = *cp++)) {
        switch(c) {
        case '"':
            {
                while ((c = *cp++) != '"')
                    if ((c == '\0') || (c == '\\' && !*cp++)) return 0;

                if (*cp++ != delim) { return -1; }
                size++;
                break;
            }

        case '(':
            {
                if (*cp == '{') {
                    *str = ++cp;
                    if (!restore_internal_size(str, 0, save_svalue_depth++)) return -1;
                }
                else if (*cp == '[') {
                    *str = ++cp;
                    if (!restore_internal_size(str, 1, save_svalue_depth++)) return -1;
                }
                else if (*cp == '/') {
                    *str = ++cp;
                    if (!restore_internal_size(str, 0, save_svalue_depth++)) return -1;
                } else { return -1; }

                if (*(cp = *str) != delim) { return -1;}
                cp++;
                size++;
                break;
            }

        case ']':
            {
                save_svalue_depth = 0;
                if (*cp++ == ')' && is_mapping) {
                    *str = cp;
                    return size;
                }
                else { return -1;}
            }

        case '/':
        case '}':
            {
                save_svalue_depth = 0;
                if (*cp++ == ')' && !is_mapping) {
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
        if (is_mapping) delim = (toggle ^= 1) ? ',' : ':';
    }
    return -1;
}

INLINE_STATIC int
restore_interior_string (char ** val, svalue_t * sv)
{
    register char *cp = *val;
    char *start = cp, *newstr;
    char c;
    int len;

    while ((c = *cp++) != '"') {
        switch (c) {
        case '\r':
            {
                *(cp-1) = '\n';
                break;
            }

        case '\\':
            {
                char *news = cp - 1;

                if ((*news++ = *cp++)) {
                    while ((c = *cp++) != '"') {
                        if (c == '\\') {
                            if (!(*news++ = *cp++)) return ROB_STRING_ERROR;
                        }
                        else {
                            if (c == '\r')
                                *news++ = '\n';
                            else *news++ = c;
                        }
                    }
                    if (c == '\0') return ROB_STRING_ERROR;
                    *news = '\0';
                    *val = cp;
                    newstr = new_string(len = (news - start),
                                              "restore_string");
                    strcpy(newstr, start);
                    sv->u.string = newstr;
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
    newstr = new_string(len, "restore_string");
    strcpy(newstr, start);
    sv->u.string = newstr;
    sv->type = T_STRING;
    sv->subtype = STRING_MALLOC;
    return 0;
}

static int parse_numeric (char ** cpp, unsigned char c, svalue_t * dest)
{
    char *cp = *cpp;
    long res, neg;

    if (c == '-') {
        neg = 1;
        res = 0;
        c = *cp++;
        if (!isdigit(c))
            return 0;
    } else
      neg = 0;
    res = c - '0';

    while ((c = *cp++) && isdigit(c)) {
        res *= 10;
        res += c - '0';
    }
    if (c == '.') {
        double f1 = 0.0, f2 = 10.0;

        c = *cp++;
        if (!c) {
            cp--;
            c = '0';
        }
        if (!isdigit(c)) return 0;

        do {
            f1 += (c - '0')/f2;
            f2 *= 10;
        } while ((c = *cp++) && isdigit(c));

        f1 += res;
        if (c == 'e') {
            int expo = 0;

            if ((c = *cp++) == '+') {
                while ((c = *cp++) && isdigit(c)) {
                    expo *= 10;
                    expo += (c - '0');
                }
                f1 *= pow(10.0, expo);
            } else if (c == '-') {
                while ((c = *cp++) && isdigit(c)) {
                    expo *= 10;
                    expo += (c - '0');
                }
                f1 *= pow(10.0, -expo);
            } else
                return 0;
        }

        dest->type = T_REAL;
        dest->u.real = (neg ? -f1 : f1);
        *cpp = cp;
        return 1;
    } else if (c == 'e') {
        int expo = 0;
        double f1;

        if ((c = *cp++) == '+') {
            while ((c = *cp++) && isdigit(c)) {
                expo *= 10;
                expo += (c - '0');
            }
            f1 = res * pow(10.0, expo);
        } else if (c == '-') {
            while ((c = *cp++) && isdigit(c)) {
                expo *= 10;
                expo += (c - '0');
            }
            f1 = res * pow(10.0, -expo);
        } else
            return 0;

        dest->type = T_REAL;
        dest->u.real = (neg ? -f1 : f1);
        *cpp = cp;
        return 1;
    } else {
        dest->type = T_NUMBER;
        dest->u.number = (neg ? -res : res);
        dest->subtype = 0;
        *cpp = cp;
        return 1;
    }
}

INLINE_STATIC void add_map_stats (mapping_t * m, int count)
{
    total_mapping_nodes += count;
    total_mapping_size += count * sizeof(mapping_node_t);
#ifdef PACKAGE_MUDLIB_STATS
    add_array_size(&m->stats, count << 1);
#endif
    m->count = count;
}

static int
restore_mapping (char **str, svalue_t * sv)
{
    int size, i, mask, count = 0;
    unsigned long oi;
    char c;
    mapping_t *m;
    svalue_t key, value;
    mapping_node_t **a, *elt, *elt2;
    char *cp = *str;
    int err;

    if (save_svalue_depth) size = sizes[save_svalue_depth-1];
    else if ((size = restore_size((const char **)str, 1)) < 0) return 0;

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
                if ((err = restore_hash_string(str, &key)))
                    goto key_error;
                cp = *str;
                cp++;
                break;
            }

        case '(':
            {
                save_svalue_depth++;
                if (*cp == '[') {
                    *str = ++cp;
                    if ((err = restore_mapping(str, &key)))
                        goto key_error;
                }
                else if (*cp == '{') {
                    *str = ++cp;
                    if ((err = restore_array(str, &key)))
                        goto key_error;
                }
                else if (*cp == '/') {
                    *str = ++cp;
                    if ((err = restore_class(str, &key)))
                        goto key_error;
                }
                else goto generic_key_error;
                cp = *str;
                cp++;
                break;
            }

        case ':':
            {
                key.u.number = 0;
                key.type = T_NUMBER;
		key.subtype = 0;
                break;
            }

        case ']':
            *str = ++cp;
            add_map_stats(m, count);
            sv->type = T_MAPPING;
            sv->u.map = m;
            return 0;

        case '-':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            if (!parse_numeric(&cp, c, &key))
                goto key_numeral_error;
            break;

        default:
            goto generic_key_error;
        }

        /* At this point, key is a valid, referenced svalue and we're
           responsible for it */

        switch (c = *cp++) {
        case '"':
            {
                *str = cp;
                if ((err = restore_interior_string(str, &value)))
                    goto value_error;
                cp = *str;
                cp++;
                break;
            }

        case '(':
            {
                save_svalue_depth++;
                if (*cp == '[') {
                    *str = ++cp;
                    if ((err = restore_mapping(str, &value)))
                        goto value_error;
                }
                else if (*cp == '{') {
                    *str = ++cp;
                    if ((err = restore_array(str, &value)))
                        goto value_error;
                } else if (*cp == '/') {
                    *str = ++cp;
                    if ((err = restore_class(str, &value)))
                        goto value_error;
                }
                else goto generic_value_error;
                cp = *str;
                cp++;
                break;
            }

        case '-':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            if (!parse_numeric(&cp, c, &value))
                goto value_numeral_error;
            break;

        case ',':
            {
                value.u.number = 0;
                value.type = T_NUMBER;
                value.subtype = 0;
                break;
            }

        default:
            goto generic_value_error;
        }

        /* both key and value are valid, referenced svalues */

        oi = MAP_SVAL_HASH(key);
        i = oi & mask;
        if ((elt2 = elt = a[i])) {
            do {
                /* This should never happen, but don't bail on it */
                if (msameval(&key, elt->values)) {
                    free_svalue(&key, "restore_mapping: duplicate key");
                    free_svalue(elt->values+1, "restore_mapping: replaced value");
                    *(elt->values+1) = value;
                    break;
                }
            } while ((elt = elt->next));
            if (elt)
                continue;
        } else if (!(--m->unfilled)) {
            if (growMap(m)) {
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

        elt = new_map_node();
        *elt->values = key;
        *(elt->values + 1) = value;
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


INLINE_STATIC int
restore_class (char ** str, svalue_t * ret)
{
    int size;
    char c;
    array_t *v;
    svalue_t *sv;
    char *cp = *str;
    int err;

    if (save_svalue_depth) size = sizes[save_svalue_depth-1];
    else if ((size = restore_size((const char **)str,0)) < 0) return ROB_CLASS_ERROR;

    v = allocate_class_by_size(size); /* after this point we have to clean up
                                         or we'll leak */
    sv = v->item;

    while (size--) {
        switch (c = *cp++) {
        case '"':
            *str = cp;
            if ((err = restore_interior_string(str, sv)))
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
                if (*cp == '[') {
                    *str = ++cp;
                    if ((err = restore_mapping(str, sv)))
                        goto error;
                }
                else if (*cp == '{') {
                    *str = ++cp;
                    if ((err = restore_array(str, sv)))
                        goto error;
                }
                else if (*cp == '/') {
                    *str = ++cp;
                    if ((err = restore_class(str, sv)))
                        goto error;
                }
                else goto generic_error;
                sv++;
                cp = *str;
                cp++;
                break;
            }

        case '-':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            if (parse_numeric(&cp, c, sv))
                sv++;
            else
                goto numeral_error;
            break;

        default:
            goto generic_error;
        }
    }

    cp += 2;
    *str = cp;
    ret->u.arr = v;
    ret->type = T_CLASS;
    return 0;
    /* something went wrong */
 numeral_error:
    err = ROB_NUMERAL_ERROR;
    goto error;
 generic_error:
    err = ROB_CLASS_ERROR;
 error:
    free_class(v);
    return err;
}

INLINE_STATIC int
restore_array (char ** str, svalue_t * ret)
{
    int size;
    char c;
    array_t *v;
    svalue_t *sv;
    char *cp = *str;
    int err;

    if (save_svalue_depth) size = sizes[save_svalue_depth-1];
    else if ((size = restore_size((const char **)str,0)) < 0) return ROB_ARRAY_ERROR;

    v = allocate_array(size); /* after this point we have to clean up
                                 or we'll leak */
    sv = v->item;

    while (size--) {
        switch (c = *cp++) {
        case '"':
            *str = cp;
            if ((err = restore_interior_string(str, sv)))
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
                if (*cp == '[') {
                    *str = ++cp;
                    if ((err = restore_mapping(str, sv)))
                        goto error;
                }
                else if (*cp == '{') {
                    *str = ++cp;
                    if ((err = restore_array(str, sv)))
                        goto error;
                }
                else if (*cp == '/') {
                    *str = ++cp;
                    if ((err = restore_class(str, sv)))
                        goto error;
                }
                else goto generic_error;
                sv++;
                cp = *str;
                cp++;
                break;
            }

        case '-':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            if (parse_numeric(&cp, c, sv))
                sv++;
            else
                goto numeral_error;
            break;

        default:
            goto generic_error;
        }
    }

    cp += 2;
    *str = cp;
    ret->u.arr = v;
    ret->type = T_ARRAY;
    return 0;
    /* something went wrong */
 numeral_error:
    err = ROB_NUMERAL_ERROR;
    goto error;
 generic_error:
    err = ROB_ARRAY_ERROR;
 error:
    free_array(v);
    return err;
}

INLINE_STATIC int
restore_string (char * val, svalue_t * sv)
{
    register char *cp = val;
    char *start = cp, *newstr;
    char c;
    int len;

    while ((c = *cp++) != '"') {
        switch (c) {
        case '\r':
            {
                *(cp-1) = '\n';
                break;
            }

        case '\\':
            {
                char *news = cp - 1;
                if ((*news++ = *cp++)) {
                    while ((c = *cp++) != '"' && c) {
                        if (c == '\\') {
                            if (!(*news++ = *cp++))
                            	return ROB_STRING_ERROR;
                        } else {
                            if (c == '\r')
                                *news++ = '\n';
                            else
                            	*news++ = c;
                        }
                    }
                    if ((c == '\0') || (*cp != '\0'))
                    	return ROB_STRING_ERROR;
                    *news = '\0';
                    newstr = new_string(news - start, "restore_string");
                    strcpy(newstr, start);
                    sv->u.string = newstr;
                    sv->type = T_STRING;
                    sv->subtype = STRING_MALLOC;
                    return 0;
                }
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
    newstr = new_string(len, "restore_string");
    strcpy(newstr, start);
    sv->u.string = newstr;
    sv->type = T_STRING;
    sv->subtype = STRING_MALLOC;
    return 0;
}

/* for this case, the variable in question has been set to zero already,
   and we don't have to worry about preserving it */
INLINE int
restore_svalue (char * cp, svalue_t * v)
{
    int ret;
    char c;

    switch (c = *cp++) {
    case '"':
        return restore_string(cp, v);
    case '(':
        if (*cp == '{') {
            cp++;
            ret = restore_array(&cp, v);
        } else if (*cp == '[') {
            cp++;
            ret = restore_mapping(&cp, v);
        } else if (*cp++ == '/') {
            ret = restore_class(&cp, v);
        }
        else ret = ROB_GENERAL_ERROR;

        if (save_svalue_depth) {
            save_svalue_depth = max_depth = 0;
            if (sizes)
                FREE((char *) sizes);
            sizes = (int *) 0;
        }
        return ret;

    case '-':
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        if (!parse_numeric(&cp, c, v))
            return ROB_NUMERAL_ERROR;
        break;

    default:
        v->type = T_NUMBER;
        v->u.number = 0;
    }

    return 0;
}

/* for this case, we're being careful and want to leave the value alone on
   an error */
INLINE_STATIC int
safe_restore_svalue (char * cp, svalue_t * v)
{
    int ret;
    svalue_t val;
    char c;

    val.type = T_NUMBER;
    switch (c = *cp++) {
    case '"':
        if ((ret = restore_string(cp, &val))) return ret;
        break;
    case '(':
        {
            if (*cp == '{') {
                cp++;
                ret = restore_array(&cp, &val);
            } else if (*cp == '[') {
                cp++;
                ret = restore_mapping(&cp, &val);
            } else if (*cp++ == '/') {
                ret = restore_class(&cp, &val);
            }
            else return ROB_GENERAL_ERROR;

            if (save_svalue_depth) {
                save_svalue_depth = max_depth = 0;
                if (sizes)
                    FREE((char *) sizes);
                sizes = (int *) 0;
            }
            if (ret)
                return ret;
            break;
        }

    case '-':
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        if (!parse_numeric(&cp, c, &val))
            return ROB_NUMERAL_ERROR;
        break;

    default:
        val.type = T_NUMBER;
        val.u.number = 0;
    }
    free_svalue(v, "safe_restore_svalue");
    *v = val;
    return 0;
}

static int fgv_recurse (program_t * prog, int * idx,
                          char * name, unsigned short * type,
                          int check_nosave) {
    int i;
    for (i = 0; i < prog->num_inherited; i++) {
        if (fgv_recurse(prog->inherit[i].prog, idx, name, type, check_nosave)) {
            *type = DECL_MODIFY(prog->inherit[i].type_mod, *type);

            return 1;
        }
    }
    for (i = 0; i < prog->num_variables_defined; i++) {
        if (prog->variable_table[i] == name &&
            (!check_nosave || !(prog->variable_types[i] & DECL_NOSAVE))) {
            *idx += i;
            *type = prog->variable_types[i];
            return 1;
        }
    }
    *idx += prog->num_variables_defined;
    return 0;
}

int find_global_variable (program_t * prog, const char * const name,
                            unsigned short * type, int check_nosave) {
    int idx = 0;
    char *str = findstring(name);

    if (str && fgv_recurse(prog, &idx, str, type, check_nosave))
        return idx;

    return -1;
}

void
restore_object_from_line (object_t * ob, char * line, int noclear)
{
    char *space;
    svalue_t *v;
    char var[100];
    int idx;
    svalue_t *sv = ob->variables;
    int rc;
    unsigned short t;


    if (line[0] == '#') { /* ignore 'comments' in savefiles */
        return ;
    }
    space = strchr(line, ' ');
    if (!space || ((space - line) >= sizeof(var))) {
        error("restore_object(): Illegal file format - 1 (%s).\n", line);
    }
    (void)strncpy(var, line, space - line);
    var[space - line] = '\0';
    idx = find_global_variable(current_object->prog, var, &t, 1);
    if (idx == -1) {
        push_number(0);
	rc = restore_svalue(space+1, sp);
    } else {

      v = &sv[idx];
      if (noclear) {
        rc = safe_restore_svalue(space+1, v);
      } else {
        rc = restore_svalue(space+1, v);
      }
    }
    if (rc & ROB_ERROR) {
        if (rc & ROB_GENERAL_ERROR) {
            error("restore_object(): Illegal general format while restoring %s.\n", var);
        } else if (rc & ROB_NUMERAL_ERROR) {
            error("restore_object(): Illegal numeric format while restoring %s.\n", var);
        } else if (rc & ROB_ARRAY_ERROR) {
                   error("restore_object(): Illegal array format while restoring %s.\n", var);
        } else if (rc & ROB_MAPPING_ERROR) {
                   error("restore_object(): Illegal mapping format while restoring %s.\n", var);
        } else if (rc & ROB_STRING_ERROR) {
            error("restore_object(): Illegal string format while restoring %s.\n", var);
        } else if (rc & ROB_CLASS_ERROR) {
            error("restore_object(): Illegal class format while restoring %s.\n", var);
        }
    }
    if(idx == -1){
      copy_and_push_string(var);
      apply("restore_lost_variable", ob, 2, ORIGIN_DRIVER);
    }
}

#ifdef HAVE_ZLIB
int
restore_object_from_gzip (object_t * ob,
                            gzFile gzf,
                            int noclear, int *count)
{
    static char *buff = NULL;
    static long buffsize = 0;
    const char* tmp = "";
    int idx;
    int t;

    t = 65536 << *count; //should be big enough most of the time
    if (buff && buffsize < t) {
        FREE(buff);
        buff = NULL;
    }

    if(!buff){
      buff = (char *)DXALLOC(t, TAG_TEMPORARY, "restore_object: 6");
	buffsize = t;
    }

    t = buffsize;

    while (!gzeof(gzf) && tmp != Z_NULL) {
        idx = 0;
        buff[t - 2] = 0;
        // gzgets appaears to pay attension to zero termination even on short
        // strings
        buff[0] = 0;
        tmp = gzgets(gzf, buff, t);

        if (buff[t - 2] != 0 && buff[t - 2] != '\n' && !gzeof(gzf)) {
           return -1; //retry with bigger buffer
        }

        if (buff[0]) {
            char *tmp2 = strchr(buff, '\n');
            if (tmp2) {
                *tmp2 = '\0';
                if (tmp2 > buff && tmp2[-1] == '\r') {
                    *(--tmp2) = '\0';
                }
            }
            restore_object_from_line(ob, buff, noclear);
        }
    }
    return 0;
}
#endif

void
restore_object_from_buff (object_t * ob, char * theBuff,
                            int noclear)
{
    char *buff, *nextBuff, *tmp;

    nextBuff = theBuff;
    while ((buff = nextBuff) && *buff) {

        if ((tmp = strchr(buff, '\n'))) {
            *tmp = '\0';
            if (tmp > buff && tmp[-1] == '\r')
                *(--tmp) = '\0';
            nextBuff = tmp + 1;
        } else {
            nextBuff = 0;
        }
        restore_object_from_line(ob, buff, noclear);
    }
}


/*
 * Save an object to a file.
 * The routine checks with the function "valid_write()" in /obj/master.c
 * to assertain that the write is legal.
 * If 'save_zeros' is set, 0 valued variables will be saved
 */
#ifdef HAVE_ZLIB
static int save_object_recurse (program_t * prog, svalue_t **
                                  svp, int type, int save_zeros,
                                  FILE * f, gzFile gzf)
#else
static int save_object_recurse (program_t * prog, svalue_t **
                                  svp, int type, int save_zeros,
                                  FILE * f)
#endif
{
    int i;
    int textsize = 1;
    int tmp;
    int theSize;
    int oldSize;
    char *new_str, *p;

    for (i = 0; i < prog->num_inherited; i++) {
#ifdef HAVE_ZLIB
        if (!(tmp = save_object_recurse(prog->inherit[i].prog, svp,
                                 prog->inherit[i].type_mod | type,
                                 save_zeros, f, gzf)))
#else

        if (!(tmp = save_object_recurse(prog->inherit[i].prog, svp,
                                 prog->inherit[i].type_mod | type,
                                 save_zeros, f)))
#endif
            return 0;
        textsize += tmp;
    }
    if (type & DECL_NOSAVE) {
        (*svp) += prog->num_variables_defined;
        return 1;
    }
    oldSize = -1;
    new_str = NULL;
    for (i = 0; i < prog->num_variables_defined; i++) {
        if (prog->variable_types[i] & DECL_NOSAVE) {
            (*svp)++;
            continue;
        }
        save_svalue_depth = 0;
        theSize = svalue_save_size(*svp);

        // Try not to malloc/free too much.
        if (theSize > oldSize) {
            if (new_str) {
                FREE(new_str);
            }
        new_str = (char *)DXALLOC(theSize, TAG_TEMPORARY, "save_object: 2");
            oldSize = theSize;
        }

        *new_str = '\0';
        p = new_str;
        save_svalue((*svp)++, &p);
        DEBUG_CHECK(p - new_str != theSize - 1, "Length miscalculated in save_object!");
        /* FIXME: shouldn't use fprintf() */
        if (save_zeros || new_str[0] != '0' || new_str[1] != 0) { /* Armidale */
            textsize += theSize;
            textsize += strlen(prog->variable_table[i]);
            textsize += 2;
#ifdef HAVE_ZLIB
            if (gzf) {
                gzputs(gzf, prog->variable_table[i]);
                gzputs(gzf, " ");
                gzputs(gzf, new_str);
                gzputs(gzf, "\n");
            } else
#endif
            {
            if (fprintf(f, "%s %s\n", prog->variable_table[i], new_str) < 0) {
                debug_perror("save_object: fprintf", 0);
                FREE(new_str);
                return 0;
            }
            }
        }
    }
    if (new_str) {
        FREE(new_str);
    }
    return textsize;
}

/*
 * Save an object to a file.
 * The routine checks with the function "valid_write()" in /obj/master.c
 * to assertain that the write is legal.
 * If 'save_zeros' is set, 0 valued variables will be saved
 */

static int save_object_recurse_str (program_t * prog, svalue_t **svp, int type, int save_zeros, char *buf, int bufsize){
    int i;
    int textsize = 1;
    int tmp;
    int theSize;
    int oldSize;
    char *new_str, *p;

    for (i = 0; i < prog->num_inherited; i++) {
        if (!(tmp = save_object_recurse_str(prog->inherit[i].prog, svp,
                                 prog->inherit[i].type_mod | type,
                                 save_zeros, buf+textsize-1, bufsize)))
            return 0;
        textsize += tmp - 1;
    }
    if (type & DECL_NOSAVE) {
        (*svp) += prog->num_variables_defined;
        return 1;
    }
    oldSize = -1;
    new_str = NULL;
    for (i = 0; i < prog->num_variables_defined; i++) {
        if (prog->variable_types[i] & DECL_NOSAVE) {
            (*svp)++;
            continue;
        }
        save_svalue_depth = 0;
        theSize = svalue_save_size(*svp);
        if(textsize+theSize + 2 +  strlen(prog->variable_table[i])> bufsize)
        	return 0;
        // Try not to malloc/free too much.

        if (theSize > oldSize) {
            if (new_str) {
                FREE(new_str);
            }
        new_str = (char *)DXALLOC(theSize, TAG_TEMPORARY, "save_object: 2");
            oldSize = theSize;
        }

        *new_str = '\0';
        p = new_str;
        save_svalue((*svp)++, &p);
        DEBUG_CHECK(p - new_str != theSize - 1, "Length miscalculated in save_object!");
        if (save_zeros || new_str[0] != '0' || new_str[1] != 0) { /* Armidale */
        	if (sprintf(buf+textsize-1, "%s %s\n", prog->variable_table[i], new_str) < 0) {
        		debug_perror("save_object: fprintf", 0);
        	    FREE(new_str);
        	    return 0;
        	}
        	textsize += theSize;
            textsize += strlen(prog->variable_table[i]);
            textsize ++;

        }
    }
    if (new_str) {
        FREE(new_str);
    }
    return textsize;
}

int sel = -1;

#ifdef HAVE_ZLIB
int gz_sel = -1;
#endif

int
save_object (object_t * ob, const char * file, int save_zeros)
{
    char *name, *p;
    static char save_name[256], tmp_name[256];
    int len;
    FILE *f;
    int success;
    svalue_t *v;
#ifdef HAVE_ZLIB
    gzFile gzf;
    int save_compressed;

    if (save_zeros & 2) {
        save_compressed = 1;
        save_zeros &= ~2;
    } else {
        save_compressed = 0;
    }
#endif

    if (ob->flags & O_DESTRUCTED)
        return 0;

    len = strlen(file);
    if (file[len-2] == '.' && file[len - 1] == 'c')
        len -= 2;

    if (sel == -1) sel = strlen(SAVE_EXTENSION);
    if (strcmp(file + len - sel, SAVE_EXTENSION) == 0)
        len -= sel;
#ifdef HAVE_ZLIB
    if (gz_sel == -1) {
        gz_sel = strlen(SAVE_GZ_EXTENSION);
    }
    if (save_compressed) {
        name = new_string(len + gz_sel, "save_object");
        strcpy(name, file);
        strcpy(name + len, SAVE_GZ_EXTENSION);
    } else
#endif
    {
        name = new_string(len + sel, "save_object");
    strcpy(name, file);
    strcpy(name + len, SAVE_EXTENSION);
    }

    push_malloced_string(name);    /* errors */

    file = check_valid_path(name, ob, "save_object", 1);
    free_string_svalue(sp--);
    if (!file)
        error("Denied write permission in save_object().\n");

    strcpy(save_name, ob->obname);
    if ((p = strrchr(save_name, '#')) != 0)
        *p = '\0';
    p = save_name + strlen(save_name) - 1;
    if (*p != 'c' && *(p - 1) != '.')
        strcat(p, ".c");

    /*
     * Write the save-files to different directories, just in case
     * they are on different file systems.
     */
    sprintf(tmp_name, "%.250s.tmp", file);

#ifdef HAVE_ZLIB
    gzf = NULL;
    f = NULL;
    if (save_compressed) {
        gzf = gzopen(tmp_name, "w");
        if (!gzf) {
            error("Could not open /%s for a save.\n", tmp_name);
        }
        if (!gzprintf(gzf, "#/%s\n", ob->prog->filename)) {
            error("Could not open /%s for a save.\n", tmp_name);
        }
    } else
#endif
    {
    if (!(f = fopen(tmp_name, "w")) || fprintf(f, "#/%s\n", save_name) < 0) {
        error("Could not open /%s for a save.\n", tmp_name);
    }
    }
    v = ob->variables;
#ifdef HAVE_ZLIB
    success = save_object_recurse(ob->prog, &v, 0, save_zeros, f, gzf);

    if (gzf && gzclose(gzf)) {
        debug_perror("save_object", file);
        success = 0;
    }
#else
    success = save_object_recurse(ob->prog, &v, 0, save_zeros, f);
#endif

    if (f && fclose(f) < 0) {
        debug_perror("save_object", file);
        success = 0;
    }

    if (!success) {
        debug_message("Failed to completely save file. Disk could be full.\n");
        unlink(tmp_name);
    } else {
#ifdef WIN32
        /* Need to erase it to write over it. */
        unlink(file);
#endif
        if (rename(tmp_name, file) < 0) {
            debug_perror("save_object", file);
            debug_message("Failed to rename /%s to /%s\n", tmp_name, file);
            debug_message("Failed to save object!\n");
            unlink(tmp_name);
        }
#ifdef HAVE_ZLIB
        else if (save_compressed) {
            char buf[1024];
            // When compressed, unlink the uncompressed name too.
            strcpy(buf, file);
            len = strlen(buf) - gz_sel;
            strcpy(buf + len, SAVE_EXTENSION);
            unlink(buf);
        }
#endif

    }

    return success;
}

int
save_object_str (object_t *ob, int save_zeros, char *saved, int size)
{
    char *p;
    int success;
    svalue_t *v;
    char *now = saved;
    int left;
    if (ob->flags & O_DESTRUCTED)
        return 0;
    strcpy(now, "#/");
    now+=2;
    strcpy(now, ob->obname);
    if ((p = strrchr(now, '#')) != 0)
        *p = '\0';
    p = now + strlen(now) - 1;
    if (*p != 'c' && *(p - 1) != '.')
        strcat(p, ".c");
    now = now + strlen(now);
	*now++ = '\n';
	left = size - (now - saved);
	*now = 0;
    /*
     * Write the save-files to different directories, just in case
     * they are on different file systems.
     */
    v = ob->variables;
    success = save_object_recurse_str(ob->prog, &v, 0, save_zeros, now, left);

    if (!success) {
        debug_message("Failed to completely save file. string size too small?.\n");
    }
    return success;
}

/*
 * return a string representing an svalue in the form that save_object()
 * would write it.
 */
char *
save_variable (svalue_t * var)
{
    int theSize;
    char *new_str, *p;

    save_svalue_depth = 0;
    theSize = svalue_save_size(var);
    new_str = new_string(theSize - 1, "save_variable");
    *new_str = '\0';
    p = new_str;
    save_svalue(var, &p);
    DEBUG_CHECK(p - new_str != theSize - 1, "Length miscalculated in save_variable");
    return new_str;
}

static void cns_just_count (int * idx, program_t * prog) {
    int i;

    for (i = 0; i < prog->num_inherited; i++)
        cns_just_count(idx, prog->inherit[i].prog);
    *idx += prog->num_variables_defined;
}

static void cns_recurse (object_t * ob, int * idx, program_t * prog) {
    int i;

    for (i = 0; i < prog->num_inherited; i++) {
        if (prog->inherit[i].type_mod & DECL_NOSAVE)
            cns_just_count(idx, prog->inherit[i].prog);
        else
            cns_recurse(ob, idx, prog->inherit[i].prog);
    }
    for (i = 0; i < prog->num_variables_defined; i++) {
        if (!(prog->variable_types[i] & DECL_NOSAVE)) {
            free_svalue(&ob->variables[*idx + i], "cns_recurse");
            ob->variables[*idx + i] = const0u;
        }
    }
    *idx += prog->num_variables_defined;
}

void clear_non_statics (object_t * ob) {
    int idx = 0;
    cns_recurse(ob, &idx, ob->prog);
}

int restore_object (object_t * ob, const char * file, int noclear)
{
    char *name;
    int len;
    object_t *save = current_object;
    struct stat st;
#ifdef HAVE_ZLIB
    int pos;
    gzFile gzf;
    static int count = 0;
#else
    FILE *f;
    // Try and keep one buffer for droping all the restores into
    static char *theBuff = NULL;
    static int buff_len = 0;
    int tmp_len;
#endif

    if (ob->flags & O_DESTRUCTED)
        return 0;

    len = strlen(file);
    if (file[len-2] == '.' && file[len - 1] == 'c')
        len -= 2;

    if (sel == -1) sel = strlen(SAVE_EXTENSION);
    if (strcmp(file + len - sel, SAVE_EXTENSION) == 0)
        len -= sel;

#ifdef HAVE_ZLIB
    else {
      if (gz_sel == -1) gz_sel = strlen(SAVE_GZ_EXTENSION);
      if (strcmp(file + len - gz_sel, SAVE_GZ_EXTENSION) == 0)
          len -= gz_sel;
    }
    name = new_string(len + gz_sel, "restore_object");
    strncpy(name, file, len);
    strcpy(name + len, SAVE_GZ_EXTENSION);
    pos = 0;
    while (name[pos] == '/') {
        pos++;
    }
    // See if the gz file exists.
    if (stat(name + pos, &st) == -1)
    {
        FREE_MSTR(name);
#else
    {
#endif
        name = new_string(len + sel, "restore_object");
    strncpy(name, file, len);
    strcpy(name + len, SAVE_EXTENSION);
    }

    push_malloced_string(name);    /* errors */

    file = check_valid_path(name, ob, "restore_object", 0);
    free_string_svalue(sp--);
    if (!file) error("Denied read permission in restore_object().\n");


#ifdef HAVE_ZLIB
    gzf = gzopen(file, "r");
    if (!gzf) {
        if (gzf) {
            (void)gzclose(gzf);
        }
        return 0;
    }

    /* This next bit added by Armidale@Cyberworld 1/1/93
     * If 'noclear' flag is not set, all non-static variables will be
     * initialized to 0 when restored.
     */
    if (!noclear) {
        clear_non_statics(ob);
    }
    error_context_t econ;
    save_context(&econ);
    if (SETJMP(econ.context)){
        restore_context(&econ);
        pop_context(&econ);
        gzclose(gzf);
        return 0;
    }
    while((restore_object_from_gzip(ob, gzf, noclear, &count))){
	  count++;
          gzseek(gzf, 0, SEEK_SET);
	  if (!noclear) {
	      clear_non_statics(ob);
	  }
    }
    gzclose(gzf);
    pop_context(&econ);
#else
    f = fopen(file, "r");
    if (!f || fstat(fileno(f), &st) == -1) {
        if (f)
            (void)fclose(f);
        return 0;
    }

    if (!(tmp_len = st.st_size)) {
        (void)fclose(f);
        return 0;
    }

    if (tmp_len > buff_len) {
        if (theBuff) {
            FREE(theBuff);
        }
        theBuff = DXALLOC(tmp_len + 1, TAG_TEMPORARY, "restore_object: 4");
        buff_len = tmp_len;
    }
#ifdef WIN32
    tmp_len = read(_fileno(f), theBuff, tmp_len);
#else
    fread(theBuff, 1, tmp_len, f);
#endif

    fclose(f);
    theBuff[tmp_len] = '\0';
    current_object = ob;

    /* This next bit added by Armidale@Cyberworld 1/1/93
     * If 'noclear' flag is not set, all non-static variables will be
     * initialized to 0 when restored.
     */
    if (!noclear)
        clear_non_statics(ob);

    restore_object_from_buff(ob, theBuff, noclear);
#endif
    current_object = save;
    debug(d_flag, ("Object /%s restored from /%s.\n", ob->obname, file));

    return 1;
}

void restore_variable (svalue_t * var, char * str)
{
    int rc;
    rc = restore_svalue(str, var);

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

void tell_npc (object_t * ob, const char * str)
{
    copy_and_push_string(str);
    apply(APPLY_CATCH_TELL, ob, 1, ORIGIN_DRIVER);
}

/*
 * tell_object: send a message to an object.
 * If it is an interactive object, it will go to his
 * screen. Otherwise, it will go to a local function
 * catch_tell() in that object. This enables communications
 * between users and NPC's, and between other NPC's.
 * If INTERACTIVE_CATCH_TELL is defined then the message always
 * goes to catch_tell unless the target of tell_object is interactive
 * and is the current_object in which case it is written via add_message().
 */
void tell_object (object_t * ob, const char * str, int len)
{
    if (!ob || (ob->flags & O_DESTRUCTED)) {
        add_message(0, str, len);
        return;
    }
    /* if this is on, EVERYTHING goes through catch_tell() */
#ifndef INTERACTIVE_CATCH_TELL
    if (ob->interactive)
        add_message(ob, str, len);
    else
#endif
        tell_npc(ob, str);
}

void dealloc_object (object_t * ob, const char * from)
{
#ifdef DEBUG
    object_t *tmp, *prev_all = 0;
#endif

    debug(d_flag, ("free_object: /%s.\n", ob->obname));

    if (!(ob->flags & O_DESTRUCTED)) {
        if(ob->next_all != ob)
            /* This is fatal, and should never happen. */
            fatal("FATAL: Object 0x%x /%s ref count 0, but not destructed (from %s).\n",
                  ob, ob->obname, from);
        else {
            destruct_object(ob);
            return;
        }
    }
    DEBUG_CHECK(ob->interactive, "Tried to free an interactive object.\n");
    /*
     * If the program is freed, then we can also free the variable
     * declarations.
     */
    if (ob->prog) {
        tot_alloc_object_size -=
            (ob->prog->num_variables_total - 1) * sizeof(svalue_t) +
            sizeof(object_t);
        free_prog(&ob->prog);
        ob->prog = 0;
    }
    if (ob->replaced_program) {
        FREE_MSTR(ob->replaced_program);
        ob->replaced_program = 0;
    }
#ifdef PRIVS
    if (ob->privs)
        free_string(ob->privs);
#endif
    if (ob->obname) {
        debug(d_flag, ("Free object /%s\n", ob->obname));

        DEBUG_CHECK1(lookup_object_hash(ob->obname) == ob,
                     "Freeing object /%s but name still in name table", ob->obname);
        FREE((char *)ob->obname);
        SETOBNAME(ob, 0);
    }
#ifdef DEBUG
    prev_all = ob->prev_all;
    if (prev_all){
      prev_all->next_all = ob->next_all;
      if(ob->next_all)
        ob->next_all->prev_all = prev_all;
    } else {
      obj_list_dangling = ob->next_all;
      if(obj_list_dangling)
	     obj_list_dangling->prev_all = 0;
    }
    ob->next_all = 0;
    ob->prev_all = 0;
    tot_dangling_object--;
#endif
    tot_alloc_object--;
    FREE((char *) ob);
}

void free_object (object_t ** ob, const char * const from)
{
    //note that we get a pointer to a pointer unlike MudOS where it's a pointer to the object
	//this is so we can clear the variable holding the reference as that shouldn't be used anymore
	//after freeing it! don't set to NULL as that might still hide such bugs, and I suspect it may
	//be related to some of the corrupted memory crashes (which dw stopped doing, oh well, I'm sure it
	//will be back.) Better to find and fix than to hide!
  if(*ob)
    (*ob)->ref--;

  if ((*ob)->ref > 0) {
    *ob = (object_t *)9;
    return;
  }
  dealloc_object(*ob, from);
  *ob = (object_t *)1;
}

/*
 * Allocate an empty object, and set all variables to 0. Note that a
 * 'object_t' already has space for one variable. So, if no variables
 * are needed, we waste one svalue worth of memory (or we'd write too
 * much memory in copying the NULL_object over.
 */
object_t *get_empty_object (int num_var)
{
  //static object_t NULL_object;
    object_t *ob;
    int size = sizeof(object_t) +
    (num_var - !!num_var) * sizeof(svalue_t);
    int i;

    tot_alloc_object++;
    tot_alloc_object_size += size;
    ob = (object_t *) DXALLOC(size, TAG_OBJECT, "get_empty_object");
    /*
     * marion Don't initialize via memset, this is incorrect. E.g. the bull
     * machines have a (char *)0 which is not zero. We have structure
     * assignment, so use it.
     */
    //*ob = NULL_object; gives a warning on const pointers
    //memcpy(ob, &NULL_object, sizeof NULL_object);
    //screw the "bull machines" we're in the 21st century now
    memset(ob, 0, sizeof(object_t));
    ob->ref = 1;
    for (i = 0; i < num_var; i++)
        ob->variables[i] = const0u;
    return ob;
}

void reset_object (object_t * ob)
{
    /* Be sure to update time first ! */
    ob->next_reset = current_time + TIME_TO_RESET / 2 +
        random_number(TIME_TO_RESET / 2);

    save_command_giver(0);
    if (!apply(APPLY_RESET, ob, 0, ORIGIN_DRIVER)) {
        /* no reset() in the object */
        ob->flags &= ~O_WILL_RESET;     /* don't call it next time */
    }
    restore_command_giver();
    ob->flags |= O_RESET_STATE;
}

void call_create (object_t * ob, int num_arg)
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

#ifdef F_SET_HIDE
INLINE int object_visible (object_t * ob)
{
    if (ob->flags & O_HIDDEN) {
        if (current_object->flags & O_HIDDEN)
            return 1;

        return valid_hide(current_object);
    } else
        return 1;
}
#endif

void reload_object (object_t * obj)
{
    int i;

    if (!obj->prog)
        return;
    for (i = 0; i < obj->prog->num_variables_total; i++) {
        free_svalue(&obj->variables[i], "reload_object");
        obj->variables[i] = const0u;
    }
#if defined(PACKAGE_SOCKETS) || defined(PACKAGE_EXTERNAL)
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
        object_t *ob2;
        object_t *otmp;

        for (ob2 = obj->shadowed; ob2;) {
            otmp = ob2;
            ob2 = ob2->shadowed;
            otmp->shadowed = 0;
            otmp->shadowing = 0;
            destruct_object(otmp);
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
    remove_living_name(obj);
    set_heart_beat(obj, 0);
    remove_all_call_out(obj);
#ifndef NO_LIGHT
    add_light(obj, -(obj->total_light));
#endif
#ifdef PACKAGE_UIDS
#ifdef AUTO_SETEUID
    obj->euid = obj->uid;
#else
    obj->euid = NULL;
#endif
#endif
    call_create(obj, 0);
}

void get_objects (object_t *** list, int * size, get_objectsfn_t callback, void * data)
{
    object_t *ob;
#ifdef F_SET_HIDE
    int display_hidden = 0;

    if (num_hidden > 0) {
        if (current_object->flags & O_HIDDEN) {
            display_hidden = 1;
        } else {
            display_hidden = valid_hide(current_object);
        }
    }
    *list = (object_t **)new_string(((tot_alloc_object - (display_hidden ? 0 : num_hidden)) * sizeof(object_t *)) - 1, "get_objects");
#else
    *list = (object_t **)new_string((tot_alloc_object * sizeof(object_t *)) - 1, "get_objects");
#endif

    if (!*list)
        fatal("Out of memory!\n");
    push_malloced_string((char *)*list);

    for (*size = 0, ob = obj_list;  ob;  ob = ob->next_all) {
#ifdef F_SET_HIDE
        if (!display_hidden && (ob->flags & O_HIDDEN))
            continue;
#endif
        if (!callback || callback(ob, data))
            (*list)[(*size)++] = ob;
    }
}

static object_t *command_giver_stack[CFG_MAX_CALL_DEPTH];
object_t **cgsp = command_giver_stack;

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_command_giver_stack (void)
{
    object_t **ob;

    for (ob = &command_giver_stack[0];  ob < cgsp;  ob++) {
        if (*ob)
            (*ob)->extra_ref++;
    }
    if (command_giver)
        command_giver->extra_ref++;
}
#endif

/* set a new command giver, saving the old one */
void save_command_giver (object_t * ob)
{
    DEBUG_CHECK(cgsp == &command_giver_stack[CFG_MAX_CALL_DEPTH], "command_giver stack overflow");
    *(++cgsp) = command_giver;

    command_giver = ob;
    if (command_giver)
        add_ref(command_giver, "save_command_giver");
}

/* restore the saved command giver */
void restore_command_giver (void)
{
    if (command_giver)
        free_object(&command_giver, "command_giver_error_handler");
    DEBUG_CHECK(cgsp == command_giver_stack, "command_giver stack underflow");
    command_giver = *(cgsp--);
}

/* set a new command giver */
void set_command_giver (object_t * ob)
{
    if (command_giver)
        free_object(&command_giver, "set_command_giver");

    command_giver = ob;
    if (command_giver != 0)
        add_ref(command_giver, "set_command_giver");
}
