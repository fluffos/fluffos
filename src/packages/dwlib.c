
#include "../lpc_incl.h"
#include "../comm.h"
#include "../file_incl.h"
#include "../file.h"
#include "../backend.h"
#include "../swap.h"
#include "../compiler.h"
#include "../main.h"
#include "../eoperators.h"
#include "../efun_protos.h"
#include "../simul_efun.h"
#include "../add_action.h"

#ifdef F_QUERY_MULTIPLE_SHORT
/* Hideous mangling of C code by Taffyd. */ 
void query_multiple_short P6(svalue_t *, arg, char *, type, int, no_dollars, int, quiet, int, dark, int, num_arg) { 
    int qms_buffer_size = max_string_length;
        char m[] = "$M$";
    char s[] = "_short";
    char default_function[] = "a_short";
    char separator[] = ", ";
    char andsep[] = " and ";
    int mlen = strlen(m);
    int slen = strlen(s);
    int seplen = strlen( separator );
    int andlen = strlen( andsep );

        array_t *arr = arg->u.arr;
        svalue_t *sv;
    svalue_t *v;
        int size = arr->size;
        int i;
        int len;
        int total_len;
        char *pos;
        char *str, *res;
        char *tmp;
    object_t *ob;
    char *fun; 

        if (!size) {
                str = new_string(0, "f_query_multiple_short");
                str[0] = '\0';
        pop_n_elems(num_arg);
        put_malloced_string(str);
        return; 
        }
    
    /* 
    if (no_dollars && sizeof(args) && objectp(args[0]) && undefinedp(dark) && 
        this_player() && environment(this_player())) {
        dark = this_player()->check_dark(environment(this_player())->query_light());
        if (dark) {
        return "some objects you cannot make out";
        }
    } */ 

    if (no_dollars && arr->item->type == T_OBJECT && !dark && command_giver &&
        command_giver->super) { 

        v = apply("query_light", command_giver->super, 0, ORIGIN_EFUN);
        push_svalue(v);
        v = apply("check_dark", command_giver, 1, ORIGIN_EFUN);
        
        if (v->type == T_NUMBER && v->u.number) {
            pop_n_elems(num_arg);
            copy_and_push_string("some objects you cannot make out"); 
            return;
        }
    }

    /* If we don't have a type parameter, then use default_function */ 
    /* We need to free this value with FREE_MSTR() */ 

    if ( !type ) { 
        len = strlen( default_function );
        fun = new_string( len, "f_query_multiple_short");
        fun[len] = '\0';
        strncpy( fun, default_function, len );
    }
    else { 
        len = strlen( type ) + slen;
        fun = new_string( len, "f_query_multiple_short");
        fun[len] = '\0';
        strncpy( fun, type, len );
        strncpy( fun + strlen( type ), s, slen);
    }
   
    /* Check to see if there are any non-objects in the array. */ 
    for (i = 0; i < size; i++) {
        if ((arr->item + i)->type != T_OBJECT) {
            break;
        }
    }

    /* The array consists only of objects, and will use the $M$ 
       expansion code. */ 
    if (i == size && !no_dollars) {
        str = new_string(qms_buffer_size, "f_query_multiple_short");
        str[max_string_length]= '\0';
        strncpy(str, m, mlen);
        total_len = mlen;

        for ( i = 0; i < size; i++ ) {
            sv = (arr->item + i);
            push_number(quiet);
            v = apply(fun, sv->u.ob, 1, ORIGIN_EFUN);

            if (!v || v->type != T_STRING) {
                continue;                
            }

            strncat(str, v->u.string, (len = SVALUE_STRLEN(v)));
            total_len += len;
        }

        strncat(str, m, mlen);
        total_len += mlen;

        res = new_string( total_len, "f_query_multiple_short" );
        res[ total_len ] = '\0';
        memcpy(res, str, total_len);

        /* Clean up our temporary buffer. */ 

        FREE_MSTR(str);
        FREE_MSTR(fun);

        pop_n_elems(num_arg);
        push_malloced_string(res);
        return;
    }

    /* This is a mixed array, so we don't use $M$ format.  Instead, we 
       do as much $a_short$ conversion as we can etc.  */ 

    str = new_string(qms_buffer_size, "f_query_multiple_short");
    str[max_string_length]= '\0';
    total_len = 0;

    for ( i = 0; i < size; i++ ) {
        sv = (arr->item + i);
    
        switch(sv->type) {
            case T_STRING:
                len = SVALUE_STRLEN(sv);
                strncpy( str + total_len, sv->u.string, len );
                total_len += len;
            break;

            case T_OBJECT:
                push_number(quiet);
                v = apply(fun, sv->u.ob, 1, ORIGIN_EFUN);

                if (!v || v->type != T_STRING) {
                    continue;                
                }

                strncat(str, v->u.string, (len = SVALUE_STRLEN(v)));
                total_len += len;

                break;
            case T_ARRAY:
                    /* Does anyone use this? */ 
                    /* args[ i ] = "$"+ type +"_short:"+ file_name( args[ i ][ 1 ] ) +"$"; */ 
            default:    
                /* Get the next element. */ 
                continue;            
                break;
        }
        
        if ( len && size > 1 ) {
            if ( i < size - 2 ) {
                strncpy( str + total_len, separator, seplen );
                total_len += seplen;
            }
            else { 
                if ( i < size - 1 ) {    
                    strncpy( str + total_len, andsep, andlen );
                    total_len += andlen;
                }
            }
        }
    }

    FREE_MSTR(fun);

    res = new_string(total_len, "f_query_multiple_short");
    res[total_len] = '\0';
    memcpy(res, str, total_len);

    FREE_MSTR(str);

    /* Ok, now that we have cleaned up here we have to decide what to do
       with it. If nodollars is 0, then we need to pass it to an object
       for conversion. */ 

    if (no_dollars) { 
        if (command_giver) { 
            /* We need to call on this_player(). */ 
            push_malloced_string(res);
            v = apply("convert_message", command_giver, 1, ORIGIN_EFUN);
            
            if (v && v->type == T_STRING) { 
                pop_n_elems(num_arg);
                share_and_push_string(v->u.string);
            }
            else { 
                pop_n_elems(num_arg);
                push_undefined();
            }
            
        }
        else {
            /* We need to find /global/player. */ 
            /* Does this work? Seems not to. */ 
            ob = find_object("/global/player");
            
            if (ob) {
                push_malloced_string(res);
                v = apply("convert_message", ob, 1, ORIGIN_EFUN);
                
                /* Return the result! */ 
                if (v && v->type == T_STRING) { 
                    pop_n_elems(num_arg);
                    share_and_push_string(v->u.string);
                }
                else { 
                    pop_n_elems(num_arg);
                    push_undefined();
                }
            }
            else { 
                pop_n_elems(num_arg);
                push_undefined();
            }
        }

    }
    else { 
        pop_n_elems(num_arg);
        push_malloced_string(res);
    }
} /* query_multiple_short() */


void
f_query_multiple_short PROT((void))
{
    svalue_t *sv = sp - st_num_arg + 1;
    char *type = NULL;
    int no_dollars = 0, quiet = 0, dark = 0;
    
    if ( st_num_arg > 4)  {
        if (sv[4].type == T_NUMBER) {
            dark = sv[4].u.number;
        }
    }

    if ( st_num_arg > 3)  {
        if (sv[3].type == T_NUMBER) {
            quiet = sv[3].u.number;
        }
    }

    if ( st_num_arg > 2)  {
        if (sv[2].type == T_NUMBER) {
            no_dollars = sv[2].u.number;
        }
    }

    if (st_num_arg >= 2) {
        if (sv[1].type == T_STRING) { 
            type = sv[1].u.string;
        }
    }


        query_multiple_short(sv, type, no_dollars, quiet, dark, st_num_arg);
}

#endif

