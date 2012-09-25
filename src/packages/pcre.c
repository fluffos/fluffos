/* FILES
 *    - pcre.c
 *    - pcre_spec.c
 *    - pcre.h
 *
 * PCRE (Perl Compatible Regular Expressions)
 *    Efuns using system library of PCRE (http://www.pcre.org/)
 *    The PCRE library was created by Philip Hazel at the
 *    University of Cambridge
 *
 *    For regular expressions syntactics:
 *       http://perldoc.perl.org/perlre.html
 *
 *
 * AUTHOR
 *    Volothamp @ Final Realms 
 *         nfa106 [a] gmail.com
 *
 * HISTORY
 *    June-July 2009:
 *       Package created
 * 
 *
 * DESCRIPTION 
 * 
 *   This package introduces the following efuns:
 *
 *      string pcre_version();
 *         - returns the version of the compiled PCRE library used
 *
 *      mixed pcre_match(string | string *, string, void | int);
 *         - analog with regexp(string | string *, string, void | int);
 *           for backwards compatibility reasons but utilizing the PCRE 
 *           library.
 *
 *      mixed *pcre_assoc(string, string *, mixed *, mixed | void);
 *         - analog with reg_assoc(string, string *, mixed *, mixed | void);
 *           for backwards compatibility reasons but utilizing the PCRE
 *           library.
 *
 *      string *pcre_extract(string subject, string pattern);
 *         - returns an array of captured groups specified in pattern  
 *
 *      string pcre_replace(string subject, string pattern, string *replacement);
 *         - returns a string where all captured groups have been replaced by the 
 *           elements of the replacement array. Number of subgroups and the size of 
 *           the replacement array must match.
 * 
 *      string pcre_replace_callback(string subject, string pattern, function | string fun, object ob);
 *         - returns a string where all captured groups have been replaced by the 
 *           return value of function pointe fun or function fun in object ob.
 */
//TODO
// study pattern
// extra options when matching, greedy, lazy, possesive, etc.
// match_all ? see previous
// store reg->error & reg->erroffset before error(..)

#include "lpc_incl.h"
#include <pcre.h>
#include "pcre.h"
#include "macros.h"
#include "mapping.h"

// Prototype declarations
static void       pcre_free_memory(pcre_t *p);
static pcre      *pcre_local_compile(pcre_t *p);
static int        pcre_local_exec(pcre_t *p);
static int        pcre_magic(pcre_t *p);
static int        pcre_query_match(pcre_t *p);
static int        pcre_match_single(svalue_t *str, svalue_t *pattern);
static array_t   *pcre_match(array_t *v, svalue_t *pattern, int flag);
static array_t   *pcre_assoc(svalue_t *str, array_t *pat, array_t *tok, svalue_t *def);
static char      *pcre_get_replace(pcre_t *run, array_t *replacements);
static array_t   *pcre_get_substrings(pcre_t *run);
//Caching functions
static int        pcre_cache_pattern(struct pcre_cache_t *table, pcre *cpat, svalue_t *pattern);
static pcre      *pcre_get_cached_pattern(struct pcre_cache_t *table, svalue_t *pattern);
static mapping_t *pcre_get_cache();
int pcrecachesize = 0;
// Globals
struct pcre_cache_t pcre_cache = {{ 0 }};


// efuns
void f_pcre_version(void)
{
	char *version;
	version = (char *)pcre_version();
	push_constant_string(version);
}

void f_pcre_match(void)
{
	array_t *v;
	int flag = 0;

	if (st_num_arg > 2)
	{
		if (sp->type != T_NUMBER)
			error("Bad argument 3 to pcre_match()\n");
		if ((sp - 2)->type == T_STRING)
			error("3rd argument illegal for pcre_match(string, string)\n");

		flag = (sp--)->u.number;
	}

	if ((sp - 1)->type == T_STRING )
	{
		flag = pcre_match_single((sp-1), sp);

		free_string_svalue(sp--);
		free_string_svalue(sp);
		put_number(flag);
	}
	else
	{
		v = pcre_match((sp - 1)->u.arr, sp, flag);

		free_string_svalue(sp--);
		free_array(sp->u.arr);
		sp->u.arr = v;
	}
}

void f_pcre_assoc(void)
{
	svalue_t *arg;
	array_t *vec;

	arg = sp - st_num_arg + 1;

	if ((arg + 2)->type != T_ARRAY)
		error("Bad argument 3 to pcre_assoc()\n");

	vec = pcre_assoc(arg, (arg+1)->u.arr, (arg+2)->u.arr,
			st_num_arg > 3 ? (arg+3) : &const0);

	if (st_num_arg == 4)
		pop_3_elems();
	else
		pop_2_elems();

	free_string_svalue(sp);

	sp->type  = T_ARRAY;
	sp->u.arr = vec;
}

void f_pcre_extract(void)
{
	pcre_t *run;
	array_t *ret;

	run = CALLOCATE(1, pcre_t, TAG_TEMPORARY, "f_pcre_extract : run");
	assign_svalue_no_free(&run->pattern, sp);
	run->subject  = (sp - 1)->u.string;
	run->s_length = SVALUE_STRLEN(sp - 1);
	run->ovector = NULL;
	run->ovecsize = 0;

	if(pcre_magic(run) < 0)
	{
		error("PCRE compilation failed at offset %d: %s\n", run->erroffset,
				run->error);
		pop_2_elems();
		pcre_free_memory(run);
		return;
	}

	/* Pop the 2 arguments from the stack */

	if (run->rc < 0) /* No match. could do handling of matching errors if wanted */
	{
		pop_2_elems();
		pcre_free_memory(run);
		push_refed_array(&the_null_array);
		return;
	}
	else if (run->rc > (run->ovecsize/3 - 1))
	{
		pop_2_elems();
		pcre_free_memory(run);
		error("Too many substrings.\n");
		return;
	}


	ret = pcre_get_substrings(run);
	pop_2_elems();

	push_refed_array(ret);

	pcre_free_memory(run);
	return;
}



void f_pcre_replace(void)
{
	pcre_t *run;
	array_t *replacements;

	unsigned int i;
	char *ret;

	run = CALLOCATE(1, pcre_t, TAG_TEMPORARY, "f_pcre_replace: run");

	run->ovector = NULL;
	run->ovecsize = 0;
	assign_svalue_no_free(&run->pattern, (sp - 1));
	run->subject = (sp - 2)->u.string;
	replacements = sp->u.arr;

	run->s_length = SVALUE_STRLEN(sp - 2);


	if(pcre_magic(run) < 0)
	{
		pcre_free_memory(run);
		error("PCRE compilation failed at offset %d: %s\n", run->erroffset,
				run->error);
	}


	if (run->rc < 0) /* No match. could do handling of matching errors if wanted */
	{
		pcre_free_memory(run);
		pop_2_elems();
		return;
	}


	if (run->rc > (run->ovecsize/3-1))
	{
		pcre_free_memory(run);
		error("Too many substrings.\n");
	}
	if ( (run->rc - 1) != replacements->size )
	{
		int tmp = run->rc-1;
		pcre_free_memory(run);
		error("Number of captured substrings and replacements do not match, "
				"%d vs %d.\n", tmp, replacements->size);
	}



	if (run->rc == 1)
	{
		/* No captured substrings, return subject */
		pcre_free_memory(run);
		pop_2_elems();
		return;
		//push_malloced_string(run->subject);
	}

	ret = pcre_get_replace(run, replacements);


	pop_3_elems();
	push_malloced_string(ret);
	pcre_free_memory(run);
	return;
}

//string pcre_replace_callback(string, string, function)
void f_pcre_replace_callback(void)
{
	int num_arg = st_num_arg, i;
	char *ret;
	pcre_t *run;
	svalue_t *arg;
	array_t *arr, *r;
	function_to_call_t ftc;

	arg = sp - num_arg + 1;

	run = CALLOCATE(1, pcre_t, TAG_TEMPORARY, "f_pcre_replace: run");
	run->ovector = NULL;
	run->ovecsize = 0;
	run->subject = arg->u.string;
	assign_svalue_no_free(&run->pattern, (arg + 1));

	run->s_length = SVALUE_STRLEN(arg);

	if(pcre_magic(run) < 0)
	{
		pcre_free_memory(run);
		error("PCRE compilation failed at offset %d: %s\n", run->erroffset,
				run->error);
	}


	if (run->rc < 0) /* No match. could do handling of matching errors if wanted */
	{
		pop_n_elems(num_arg-1);
		pcre_free_memory(run);
		return;
	}


	if (run->rc > (run->ovecsize/3-1))
	{
		pcre_free_memory(run);
		error("Too many substrings.\n");
	}

	arr = pcre_get_substrings(run);

	if(arg[2].type == T_FUNCTION || arg[2].type == T_STRING)
		process_efun_callback(2, &ftc, F_PCRE_REPLACE_CALLBACK);
	else {// 0
		pcre_free_memory(run);
		error("Illegal third argument (0) to pcre_replace_callback");
	}


	r = allocate_array(run->rc - 1); //can't use the empty variant in case we error below

	push_refed_array(r);
	push_refed_array(arr);
	error_context_t econ;

	if (!save_context(&econ)){
		pcre_free_memory(run);
		error("context stack full!\n");
	}
	if (SETJMP(econ.context)) {
		restore_context(&econ);
		/* condition was restored to where it was when we came in */
		pcre_free_memory(run);
		pop_context(&econ);
		error("error in callback!\n");
	}

	for (i = 0; i < run->rc - 1; i++)
	{
		svalue_t *v;
		push_svalue(arr->item + i);
		push_number(i);
		v = call_efun_callback(&ftc, 2);


		/* Mimic behaviour of map(string, function) when function pointer returns null,
         ie return the input.  */ 
		if (v && v->type == T_STRING && v->u.string != NULL)
			assign_svalue_no_free(&r->item[i], v);
		else
			assign_svalue_no_free(&r->item[i], &arr->item[i]);
	}
	pop_context(&econ);
	ret = pcre_get_replace(run, r);

	pop_n_elems(num_arg+2); //refed arrays
	push_malloced_string(ret);
	pcre_free_memory(run);
	return;
}


void f_pcre_cache(void) 
{
	mapping_t *m=NULL;
	m = pcre_get_cache();
	if (!m)
		push_number(0);
	else
		push_refed_mapping(m);
}


// Internal functions utilized by the efuns 
static pcre *pcre_local_compile(pcre_t *p)
{
	p->re = pcre_compile(
			p->pattern.u.string,
			0,
			&p->error,
			&p->erroffset,
			NULL);

	return p->re;
}

static int pcre_local_exec(pcre_t *p)
{
	int size;
	pcre_fullinfo(p->re, NULL, PCRE_INFO_CAPTURECOUNT, &size);
	size+=2;
	size *=3;
	if(p->ovector)
		FREE(p->ovector);
	p->ovector = CALLOCATE(size+1, int, TAG_TEMPORARY, "pcre_local_exec"); //too much, but who cares
	p->ovecsize = size;
	p->rc = pcre_exec(
			p->re,
			NULL,
			p->subject,
			p->s_length,
			0,
#ifndef USE_ICONV
			PCRE_NO_UTF8_CHECK,
#else
			0,
#endif
			p->ovector,
			size);

	return p->rc;
}

static int pcre_magic(pcre_t *p)
{
	p->re = pcre_get_cached_pattern(&pcre_cache, &p->pattern);

	if (p->re == NULL)
	{
		pcre_local_compile(p);
		pcre_cache_pattern(&pcre_cache, p->re, &p->pattern);
	}


	if (p->re == NULL)
		return -1;

	/* Add support for studied patterns here */
	//if(p->study && !found)
	// {

	// }
	pcre_local_exec(p);


	return 1;
}

static int pcre_query_match(pcre_t *p)
{
	return p->rc < 0 ? 0 : 1;
}

static int pcre_match_single(svalue_t *str, svalue_t *pattern)
{
	pcre_t *run;
	int ret;

	run = CALLOCATE(1, pcre_t, TAG_TEMPORARY, "pcre_match_single : run");
	run->ovector = NULL;
	run->ovecsize = 0;
	assign_svalue_no_free(&run->pattern, pattern);
	run->subject  = str->u.string;
	run->s_length = SVALUE_STRLEN(str);


	if(pcre_magic(run) < 0)
	{
		error("PCRE compilation failed at offset %d: %s\n", run->erroffset,
				run->error);
		pcre_free_memory(run);
		return 0;
	}

	ret = pcre_query_match(run);

	/* Free memory */
	pcre_free_memory(run);
	return ret;
}

static array_t *pcre_match(array_t *v, svalue_t *pattern, int flag)
{
	pcre_t *run;
	array_t *ret;
	svalue_t *sv1, *sv2;
	char *res;
	int num_match, size, match = !(flag & 2);

	if (!(size = v->size))
		return &the_null_array;

	run = CALLOCATE(1, pcre_t, TAG_TEMPORARY, "pcre_match : run");
	run->ovector = NULL;
	run->ovecsize = 0;
	assign_svalue_no_free(&run->pattern, pattern);

	run->re = pcre_get_cached_pattern(&pcre_cache, &run->pattern);

	if (run->re == NULL)
	{
		if (pcre_local_compile(run) == NULL)
		{
			const char *rerror = run->error;
			int offset = run->erroffset;

			pcre_free_memory(run);
			error("PCRE compilation failed at offset %d: %s\n", offset, rerror);
		}
		else
			pcre_cache_pattern(&pcre_cache, run->re, &run->pattern);
	}

	res       = (char*)DMALLOC(size, TAG_TEMPORARY, "prcre_match: res");
	sv1       = v->item + size;
	num_match = 0;

	while (size--)
	{
		if ((--sv1)->type != T_STRING)
		{
			res[size] = 0;
			continue;
		}

		run->subject  = sv1->u.string;
		run->s_length = SVALUE_STRLEN(sv1);

		pcre_local_exec(run);

		if (pcre_query_match(run) != match) //was not checking for match! (woom)
		{
			res[size] = 0;
			continue;
		}

		res[size] = 1;
		num_match++;
	}

	flag &= 1;
	ret  = allocate_empty_array(num_match << flag);
	sv2  = ret->item + (num_match << flag);
	size = v->size;

	while (size--)
	{
		if (res[size])
		{
			if (flag)
			{
				(--sv2)->type = T_NUMBER;
				sv2->u.number = size + 1;
			}

			(--sv2)->type = T_STRING;

			sv1  = v->item + size;
			*sv2 = *sv1;

			if (sv1->subtype & STRING_COUNTED)
			{
				INC_COUNTED_REF(sv1->u.string);
				ADD_STRING(MSTR_SIZE(sv1->u.string));
			}

			if (!--num_match)
				break;
		}
	}
	FREE(res);
	pcre_free_memory(run);

	return ret;
}

/* This is mostly copy/paste from reg_assoc, some parts are changed
 * TODO: rewrite with new logic
 */
static array_t *pcre_assoc(svalue_t *str, array_t *pat, array_t *tok, svalue_t *def)
{
	int i, size;
	const char *tmp;
	array_t *ret;

	if ((size = pat->size) != tok->size)
		error("Pattern and token array size must be identical.\n");

	for (i = 0; i < size; i++)
		if (pat->item[i].type != T_STRING)
			error("Non-string found in pattern array.\n");

	ret = allocate_empty_array(2);

	if (size)
	{
		pcre_t **rgpp;
		struct reg_match {
			int tok_i;
			const char *begin, *end;
			struct reg_match *next;
		} *rmp = (struct reg_match *) 0, *rmph = (struct reg_match *) 0;
		int num_match = 0, length;
		svalue_t *sv1, *sv2, *sv;
		int regindex;
		pcre_t *tmpreg;
		int laststart;

		rgpp = CALLOCATE(size, pcre_t *, TAG_TEMPORARY, "pcre_assoc : rgpp");

		for (i = 0; i < size; i++)
		{
			rgpp[i] = CALLOCATE(1, pcre_t, TAG_TEMPORARY, "pcre_assoc : rgpp[i]");
			rgpp[i]->ovector = NULL;
			rgpp[i]->ovecsize = 0;
			assign_svalue_no_free(&rgpp[i]->pattern, &pat->item[i]);
			rgpp[i]->re = pcre_get_cached_pattern(&pcre_cache, &rgpp[i]->pattern);

			if (rgpp[i]->re == NULL)
			{
				if (pcre_local_compile(rgpp[i]) == NULL)
				{
					const char *rerror = rgpp[i]->error;
					int offset = rgpp[i]->erroffset;

					while (i--)
						pcre_free_memory(rgpp[i]);

					FREE(rgpp);
					free_empty_array(ret);
					error("PCRE compilation failed at offset %d: %s\n", offset, rerror);
				}
				else
					pcre_cache_pattern(&pcre_cache, rgpp[i]->re, &rgpp[i]->pattern);
			}
		}


		tmp = str->u.string;
		int totalsize = SVALUE_STRLEN(str);
		int used = 0;
		while (*tmp)
		{
			laststart =  0;
			regindex  = -1;

			for (i = 0; i < size; i++)
			{

				rgpp[i]->subject = tmp;
				rgpp[i]->s_length = totalsize-used;

				pcre_local_exec(tmpreg = rgpp[i]);

				if (pcre_query_match(tmpreg))
				{
					char *curr_temp;
					size_t curr_temp_sz;

					curr_temp_sz = totalsize - used - tmpreg->ovector[0];
					if(!tmpreg->ovector[0]){
					  regindex = i;
					  break;
					}
					if (laststart < curr_temp_sz)
					{
						laststart = curr_temp_sz;
						regindex = i;
					}
				}
			}

			if (regindex >= 0)
			{
				const char *rmpb_tmp, *rmpe_tmp;
				size_t rmpb_tmp_sz, rmpe_tmp_sz;
				num_match++;

				if (rmp)
				{
					rmp->next = ALLOCATE(struct reg_match,
							TAG_TEMPORARY, "pcre_assoc : rmp->next");
					rmp = rmp->next;
				}
				else
					rmph = rmp = ALLOCATE(struct reg_match,
							TAG_TEMPORARY, "pcre_assoc : rmp");

				tmpreg = rgpp[regindex];

				rmpb_tmp = tmp + tmpreg->ovector[0];
				rmpe_tmp = tmp + tmpreg->ovector[1];

				rmp->begin = rmpb_tmp;
				rmp->end = tmp = rmpe_tmp;
				used+=tmpreg->ovector[1];
				rmp->tok_i = regindex;
				rmp->next = (struct reg_match *) 0;
			}
			else
				break;

			if (rmp->begin == tmp && (!*++tmp))
				break;
		}

		sv       = ret->item;
		sv->type = T_ARRAY;
		sv1      = (sv->u.arr = allocate_empty_array(2*num_match + 1))->item;

		sv++;
		sv->type = T_ARRAY;
		sv2      = (sv->u.arr = allocate_empty_array(2*num_match + 1))->item;

		rmp = rmph;

		tmp = str->u.string;
		
		while (num_match--){
		    char *svtmp;
		    length        = rmp->begin - tmp;
		    
		    sv1->type     = T_STRING;
		    sv1->subtype  = STRING_MALLOC;
		    sv1->u.string = svtmp = new_string(length, "pcre_assoc : sv1");
		    strncpy(svtmp, tmp, length);
		    svtmp[length] = 0;
		    
		    sv1++;
		    assign_svalue_no_free(sv2++, def);
		    tmp += length;
		    
		    length        = rmp->end - rmp->begin;
		    
		    sv1->type     = T_STRING;
		    sv1->subtype  = STRING_MALLOC;
		    sv1->u.string = svtmp = new_string(length, "pcre_assoc : sv1");
		    strncpy(svtmp, tmp, length);
		    svtmp[length] = 0;
		    
		    sv1++;
		    assign_svalue_no_free(sv2++, &tok->item[rmp->tok_i]);
		    tmp += length;
		    rmp = rmp->next;
		}
		
		sv1->type     = T_STRING;
		sv1->subtype  = STRING_MALLOC;
		sv1->u.string = string_copy(tmp, "pcre_assoc");
		assign_svalue_no_free(sv2, def);
		
		for (i = 0; i < size; i++)
		  pcre_free_memory(rgpp[i]);
		
		FREE(rgpp);
		
		while (rmp = rmph){
		  rmph = rmp->next;
		  FREE((char *) rmp);
		}
		return ret;
	}
	else {
	  svalue_t *temp;
	  svalue_t *sv;
	    
	  (sv = ret->item)->type = T_ARRAY;
	  temp = (sv->u.arr = allocate_empty_array(1))->item;
	  assign_svalue_no_free(temp, str);
	  sv = &ret->item[1];
	  sv->type = T_ARRAY;
	  assign_svalue_no_free((sv->u.arr = allocate_empty_array(1))->item, def);
	  return ret;
	}
}

static array_t *pcre_get_substrings(pcre_t *run)
{
	array_t *ret;
	unsigned int i;
	char **matches;

	ret = allocate_empty_array(run->rc - 1);

	if (run->rc != 1)
	{
		for (i = 1; i<= (run->rc-1); i++)
		{
			unsigned int start, length;
			/* Allocate enough for the match */
			length  = run->ovector[2*i + 1] - run->ovector[2*i];
			start = run->ovector[2*i];
			char *match = new_string(length, "pcre get substrings");

			sprintf(match, "%.*s", length, run->subject + start);
			ret->item[i-1].type     = T_STRING;
			ret->item[i-1].subtype  = STRING_MALLOC;
			ret->item[i-1].u.string = match;
		}
	}
	return ret;
}

static char *pcre_get_replace(pcre_t *run, array_t *replacements)
{
	unsigned int ret_pos = 0, i;
	size_t ret_sz;
	char *ret;

	/* Set size of return string to subject length */
	ret_sz = run->s_length;

	/* Subtract total size of all substrings */
	int prev = run->ovector[2];
	for (i = 1; i <= (run->rc - 1); i++){
		//printf("%d %d %d\n", ret_sz, run->ovector[2*i],run->ovector[2*i+1] );
		if(run->ovector[2*i] >= prev){
			ret_sz -= (size_t)(run->ovector[2*i + 1] - run->ovector[2*i]);
			prev = run->ovector[2*i + 1];
			/* Add total size of all replacements */
			ret_sz += SVALUE_STRLEN(&replacements->item[i-1]);
		}
	}

	/* Allocate space for the return string */
	ret = new_string((ret_sz), "pcre get replace");
	//printf("ret_sz:%d\n", ret_sz);

	/* Copy start of subject up until first match */
	if(run->rc > 1){
		strncpy(ret, run->subject, run->ovector[2]);
		ret_pos = run->ovector[2];
	}
	else
		strncpy(ret, run->subject, ret_sz);

	for (i = 1; i <= (run->rc - 1); i++)
	{

		unsigned int start, end, len_nxt;
		const char *rep;
		size_t rep_sz;

		start     = run->ovector[2*i];
		end       = run->ovector[2*i + 1];

		if ( i == (run->rc - 1))
			len_nxt = run->s_length - end;
		else
			len_nxt = run->ovector[2*i + 2] - end;

		if(len_nxt > MAX_STRING_LENGTH)
			continue; //nested ()s

		rep    = replacements->item[i - 1].u.string;
		rep_sz = SVALUE_STRLEN(&replacements->item[i - 1]);


		/* Copy first substring into return variable */
		strncpy(ret + ret_pos, rep, rep_sz);


		/* increment position in return variable by replacement size */
		ret_pos += rep_sz;


		strncpy(ret + ret_pos, run->subject + end, len_nxt);


		ret_pos += len_nxt;
	}

	*(ret+ret_sz) = '\0';


	return ret;
}


static void pcre_free_memory(pcre_t *p)
{
	free_svalue(&p->pattern, "pcre_free_memory");
	if(p->ovector)
		FREE(p->ovector);
	FREE(p);

}

//Caching functions, add new ones at the front of the bucket so we find them faster
static int pcre_cache_pattern(struct pcre_cache_t *table, pcre *cpat, svalue_t *pattern) //don't keep pointers to strings that will get freed elsewhere!
{
	unsigned int bucket = svalue_to_int(pattern) % PCRE_CACHE_SIZE;
	int sz;
	struct pcre_cache_bucket_t *tmp;
	struct pcre_cache_bucket_t *node;
	int full;

	tmp = table->buckets[bucket];

	// Calculate size of compiled pattern
	pcre_fullinfo(cpat, NULL, PCRE_INFO_SIZE, &sz);

	full = (pcrecachesize > 2 * PCRE_CACHE_SIZE);
	while (tmp)
	{
		if (pattern->u.string == tmp->pattern.u.string)
			break;

		tmp = tmp->next;
	}

	if (tmp)
	{
		//does this even make sense? same pattern will always compile the same way?
		pcre_free(tmp->compiled_pattern);
		node = tmp;
	}
	else
	{
		node = CALLOCATE(1, struct pcre_cache_bucket_t, TAG_TEMPORARY, "pcre_cache_pattern : node");
		node->pattern.type = T_NUMBER; //so we don't free invalids
		if (node == NULL)
			return -1;
		if(!full)
			pcrecachesize++;
		else{
			if((tmp = table->buckets[bucket])){
				while(tmp->next)
					tmp = tmp->next;
				if(tmp == table->buckets[bucket]){//if the hash version works, most of the time
					pcre_free(tmp->compiled_pattern);
					FREE(tmp);
					table->buckets[bucket] = NULL;
				} else {
					struct pcre_cache_bucket_t *tmp2;
					tmp2 = table->buckets[bucket];
					while(tmp2->next != tmp)
						tmp2 = tmp2->next; //shouldn't get here often
					pcre_free(tmp->compiled_pattern);
					FREE(tmp);
					tmp2->next = NULL;
				}
			}
		}
		node->next = table->buckets[bucket];
		table->buckets[bucket] = node;
	}

	assign_svalue(&node->pattern, pattern);
	node->compiled_pattern = cpat;
	node->size             = sz;

	return 0;
}

static pcre *pcre_get_cached_pattern(struct pcre_cache_t *table, svalue_t *pattern)
{
	unsigned int bucket = svalue_to_int(pattern) % PCRE_CACHE_SIZE;
	struct pcre_cache_bucket_t *node;
	struct pcre_cache_bucket_t *lnode = NULL;
	node = table->buckets[bucket];

	while (node)
	{
		if (pattern->u.string == node->pattern.u.string){
			if(node != table->buckets[bucket]){
				//not at the front, move it there, so the most used pattern is fastest
				lnode->next = node->next;
				node->next = table->buckets[bucket];
				table->buckets[bucket] = node;
			}
			return node->compiled_pattern;
		}
		lnode = node;
		node = node->next;
	}
	return NULL;
}


static mapping_t *pcre_get_cache()
{
	int size = 0, i;
	mapping_t *ret;
	struct pcre_cache_bucket_t *node;

	// Calculate size for mapping
	for (i = 0; i < PCRE_CACHE_SIZE; i++)
	{
		if (pcre_cache.buckets[i] != NULL)
		{
			node = pcre_cache.buckets[i];

			while (node)
			{
				node = node->next;
				size++;
			}
		}
	}

	ret = allocate_mapping(size);

	for (i = 0; i< PCRE_CACHE_SIZE; i++)
	{
		if (pcre_cache.buckets[i] != NULL)
		{
			node = pcre_cache.buckets[i];

			while (node)
			{
				add_mapping_pair(ret, node->pattern.u.string, node->size);
				node = node->next;
			}
		}
	}
	return ret;
}


