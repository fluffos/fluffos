/*
 *  Parsing efuns.  Many of the concepts and algorithms here are stolen from
 *  an earlier LPC parser written by Rust@ZorkMUD.
 */
/*
 * TODO:
 * . he, she, it, him, her, them -> "look at tempress.  get sword.  kill her with it"
 * . compound input -> "n. e then s."
 * . OBS: and, all and everything (all [of] X, X except [for] Y, X and Y)
 * . OBS in OBS
 * . possesive: my his her its their Beek's
 * . first, second, etc
 * . one, two, ...
 * . questions.  'Take what?'
 * . oops
 * . the 'her' ambiguity
 * . foo, who is ...   foo, where is ...   foo, what is ...   foo, go south
 * . where is ... "where is sword" -> "In the bag on the table"
 * . > Which button do you mean, the camera button or the recorder button?
 * . Zifnab's crasher
 */

#include "std.h"
#include "lpc_incl.h"
#include "parser.h"
#include "md.h"

char *pluralize PROT((char *));

static parse_info_t *pi = 0;
static hash_entry_t *hash_table[HASH_SIZE];
static special_word_t *special_table[SPECIAL_HASH_SIZE];
static verb_t *verbs[VERB_HASH_SIZE];
static int objects_loaded = 0;
static int num_objects, num_people, me_object;
static struct object_s *loaded_objects[MAX_NUM_OBJECTS];
static int object_flags[MAX_NUM_OBJECTS];
static int num_literals = -1;
static char **literals;
static word_t words[256];
static int num_words = 0;
static verb_node_t *parse_vn;
static verb_t *parse_verb_entry;
static object_t *parse_restricted;
static object_t *parse_user;
static unsigned int cur_livings[NUM_BITVEC_INTS];
static unsigned int cur_accessible[NUM_BITVEC_INTS];
static int best_match;
static int best_error_match;
static int best_num_errors;
static parse_result_t *best_result = 0;
static match_t matches[10];
static char *current_error = 0;
#ifdef DEBUG
static int debug_parse_depth = 0;
static int debug_parse_verbose = 0;

#define DEBUG_PP(x) if (debug_parse_depth && debug_parse_verbose) debug_parse x
#define DEBUG_P(x) if (debug_parse_depth) debug_parse x
#define DEBUG_INC  if (debug_parse_depth) debug_parse_depth++
#define DEBUG_DEC  if (debug_parse_depth) debug_parse_depth--
#else
#define DEBUG_PP(x)
#define DEBUG_P(x)
#define DEBUG_INC
#define DEBUG_DEC
#endif

static void parse_rule PROT((parse_state_t *));

#define isignore(x) (!isascii(x) || !isprint(x) || x == '\'')
#define iskeep(x) (isalnum(x) || x == '*')

/* parse_init() - setup the object
 * parse_refresh() - refresh an object's parse data
 * parse_add_rule(verb, rule, handler) - add a rule for a verb
 * parse_sentence(sent) - do the parsing :)
 */

#ifdef DEBUGMALLOC_EXTENSIONS
void parser_mark_verbs() {
    int i;

    for (i = 0; i < VERB_HASH_SIZE; i++) {
	verb_t *verb_entry = verbs[i];
	
	while (verb_entry) {
	    EXTRA_REF(BLOCK(verb_entry->name))++;
	    verb_entry = verb_entry->next;
	}
    }
    
    for (i = 0; i < num_literals; i++) {
	EXTRA_REF(BLOCK(literals[i]))++;
    }

    for (i = 0; i < SPECIAL_HASH_SIZE; i++) {
	special_word_t *swp = special_table[i];
	
	while (swp) {
	    EXTRA_REF(BLOCK(swp->wrd))++;
	    swp = swp->next;
	}
    }
}
    
void parser_mark P1(parse_info_t *, pinfo) {
    int i;

    if (!(pinfo->flags & PI_SETUP))
	return;
    
    for (i = 0; i < pinfo->num_ids; i++) {
	EXTRA_REF(BLOCK(pinfo->ids[i]))++;
    }
    for (i = 0; i < pinfo->num_adjs; i++) {
	EXTRA_REF(BLOCK(pinfo->adjs[i]))++;
    }
    for (i = 0; i < pinfo->num_plurals; i++) {
	EXTRA_REF(BLOCK(pinfo->plurals[i]))++;
    }
}
#endif

#ifdef DEBUG
/* Usage:  DEBUG_P(("foo: %s:%i", str, i)); */
void debug_parse P1V(char *, fmt) {
    va_list args;
    char buf[2048];
    char *p = buf;
    int n = debug_parse_depth - 1;
    V_DCL(char *fmt);
    
    while (n--) {
	*p++ = ' ';
	*p++ = ' ';
    }
    
    V_START(args, fmt);
    V_VAR(char *, fmt, args);
    vsprintf(p, fmt, args);
    va_end(args);

    tell_object(command_giver, buf);
    tell_object(command_giver, "\n");
}
#endif

INLINE static void bitvec_copy P2(unsigned int *, b1, unsigned int *, b2) {
    int i;
    for (i = 0; i < NUM_BITVEC_INTS; i++)
	b1[i] = b2[i];
}

INLINE static void bitvec_zero P1(unsigned int *, bv) {
    int i;
    for (i = 0; i < NUM_BITVEC_INTS; i++)
	bv[i] = 0;
}

INLINE static int intersect P2(unsigned int *, bv1, unsigned int *, bv2) {
    int i, found = 0;

    for (i = 0; i < NUM_BITVEC_INTS; i++)
	if (bv1[i] &= bv2[i]) found = 1;
    
    return found;
}

void all_objects P2(unsigned int *, bv, int, remote_flag) {
    int i;
    int num = (remote_flag ? num_objects : num_objects - num_people);
    int last = BV_WHICH(num);
    
    i = last;
    while (i--)
	bv[i] = ~0;
    if (BV_WHICH(num + 1) != last)
	bv[last] = ~0;
    else
	bv[last] = 2 * BV_BIT(num) - 1;
    for (i = last + 1; i < NUM_BITVEC_INTS; i++)
	bv[i] = 0;
}

/* Note:
 * For efficiency reasons, there is very little memory allocation in the
 * code.  We take advantage of the fact that the algorithm is recursive,
 * and a parse_* routine doesn't return until that branch has been completely
 * evaluated.  This allows us to safely keep pointers to temporary buffers
 * on the stack.
 *
 * alloca() would be better for this, but MudOS doesn't currently use it.
 */

INLINE static match_t *add_match P4(parse_state_t *, state, int, token, 
				    int, start, int, end) {
    match_t *ret;

    DEBUG_PP(("Adding match: tok = %i start = %i end = %i", token, start, end));
    ret = &matches[state->num_matches++];
    ret->first = start;
    ret->last = end;
    ret->token = token;
    if (token == ERROR_TOKEN)
	state->num_errors++;

    DEBUG_PP(("State is: %x, num_errors: %i\n", state, state->num_errors));
    return ret;
}

static void parse_copy_array P2(array_t *, arr, char ***, sarrp) {
    char **table;
    int j;

    if (!arr->size) {
	*sarrp = 0;
	return;
    }
    
    table = *sarrp = CALLOCATE(arr->size,char *, 
			       TAG_PARSER, "parse_copy_array");
    for (j = 0; j < arr->size; j++) {
	if (arr->item[j].type == T_STRING) {
	    DEBUG_PP(("Got: %s", "arr->item[j].u.string"));
	    if (arr->item[j].subtype == STRING_SHARED) {
		table[j] = ref_string(arr->item[j].u.string);
	    } else {
		table[j] = make_shared_string(arr->item[j].u.string);
	    }
	} else {
	    table[j] = 0;
	}
    }
}

static void add_special_word P3(char *, wrd, int, kind, int, arg) {
    char *p = make_shared_string(wrd);
    int h = DO_HASH(p, SPECIAL_HASH_SIZE);
    special_word_t *swp = ALLOCATE(special_word_t, TAG_PARSER, "add_special_word");
    
    swp->wrd = p;
    swp->kind = kind;
    swp->arg = arg;
    swp->next = special_table[h];
    special_table[h] = swp;
}

static int check_special_word P2(char *, wrd, int *, arg) {
    int h = DO_HASH(wrd, SPECIAL_HASH_SIZE);
    special_word_t *swp = special_table[h];

    while (swp) {
	if (swp->wrd == wrd) {
	    *arg = swp->arg;
	    return swp->kind;
	}
	swp = swp->next;
    }
    return SW_NONE;
}

static void interrogate_master PROT((void)) {
    svalue_t *ret;

    DEBUG_PP(("[master::parse_command_prepos_list]"));
    ret = apply_master_ob("parse_command_prepos_list", 0);
    if (ret && ret->type == T_ARRAY) {
	num_literals = ret->u.arr->size;
	parse_copy_array(ret->u.arr, &literals);
    }

    add_special_word("the", SW_ARTICLE, 0);
    add_special_word("a", SW_ARTICLE, 0);

    add_special_word("me", SW_SELF, 0);
    add_special_word("myself", SW_SELF, 0);

    add_special_word("first", SW_ORDINAL, 1);
    add_special_word("1st", SW_ORDINAL, 1);
    add_special_word("second", SW_ORDINAL, 2);
    add_special_word("2nd", SW_ORDINAL, 2);
    add_special_word("third", SW_ORDINAL, 3);
    add_special_word("3rd", SW_ORDINAL, 3);
    add_special_word("fourth", SW_ORDINAL, 4);
    add_special_word("4th", SW_ORDINAL, 4);
    add_special_word("fifth", SW_ORDINAL, 5);
    add_special_word("5th", SW_ORDINAL, 5);
    add_special_word("sixth", SW_ORDINAL, 6);
    add_special_word("6th", SW_ORDINAL, 6);
    add_special_word("seventh", SW_ORDINAL, 7);
    add_special_word("7th", SW_ORDINAL, 7);
    add_special_word("eighth", SW_ORDINAL, 8);
    add_special_word("8th", SW_ORDINAL, 8);
    add_special_word("ninth", SW_ORDINAL, 9);
    add_special_word("9th", SW_ORDINAL, 9);
}

void f_parse_init PROT((void)) {
    parse_info_t *pi;

    if (num_literals == -1) {
	num_literals = 0;
	interrogate_master();
    }
    
    if (current_object->pinfo)
	return;

    pi = current_object->pinfo = ALLOCATE(parse_info_t, TAG_PARSER, "parse_init");
    pi->ob = current_object;
    pi->flags = 0;
}

static void remove_ids P1(parse_info_t *, pinfo) {
    int i;
    
    if (pinfo->flags & PI_SETUP) {
	if (pinfo->num_ids) {
	    for (i = 0; i < pinfo->num_ids; i++)
		free_string(pinfo->ids[i]);
	    FREE(pinfo->ids);
	}
	if (pinfo->num_plurals) {
	    for (i = 0; i < pinfo->num_plurals; i++)
		free_string(pinfo->plurals[i]);
	    FREE(pinfo->plurals);
	}
	if (pinfo->num_adjs) {
	    for (i = 0; i < pinfo->num_adjs; i++)
		free_string(pinfo->adjs[i]);
	    FREE(pinfo->adjs);
	}
    }
}

void f_parse_refresh PROT((void)) {
    if (!(current_object->pinfo))
	error("%s is not known by the parser.  Call parse_init() first.\n",
	      current_object->name);
    remove_ids(current_object->pinfo);
    current_object->pinfo->flags &= PI_VERB_HANDLER;
}

/* called from free_object() */
void parse_free P1(parse_info_t *, pinfo) {
    int i;

    if (pinfo->flags & PI_VERB_HANDLER) {
	for (i = 0; i < VERB_HASH_SIZE; i++) {
	    verb_t *v = verbs[i];
	    while (v) {
		verb_node_t **vn = &(v->node), *old;
		while (*vn) {
		    if ((*vn)->handler == pinfo->ob) {
			old = *vn;
			*vn = (*vn)->next;
			FREE(old);
		    } else vn = &((*vn)->next);
		}
		v = v->next;
	    }
	}
    }
    remove_ids(pinfo);
    FREE(pinfo);
}

static void hash_clean PROT((void)) {
    int i;
    hash_entry_t **nodep, *next;

    for (i = 0; i < HASH_SIZE; i++) {
	for (nodep = &hash_table[i]; *nodep && ((*nodep)->flags & HV_PERM);
	     nodep = &((*nodep)->next))
	    ;
	while (*nodep) {
	    next = (*nodep)->next;
	    free_string((*nodep)->name);
	    FREE((*nodep));
	    *nodep = next;
	}
    }
}

static void free_parse_result P1(parse_result_t *, pr) {
    int i, j;

    if (pr->ob)
	free_object(pr->ob, "free_parse_result");
    
    for (i = 0; i < 4; i++) {
	if (pr->res[i].func) FREE_MSTR(pr->res[i].func);
	if (pr->res[i].args) {
    	    for (j = 0; j < pr->res[i].num; j++)
	        free_svalue(((svalue_t *)pr->res[i].args) + j, "free_parse_result");
	    FREE(pr->res[i].args);
	}
    }
    FREE(pr);
}

static void clear_result P1(parse_result_t *, pr) {
    int i;

    pr->ob = 0;
    
    for (i = 0; i < 4; i++) {
	pr->res[i].func = 0;
	pr->res[i].args = 0;
    }
}

static void free_parse_globals PROT((void)) {
    int i;

    pi = 0;
    hash_clean();
    if (objects_loaded) {
	for (i = 0; i < num_objects; i++)
	    free_object(loaded_objects[i], "free_parse_globals");
	objects_loaded = 0;
    }
#ifdef DEBUG
    debug_parse_depth = 0;
#endif
}

token_def_t tokens[] = {
    { "OBJ", OBJ_A_TOKEN, 1 },
    { "STR", STR_TOKEN, 0 },
    { "WRD", WRD_TOKEN, 0 },
    { "LIV", LIV_A_TOKEN, 1 },
    { "OBS", OBS_TOKEN, 1 },
    { "LVS", LVS_TOKEN, 1 },
    { 0, 0 }
};

#define STR3CMP(x, y) (x[0] == y[0] && x[1] == y[1] && x[2] == y[2])

static int tokenize P2(char **, rule, int *, weightp) {
    char *start = *rule;
    int i, n;
    token_def_t *td;
    
    while (*start == ' ') start++;

    if (!*start)
	return 0; /* at the end */

    *rule = strchr(start, ' ');

    if (!*rule)
	*rule = start + strlen(start);
    
    n = *rule - start;

    if (n == 3 || (n > 4 && start[3] == ':')) {
	td = tokens;
	while (td->name) {
	    if (STR3CMP(td->name, start)) {
		i = td->token;
		if (n != 3) {
		    if (!td->mod_legal)
			error("Illegal to have modifiers to '%s'\n", td->name);
		    /* modifiers */
		    start += 4;
		    n -= 4;
		    while (n--) {
			switch(*start++) {
			case 'l':
			    i = -((-i) | LIV_MODIFIER);
			    break;
			case 'v':
			    i = -((-i) | VIS_ONLY_MODIFIER);
			    break;
			case 'p':
			    i = -((-i) | PLURAL_MODIFIER);
			    break;
			default:
			    error("Unknown modifier '%c'\n", start[-1]);
			}
		    }
		}
		switch (i) {
		default:
		    /* some kind of object */
		    (*weightp) += 2;
		    /* This next one is b/c in the presence of a singular
		     * and plural match, we want the singular.  Consider
		     * 'take fish' with >1 fish. */
		    if ((-i) & PLURAL_MODIFIER)
			(*weightp)--;
		    if ((-i) & LIV_MODIFIER)
			(*weightp)++;
		    if (!((-i) & VIS_ONLY_MODIFIER))
			(*weightp)++;
		    break;
		case STR_TOKEN:
		case WRD_TOKEN:
		    (*weightp)++;
		}
		return i;
	    }
	    td++;
	}
    }

    (*weightp)++; /* must be a literal */

    /* it's not a standard token.  Check the literals */
    for (i = 0; i < num_literals; i++) {
	if ((literals[i][n] == 0) && (strncmp(literals[i], start, n) == 0))
	    return i + 1;
    }

    {
	char buf[256];

	if (n > 50) {
	    strncpy(buf, start, 50);
	    strcpy(buf + 50, "...");
	} else {
	    strncpy(buf, start, n);
	    buf[n] = 0;
	}

	error("Unknown token '%s'\n", buf);
    }
    return 0;
}

static void make_rule P3(char *, rule, int *, tokens, int *, weightp) {
    int idx = 0;

    *weightp = 1;
    while (idx < 10) {
	if (!(tokens[idx++] = tokenize(&rule, weightp)))
	    return; /* we got to the end */
    }
    error("Only 10 tokens permitted per rule!\n");
}

static void free_words PROT((void)) {
    int i;

    for (i = 0; i < num_words; i++)
	if (words[i].type == WORD_ALLOCATED)
	    FREE_MSTR(words[i].string);
    num_words = 0;
}

static void interrogate_object P1(object_t *, ob) {
    svalue_t *ret;

    if (ob->pinfo->flags & PI_SETUP)
	return;
    
    DEBUG_P(("Interogating %s.", ob->name));
    
    DEBUG_PP(("[parse_command_id_list]"));
    ret = apply("parse_command_id_list", ob, 0, ORIGIN_DRIVER);
    if (ret && ret->type == T_ARRAY) {
	ob->pinfo->num_ids = ret->u.arr->size;
	parse_copy_array(ret->u.arr, &ob->pinfo->ids);
    } else ob->pinfo->num_ids = 0;
    if (ob->flags & O_DESTRUCTED) return;
    ob->pinfo->flags |= PI_SETUP;

    DEBUG_PP(("[parse_command_plural_id_list]"));
    ret = apply("parse_command_plural_id_list", ob, 0, ORIGIN_DRIVER);
    if (ret && ret->type == T_ARRAY) {
	ob->pinfo->num_plurals = ret->u.arr->size;
	parse_copy_array(ret->u.arr, &ob->pinfo->plurals);
    } else ob->pinfo->num_plurals = 0;
    if (ob->flags & O_DESTRUCTED) return;

    DEBUG_PP(("[parse_command_adjectiv_id_list]"));
    ret = apply("parse_command_adjectiv_id_list", ob, 0, ORIGIN_DRIVER);
    if (ret && ret->type == T_ARRAY) {
	ob->pinfo->num_adjs = ret->u.arr->size;
	parse_copy_array(ret->u.arr, &ob->pinfo->adjs);
    } else ob->pinfo->num_adjs = 0;
    if (ob->flags & O_DESTRUCTED) return;

    DEBUG_PP(("[is_living]"));
    ret = apply("is_living", ob, 0, ORIGIN_DRIVER);
    if (!IS_ZERO(ret)) {
	ob->pinfo->flags |= PI_LIVING;
	DEBUG_PP(("(yes)"));
    }
    if (ob->flags & O_DESTRUCTED) return;

    DEBUG_PP(("[inventory_accessible]"));
    ret = apply("inventory_accessible", ob, 0, ORIGIN_DRIVER);
    if (!IS_ZERO(ret)) {
	ob->pinfo->flags |= PI_INV_ACCESSIBLE;
	DEBUG_PP(("(yes)"));
    }
    if (ob->flags & O_DESTRUCTED) return;

    DEBUG_PP(("[inventory_visible]"));
    ret = apply("inventory_visible", ob, 0, ORIGIN_DRIVER);
    if (!IS_ZERO(ret)) {
	ob->pinfo->flags |= PI_INV_VISIBLE;
	DEBUG_PP(("(yes)"));
    }
}

static void rec_add_object P2(object_t *, ob, int, inreach) {
    object_t *o;

    if (!ob) return;
    if (ob->flags & O_DESTRUCTED) return;
    if (ob->pinfo) {
	if (ob == parse_user)
	    me_object = num_objects;
	object_flags[num_objects] = inreach;
	loaded_objects[num_objects++] = ob;
	add_ref(ob, "rec_add_object");
	if (!(ob->pinfo->flags & PI_INV_VISIBLE))
	    return;
	if (!(ob->pinfo->flags & PI_INV_ACCESSIBLE))
	    inreach = 0;
    }
    o = ob->contains;
    while (o) {
	rec_add_object(o, inreach);
	o = o->next_inv;
    }
}

static void find_uninited_objects P1(object_t *, ob) {
    object_t *o;
    
    if (!ob) return;
    if (ob->flags & O_DESTRUCTED) return;
    if (ob->pinfo && !(ob->pinfo->flags & PI_SETUP)) {
	loaded_objects[num_objects++] = ob;
	add_ref(ob, "find_united_objects");
    }
    o = ob->contains;
    while (o) {
	find_uninited_objects(o);
	o = o->next_inv;
    }
}    

static hash_entry_t *add_hash_entry P1(char *, str) {
    int h = DO_HASH(str, HASH_SIZE);
    hash_entry_t *he;

    DEBUG_PP(("add_hash_entry: %s", str));
    he = hash_table[h];
    while (he) {
	if (he->name == str)
	    return he;
	he = he->next;
    }

    he = ALLOCATE(hash_entry_t, TAG_PARSER, "add_hash_entry");
    he->name = ref_string(str);
    bitvec_zero(he->pv.noun);
    bitvec_zero(he->pv.plural);
    bitvec_zero(he->pv.adj);
    he->next = hash_table[h];
    he->flags = 0;
    hash_table[h] = he;
    return he;
}

static void add_to_hash_table P2(object_t *, ob, int, index) {
    int i;
    parse_info_t *pi = ob->pinfo;
    hash_entry_t *he;

    if (!pi) /* woops.  Dested during parse_command_users() or something
	        similarly nasty. */
	return;
    DEBUG_PP(("add_to_hash_table: %s", ob->name));
    for (i = 0; i < pi->num_ids; i++) {
	he = add_hash_entry(pi->ids[i]);
	he->flags |= HV_NOUN;
	he->pv.noun[BV_WHICH(index)] |= BV_BIT(index);
    }
    for (i = 0; i < pi->num_plurals; i++) {
	he = add_hash_entry(pi->plurals[i]);
	he->flags |= HV_PLURAL;
	he->pv.plural[BV_WHICH(index)] |= BV_BIT(index);
    }
    for (i = 0; i < pi->num_adjs; i++) {
	he = add_hash_entry(pi->adjs[i]);
	he->flags |= HV_ADJ;
	he->pv.adj[BV_WHICH(index)] |= BV_BIT(index);
    }

    if (pi->flags & PI_LIVING)
	cur_livings[BV_WHICH(index)] |= BV_BIT(index);
    
    if (object_flags[index])
	cur_accessible[BV_WHICH(index)] |= BV_BIT(index);
}

static void init_users() {
    svalue_t *ret;
    int i;
    object_t *ob;
    
    /* this should be cached */
    DEBUG_PP(("[master::parse_command_users]"));
    ret = apply_master_ob("parse_command_users", 0);
    if (!ret || ret->type != T_ARRAY) return;
    
    for (i = 0; i < ret->u.arr->size; i++) {
	if (ret->u.arr->item[i].type == T_OBJECT
	    && (ob = ret->u.arr->item[i].u.ob)->pinfo
	    && !(ob->pinfo->flags & PI_SETUP)) {
	    DEBUG_PP(("adding: %s", ob->name));
	    loaded_objects[num_objects++] = ob;
	    add_ref(ob, "init_users");
	}
    }
}

static void load_objects PROT((void)) {
    int i;
    svalue_t *ret;
    object_t *ob, *env;

    /* 1. Find things that need to be interrogated
     * 2. interrogate them
     * 3. build the object list
     *
     * If some of this looks suboptimal, consider:
     * 1. LPC code can error, and we don't want to leak, so we must be
     *    careful that all structures are consistent when we call LPC code
     * 2. LPC code can move objects, so we don't want to call LPC code
     *    while walking inventories.
     */
    bitvec_zero(cur_livings);
    bitvec_zero(cur_accessible);
    num_objects = 0;
    if (!parse_user || parse_user->flags & O_DESTRUCTED)
	error("No this_player()!\n");

    find_uninited_objects(parse_user->super);
    init_users();
    objects_loaded = 1;
    for (i = 0; i < num_objects; i++)
	interrogate_object(loaded_objects[i]);
    for (i = 0; i < num_objects; i++)
	free_object(loaded_objects[i], "free_parse_globals");
    num_objects = 0;
    
    rec_add_object(parse_user->super, 1);
    /* this should be cached */
    DEBUG_PP(("[master::parse_command_users]"));
    ret = apply_master_ob("parse_command_users", 0);
    num_people = 0;
    if (ret && ret->type == T_ARRAY) {
	for (i = 0; i < ret->u.arr->size; i++) {
	    if (ret->u.arr->item[i].type != T_OBJECT) continue;
	    /* check if we got them already */
	    ob = ret->u.arr->item[i].u.ob;
	    if (!(ob->pinfo))
		continue;
	    env = ob;
	    while (env) {
		if (env == parse_user->super)
		    break;
		env = env->super;
	    }
	    if (env) continue;
	    object_flags[num_objects + num_people] = 1;
	    loaded_objects[num_objects + num_people++] = ob;
	    add_ref(ob, "load_objects");
	}
    }
    num_objects += num_people;

    for (i = 0; i < num_objects; i++)
	add_to_hash_table(loaded_objects[i], i);
}

static int get_single P1(unsigned int *, bv) {
    int i, res = -1;

    DEBUG_PP(("get_single: %x %x %x %x", bv[0], bv[1], bv[2], bv[3]));
    for (i = 0; i < NUM_BITVEC_INTS; i++) {
	if (bv[i]) {
	    if (res != -1) return -1;
	    res = i;
	}
    }
    if (res < 0) return -1;

    i = bv[res];
    res *= 32;
    /* Binary search for the set bit, unrolled for speed. */
    if (i & 0x0000ffff) {
	if (i & 0xffff0000)
	    return -1;
    } else {
	i >>= 16;
	res += 16;
    }

    if (i & 0x00ff) {
	if (i & 0xff00)
	    return -1;
    } else {
	i >>= 8;
	res += 8;
    }
    
    if (i & 0x0f) {
	if (i & 0xf0)
	    return -1;
    } else {
	i >>= 4;
	res += 4;
    }
    
    if (i & 0x3) {
	if (i & 0xc)
	    return -1;
    } else {
	i >>= 2;
	res += 2;
    }
    
    if (i & 0x1) {
	if (i & 0x2)
	    return -1;
	DEBUG_PP((" -> %i", res));
	return res;
    } else {
	DEBUG_PP((" -> %i", res));
	return res + 1;
    }
}

/* equivalent to strcpy(x, y); return x + strlen(y), but faster */
static char *strput P2(char *, x, char *, y) {
    while ((*x++ = *y++))
	;
    return x - 1;
}

static char *query_the_short P2(char *, start, object_t *, ob) {
    svalue_t *ret;
    
    if (ob->flags & O_DESTRUCTED || 
	!(ret = apply("the_short", ob, 0, ORIGIN_DRIVER))
	|| ret->type != T_STRING) {
	return strput(start, "the thing");
    }
    return strput(start, ret->u.string);
}

static void error_is_not P4(char *, buf, hash_entry_t *, adj, 
			    int, multiple, hash_entry_t *, noun) {
    unsigned int *objects = noun->pv.noun;
    int ob;

    /* first case.  If the multiple flag is set, there was more than one
     * adjective.  We don't bother to figure out which adjectives do and
     * do not apply.  For example: 'sharp red sword'
     * There could be sharp swords, and red swords, but no sharp red ones.
     * We have a adjective they specified at this point, however, so we
     * use it to make a decent error message
     * We don't count swords in this case, since we can't do any better for
     * one object, unless we want to go through all the adjectives figuring
     * out which one (or more) is the bad one.
     */
    if (multiple) {
	sprintf(buf, "There is no such %s %s here.\n", 
		adj->name, noun->name);
	return;
    }

    /* If adj is zero, the failure was due to refering to a remote living */
    if (!adj) {
	sprintf(buf, "%s isn't here.\n", noun->name);
	if (islower(*buf))
	    *buf = toupper(*buf);
	return;
    }
    /* check if there is more than one of that noun.  In this case, we've
       only been called with zero or one adjectives */
    if ((ob = get_single(objects)) == -1) {
	/* 'the red sword' with many swords, none of
	   which are red */
	char *pl = pluralize(noun->name);
	
	sprintf(buf, "None of the %s are %s.\n", pl,
		adj->name);
	FREE_MSTR(pl);
	return;
    } else {
	/* Easy case.  'the red sword', one sword, which isn't red. */
	char *p;
	
	p = query_the_short(buf, loaded_objects[ob]);
	sprintf(p, " is not %s.\n", adj->name);
	if (islower(*buf))
	    *buf = toupper(*buf);
	return;
    }
}

static char *strput_words P3(char *, str, int, first, int, last) {
    char *p = words[first].start;
    char *end = words[last].end;

    /* can these happen any more? */
    while (isspace(p[0]))
	p++;
    while (isspace(end[0]))
	end--;
    while (p <= end)
	*str++ = *p++;
    *str = 0;
    return str;
}

static void error_there_is_no P3(char *, buf, int, start, int, end) {
    char *p;
    
    p = strput(buf, "There is no ");
    p = strput_words(p, start, end);
    strput(p, " here.\n");
}

static void error_not_living P4(char *, buf, unsigned int *, objects,
				hash_entry_t *, noun, hash_entry_t *, adj) {
    /* The X isn't alive. (if only 1 of noun)
       The X you refered to isn't alive.
       None of the X are alive.
       None of the X you refered to are alive */
    if (get_single(objects) != -1) {
	if (adj)
	    sprintf(buf, "The %s you refered to isn't alive.\n", noun->name);
	else
	    sprintf(buf, "The %s isn't alive.\n", noun->name);
    } else {
	char *pl = pluralize(noun->name);
	if (adj)
	    sprintf(buf, "None of the %s you refered to are alive.\n", pl);
	else
	    sprintf(buf, "None of the %s are alive.\n", pl);
	FREE_MSTR(pl);
    }
}

static void error_not_accessible P4(char *, buf, unsigned int *, objects,
				   hash_entry_t *, noun, hash_entry_t *, adj) {
    if (get_single(objects) != -1) {
	if (adj)
	    sprintf(buf, "The %s you refered to isn't within reach.\n", noun->name);
	else
	    sprintf(buf, "The %s isn't within reach.\n", noun->name);
    } else {
	char *pl = pluralize(noun->name);
	if (adj)
	    sprintf(buf, "None of the %s you refered to are within reach.\n", pl);
	else
	    sprintf(buf, "None of the %s are within reach.\n", pl);
	FREE_MSTR(pl);
    }
}

static int handle_ordinal P4(int, ordinal, unsigned int *, objects,
			     char *, buf, char *, noun) {
    int i,j,k, t = ordinal;
    
    for (i = 0; i < NUM_BITVEC_INTS; i++) {
	if (objects[i]) {
	    j = 1;
	    k = 0;
	    while (j) {
		if (objects[i] & j) {
		    if (!--t)
			return 32 * i + k;
		}
		j <<= 1;
		k++;
	    }
	}
    }
    i = ordinal - t;
    if (i == 1) {
	sprintf(buf, "There is only 1 %s here.\n", noun);
    } else {
	char *pl = pluralize(noun);
	sprintf(buf, "There are only %i %s here.\n", i, pl);
	FREE_MSTR(pl);
    }
    return -1;
}

static void error_ambig P3(char *, buf, hash_entry_t *, noun,
			   unsigned int *, objects) {
    int num_ambig = 0;
    int i, j, k;
    struct ambig_s {
	char *adj;
	int count;
    } ambig[4];
    char *p, *pl;
    
    /* Several cases here: 
     * A X, B X, C X -> Do you mean the A X, the B X, or the C X ?
     * A X, A X, B X -> Do you mean one of the A Xs, or the C X ?
     * A X, A X, A X -> Which X do you mean?
     */
    for (i = 0; i < NUM_BITVEC_INTS; i++) {
	if (objects[i]) for (j = 0; j < 32; j++) {
	    if (objects[i] & (1 << j)) {
		parse_info_t *pinfo = loaded_objects[i*32 + j]->pinfo;
		char *adj;
		if (pinfo->num_adjs)
		    adj = pinfo->adjs[0];
		else adj = 0;
		for (k = 0; k < num_ambig; k++) {
		    if (ambig[k].adj == adj) {
			ambig[k].count++;
			break;
		    }
		}
		if (k == num_ambig) {
		    num_ambig++;
		    ambig[k].adj = adj;
		    ambig[k].count = 1;
		}
	    }
	}
    }
    
    if (num_ambig == 1) {
	sprintf(buf, "Which %s do you mean?\n", noun->name);
	return;
    }

    pl = pluralize(noun->name);
    p = strput(buf, "Do you mean ");
    for (i = 0; i < num_ambig; i++) {
	if (i) p = strput(p, ", ");
	if (i == num_ambig - 1)
	    p = strput(p, "or ");
	if (ambig[i].count > 1)
	    p = strput(p, "one of ");
	p = strput(p, "the ");
	if (ambig[i].adj)
	    p = strput(p, ambig[i].adj);
	else
	    p = strput(p, "other");
	*p++ = ' ';
	if (ambig[i].count == 1)
	    p = strput(p, noun->name);
	else
	    p = strput(p, pl);
    }
    strput(p, "?\n");
    FREE_MSTR(pl);
}

static void parse_obj P3(int, tok, parse_state_t *, state,
			 int, ordinal) {
    parse_state_t local_state;
    unsigned int objects[NUM_BITVEC_INTS];
    unsigned int save_obs[NUM_BITVEC_INTS];
    unsigned int err_obs[NUM_BITVEC_INTS];
    int start = state->word_index;
    char buf[1024];
    char *str;
    hash_entry_t *hnode, *last_adj = 0;
    int multiple_adj = 0;
    int tmp, ord_legal = (ordinal == 0);
    match_t *mp;

    DEBUG_INC;
    DEBUG_P(("parse_obj:"));
    
    all_objects(objects, parse_vn->handler->pinfo->flags & PI_REMOTE_LIVINGS);

    while (1) {
	str = words[state->word_index++].string;
	DEBUG_PP(("Word is %s", str));
	switch (check_special_word(str, &tmp)) {
	case SW_ARTICLE:
	    continue;
	case SW_SELF: 
	    {
		local_state = *state;
		mp = add_match(&local_state, tok,
			       start, state->word_index - 1);
		mp->val.number = me_object;
		parse_rule(&local_state);
		break;
	    }
	case SW_ORDINAL:
	    if (ord_legal) {
		local_state = *state;
		parse_obj(tok, &local_state, tmp);
	    }
	    break;
	}
	ord_legal = 0;
	hnode = hash_table[DO_HASH(str, HASH_SIZE)];
	while (hnode) {
	    if (hnode->name == str) {
		if (hnode->flags & HV_NOUN) {
		    int explore_errors = !best_match &&
			state->num_errors < best_num_errors;
		    DEBUG_P(("Found noun: %s", str));
		    local_state = *state;
		    bitvec_copy(save_obs, objects);

		    /* Sigh, I want to throw exceptions */
		    if (!intersect(objects, hnode->pv.noun)) {
			if (!explore_errors) goto skip_it;
			
			error_is_not(buf, last_adj, multiple_adj, hnode);
			goto we_have_an_error;
		    }
		    if ((-tok) & LIV_MODIFIER) {
			if (explore_errors)
			    bitvec_copy(err_obs, objects);
			if (!intersect(objects, cur_livings)) {
			    if (!explore_errors) goto skip_it;
			
			    error_not_living(buf, err_obs, hnode, last_adj);
			    goto we_have_an_error;
			}
		    }
		    if (!((-tok) & VIS_ONLY_MODIFIER)) {
			if (explore_errors)
			    bitvec_copy(err_obs, objects);
			if (!intersect(objects, cur_accessible)) {
			    if (!explore_errors) goto skip_it;
			
			    error_not_accessible(buf, err_obs, hnode, last_adj);
			    goto we_have_an_error;
			}
		    }
		    if (ordinal) {
			tmp = handle_ordinal(ordinal, objects, buf, str);
			if (tmp == -1) goto we_have_an_error;
		    } else
		    if ((tmp = get_single(objects)) == -1) {
			if (!explore_errors) goto skip_it;
			
			error_ambig(buf, hnode, objects);
			goto we_have_an_error;
		    }
		    DEBUG_P(("Found object: %s", loaded_objects[tmp]->name));
		    mp = add_match(&local_state, tok,
				   start, state->word_index - 1);
		    mp->val.number = tmp;
		    goto do_the_parse;

		we_have_an_error:
		    DEBUG_P((buf));
		    mp = add_match(&local_state, ERROR_TOKEN,
				   start, state->word_index - 1);
		    mp->val.string = buf;

		do_the_parse:
		    parse_rule(&local_state);

		skip_it:
		    bitvec_copy(objects, save_obs);
		}
		if (hnode->flags & HV_ADJ) {
		    DEBUG_P(("Found adj: %s", str));
		    intersect(objects, hnode->pv.adj);
		    if (last_adj) multiple_adj = 1;
		    last_adj = hnode;
		} else {
		    DEBUG_DEC;
		    return;
		}
		break;
	    }
	    hnode = hnode->next;
	}
	if (!hnode) break;
    }
    DEBUG_PP(("exiting ..."));
    DEBUG_DEC;
}

/* misnomer; this func is not recursive */
static verb_t *parse_verb P1(char *, str) {
    char *vb = findstring(str);
    verb_t *ret;

    if (!vb) return 0;

    ret = verbs[DO_HASH(vb, VERB_HASH_SIZE)];
    while (ret) {
	if (ret->name == vb) {
	    words[0].string = vb;
	    words[0].type = 0;
	    return ret;
	}
	ret = ret->next;
    }
    return 0;
}

static char *make_error_message P1(int, which) {
    char buf[1024];
    char *p;
    int cnt = 0;
    int tok;
    int index = 0;
    
    p = strput(buf, "You can't ");
    p = strput(p, words[0].string);
    *p++ = ' ';
    while ((tok = parse_vn->token[index++])) {
	switch (tok) {
	case STR_TOKEN:
	    if (cnt == which - 1) {
		p = strput(p, "that ");
		cnt++;
		break;
	    }
	    /* FALLTHRU */
	case WRD_TOKEN:
	    p = strput_words(p, matches[cnt].first, matches[cnt].last);
	    *p++ = ' ';
	    cnt++;
	    break;
	default:
	    if (tok >= 0) {
		p = strput(p, literals[tok - 1]);
		*p++ = ' ';
	    } else {
		if (cnt == which - 1) {
		    p = strput(p, "that ");
		} else {
		    p = query_the_short(p, loaded_objects[matches[cnt].val.number]);
		    *p++ = ' ';
		}
		cnt++;
	    }
	    break;
	}
    }
    p--;
    strcpy(p, ".\n");
    DEBUG_P((buf));
    return string_copy(buf, "make_error_message");
}

static char *current_possible_error = 0;

static int process_answer P3(parse_state_t *, state, svalue_t *, sv,
			     int, which) {
    if (!sv) return 0;
    if (sv->type == T_NUMBER) {
	DEBUG_P(("Return value was: %i", sv->u.number));
	if (sv->u.number)
	    return 1;
	if (state->num_errors == best_num_errors) {
	    DEBUG_P(("Have a better match; aborting ..."));
	    return -2;
	}
	if (state->num_errors++ == 0) {
	    if (current_possible_error) FREE_MSTR(current_possible_error);
	    current_possible_error = make_error_message(which);
	}
	return -1;
    }
    if (sv->type != T_STRING) {
	DEBUG_P(("Return value was not a string or number.", sv->u.number));
	return 0;
    }
    DEBUG_P(("Returned string was: %s", sv->u.string));
    if (state->num_errors == best_num_errors) {
	DEBUG_P(("Have a better match; aborting ..."));
	return -2;
    }
    if (state->num_errors++ == 0) {
	if (current_possible_error) FREE_MSTR(current_possible_error);
	current_possible_error = string_copy(sv->u.string, "process_answer");
    }
    return -1;
}

static int push_real_names P1(int, try) {
    int index = 0, match = 0;
    int tok;

    if (try >= 2) {
	char tmpbuf[1024];
	strput_words(tmpbuf, 0, 0);
	push_string(tmpbuf, STRING_SHARED);
    }
    
    while ((tok = parse_vn->token[index++])) {
	if (tok < 0) {
	    char tmp[1024];

	    strput_words(tmp, matches[match].first, matches[match].last);
	    push_malloced_string(string_copy(tmp, "push_real_names"));
	    match++;
	}
    }
    return match + (try >= 2);
}

char *rule_string P1(verb_node_t *, vn) {
    int index = 0;
    int tok;
    static char buf[1024];
    char *p;

    p = buf;
    
    while (1) {
	switch (tok = vn->token[index++]) {
	case OBJ_A_TOKEN:
	case OBJ_TOKEN:
	    p = strput(p, "OBJ ");
	    break;
	case LIV_A_TOKEN:
	case LIV_TOKEN:
	    p = strput(p, "LIV ");
	    break;
	case OBS_TOKEN:
	case ADD_MOD(OBS_TOKEN, VIS_ONLY_MODIFIER):
	    p = strput(p, "OBS ");
	    break;
	case LVS_TOKEN:
	case ADD_MOD(LVS_TOKEN, VIS_ONLY_MODIFIER):
	    p = strput(p, "LVS ");
	    break;
	case STR_TOKEN:
	    p = strput(p, "STR ");
	    break;
	case WRD_TOKEN:
	    p = strput(p, "WRD ");
	    break;
	case 0:
	    if (p == buf) {
		*buf = 0;
	    } else {
		*(p-1) = 0; /* nuke last space */
	    }
            return buf;
	default:
	    p = strput(p, literals[tok - 1]);
	    *p++ = ' ';
	    break;
	}
    }
}

static int make_function P4(char *, buf, char *, pre, 
			    parse_state_t *, state, int, try) {
    int index = 0, match = 0;
    int on_stack = 0;
    int tok;
    /* try = 0: "read_about_str_from_obj"
     * try = 1: "read_word_str_word_obj"
     * try = 2: "verb_word_str_word_obj"
     * try = 3: "verb_rule"
     */

    buf = strput(buf, pre);
    if (try < 2) {
	buf = strput(buf, words[0].string);
    } else {
	buf = strput(buf, "verb");
	push_string(words[0].string, STRING_SHARED);
	on_stack++;
    }

    if (try == 3) {
	buf = strput(buf, "_rule");
	/* leave the 0; this effectively truncates the string. */
	buf++;
	push_constant_string(rule_string(parse_vn));
	on_stack++;
    }
    while ((tok = parse_vn->token[index++])) {
	*buf++ = '_';
	switch (tok) {
	case OBJ_TOKEN:
	case OBJ_A_TOKEN:
	    buf = strput(buf, "obj");
	    goto put_obj_value;

	case OBS_TOKEN:
	case ADD_MOD(OBS_TOKEN, VIS_ONLY_MODIFIER):
	    buf = strput(buf, "obs");
	    goto put_obj_value;
	    
	case LVS_TOKEN:
	case ADD_MOD(LVS_TOKEN, VIS_ONLY_MODIFIER):
	    buf = strput(buf, "lvs");
	    goto put_obj_value;

	case LIV_TOKEN:
	case LIV_A_TOKEN:
	    buf = strput(buf, "liv");

	put_obj_value:
	    if (matches[match].token == ERROR_TOKEN || 
		(loaded_objects[matches[match].val.number]->flags & O_DESTRUCTED)) {
		push_number(0);
	    } else {
		push_object(loaded_objects[matches[match].val.number]);
	    }
	    match++;
	    on_stack++;
	    break;
	case STR_TOKEN:
	    {
		char tmp[1024];
		buf = strput(buf, "str");
		strput_words(tmp, matches[match].first, matches[match].last);
		push_malloced_string(string_copy(tmp, "push_real_names"));
		match++;
		on_stack++;
	    }
	    break;
	case WRD_TOKEN:
	    {
		char tmp[1024];
		buf = strput(buf, "wrd");
		strput_words(tmp, matches[match].first, matches[match].last);
		push_malloced_string(string_copy(tmp, "push_real_names"));
		match++;
		on_stack++;
	    }
	    break;
	default:
	    if (!try) {
		buf = strput(buf, literals[tok - 1]);
	    } else if (try < 3) {
		buf = strput(buf, "word");
		push_string(literals[tok - 1], STRING_SHARED);
		on_stack++;
	    }
	}
    }
    return on_stack;
}

static char *prefixes[] = { "can_", "direct_", "indirect_" };

#define SET_OB(x) ob = (x); if (ob->flags & O_DESTRUCTED) return;

static void we_are_finished P1(parse_state_t *, state) {
    char func[256];
    char *p;
    int ret;
    int which, try, args, mtch = 0;
    object_t *ob;
    int local_error;

    DEBUG_INC;
    DEBUG_P(("we_are_finished"));
    
    /* ignore it if we already have somethign better */
    if (best_match >= parse_vn->weight) {
	DEBUG_P(("Have a better match; aborting ..."));
	DEBUG_DEC;
	return;
    }
    if (state->num_errors) {
	local_error = 0;
	if (state->num_errors > best_num_errors) return;
	if (state->num_errors == best_num_errors
	    && parse_vn->weight < best_error_match) return;
    } else local_error = 1; /* if we have an error, it was local */
    SET_OB(parse_user);

    for (which = 0; which < 3; which++) {
	if (ob)
	    for (try = 0, ret = 0; !ret && try < 8; try++) {
		if (try == 4) {
		    SET_OB(parse_vn->handler);
		}
		args = make_function(func, prefixes[which], state, try % 4);
		args += push_real_names(try);
		DEBUG_P(("Trying %s ...", func));
		ret = process_answer(state,
				     apply(func, ob, args, ORIGIN_DRIVER),
				     which);
		if (ob->flags & O_DESTRUCTED) {
		    DEBUG_DEC;
		    return;
		}
		if (ret == -2) {
		    DEBUG_DEC;
		    return;
		}
		if (ret) break;
	    }
	if (try == 8) {
	    if (state->num_errors == best_num_errors) {
		DEBUG_P(("Nothing matched and we have a better match"));
		DEBUG_DEC;
		return;
	    }
	    if (state->num_errors++ == 0) {
		if (current_possible_error) FREE_MSTR(current_possible_error);
		current_possible_error = make_error_message(which);
	    }
	}
	while (mtch != state->num_matches && !((-matches[mtch].token) & OBJ_A_TOKEN)) {
	    mtch++;
	    if (mtch == state->num_matches) break;
	}
	if (mtch == state->num_matches) break;
	if (matches[mtch].token != ERROR_TOKEN) {
	    SET_OB(loaded_objects[matches[mtch].val.number]);
	} else {
	    DEBUG_P(("Skipping next iteration; match is an error"));
	    ob = 0;
	}
	mtch++;
    }
    if (state->num_errors) {
	if (state->num_errors == best_num_errors &&
	    parse_vn->weight <= best_error_match) {
	    DEBUG_P(("Have better match; aborting ..."));
	    DEBUG_DEC;
	    return;
	}
	best_num_errors = state->num_errors;
	best_error_match = parse_vn->weight;
	if (current_error) FREE_MSTR(current_error);
	if (local_error) {
	    current_error = current_possible_error;
	    current_possible_error = 0;
	} else {
	    for (mtch = 0; matches[mtch].token != ERROR_TOKEN; mtch++)
		;
	    current_error = string_copy(matches[mtch].val.string, 
					"we_are_finished");		
	}
	DEBUG_P(("current error set to %s", current_error));
    } else {
	best_match = parse_vn->weight;
	if (best_result) free_parse_result(best_result);
	best_result = ALLOCATE(parse_result_t, TAG_PARSER, "we_are_finished");
	clear_result(best_result);
	SET_OB(parse_vn->handler);
	best_result->ob = ob;
	add_ref(ob, "best_result");
	for (try = 0; try < 4; try++) {
	    args = make_function(func, "do_", state, try);
	    args += push_real_names(try);
	    best_result->res[try].func = string_copy(func, "best_result");
	    best_result->res[try].num = args;
	    if (args) {
		p = (char *)(best_result->res[try].args = CALLOCATE(args,
				       svalue_t, TAG_PARSER, "best_result"));
		memcpy(p, (char *)(sp - args + 1), args * sizeof(svalue_t));
		sp -= args;
	    }
	}
	DEBUG_P(("Saving successful match: %s", best_result->res[0].func));
    }
    DEBUG_DEC;
}

static void do_the_call PROT((void)) {
    int i, n;
    object_t *ob = best_result->ob;
    
    for (i = 0; i < 4; i++) {
	if (ob->flags & O_DESTRUCTED) return;
	n = best_result->res[i].num;
	if (n) {
	    memcpy((char *)(sp + 1), best_result->res[i].args, n*sizeof(svalue_t));
	    /*
	     * Make sure we haven't dumped any dested obs onto the stack;
	     * this also updates sp.
	     */
	    while (n--) {
		if ((++sp)->type == T_OBJECT && (sp->u.ob->flags & O_DESTRUCTED)) {
		    free_object(sp->u.ob, "do_the_call");
		    *sp = const0;
		}
	    }
	    FREE(best_result->res[i].args);
	}
	best_result->res[i].args = 0;
	DEBUG_P(("Calling %s ...", best_result->res[i].func));
	if (apply(best_result->res[i].func, ob,
		  best_result->res[i].num, ORIGIN_DRIVER)) return;
    }
    error("Parse accepted, but no do_* function found in object %s!\n",
	  ob->name);
}

static void parse_rule P1(parse_state_t *, state) {
    char buf[1024];
    int tok;
    parse_state_t local_state;
    match_t *mp;
    int start;

    DEBUG_INC;
    DEBUG_P(("parse_rule"));
    while (1) {
	tok = parse_vn->token[state->tok_index++];
	if (state->word_index == num_words && tok) {
	    DEBUG_P(("Ran out of words to parse."));
	    DEBUG_DEC;
	    return;
	}
	switch (tok) {
	case 0:
	    if (state->word_index == num_words)
		we_are_finished(state);
	    DEBUG_P(("exiting parse_rule ..."));
	    DEBUG_DEC;
	    return;
	case OBJ_TOKEN:
	case LIV_TOKEN:
	case OBJ_A_TOKEN:
	case LIV_A_TOKEN:
	case OBS_TOKEN:
	case ADD_MOD(OBS_TOKEN, VIS_ONLY_MODIFIER):
	case LVS_TOKEN:
	case ADD_MOD(LVS_TOKEN, VIS_ONLY_MODIFIER):
	    local_state = *state;
	    parse_obj(tok, &local_state, 0);
	    if (!best_match && state->num_errors < best_num_errors) {
		start = state->word_index++;
		while (state->word_index <= num_words) {
		    local_state = *state;
		    mp = add_match(&local_state, ERROR_TOKEN,
				   start, state->word_index - 1);
		    error_there_is_no(buf, start, state->word_index - 1);
		    mp->val.string = buf;
		    parse_rule(&local_state);
		    state->word_index++;
		}
	    }
	    DEBUG_P(("Done trying to match OBJ"));
	    DEBUG_DEC;
	    return;
	case STR_TOKEN:
	    if (!parse_vn->token[state->tok_index]) {
		/* At end; match must be the whole thing */
		start = state->word_index;
		state->word_index = num_words;
		add_match(state, STR_TOKEN, start, num_words - 1);
		DEBUG_P(("Taking rest of sentence as STR"));
		parse_rule(state);
	    } else {
		start = state->word_index++;
		while (state->word_index <= num_words) {
		    local_state = *state;
		    add_match(&local_state, STR_TOKEN,
			      start, state->word_index - 1);
		    DEBUG_P(("Trying potential STR match"));
		    parse_rule(&local_state);
		    state->word_index++;
		}
	    }
	    DEBUG_P(("Done trying to match STR"));
	    DEBUG_DEC;
	    return;
	case WRD_TOKEN:
	    add_match(state, WRD_TOKEN, state->word_index, state->word_index);
	    state->word_index++;
	    DEBUG_P(("Trying WRD match"));
	    parse_rule(state);
	    return;
	default:
	    if (literals[tok - 1] == words[state->word_index].string) {
		state->word_index++;
		DEBUG_P(("Matched literal: %s", literals[tok - 1]));
	    } else {
		if (state->tok_index == 1) {
		    DEBUG_DEC;
		    return;
		}
		
		DEBUG_P(("last match to error ..."));
		switch (parse_vn->token[state->tok_index - 2]) {
		case STR_TOKEN:
		    DEBUG_P(("Nope.  STR rule last."));
		    DEBUG_DEC;
		    return;
		case OBJ_TOKEN:
		case LIV_TOKEN:
		case OBJ_A_TOKEN:
		case LIV_A_TOKEN:
		case OBS_TOKEN:
		case ADD_MOD(OBS_TOKEN, VIS_ONLY_MODIFIER):
		case LVS_TOKEN:
		case ADD_MOD(LVS_TOKEN, VIS_ONLY_MODIFIER):
		    {
			match_t *last;
			
			while (literals[tok - 1] != words[state->word_index++].string) {
			    if (state->word_index == num_words) {
				DEBUG_P(("Literal not found in forward search"));
				DEBUG_DEC;
				return;
			    }
			}
			last = &matches[state->num_matches-1];
			DEBUG_P(("Changing last match."));
			last->token = ERROR_TOKEN;
			last->last = state->word_index-1;
			if (state->num_errors++ == 0) {
			    error_there_is_no(buf, last->first, 
					      state->word_index - 1);
			    last->val.string = buf;
			}
		    }
		    break;
		default:
		    DEBUG_P(("default case"));
		    DEBUG_DEC;
		    return;
		}
	    }
	}
    }
    DEBUG_DEC;
}

static int check_literal P2(int, lit, int, start) {
    DEBUG_PP(("check_literal: %s", literals[lit - 1]));

    while (start < num_words) {
	if (literals[lit - 1] == words[start++].string) {
	    DEBUG_PP(("yes"));
	    return start;
	}
    }
    DEBUG_PP(("no"));
    return 0;
}

static void parse_rules PROT((void)) {
    int pos;
    parse_state_t local_state;

    parse_vn = parse_verb_entry->node;
    while (parse_vn) {
	DEBUG_PP(("Rule: %s", rule_string(parse_vn)));
	if ((!parse_restricted || parse_vn->handler == parse_restricted)
	    && (best_match <= parse_vn->weight))  {
	    pos = 0;
	    if ((!parse_vn->lit[0] ||
		 (pos = check_literal(parse_vn->lit[0], 1)))
		&& (!parse_vn->lit[1] ||
		    check_literal(parse_vn->lit[1], pos))) {
		DEBUG_P(("Trying rule: %s", rule_string(parse_vn)));

		local_state.tok_index = 0;
		local_state.word_index = 1;
		local_state.num_matches = 0;
		local_state.num_errors = 0;
		parse_rule(&local_state);
	    }
	}
	parse_vn = parse_vn->next;
    }
}

static void reset_error PROT((void)) {
    if (current_error)
	FREE_MSTR(current_error);
    current_error = 0;
    best_match = 0;
    best_error_match = 0;
    best_num_errors = 5732; /* Yes.  Exactly 5,732 errors.  Don't ask. */
}

static void parse_recurse P3(char **, iwords, char **, ostart, char **, oend) {
    char buf[1024];
    char *p, *q;
    char **iwp = iwords;
    int first = 1;
    int l, idx;

    if (*iwords[0]) {
	*buf = 0;
	p = buf;
	do {
	    l = iwp[1] - iwp[0] - 1;
	    strcpy(p, *iwp++);
	    p += l;
	    if ((q = findstring(buf))) {
		words[num_words].type = 0;
		words[num_words].start = ostart[0];
		words[num_words].end = oend[iwp - iwords - 1];
		words[num_words++].string = q;
		idx = iwp - iwords;
		parse_recurse(iwp, ostart + idx, oend + idx);
		num_words--;
	    } else if (first) {
		l = p - buf;
		words[num_words].type = WORD_ALLOCATED;
		words[num_words].string = new_string(l, "parse_recurse");
		words[num_words].start = ostart[0];
		words[num_words].end = oend[iwp - iwords - 1];
		memcpy(words[num_words].string, buf, l);
		words[num_words++].string[l] = 0;
		idx = iwp - iwords;
		parse_recurse(iwp, ostart + idx, oend + idx);
		num_words--;
		FREE_MSTR(words[num_words].string);
	    }
	    first = 0;
	    *p++ = ' ';
	} while (*iwp[0]);
    } else {
#ifdef DEBUG
	if (debug_parse_depth) {
	    char dbuf[1024];
	    int i;
	    char *p;
	    p = strput(dbuf, "Trying interpretation: ");
	    for (i = 0; i < num_words; i++) {
		p = strput(p, words[i].string);
		p = strput(p, ":");
	    }
	    DEBUG_P((dbuf));
	}
#endif
	parse_rules();
    }
}

static void parse_sentence P1(char *, input) {
    char *starts[256];
    char *orig_starts[256];
    char *orig_ends[256];
    char c, buf[1024], *p, *start, *inp;
    int n = 0;
    int i;
    int flag;
    
    free_words();
    p = start = buf;
    flag = 0;
    inp = input;
    while (*inp && (isspace(*inp) || isignore(*inp)))
	inp++;
    orig_starts[0] = inp;
    
    while ((c = *inp++)) {
	if (isignore(c)) continue;
	if (isupper(c))
	    c = tolower(c);
	if (iskeep(c)) {
	    if (!flag)
		flag = 1;
	    *p++ = c;
	} else {
	    /* whitespace or punctuation */
	    if (!isspace(c))
		while (*inp && !iskeep(*inp) && !isspace(*inp))
		    inp++;
	    else
		inp--;

	    if (flag) {
		flag = 0;
		*p++ = 0;
		orig_ends[n] = inp - 1; /* points to where c was */
		starts[n++] = start;
		start = p;
		while (*inp && isspace(*inp))
		    inp++;
		orig_starts[n] = inp;
	    } else {
		while (*inp && isspace(*inp))
		    inp++;
	    }
	}
    }
    if (flag) {
	*p++ = 0;
	orig_ends[n] = inp - 2;
	starts[n++] = start;
    } else {
	if (n)
	    orig_ends[n - 1] = inp - 2;
	else
	    orig_ends[0] = inp - 2;
    }
    starts[n] = p;
    *p = 0;

    reset_error();
    /* find an interpretation, first word must be shared (verb) */
    for (i = 1; i <= n; i++) {
	if ((parse_verb_entry = parse_verb(buf))) {
	    if (!objects_loaded && (parse_verb_entry->flags & VB_HAS_OBJ)) 
		load_objects();
	    num_words = 1;
	    words[0].start = orig_starts[0];
	    words[0].end = orig_ends[i-1];
	    parse_recurse(&starts[i], &orig_starts[i], &orig_ends[i]);
	}
	starts[i][-1] = ' ';
    }
}

void f_parse_sentence PROT((void)) {
    if (!current_object->pinfo)
	error("%s is not known by the parser.  Call parse_init() first.\n",
	      current_object->name);

    if (pi)
	error("Illegal to call parse_sentence() recursively.\n");
    
    /* may not be done in case of an error, or in case of tail recursion.
       if we are called tail recursively, we don't need this any more.
       */
    if (best_result) {
	free_parse_result(best_result);
	best_result = 0;
    }

    if (st_num_arg == 2 && (sp--)->u.number) {
#ifdef DEBUG
	debug_parse_depth = 1;
	if ((sp + 1)->u.number > 1)
	    debug_parse_verbose = 1;
	else
	    debug_parse_verbose = 0;
    } else {
	debug_parse_depth = 0;
#endif
    }

    (++sp)->type = T_ERROR_HANDLER;
    sp->u.error_handler = free_parse_globals;

    parse_user = current_object;
    pi = current_object->pinfo;
    parse_restricted = 0;
    parse_sentence((sp-1)->u.string);

    free_parse_globals();
    
    if (best_match) {
	do_the_call();
	sp--; /* pop the error handler */
	free_string_svalue(sp);
	put_number(1);
    } else {
	sp--; /* pop the error handler */
	free_string_svalue(sp);
	if (current_error) {
	    sp->subtype = STRING_MALLOC;
	    sp->u.string = current_error;
	    current_error = 0;
	} else put_number(0);
    }

    if (best_result) {
	free_parse_result(best_result);
	best_result = 0;
    }
}

void f_parse_my_rules PROT((void)) {
    int flag = (st_num_arg == 3 ? (sp--)->u.number : 0);
    
    if (!(sp-1)->u.ob->pinfo)
	error("%s is not known by the parser.  Call parse_init() first.\n",
	      (sp-1)->u.ob->name);
    if (!current_object->pinfo)
	error("%s is not known by the parser.  Call parse_init() first.\n",
	      current_object->name);

    if (pi)
	error("Illegal to call parse_sentence() recursively.\n");
    
    (++sp)->type = T_ERROR_HANDLER;
    sp->u.error_handler = free_parse_globals;

    parse_user = (sp-2)->u.ob;
    pi = parse_user->pinfo;
    parse_restricted = current_object;
    parse_sentence((sp-1)->u.string);
    
    if (best_match) {
	if (flag) {
	    do_the_call();
	    sp--; /* pop the error handler */
	    free_string_svalue(sp--);
	    free_svalue(sp, "parse_my_rules"); /* may have been destructed */
	    put_number(1);
	} else {
	    int n;
	    array_t *arr;
	    /* give them the info for the wildcard call */
	    n = best_result->res[3].num;
	    arr = allocate_empty_array(n);
	    if (n) {
		memcpy((char *)arr->item, best_result->res[3].args, n*sizeof(svalue_t));
		while (n--) {
		    if (arr->item[n].type == T_OBJECT && arr->item[n].u.ob->flags & O_DESTRUCTED) {
			free_object(arr->item[n].u.ob, "parse_my_rules");
			arr->item[n] = const0;
		    }
		}
		FREE(best_result->res[3].args);
	    }
	    best_result->res[3].args = 0;
	    sp--; /* pop the error handler */
	    free_string_svalue(sp--);
	    free_svalue(sp, "parse_my_rules"); /* may have been destructed */
	    put_array(arr);
	}
    } else {
	sp--; /* pop the error handler */
	free_string_svalue(sp--);
	free_svalue(sp, "parse_my_rules"); /* may have been destructed */
	if (current_error) {
	    sp->type = T_STRING;
	    sp->subtype = STRING_MALLOC;
	    sp->u.string = current_error;
	    current_error = 0;
	} else put_number(0);
    }
    free_parse_globals();
}

void f_parse_add_rule() {
    int tokens[10];
    int lit[2], i, j;
    svalue_t *ret;
    char *verb, *rule;
    object_t *handler;
    verb_t *verb_entry;
    verb_node_t *verb_node;
    int h;
    int weight;
    
    rule = (sp-1)->u.string;
    verb_entry = 0;
    verb = findstring((sp-2)->u.string);
    CHECK_TYPES(sp, T_OBJECT, 3, F_PARSE_ADD_RULE);
    handler = sp->u.ob;
    if (!(handler->pinfo))
	error("%s is not known by the parser.  Call parse_init() first.\n",
	      handler->name);

    /* Create the rule */
    make_rule(rule, tokens, &weight);

    /* Now find a verb entry to put it in */
    if (verb) {
	verb_entry = verbs[DO_HASH(verb, VERB_HASH_SIZE)];
	while (verb_entry) {
	    if (verb_entry->name == verb)
		break;
	    verb_entry = verb_entry->next;
	}
    }
    
    if (!verb_entry) {
	if (!verb)
	    verb = make_shared_string((sp-2)->u.string);
	else
	    ref_string(verb);
	
	h = DO_HASH(verb, VERB_HASH_SIZE);
	verb_entry = ALLOCATE(verb_t, TAG_PARSER, "parse_add_rule");
	verb_entry->name = verb;
	verb_entry->node = 0;
	verb_entry->flags = 0;
	verb_entry->next = verbs[h];
	verbs[h] = verb_entry;
    }

    /* Add a new node */
    for (i = 0, j = 0; tokens[i]; i++) {
	if (tokens[i] > 0 && j < 2)
	    lit[j++] = tokens[i];
    }

    while (j < 2)
	lit[j++] = 0;

    verb_node = (verb_node_t *)DXALLOC(sizeof(verb_node_t) + sizeof(int)*i,
				       TAG_PARSER, "parse_add_rule");

    verb_node->lit[0] = lit[0];
    verb_node->lit[1] = lit[1];
    for (j = 0; j <= i; j++) {
	if (tokens[j] <= OBJ_A_TOKEN)
	    verb_entry->flags |= VB_HAS_OBJ;
	verb_node->token[j] = tokens[j];
    }
    verb_node->weight = weight;
    verb_node->handler = handler;
    handler->pinfo->flags |= PI_VERB_HANDLER;
    verb_node->next = verb_entry->node;
    verb_entry->node = verb_node;

    ret = apply("livings_are_remote", handler, 0, ORIGIN_DRIVER);
    if (!IS_ZERO(ret))
        handler->pinfo->flags |=PI_REMOTE_LIVINGS;

    /* return */
    pop_stack();
    free_string_svalue(sp--);
    free_string_svalue(sp--);
}

void f_parse_dump PROT((void))
{
    int i;
    outbuffer_t ob;

    outbuf_zero(&ob);
    for (i = 0; i < VERB_HASH_SIZE; i++) {
	verb_t *v = verbs[i];
	while (v) {
	    verb_node_t *vn = v->node;
	    outbuf_addv(&ob, "Verb %s:\n", v->name);
	    while (vn) {
		outbuf_addv(&ob, "  (%s) %s\n", vn->handler->name, rule_string(vn));
		vn = vn->next;
	    }
	    v = v->next;
	}
    }
    outbuf_push(&ob);
}
