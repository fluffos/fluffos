#include "../lpc_incl.h"
#include "../comm.h"
#include "../file_incl.h"
#include "../file.h"
#include "../backend.h"
#include "../compiler.h"
#include "../main.h"
#include "../eoperators.h"
#include "../efuns_main.h"
#include "../efun_protos.h"
#include "../simul_efun.h"
#include "../add_action.h"
#include "../array.h"
#include "../master.h"
#include "../port.h"
#include "../array.h"

#ifdef F_QUERY_MULTIPLE_SHORT
/* Hideous mangling of C code by Taffyd. */
void query_multiple_short(svalue_t * arg, const char * type, int no_dollars, int quiet, int dark, int num_arg) {
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
	char *str, *res;
	object_t *ob;
	char *fun;

	if (!size) {
		str = new_string(0, "f_query_multiple_short");
		str[0] = '\0';
		pop_n_elems(num_arg);
		push_malloced_string(str);
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
		call_origin = ORIGIN_EFUN;
		if(!apply_low("query_light", command_giver->super, 0))
		push_number(0);
		v = apply("check_dark", command_giver, 1, ORIGIN_EFUN);

		if (v && v->type == T_NUMBER && v->u.number) {
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
		str = new_string(max_string_length, "f_query_multiple_short");
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
			if(total_len + SVALUE_STRLEN(v) > max_string_length - mlen)
			continue;
			strncpy(str + total_len, v->u.string, (len = SVALUE_STRLEN(v)));
			total_len += len;
		}

		strncpy(str + total_len, m, mlen);
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

	str = new_string(max_string_length, "f_query_multiple_short");
	str[max_string_length]= '\0';
	total_len = 0;

	for ( i = 0; i < size; i++ ) {
		sv = (arr->item + i);

		switch(sv->type) {
			case T_STRING:
			len = SVALUE_STRLEN(sv);
			if(total_len + len < max_string_length) {
				strncpy(str + total_len, sv->u.string, len);
				total_len += len;
			}
			break;
			case T_OBJECT:
			push_number(quiet);
			v = apply(fun, sv->u.ob, 1, ORIGIN_EFUN);

			if (!v || v->type != T_STRING) {
				continue;
			}

			if(total_len + SVALUE_STRLEN(v) < max_string_length) {
				strncpy(str + total_len, v->u.string,
						(len = SVALUE_STRLEN(v)));
				total_len += len;
			}

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
				if(total_len+seplen < max_string_length) {
					strncpy( str + total_len, separator, seplen );
					total_len += seplen;
				}
			}
			else {
				if ( i < size - 1 ) {
					if(total_len+andlen < max_string_length) {
						strncpy( str + total_len, andsep, andlen );
						total_len += andlen;
					}
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
f_query_multiple_short()
{
	svalue_t *sv = sp - st_num_arg + 1;
	const char *type = NULL;
	int no_dollars = 0, quiet = 0, dark = 0;

	if ( st_num_arg > 4) {
		if (sv[4].type == T_NUMBER) {
			dark = sv[4].u.number;
		}
	}

	if ( st_num_arg > 3) {
		if (sv[3].type == T_NUMBER) {
			quiet = sv[3].u.number;
		}
	}

	if ( st_num_arg > 2) {
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

#ifdef F_REFERENCE_ALLOWED
/* Hideous mangling of C code by Taffyd. */

/* varargs int reference_allowed(object referree, mixed referrer) */

#define PLAYTESTER_HANDLER "/obj/handlers/playtesters"
#define PLAYER_HANDLER "/obj/handlers/player_handler"

int _in_reference_allowed = 0;

int reference_allowed(object_t * referee, object_t * referrer_obj, const char * referrer_name)
{
	int invis = 0;
	int referee_creator = 0;
	svalue_t *v;
	svalue_t *item;
	array_t *vec;
	const char *referee_name = NULL;
	object_t *playtester_handler = NULL;
	object_t *player_handler = NULL;
	int referrer_playtester = 0;
	int referrer_match = 0;
	int playtester_match = 0;
	int i;
	int size;
	int ret = 0;

	/* Check to see whether we're invisible */
	v = apply("query_invis", referee, 0, ORIGIN_EFUN);

	if (v && v->type == T_NUMBER) {
		invis = v->u.number;
	}
	/* And if we're a creator. */
	v = apply("query_creator", referee, 0, ORIGIN_EFUN);

	if ( v && v->type == T_NUMBER) {
		referee_creator = v->u.number;
	}

	/* If we're not invisible, or not a creator, or currently in a 
	 reference allowed call, then we can see them. */
	if (!invis || !referee_creator || _in_reference_allowed) {
		return 1;
	}

	_in_reference_allowed = 1;

	if (referrer_obj && referee == referrer_obj) {
		_in_reference_allowed = 0;
		return 1;
	}

	/* Determine the names of these guys.
	 * We might need to make copies of these strings? apply()
	 * has a few oddities. 
	 */

	v = apply("query_name", referee, 0, ORIGIN_EFUN);

	if (v && v->type == T_STRING) {
		referee_name = v->u.string;
	}

	if (referrer_obj && !referrer_name) {
		v = apply("query_name", referrer_obj, 0, ORIGIN_EFUN);

		if (v && v->type == T_STRING) {
			referrer_name = v->u.string;
		}
	}

	if (!referee_name || !referrer_name) {
		_in_reference_allowed = 0;
		return 1;
	}

	playtester_handler = find_object(PLAYTESTER_HANDLER);

	if ( playtester_handler ) {
		copy_and_push_string(referrer_name);
		v = apply("query_playtester", playtester_handler, 1, ORIGIN_EFUN);

		if (v && v->type == T_NUMBER) {
			referrer_playtester = v->u.number;
		}
	}

	v = apply( "query_allowed", referee, 0, ORIGIN_EFUN);
	//printf("allow refs:%d\n", v->u.arr->ref);
	if (v && v->type == T_ARRAY) {
		vec = v->u.arr;
		size = vec->size;
		/* Iterate through the allowed array. */
		for (i = 0; i < size; i++) {
			item = vec->item + i;

			if (strcmp(referrer_name, item->u.string) == 0) {
				referrer_match = 1;

				/* If they're not a playtester, then bail as soon as 
				 * we make this match. */
				if (!referrer_playtester) {
					break;
				}
			}

			if (referrer_playtester &&
					strcmp("playtesters", item->u.string) == 0) {
				playtester_match = 1;

				/* If we've already made a referrer match, then time
				 * to bail now. */
				if (referrer_match) {
					break;
				}
			}
		}
	}

	/* If we found a match, then they are allowed so bail. */
	if ( referrer_match || playtester_match ) {
		_in_reference_allowed = 0;
		return 1;
	}

	switch(invis) {
		case 3:
		/* Check for High Lord Invis. */

		copy_and_push_string(referrer_name);
		v = apply("high_programmer", master_ob, 1, ORIGIN_EFUN);

		if (v && v->type == T_NUMBER) {
			ret = v->u.number;
		}
		break;

		case 2:
		/* Check for Lord Invis */

		copy_and_push_string(referrer_name);
		v = apply("query_lord", master_ob, 1, ORIGIN_EFUN);

		if (v && v->type == T_NUMBER) {
			ret = v->u.number;
		}
		break;

		case 1: {
			/* Creator Invis */
			if (referrer_obj) {
				v = apply("query_creator", referrer_obj, 0, ORIGIN_EFUN);

				if (v && v->type == T_NUMBER) {
					ret = v->u.number;
				}
			}
			else {
				player_handler = find_object(PLAYER_HANDLER);
				copy_and_push_string(referrer_name);
				v = apply("test_creator", player_handler, 1, ORIGIN_EFUN);

				if (v && v->type == T_NUMBER) {
					ret = v->u.number;
				}
			}
		}
		break;

		default:
		/* A normal player.
		 * Shouldn't we do this somewhere else? */

		ret = 1;
		break;
	}

	_in_reference_allowed = 0;
	return ret;
}

void
f_reference_allowed()
{
	svalue_t *sv = sp - st_num_arg + 1;
	svalue_t *v;
	object_t *referee = NULL;
	object_t *referrer_obj = command_giver; /* Default to this_player(). */
	const char *referrer_name = NULL;
	int result = 0;
	int num_arg = st_num_arg;

	/* Maybe I could learn how to use this :p 
	 CHECK_TYPES(sp-1, T_NUMBER, 1, F_MEMBER_ARRAY); */
	
	if(referrer_obj && referrer_obj->flags & O_DESTRUCTED)
	  referrer_obj = NULL;

	if (sv->type == T_OBJECT && sv->u.ob) {
		referee = sv->u.ob;
	}

	if (st_num_arg > 1) {
		if (sv[1].type == T_STRING && sv[1].u.string) {
			/* We've been passed in a string, now we need to call 
			 * find_player() */
#ifdef F_FIND_PLAYER
			/* If we have a find_player() efun, then we need to sue 
			 * the following method.  This hasn't been tested!
			 */
			referrer = find_living_object(sv[1].u.string, 1);
#else
			if (simul_efun_ob) {
				push_svalue(&sv[1]);
				v = apply("find_player", simul_efun_ob, 1, ORIGIN_EFUN);

				if (v && v->type == T_OBJECT) {
					referrer_obj = v->u.ob;
					referrer_name = sv[1].u.string;
				}
				else {
					referrer_obj = NULL;
					referrer_name = sv[1].u.string;
				}
			}
#endif
		}
		if (sv[1].type == T_OBJECT && sv[1].u.ob) {
			referrer_obj = sv[1].u.ob;
			referrer_name = NULL;
		}
	}

	if (referee && (referrer_obj || referrer_name)) {
		result = reference_allowed(referee, referrer_obj, referrer_name);

		pop_n_elems(num_arg);
		push_number(result);
	} else {
		pop_n_elems(num_arg);
		push_undefined();
	}
}

#endif

#ifdef F_ADD_A

void f_add_a() {
	const char *str = sp->u.string;
	char *ret;
	char *p;
	char first;
	int len;
	int an;

	while( *str == ' ' )
	str++;

	// If *str is 0, it was only spaces.  Return "a ".
	if( *str == 0 ) {
		pop_stack();
		copy_and_push_string( "a " );
		return;
	}

	len = strlen( str );
	// Don't add anything if it already begins with a or an.
	if( !strncasecmp( str, "a ", 2 ) || !strncasecmp( str, "an ", 3 ) ) {
		return;
	}

	first = *str;
	an = 0;

	// Some special cases.
	// If it begins with "us", check the following letter.
	// "a use", "a usurper", "a user", but "an usher".
	if( !strncasecmp( str, "us", 2 ) ) {
		first = str[2];
		an = 1;
	}

	// "hour*" gets "an".
	if( !strncasecmp( str, "hour", 4 ) ) {
		first = 'o';
	}

	switch( first ) {
		case 'a':
		case 'e':
		case 'i':
		case 'o':
		case 'u':
		case 'A':
		case 'E':
		case 'I':
		case 'O':
		case 'U':
		an = !an;
		break;
		default:
		break;
	}

	if( an ) { // Add an.
		if( len + 3 > max_string_length ) {
			free_string_svalue( sp );
			error( "add_a() exceeded max string length.\n" );
		}
		ret = new_string( len + 3, "f_add_a" );
		memcpy( ret, "an ", 3 );
		p = ret + 3;
	} else { // Add a.
		if( len + 2 > max_string_length ) {
			free_string_svalue( sp );
			error( "add_a() exceeded max string length.\n" );
		}
		ret = new_string( len + 2, "f_add_a" );
		memcpy( ret, "a ", 2 );
		p = ret + 2;
	}

	// Add the rest of the string.
	memcpy( p, str, len + 1 ); // + 1: get the \0.
	free_string_svalue( sp );
	sp->type = T_STRING;
	sp->subtype = STRING_MALLOC;
	sp->u.string = ret;
}

#endif
// This along with add_a() is the only sfun in /secure/simul_efun/add_a.c
#ifdef F_VOWEL
void f_vowel() {
	char v = (char)sp->u.number;

	if( v == 'a' || v == 'e' || v == 'i' || v == 'o' || v == 'u' ||
			v == 'A' || v == 'E' || v == 'I' || v == 'O' || v == 'U' )
	sp->u.number = 1;
	else
	sp->u.number = 0;
}
#endif


#ifdef F_REPLACE

void f_replace() {
	svalue_t *arg2 = sp - st_num_arg + 2;
	if(arg2->type == T_STRING && st_num_arg == 3) {
		return f_replace_string();
	} else if (st_num_arg == 2 && arg2->type != T_STRING){
		array_t *arr = arg2->u.arr;
		int i = 0;
		if(arr->size & 1) {
			error("Wrong array size for replace.\n");
		}
		sp--;

		for (i=0;i<arr->size;i+=2) {
			if(arr->item[i].type == T_STRING && arr->item[i+1].type == T_STRING) {
				share_and_push_string(arr->item[i].u.string);
				share_and_push_string(arr->item[i+1].u.string);
				st_num_arg = 3;
				f_replace_string();
				if(sp->type != T_STRING)
				break;
			}
		}
		free_array(arr);
	} else
		error("Bad arguments for replace.\n");
}

#endif

#if defined(F_REPLACE_MXP) || defined(F_REPLACE_HTML)
void replace_mxp_html(int html, int mxp) {
	char *dst = new_string(max_string_length+8, "f_replace_mxp_html: 2");
	const char *src = sp->u.string;
	char *dst2 = dst;
	while(*src && dst2-dst < max_string_length) {
		switch(*src) {
			case '&':
			strcpy(dst2, "&amp;");
			dst2 += 5;
			break;
			case '<':
			strcpy(dst2, "&lt;");
			dst2 += 4;
			break;
			case '>':
			strcpy(dst2, "&gt;");
			dst2 += 4;
			break;
			case '\n':
			if(mxp) {
				strcpy(dst2, "\e[4z<BR>");
				dst2 += 8;
			} else
			goto def;
			break;
			case '"':
			if(html) {
				strcpy(dst2, "&quot;");
				dst2 += 6;
				break;
			}
			default:
			def:
			*dst2++ = *src;
		}
		src++;
	}
	pop_stack();
	*dst2 = 0;
	push_malloced_string(extend_string(dst, dst2 - dst));
}

#endif

#ifdef F_REPLACE_HTML

void f_replace_html() {replace_mxp_html(1,0);}

#endif

#ifdef F_REPLACE_MXP

void f_replace_mxp() {replace_mxp_html(0,1);}

#endif

#ifdef F_ROULETTE_WHEEL
void f_roulette_wheel() {
   long num;
   mapping_t *m = sp->u.map;
   int j, found;
   mapping_node_t *elt, **a = m->table;
   svalue_t *val;

   // Loop through the mapping, adding up the weights.
   j = m->table_size;
   if(!j||!m->count)
     error("empty mapping in roulette_wheel.\n");
   num = 0;
   do {
      for( elt = a[j]; elt; elt = elt->next ) {
         val = elt->values + 1;

         if( val->type != T_NUMBER || val->u.number < 0 ) {
            error( "Weights must be non-negative integers.\n" );
         }

         num += val->u.number;
      }
   } while( j-- );

   num = 1 + random_number( num );
   found = 0;

   // Loop again, and stop when the sum of the weights comes to num.
   j = m->table_size;
   do {
      for( elt = a[j]; elt; elt = elt->next ) {
         val = elt->values + 1;
         num -= val->u.number;

         if( num <= 0 ) {
            found = 1;
            break;
         }
      }
   } while( j-- && !found );

   if( !found ) {    // This shouldn't happen...
      error( "Something went wrong!\n" );
   }

   // val points to the value that tipped the scale.  val - 1 is the key.
   assign_svalue_no_free( sp, val - 1 );
   free_mapping( m );
}

#endif

#ifdef F_REPLACE_OBJECTS

svalue_t replace_tmp = {T_NUMBER};
svalue_t *replace_objects(svalue_t *thing){
	int i;
	switch(thing->type){
	case T_OBJECT:
	{

		char buf[2000];
		strcpy(buf, thing->u.ob->obname);
		svalue_t *tmp = 0;
		if(!(thing->u.ob->flags & O_DESTRUCTED)){
			push_object(thing->u.ob);
			tmp = safe_apply_master_ob(APPLY_OBJECT_NAME, 1);
		}else
			strcat(buf, " (destructed)");
		if(tmp && tmp->type == T_STRING){
			strcat(buf, " (\"");
			strcat(buf, tmp->u.string);
			strcat(buf, "\")");
		}
		copy_and_push_string(buf);
		assign_svalue(&replace_tmp, sp);
		pop_stack();
		return &replace_tmp;
	}
	case T_ARRAY:
	case T_CLASS:
	{
		array_t *ar = thing->u.arr;
		array_t *nar = allocate_array(ar->size);
		push_refed_array(nar);
		for(i=0;i<ar->size;i++)
			assign_svalue(&nar->item[i], replace_objects(&ar->item[i]));
		assign_svalue(&replace_tmp, sp);
		pop_stack();
		replace_tmp.type = thing->type;
		return &replace_tmp;
	}
	case T_MAPPING:
		push_refed_array(mapping_indices(thing->u.map));
		push_refed_mapping(allocate_mapping(thing->u.map->table_size));
		push_number(0);
		for(i=0;i<(sp-2)->u.arr->size; i++){
		    svalue_t *key = sp;
			svalue_t *tmp = find_in_mapping(thing->u.map, &(sp-2)->u.arr->item[i]);
			assign_svalue(key, replace_objects(&(sp-2)->u.arr->item[i]));
			assign_svalue_no_free(find_for_insert((sp-1)->u.map, key, 1), replace_objects(tmp));
			if((sp-1)->u.map->count-i != 1)
				printf("guilty party:%s\n", key->u.string);
		}
		assign_svalue(&replace_tmp, sp-1);
		pop_3_elems();
		return &replace_tmp;
	default:
		assign_svalue(&replace_tmp, thing);
		return &replace_tmp;
	}
}

void f_replace_objects(){
	assign_svalue(sp, replace_objects(sp));
}
#endif

#ifdef F_REPLACE_DOLLARS
void f_replace_dollars(){
	char *newstr;
	const char *oldstr = (sp-1)->u.string;
	char *currentnew;
	const char *currentold = oldstr;
	int i;
	if(sp->u.arr->size & 1)
		error("wrong array length for replace_dollars()");
	for(i=0;i<sp->u.arr->size; i++)
		if(sp->u.arr->item[i].type != T_STRING)
			error("replace array should only contain strings");
	newstr = new_string(MAX_STRING_LENGTH, "replace_dollars");
	currentnew = newstr;
	for(i=0;i<COUNTED_STRLEN(oldstr);i++){
		if(oldstr[i]=='$'){
			int j;
			for(j=0;j<sp->u.arr->size;j+=2)
			{
				const char *one = sp->u.arr->item[j].u.string;
				if(!strncmp(one, oldstr+i, COUNTED_STRLEN(one)))
				{
					const char *two = sp->u.arr->item[j+1].u.string;
					int len = i-(currentold-oldstr);
					if(currentnew + len - newstr > MAX_STRING_LENGTH){
						FREE_MSTR(newstr);
						error("string too long");
					}
					strncpy(currentnew, currentold, len);
					currentnew+=len;
					currentold+=len;
					currentold+=COUNTED_STRLEN(one);
					if(currentnew + COUNTED_STRLEN(one) - newstr > MAX_STRING_LENGTH){
						FREE_MSTR(newstr);
						error("string too long");
					}
					strcpy(currentnew, two);
					currentnew+=COUNTED_STRLEN(two);
					break;
				}
			}
		}
	}
	if(newstr == currentnew){
		//nothing happened!
		FREE_MSTR(newstr);
		pop_stack();
		return;
	}

	if(currentnew + COUNTED_STRLEN(oldstr)-currentold+oldstr - newstr > MAX_STRING_LENGTH){
		FREE_MSTR(newstr);
		error("string too long");
	}

	strcpy(currentnew, currentold);
	pop_2_elems();
	push_malloced_string(extend_string(newstr, currentnew-newstr + COUNTED_STRLEN(oldstr)-(currentold-oldstr)));
}
#endif
