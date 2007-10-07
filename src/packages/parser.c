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
 * . one, two, ...
 * . questions.  'Take what?'
 * . oops
 * . the 'her' ambiguity
 * . foo, who is ...   foo, where is ...   foo, what is ...   foo, go south
 * . where is ... "where is sword" -> "In the bag on the table"
 * . > Which button do you mean, the camera button or the recorder button?
 * . Zifnab's crasher
 */

#include "../std.h"
#include "../lpc_incl.h"
#include "parser.h"
#include "../md.h"
#include "../master.h"
#include "../efun_protos.h"

/*
 * These match routines in the LIMA mudlib.  [The fact that this file was
 * written by an author of the LIMA mudlib is just a coincidence.  Honest.]
 */
#define IS_LIVING               "is_living"
#define INVENTORY_ACCESSIBLE    "inventory_accessible"
#define INVENTORY_VISIBLE       "inventory_visible"
/* Named during a fit of madness */
#define LIVINGS_ARE_REMOTE      "livings_are_remote"

#define MAX_WORDS_PER_LINE 256
#define MAX_WORD_LENGTH 1024
#define MAX_MATCHES 10

char *pluralize (char *);

#define MS_HAS_LITERALS   1
#define MS_HAS_SPECIALS   2
#define MS_HAS_USERS      4
static int master_state = 0;

static parse_info_t *pi = 0;
static hash_entry_t *hash_table[HASH_SIZE];
static special_word_t *special_table[SPECIAL_HASH_SIZE];
static verb_t *verbs[VERB_HASH_SIZE];
static int objects_loaded = 0;
static int num_objects, num_people, me_object;
static struct object_s *loaded_objects[MAX_NUM_OBJECTS];
static int object_flags[MAX_NUM_OBJECTS];
static bitvec_t my_objects;
static char *my_string = 0;
static int num_literals = 0;
static char **literals;
static word_t words[MAX_WORDS_PER_LINE];
static int num_words = 0;
static verb_node_t *parse_vn;
static verb_t *parse_verb_entry;
static object_t *parse_restricted;
static object_t *parse_user;
static array_t *master_user_list = 0;
static bitvec_t cur_livings;
static bitvec_t cur_accessible;
static int best_match;
static int best_error_match;
static int best_num_errors;
static parse_result_t *best_result = 0;
static match_t matches[MAX_MATCHES];
static int found_level = 0;
static mapping_t *parse_nicks = 0;
static array_t *parse_env = 0;

static int direct_object, indirect_object;

static parser_error_t current_error_info;
static parser_error_t best_error_info;
static parser_error_t parallel_error_info;
static parser_error_t second_parallel_error_info;

static saved_error_t *parallel_errors = 0;

#if defined(DEBUG) || defined(PARSE_DEBUG)
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

static void parse_rule (parse_state_t *);
static void clear_parallel_errors (saved_error_t **);
static svalue_t *get_the_error (parser_error_t *, int);

#define isignore(x) (!uisprint(x) || x == '\'')
#define iskeep(x) (uisalnum(x) || x == '*')

#define SHARED_STRING(x) ((x)->subtype == STRING_SHARED ? (x)->u.string : findstring((x)->u.string))

#define NEED_REFRESH(ob) (ob->pinfo && ((ob->pinfo->flags & (PI_SETUP|PI_REFRESH)) != PI_SETUP))

/* parse_init() - setup the object
 * parse_refresh() - refresh an object's parse data
 * parse_add_rule(verb, rule) - add a rule for a verb
 * parse_sentence(sent) - do the parsing :)
 */

#ifdef DEBUGMALLOC_EXTENSIONS
static void mark_error (parser_error_t * pe) {
    if (pe->error_type == ERR_ALLOCATED) {
        MSTR_EXTRA_REF(pe->err.str)++;
    }
}

void parser_mark_verbs() {
    int i, j;

    if (best_result) {
#ifdef DEBUGMALLOC_EXTENSIONS
        if (best_result->ob)
            best_result->ob->extra_ref++;
#endif
        if (best_result->parallel)
            /* mark parallel errors */;
        
        for (i = 0; i < 4; i++) {
            if (best_result->res[i].func)
                MSTR_EXTRA_REF(best_result->res[i].func)++;
            if (best_result->res[i].args) {
                for (j = 0; j < best_result->res[i].num; j++)
                    mark_svalue(((svalue_t *)best_result->res[i].args) + j);
                /* mark best_result->res[i].args */;
            }
        }
        /* mark best_result */
    }

    mark_error(&current_error_info);
    mark_error(&best_error_info);
    mark_error(&parallel_error_info);
    mark_error(&second_parallel_error_info);

    for (i = 0; i < VERB_HASH_SIZE; i++) {
        verb_t *verb_entry = verbs[i];
        
        while (verb_entry) {
            EXTRA_REF(BLOCK(verb_entry->real_name))++;
            EXTRA_REF(BLOCK(verb_entry->match_name))++;
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
    if (my_string)
        EXTRA_REF(BLOCK(my_string))++;
#ifdef DEBUGMALLOC_EXTENSIONS
    if (master_user_list)
        master_user_list->extra_ref++;
#endif
}

void parser_mark (parse_info_t * pinfo) {
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

#if defined(DEBUG) || defined(PARSE_DEBUG)
/* Usage:  DEBUG_P(("foo: %s:%i", str, i)); */
static void debug_parse(char *fmt, ...) {
    va_list args;
    char buf[2048];
    char *p = buf;
    int n = debug_parse_depth - 1;
    
    while (n--) {
        *p++ = ' ';
        *p++ = ' ';
    }
    
    va_start(args, fmt);
    vsprintf(p, fmt, args);
    va_end(args);

    tell_object(command_giver, buf, strlen(buf));
    tell_object(command_giver, "\n", 1);
}
#endif

INLINE_STATIC void bitvec_copy (bitvec_t * b1, bitvec_t * b2) {
    int i, n = b2->last;

    b1->last = n;
    for (i = 0; i < n; i++)
        b1->b[i] = b2->b[i];
}

INLINE_STATIC void bitvec_zero (bitvec_t * bv) {
    bv->last = 0;
}

INLINE_STATIC void bitvec_set (bitvec_t * bv, int elem) {
    int which = BV_WHICH(elem);
    
    if (which >= bv->last) {
        int i;
        
        for (i = bv->last; i < which; i++)
            bv->b[i] = 0;
        bv->b[which] = BV_BIT(elem);
        bv->last = which + 1;
    } else {
        bv->b[which] |= BV_BIT(elem);
    }
}

INLINE_STATIC int intersect (bitvec_t * bv1, bitvec_t * bv2) {
    int i, found = 0;
    int n = (bv1->last < bv2->last ? bv1->last : bv2->last);
    
    bv1->last = n;
    for (i = 0; i < n; i++)
        if (bv1->b[i] &= bv2->b[i]) found = 1;
    
    return found;
}

static int bitvec_count (bitvec_t * bv) {
    static int counts[16] = { 
/* 0000 */ 0,  /* 0001 */ 1,  /* 0010 */ 1,  /* 0011 */ 2,
/* 0100 */ 1,  /* 0101 */ 2,  /* 0110 */ 2,  /* 0111 */ 3,
/* 1000 */ 1,  /* 1001 */ 2,  /* 1010 */ 2,  /* 1011 */ 3,
/* 1100 */ 2,  /* 1101 */ 3,  /* 1110 */ 3,  /* 1111 */ 4
    };
    
    int ret = 0;
    int i, j;
    
    for (i = 0; i < bv->last; i++) {
        unsigned int k = bv->b[i];
        if (k) {
            for (j = 0; j < 8; j++) {
                ret += counts[k & 15];
                k >>= 4;
            }
        }
    }
    return ret;
}

static void all_objects (bitvec_t * bv, int remote_flag) {
    int i;
    int num = (remote_flag ? num_objects : num_objects - num_people);
    int last = BV_WHICH(num);
    
    i = last;
    while (i--)
        bv->b[i] = ~0;
    if (num < MAX_NUM_OBJECTS) {
        bv->b[last] = BV_BIT(num) - 1;
        bv->last = last + 1;
    } else 
        bv->last = last;
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

INLINE_STATIC match_t *add_match (parse_state_t * state, int token, 
                                    int start, int end) {
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

static int parse_copy_array (array_t * arr, char *** sarrp) {
    char **table;
    int j;
    int n = 0;
    
    if (!arr->size) {
        *sarrp = 0;
        return 0;
    }
    
    table = *sarrp = CALLOCATE(arr->size, char *, 
                               TAG_PARSER, "parse_copy_array");
    for (j = 0; j < arr->size; j++) {
        if (arr->item[j].type == T_STRING) {
            DEBUG_PP(("Got: %s", arr->item[j].u.string));
            if (arr->item[j].subtype == STRING_SHARED) {
                table[n++] = ref_string(arr->item[j].u.string);
            } else {
                table[n++] = make_shared_string(arr->item[j].u.string);
            }
        }
    }
    if (n != arr->size)
        *sarrp = RESIZE(table, n, char *, TAG_PARSER, "parse_copy_array");
    return n;
}

static void add_special_word (char * wrd, int kind, int arg) {
    char *p = make_shared_string(wrd);
    int h = DO_HASH(p, SPECIAL_HASH_SIZE);
    special_word_t *swp = ALLOCATE(special_word_t, TAG_PARSER, "add_special_word");
    
    swp->wrd = p;
    swp->kind = kind;
    swp->arg = arg;
    swp->next = special_table[h];
    special_table[h] = swp;
}

static int check_special_word (char * wrd, long *arg) {
    int h = DO_HASH(wrd, SPECIAL_HASH_SIZE);
    special_word_t *swp = special_table[h];

    while (swp) {
        if (swp->wrd == wrd) {
            *arg = swp->arg;
            return swp->kind;
        }
        swp = swp->next;
    }

    if (isdigit(*wrd)) {
        char *p;

        *arg = strtol(wrd, &p, 10);
        if (p && *p) {
            char *ending = "th";

            if (p - wrd < 2 || *(p - 2) != '1') {
                switch (*(p - 1)) {
                    case '1': ending = "st";  break;
                    case '2': ending = "nd";  break;
                    case '3': ending = "rd";  break;
                }
            }

            if (!strcmp(p, ending))
                return SW_ORDINAL;
        }
    }

    return SW_NONE;
}

static void interrogate_master (void) {
    svalue_t *ret;

    if ((master_state & MS_HAS_USERS) == 0) {
        DEBUG_PP(("[master::parse_command_users]"));
        if (master_user_list) {
            free_array(master_user_list);
            master_user_list = 0;
        }
    
        DEBUG_PP(("[%s]", applies_table[APPLY_USERS]));
        ret = apply_master_ob(APPLY_USERS, 0);
        if (ret && ret->type == T_ARRAY) {
            master_user_list = ret->u.arr;
            ret->u.arr->ref++;
        } else {
            master_user_list = &the_null_array;
        }
        master_state |= MS_HAS_USERS;
    }
    if ((master_state & MS_HAS_LITERALS) == 0) {
        if (literals) {
            int i;
            
            for (i = 0; i < num_literals; i++)
                free_string(literals[i]);
            FREE(literals);
            num_literals = 0;
            literals = 0;
        }
        
        DEBUG_PP(("[%s]", applies_table[APPLY_LITERALS]));
        ret = apply_master_ob(APPLY_LITERALS, 0);

        if (ret && ret->type == T_ARRAY)
            num_literals = parse_copy_array(ret->u.arr, &literals);
        else
            num_literals = 0;
        master_state |= MS_HAS_LITERALS;
    }
    if ((master_state & MS_HAS_SPECIALS) == 0) {
        add_special_word("the", SW_ARTICLE, 0);

        add_special_word("me", SW_SELF, 0);
        add_special_word("myself", SW_SELF, 0);
        
        add_special_word("all", SW_ALL, 0);
        add_special_word("of", SW_OF, 0);
        
        add_special_word("and", SW_AND, 0);
        
        add_special_word("a", SW_ORDINAL, 1);
        add_special_word("an", SW_ORDINAL, 1);
        add_special_word("any", SW_ORDINAL, 1);

        add_special_word("first", SW_ORDINAL, 1);
        add_special_word("second", SW_ORDINAL, 2);
        add_special_word("other", SW_ORDINAL, 2);
        add_special_word("third", SW_ORDINAL, 3);
        add_special_word("fourth", SW_ORDINAL, 4);
        add_special_word("fifth", SW_ORDINAL, 5);
        add_special_word("sixth", SW_ORDINAL, 6);
        add_special_word("seventh", SW_ORDINAL, 7);
        add_special_word("eighth", SW_ORDINAL, 8);
        add_special_word("ninth", SW_ORDINAL, 9);

        master_state |= MS_HAS_SPECIALS;
    }
}

void f_parse_init (void) {
    parse_info_t *pi;

    if (current_object->pinfo)
        return;

    pi = current_object->pinfo = ALLOCATE(parse_info_t, TAG_PARSER, "parse_init");
    pi->ob = current_object;
    pi->flags = 0;
}

static void remove_ids (parse_info_t * pinfo) {
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
        pinfo->num_ids = pinfo->num_plurals = pinfo->num_adjs = 0;
        pinfo->ids = pinfo->plurals = pinfo->adjs = 0;
    }
}

/* The parse_refresh() efun.  Used to inform the parsing package that
 * information returned by applies to current_object may have changed,
 * and should be recached when necessary.
 */
void f_parse_refresh (void) {
    parse_info_t *pi;
    
    /* If this is the master object, prepare to go through 
     * interrogate_master() again.  Don't free the literals now, or
     * things can dangle if we are in the middle of a parse.
     */
    if (current_object == master_ob) {
        master_state &= ~MS_HAS_USERS;
        if (!master_ob->pinfo)
            return;
    }
    
    if (!(pi = current_object->pinfo))
        error("/%s is not known by the parser.  Call parse_init() first.\n",
              current_object->obname);

    if (pi->flags & PI_SETUP) {
        pi->flags &= PI_VERB_HANDLER;
        pi->flags |= PI_REFRESH;
    } else {
        pi->flags &= PI_VERB_HANDLER;
    }
    
    /* Recheck this immediately since we don't resync handler objects, only
     * object involved in the parse.
     */
    if (pi->flags & PI_VERB_HANDLER) {
        svalue_t *ret = apply(LIVINGS_ARE_REMOTE, current_object,
                              0, ORIGIN_DRIVER);
        if (current_object->flags & O_DESTRUCTED)
            return;
        
        if (!IS_ZERO(ret))
            pi->flags |= PI_REMOTE_LIVINGS;
    }
}

/* called from free_object() */
void parse_free (parse_info_t * pinfo) {
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

static void hash_clean (void) {
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

static void free_parse_result (parse_result_t * pr) {
    int i, j;

    if (pr->ob)
        free_object(&pr->ob, "free_parse_result");
    if (pr->parallel)
        clear_parallel_errors(&pr->parallel);
    
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

static void clear_result (parse_result_t * pr) {
    int i;

    pr->ob = 0;
    pr->parallel = 0;
    
    for (i = 0; i < 4; i++) {
        pr->res[i].func = 0;
        pr->res[i].args = 0;
    }
}

static void free_parse_globals (void) {
    int i;

    if (parse_nicks)
        free_mapping(parse_nicks);
    if (parse_env)
        free_array(parse_env);
    
    parse_nicks = 0;
    parse_env = 0;
    
    pi = 0;
    hash_clean();
    if (objects_loaded) {
        for (i = 0; i < num_objects; i++)
            free_object(&loaded_objects[i], "free_parse_globals");
        objects_loaded = 0;
    }
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

static int tokenize (char ** rule, int * weightp) {
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
                            i |= LIV_MODIFIER;
                            break;
                        case 'v':
                            i |= VIS_ONLY_MODIFIER;
                            break;
                        case 'p':
                            i |= PLURAL_MODIFIER;
                            break;
                        case 'c':
                            i |= CHOOSE_MODIFIER;
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
                    if (i & PLURAL_MODIFIER)
                        (*weightp)--;
                    if (i & LIV_MODIFIER)
                        (*weightp)++;
                    if (!(i & VIS_ONLY_MODIFIER))
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
        if (strlen(literals[i]) == n && (strncmp(literals[i], start, n) == 0))
            return -(i + 1);
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

static void make_rule (char * rule, int * tokens, int * weightp) {
    int idx = 0;
    int has_plural = 0;
    int has_obj = 0;
    
    *weightp = 1;
    while (idx < MAX_MATCHES) {
        if (!(tokens[idx] = tokenize(&rule, weightp)))
            return; /* we got to the end */
        if (tokens[idx] >= OBJ_A_TOKEN) {
            if (++has_obj == 3)
                error("Only two object tokens allowed per rule.\n");
            if (tokens[idx] & PLURAL_MODIFIER) {
                if (has_plural) error("Only one plural token allowed per rule.\n");
                has_plural = 1;
            }
        }
        idx++;
    }
    error("Only %i tokens permitted per rule!\n", MAX_MATCHES);
}

static void free_words (void) {
    int i;

    for (i = 0; i < num_words; i++)
        if (words[i].type == WORD_ALLOCATED)
            FREE_MSTR(words[i].string);
    num_words = 0;
}

static void interrogate_object (object_t * ob) {
    svalue_t *ret;

    if (ob->pinfo->flags & PI_REFRESH)
        remove_ids(ob->pinfo);
    
    /* don't bail if we're trying to refresh! */
    if (ob->pinfo->flags & PI_SETUP && !(ob->pinfo->flags & PI_REFRESH))
        return;
    
    DEBUG_P(("Interogating /%s.", ob->obname));
    
    DEBUG_PP(("[%s]", APPLY_NOUN));
    ret = apply(APPLY_NOUN, ob, 0, ORIGIN_DRIVER);
    if (ret && ret->type == T_ARRAY)
        ob->pinfo->num_ids = parse_copy_array(ret->u.arr, &ob->pinfo->ids);
    else
        ob->pinfo->num_ids = 0;
    if (ob->flags & O_DESTRUCTED) return;
    /* in case of an error */
    ob->pinfo->flags |= PI_SETUP;
    ob->pinfo->flags &= ~(PI_LIVING | PI_INV_ACCESSIBLE | PI_INV_VISIBLE);
    ob->pinfo->num_adjs = 0;
    ob->pinfo->num_plurals = 0;

    DEBUG_PP(("[%s]", APPLY_PLURAL));
    ret = apply(APPLY_PLURAL, ob, 0, ORIGIN_DRIVER);
    if (ret && ret->type == T_ARRAY)
        ob->pinfo->num_plurals = parse_copy_array(ret->u.arr, &ob->pinfo->plurals);
    else
        ob->pinfo->num_plurals = 0;
    if (ob->flags & O_DESTRUCTED) return;

    DEBUG_PP(("[%s]", APPLY_ADJECTIVE));
    ret = apply(APPLY_ADJECTIVE, ob, 0, ORIGIN_DRIVER);
    if (ret && ret->type == T_ARRAY)
        ob->pinfo->num_adjs = parse_copy_array(ret->u.arr, &ob->pinfo->adjs);
    else
        ob->pinfo->num_adjs = 0;
    if (ob->flags & O_DESTRUCTED) return;

    DEBUG_PP(("[%s]", IS_LIVING));
    ret = apply(IS_LIVING, ob, 0, ORIGIN_DRIVER);
    if (!IS_ZERO(ret)) {
        ob->pinfo->flags |= PI_LIVING;
        DEBUG_PP(("(yes)"));
    }
    if (ob->flags & O_DESTRUCTED) return;

    DEBUG_PP(("[%s]", INVENTORY_ACCESSIBLE));
    ret = apply(INVENTORY_ACCESSIBLE, ob, 0, ORIGIN_DRIVER);
    if (!IS_ZERO(ret)) {
        ob->pinfo->flags |= PI_INV_ACCESSIBLE;
        DEBUG_PP(("(yes)"));
    }
    if (ob->flags & O_DESTRUCTED) return;

    DEBUG_PP(("[%s]", INVENTORY_VISIBLE));
    ret = apply(INVENTORY_VISIBLE, ob, 0, ORIGIN_DRIVER);
    if (!IS_ZERO(ret)) {
        ob->pinfo->flags |= PI_INV_VISIBLE;
        DEBUG_PP(("(yes)"));
    }
}

static object_t *first_inv (object_t * ob) {
#ifndef NO_ENVIRONMENT
    return ob->contains;
#else
    svalue_t *ret;

    push_object(ob);
    ret = apply_master_ob(APPLY_PARSE_FIRST_INVENTORY, 1);
    if (ret && ret != (svalue_t *)-1 && ret->type == T_OBJECT)
        return ret->u.ob;
    return (object_t *)NULL;
#endif
}

static object_t *next_inv (object_t * parent, object_t * sibling) {
#ifndef NO_ENVIRONMENT
    return sibling->next_inv;
#else
    svalue_t *ret;

    push_object(parent);
    push_object(sibling);
    ret = apply_master_ob(APPLY_PARSE_NEXT_INVENTORY, 2);
    if (ret && ret != (svalue_t *)-1 && ret->type == T_OBJECT)
        return ret->u.ob;
    return (object_t *)NULL;
#endif
}

static object_t *super (object_t * ob) {
#ifndef NO_ENVIRONMENT
    return ob->super;
#else
    svalue_t *ret;

    push_object(ob);
    ret = apply_master_ob(APPLY_PARSE_ENVIRONMENT, 1);
    if (ret && ret != (svalue_t *)-1 && ret->type == T_OBJECT)
        return ret->u.ob;
    return (object_t *)NULL;
#endif
}

#define RAO_INREACH 1
#define RAO_MY 2

static void rec_add_object (object_t * ob, int flags) {
    object_t *o;

    if (!ob) return;
    if (ob->flags & O_DESTRUCTED) return;
    if (ob->pinfo) {
        if (num_objects == MAX_NUM_OBJECTS)
            return;
        if (flags & RAO_MY)
            bitvec_set(&my_objects, num_objects);
        if (ob == parse_user) {
            me_object = num_objects;
            flags |= RAO_MY;
        }
        object_flags[num_objects] = flags & RAO_INREACH;
        loaded_objects[num_objects++] = ob;
        add_ref(ob, "rec_add_object");
        if (!(ob->pinfo->flags & PI_INV_VISIBLE))
            return;
        if (!(ob->pinfo->flags & PI_INV_ACCESSIBLE))
            flags &= ~RAO_INREACH;
    }
    for (o = first_inv(ob);  o;  o = next_inv(ob, o))
        rec_add_object(o, flags);
}

static void add_objects_from_array (array_t * arr, int flags) {
    int i, f;
    int last_flags = 0;
    int last_was_me = 0;
    
    for (i = 0; i < arr->size; i++) {
        object_t *ob;
        
        if (arr->item[i].type == T_ARRAY) {
            f = flags;
            if (last_flags & PI_INV_VISIBLE) {
                if (!(last_flags & PI_INV_ACCESSIBLE))
                    f &= ~RAO_INREACH;
                if (last_was_me)
                    f |= RAO_MY;
                add_objects_from_array(arr->item[i].u.arr, f);
            }
        }
        last_flags = 0;
        last_was_me = 0;
        if (arr->item[i].type == T_OBJECT &&
            !((ob = arr->item[i].u.ob)->flags & O_DESTRUCTED)) {
            if (ob->pinfo) {
                if (num_objects == MAX_NUM_OBJECTS)
                    return;
                if (flags & RAO_MY)
                    bitvec_set(&my_objects, num_objects);
                if (ob == parse_user) {
                    me_object = num_objects;
                    last_was_me = 1;
                }
                object_flags[num_objects] = flags & RAO_INREACH;
                loaded_objects[num_objects++] = ob;
                add_ref(ob, "add_objects_from_array");
                last_flags = ob->pinfo->flags;
            }
        }
    }
}

static void get_objects_from_array (array_t * arr) {
    int i;
    
    for (i = 0; i < arr->size; i++) {
        object_t *ob;

        if (arr->item[i].type == T_ARRAY)
            get_objects_from_array(arr->item[i].u.arr);
        if (arr->item[i].type != T_OBJECT) continue;
        ob = arr->item[i].u.ob;

        if (ob->flags & O_DESTRUCTED) continue;
        if (NEED_REFRESH(ob)) {
            if (num_objects == MAX_NUM_OBJECTS)
                return;
            loaded_objects[num_objects++] = ob;
            add_ref(ob, "find_uninited_objects");
        }
    }
}

static void find_uninited_objects (object_t * ob) {
    object_t *o;
    
    if (!ob) return;
    if (ob->flags & O_DESTRUCTED) return;
    if (NEED_REFRESH(ob)) {
        if (num_objects == MAX_NUM_OBJECTS)
            return;
        loaded_objects[num_objects++] = ob;
        add_ref(ob, "find_uninited_objects");
    }
    for (o = first_inv(ob); o; o = next_inv(ob, o))
        find_uninited_objects(o);
}    

static hash_entry_t *add_hash_entry (char * str) {
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
    bitvec_zero(&he->pv.noun);
    bitvec_zero(&he->pv.plural);
    bitvec_zero(&he->pv.adj);
    he->next = hash_table[h];
    he->flags = 0;
    hash_table[h] = he;
    return he;
}

void mark_hash_entry (char * str) {
    int h = DO_HASH(str, HASH_SIZE);
    hash_entry_t *he;

    DEBUG_PP(("mark_hash_entry: %s", str));
    he = hash_table[h];
    while (he) {
        if (he->name == str) {
            he->flags |= HV_NICKNAME;
            return;
        }
        he = he->next;
    }

    he = ALLOCATE(hash_entry_t, TAG_PARSER, "mark_hash_entry");
    he->name = ref_string(str);
    bitvec_zero(&he->pv.noun);
    bitvec_zero(&he->pv.plural);
    bitvec_zero(&he->pv.adj);
    he->next = hash_table[h];
    he->flags = HV_NICKNAME;
    hash_table[h] = he;
}

static void add_to_hash_table (object_t * ob, int index) {
    int i;
    parse_info_t *pi = ob->pinfo;
    hash_entry_t *he;

    if (!pi) /* woops.  Dested during parse_command_users() or something
                similarly nasty. */
        return;
    DEBUG_PP(("add_to_hash_table: /%s", ob->obname));
    for (i = 0; i < pi->num_ids; i++) {
        he = add_hash_entry(pi->ids[i]);
        he->flags |= HV_NOUN;
        bitvec_set(&he->pv.noun, index);
    }
    for (i = 0; i < pi->num_plurals; i++) {
        he = add_hash_entry(pi->plurals[i]);
        he->flags |= HV_PLURAL;
        bitvec_set(&he->pv.plural, index);
    }
    for (i = 0; i < pi->num_adjs; i++) {
        he = add_hash_entry(pi->adjs[i]);
        he->flags |= HV_ADJ;
        bitvec_set(&he->pv.adj, index);
    }

    if (pi->flags & PI_LIVING)
        bitvec_set(&cur_livings, index);
    
    if (object_flags[index])
        bitvec_set(&cur_accessible, index);
}

static void init_users() {
    int i;
    object_t *ob;

    /* Note that destructed objects have pinfo == 0 [see free_object()], 
     * so we don't have to worry about them here.  We could just keep a
     * list of objects, but we keep an entire array anyway.
     */
    for (i = 0; i < master_user_list->size; i++) {
        if (master_user_list->item[i].type == T_OBJECT
            && (ob = master_user_list->item[i].u.ob)->pinfo
            && NEED_REFRESH(ob)) {
            DEBUG_PP(("adding: /%s", ob->obname));
            if (num_objects == MAX_NUM_OBJECTS)
                return;
            loaded_objects[num_objects++] = ob;
            add_ref(ob, "init_users");
        }
    }
}

/* Note extremely clever delayed evaluation to avoid having to lookup object
 * pointer -> index 
 */
static void add_nicknames (mapping_t * map) {
    int i;
    
    for (i = 0; i < map->table_size; i++) {
        mapping_node_t *mn;
        
        for (mn = map->table[i]; mn; mn = mn->next) {
            if (mn->values[0].type == T_STRING) {
                hash_entry_t *he = add_hash_entry(mn->values[0].u.string);
                he->flags |= HV_NICKNAME;
            }
        }
    }
}

static void load_objects (void) {
    int i;
    object_t *ob, *env;
    hash_entry_t *he;
    
    if (!my_string) my_string = make_shared_string("my");

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
    bitvec_zero(&cur_livings);
    bitvec_zero(&cur_accessible);
    /* Step 1: */
    num_objects = 0;

    if (parse_env) {
        get_objects_from_array(parse_env);
    } else {
        if (!parse_user || parse_user->flags & O_DESTRUCTED)
            error("No this_player()!\n");
        
        find_uninited_objects(super(parse_user));
    }
    /* get users from master object */
    interrogate_master();
    init_users();
    objects_loaded = 1;
    /* Step 2: */
    for (i = 0; i < num_objects; i++)
        interrogate_object(loaded_objects[i]);
    for (i = 0; i < num_objects; i++)
        free_object(&loaded_objects[i], "load_objects");
    /* Step 3: */
    num_objects = 0;
    me_object = -1;
    
    bitvec_zero(&my_objects);
    if (parse_env) {
        add_objects_from_array(parse_env, RAO_INREACH);
    } else {
        rec_add_object(super(parse_user), RAO_INREACH);
    }
    he = add_hash_entry(my_string);
    he->flags |= HV_ADJ;
    bitvec_copy(&he->pv.adj, &my_objects);
    if (parse_nicks)
        add_nicknames(parse_nicks);
    
    num_people = 0;
    for (i = 0; i < master_user_list->size; i++) {
        if (master_user_list->item[i].type != T_OBJECT) continue;
        /* check if we have them already */
        ob = master_user_list->item[i].u.ob;
        if (!(ob->pinfo))
            continue;
        env = ob;
        while (env) {
            if (env == super(parse_user))
                break;
            env = super(env);
            if (env && env->pinfo && !(env->pinfo->flags & PI_INV_VISIBLE))
                env = 0;
        }
        if (env) continue;
        if (num_objects + num_people == MAX_NUM_OBJECTS)
            break;
        object_flags[num_objects + num_people] = 1;
        loaded_objects[num_objects + num_people++] = ob;
        add_ref(ob, "load_objects");
    }
    num_objects += num_people;

    for (i = 0; i < num_objects; i++)
        add_to_hash_table(loaded_objects[i], i);
}

static int get_single (bitvec_t * bv) {
    static int answer[16] = {
/* 0000 */ -1,  /* 0001 */  0,  /* 0010 */  1, /* 0011 */ -1,
/* 0100 */  2,  /* 0101 */ -1,  /* 0110 */ -1, /* 0111 */ -1,
/* 1000 */  3,  /* 1001 */ -1,  /* 1010 */ -1, /* 1011 */ -1,
/* 1100 */ -1,  /* 1101 */ -1,  /* 1110 */ -1, /* 1111 */ -1
    };
    
    int i, res = -1;
    unsigned int tmp;
    
    for (i = 0; i < bv->last; i++) {
        if (bv->b[i]) {
            if (res != -1) return -1;
            res = i;
        }
    }
    if (res < 0) return -1;

    tmp = bv->b[res];
    res *= BPI;
    /* Binary search for the set bit, unrolled for speed. */
    if (tmp & 0x0000ffff) {
        if (tmp & 0xffff0000)
            return -1;
    } else {
        tmp >>= 16;
        res += 16;
    }

    if (tmp & 0x00ff) {
        if (tmp & 0xff00)
            return -1;
    } else {
        tmp >>= 8;
        res += 8;
    }
    
    if (tmp & 0x0f) {
        if (tmp & 0xf0)
            return -1;
    } else {
        tmp >>= 4;
        res += 4;
    }

    tmp = answer[tmp];
    if (tmp == -1) return tmp;

    DEBUG_PP((" -> %i", res));
    return res + tmp;
}

/* FIXME: obsolete */
static char *query_the_short (char * start, char * end, object_t * ob) {
    svalue_t *ret;
    
    if (ob->flags & O_DESTRUCTED || 
        !(ret = apply("the_short", ob, 0, ORIGIN_DRIVER))
        || ret->type != T_STRING) {
        return strput(start, end, "the thing");
    }
    return strput(start, end, ret->u.string);
}

static char *strput_words (char * str, char * limit, int first, int last) {
    char *p = words[first].start;
    char *end = words[last].end;
    int num;
    
    /* strip leading and trailing whitespace */
    while (uisspace(p[0]))
        p++;
    while (uisspace(end[0]))
        end--;

    num = end - p + 1;
    if (str + num >= limit)
        num = limit - str - 1;

    memcpy(str, p, num);
    str[num] = 0;

    return str + num;
}

static void push_words (int first, int last) {
    char *p = words[first].start;
    char *end = words[last].end;
    char *str;
    
    while (uisspace(p[0]))
        p++;
    while (uisspace(end[0]))
        end--;
    push_malloced_string(str = new_string(end - p + 1, "push_words"));
    
    while (p <= end)
        *str++ = *p++;
    *str = 0;
}

static void free_parser_error (parser_error_t * p) {
    if (p->error_type == ERR_ALLOCATED) {
        FREE_MSTR(p->err.str);
    }
    p->error_type = 0;
}

/* They actually used the word, so do the work of looking it up */
static void expand_node (hash_entry_t * he) {
    svalue_t *sv;
    object_t *ob;
    int i;
    
    he->flags &= ~HV_NICKNAME;
    sv = find_string_in_mapping(parse_nicks, he->name);
    if (sv->type != T_OBJECT) return;
    ob = sv->u.ob;
    if (ob->flags & O_DESTRUCTED) return;
    if (ob->pinfo == 0) return;

    /* linear, but we only do this once per nickname they use */
    for (i = 0; i < num_objects; i++) {
        if (loaded_objects[i] == ob) {
            he->flags |= HV_NOUN;
            bitvec_set(&he->pv.noun, i);
            return;
        }
    }
}

static void parse_obj (int tok, parse_state_t * state,
                         int ordinal) {
    parse_state_t local_state;
    bitvec_t objects, save_obs, err_obs;
    int start = state->word_index;
    char *str;
    hash_entry_t *hnode, *last_adj = 0;
    int multiple_adj = 0;
    int ord_legal = (ordinal == 0), singular_legal = 1;
    long tmp;
    match_t *mp;

    DEBUG_INC;
    DEBUG_P(("parse_obj:"));
    
    all_objects(&objects, parse_vn->handler->pinfo->flags & PI_REMOTE_LIVINGS);

    while (1) {
        if (state->word_index == num_words)
            return;
        str = words[state->word_index++].string;
        DEBUG_PP(("Word is %s", str));
        switch (check_special_word(str, &tmp)) {
        case SW_ARTICLE:
            continue;
        case SW_ALL:
            singular_legal = 0;
            if (state->word_index < num_words &&
                check_special_word(words[state->word_index].string, &tmp) == SW_OF) {
                state->word_index++;
                continue;
            }
            local_state = *state;
            if (tok & PLURAL_MODIFIER) {
                local_state.num_objs++;
                mp = add_match(&local_state, tok,
                               start, state->word_index - 1);
                bitvec_copy(&mp->val.obs, &objects);
                mp->ordinal = 0;
            } else {
                free_parser_error(&current_error_info);
                mp = add_match(&local_state, ERROR_TOKEN,
                               start, state->word_index - 1);
                current_error_info.error_type = ERR_BAD_MULTIPLE;
            }
            parse_rule(&local_state);
            break;
        case SW_SELF: 
            {
                if (me_object != -1) {
                    local_state = *state;
                    local_state.num_objs++;
                    mp = add_match(&local_state, tok,
                                   start, state->word_index - 1);
                    bitvec_zero(&mp->val.obs);
                    bitvec_set(&mp->val.obs, me_object);
                    mp->ordinal = 0;
                    parse_rule(&local_state);
                }
                break;
            }
        case SW_ORDINAL:
            if (ord_legal) {
                local_state = *state;
                parse_obj(tok, &local_state, tmp);
            }
            break;
        }
        /* This is a hack; in the future we are going to have to handle
           <possessive> <ordinal> better.  The problem is we don't want
           to accept "red 1st sword" but we do want "my first sword" and
           "my 1st red sword", and right now we don't distinguish between
           ordinals and adjectives.  Woops. */
        if (str != my_string) ord_legal = 0;
        hnode = hash_table[DO_HASH(str, HASH_SIZE)];
        while (hnode) {
            if (hnode->name == str) {
                if (hnode->flags & HV_NICKNAME)
                    expand_node(hnode);

                if (singular_legal && (hnode->flags & HV_NOUN)) {
                    int explore_errors = !best_match &&
                        state->num_errors < best_num_errors;
                    DEBUG_P(("Found noun: %s", str));
                    local_state = *state;
                    bitvec_copy(&save_obs, &objects);

                    /* Sigh, I want to throw exceptions */
                    if (!intersect(&objects, &hnode->pv.noun)) {
                        if (!explore_errors) goto skip_it;
                        
                        free_parser_error(&current_error_info);
                        current_error_info.error_type = ERR_IS_NOT;
                        current_error_info.err.noun = hnode;
                        goto we_have_an_error;
                    }
                    if (tok & LIV_MODIFIER) {
                        if (explore_errors)
                            bitvec_copy(&err_obs, &objects);
                        if (!intersect(&objects, &cur_livings)) {
                            if (!explore_errors) goto skip_it;
                        
                            free_parser_error(&current_error_info);
                            current_error_info.error_type = ERR_NOT_LIVING;
                            current_error_info.err.noun = hnode;
                            goto we_have_an_error;
                        }
                    }
                    if (!(tok & VIS_ONLY_MODIFIER)) {
                        if (explore_errors)
                            bitvec_copy(&err_obs, &objects);
                        if (!intersect(&objects, &cur_accessible)) {
                            if (!explore_errors) goto skip_it;
                        
                            free_parser_error(&current_error_info);
                            current_error_info.error_type = ERR_NOT_ACCESSIBLE;
                            current_error_info.err.noun = hnode;
                            goto we_have_an_error;
                        }
                    }
                    mp = add_match(&local_state, tok & ~PLURAL_MODIFIER,
                                   start, state->word_index - 1);
                    bitvec_copy(&mp->val.obs, &objects);
                    mp->ordinal = ordinal;
                    local_state.num_objs++;
                    goto do_the_parse;

                we_have_an_error:
                    mp = add_match(&local_state, ERROR_TOKEN,
                                   start, state->word_index - 1);

                do_the_parse:
                    parse_rule(&local_state);

                skip_it:
                    bitvec_copy(&objects, &save_obs);
                }
                if ((ordinal == 0) && (hnode->flags & HV_PLURAL)) {
                    int explore_errors = !best_match &&
                        state->num_errors < best_num_errors;
                    DEBUG_P(("Found plural: %s", str));
                    local_state = *state;
                    bitvec_copy(&save_obs, &objects);

                    if (!(tok & PLURAL_MODIFIER)) {
                        if (!explore_errors) goto p_skip_it;

                        free_parser_error(&current_error_info);
                        current_error_info.error_type = ERR_BAD_MULTIPLE;
                        goto p_we_have_an_error;
                    }
                    /* Sigh, I want to throw exceptions */
                    if (!intersect(&objects, &hnode->pv.plural)) {
                        if (!explore_errors) goto p_skip_it;
                        
                        free_parser_error(&current_error_info);
                        current_error_info.error_type = ERR_IS_NOT;
                        current_error_info.err.noun = hnode;
                        goto p_we_have_an_error;
                    }
                    if (tok & LIV_MODIFIER) {
                        if (explore_errors)
                            bitvec_copy(&err_obs, &objects);
                        if (!intersect(&objects, &cur_livings)) {
                            if (!explore_errors) goto p_skip_it;
                        
                            free_parser_error(&current_error_info);
                            current_error_info.error_type = ERR_NOT_LIVING;
                            current_error_info.err.noun = hnode;
                            goto p_we_have_an_error;
                        }
                    }
                    if (!(tok & VIS_ONLY_MODIFIER)) {
                        if (explore_errors)
                            bitvec_copy(&err_obs, &objects);
                        if (!intersect(&objects, &cur_accessible)) {
                            if (!explore_errors) goto p_skip_it;
                        
                            free_parser_error(&current_error_info);
                            current_error_info.error_type = ERR_NOT_ACCESSIBLE;
                            current_error_info.err.noun = hnode;
                            goto p_we_have_an_error;
                        }
                    }
                    mp = add_match(&local_state, tok,
                                   start, state->word_index - 1);
                    bitvec_copy(&mp->val.obs, &objects);
                    mp->ordinal = ordinal;
                    local_state.num_objs++;
                    goto p_do_the_parse;

                p_we_have_an_error:
                    mp = add_match(&local_state, ERROR_TOKEN,
                                   start, state->word_index - 1);

                p_do_the_parse:
                    parse_rule(&local_state);

                p_skip_it:
                    bitvec_copy(&objects, &save_obs);
                }
                if (hnode->flags & HV_ADJ) {
                    DEBUG_P(("Found adj: %s", str));
                    intersect(&objects, &hnode->pv.adj);
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

static void make_error_message (int which, parser_error_t * err) {
    char buf[1024];
    char *p;
    char *end = EndOf(buf);
    int cnt = 0;
    int ocnt = 0;
    int tok;
    int index = 0;

    p = strput(buf, end, "You can't ");
    p = strput(p, end, words[0].string);
    *p++ = ' ';
    while ((tok = parse_vn->token[index++])) {
        switch (tok) {
        case STR_TOKEN:
            if (cnt == which - 1) {
                p = strput(p, end, "that ");
                cnt++;
                break;
            }
            /* FALLTHRU */
        case WRD_TOKEN:
            p = strput_words(p, end, matches[cnt].first, matches[cnt].last);
            *p++ = ' ';
            cnt++;
            break;
        default:
            if (tok <= 0) {
                p = strput(p, end, literals[-(tok + 1)]);
                *p++ = ' ';
            } else {
                if (cnt == which - 1 || ++ocnt >= which
                    || (matches[cnt].token & PLURAL_MODIFIER)) {
                    p = strput(p, end, "that ");
                } else {
                    /* FIXME */
                    p = query_the_short(p, end, loaded_objects[matches[cnt].val.number]);
                    *p++ = ' ';
                }
                cnt++;
            }
            break;
        }
    }
    p--;
    p = strput(p, end, ".\n");
    DEBUG_P((buf));
    free_parser_error(err);
    err->error_type = ERR_ALLOCATED;
    err->err.str = string_copy(buf, "make_error_message");
}

/* 1 -> ok
 * 0 -> no such func
 * -1 -> returned error
 * -2 -> generated error
 * -3 -> abort
 */
static int process_answer (parse_state_t * state, svalue_t * sv,
                             int which) {
    if (!sv) return 0;
    if (sv->type == T_NUMBER) {
        DEBUG_P(("Return value was: %i", sv->u.number));
        if (sv->u.number)
            return 1;
        if (state->num_errors == best_num_errors) {
            DEBUG_P(("Have a better match; aborting ..."));
            return -3;
        }
        if (state->num_errors++ == 0)
            make_error_message(which, &current_error_info);

        return -2;
    }
    if (sv->type != T_STRING) {
        DEBUG_P(("Return value was not a string or number."));
        return 0;
    }
    DEBUG_P(("Returned string was: %s", sv->u.string));
    if (state->num_errors == best_num_errors) {
        DEBUG_P(("Have a better match; aborting ..."));
        return -3;
    }
    if (state->num_errors++ == 0) {
        free_parser_error(&current_error_info);
        current_error_info.error_type = ERR_ALLOCATED;
        current_error_info.err.str = string_copy(sv->u.string, "process_answer");
    }
    return -1;
}

/* 1 - error or accepted
 * 0 - no function
 * -1 - generated or ridiculous error
 */
static int parallel_process_answer (parse_state_t * state, svalue_t * sv,
                             int which) {
    if (!sv) return 0;
    if (sv->type == T_NUMBER) {
        DEBUG_P(("Return value was: %li", sv->u.number));
        if (sv->u.number)
            return 1;
        
        if (state->num_errors == 0)
            make_error_message(which, &parallel_error_info);
        return -1;
    }
    if (sv->type != T_STRING) {
        DEBUG_P(("Return value was not a string or number.", sv->u.number));
        return 0;
    }
    DEBUG_P(("Returned string was: %s", sv->u.string));

    free_parser_error(&parallel_error_info);
    if (sv->u.string[0] == '#') {
        parallel_error_info.error_type = ERR_ALLOCATED;
        parallel_error_info.err.str = string_copy(sv->u.string + 1, "process_answer");
        return -1;
    } else {
        parallel_error_info.error_type = ERR_ALLOCATED;
        parallel_error_info.err.str = string_copy(sv->u.string, "process_answer");
        return 1;
    }
}

static int push_real_names (int try, int which) {
    int index = 0, match = 0;
    int tok;
    char tmp[1024];

    if (try >= 2) {
        char tmpbuf[1024];
        strput_words(tmpbuf, EndOf(tmpbuf), 0, 0);
        copy_and_push_string(tmpbuf);
    }
    
    while ((tok = parse_vn->token[index++])) {
        if (tok > 0) {
            strput_words(tmp, EndOf(tmp), matches[match].first, matches[match].last);
            push_malloced_string(string_copy(tmp, "push_real_names"));
            match++;
        }
    }
    return match + (try >= 2);
}

static char *rule_string (verb_node_t * vn) {
    int index = 0;
    int tok;
    static char buf[1024];
    char *end = EndOf(buf);
    char *p;

    p = buf;
    
    while (1) {
        switch ((tok = vn->token[index++]) & ~CHOOSE_MODIFIER) {
        case OBJ_A_TOKEN:
        case OBJ_TOKEN:
            p = strput(p, end, "OBJ ");
            break;
        case LIV_A_TOKEN:
        case LIV_TOKEN:
            p = strput(p, end, "LIV ");
            break;
        case OBS_TOKEN:

        case ADD_MOD(OBS_TOKEN, VIS_ONLY_MODIFIER):
            p = strput(p, end, "OBS ");
            break;
        case LVS_TOKEN:
        case ADD_MOD(LVS_TOKEN, VIS_ONLY_MODIFIER):
            p = strput(p, end, "LVS ");
            break;
        case STR_TOKEN:
            p = strput(p, end, "STR ");
            break;
        case WRD_TOKEN:
            p = strput(p, end, "WRD ");
            break;
        case 0:
            if (p == buf) {
                *buf = 0;
            } else {
                *(p-1) = 0; /* nuke last space */
            }
            return buf;
        default:
            p = strput(p, end, literals[-(tok + 1)]);
            *p++ = ' ';
            break;
        }
    }
}

static void push_bitvec_as_array (bitvec_t * bv, int errors_too) {
    int i, k, n = 0;
    unsigned int j;
    array_t *arr;
    saved_error_t *se;

    if (errors_too) {
        se = best_result->parallel;
        while (se) {
            n++;
            se = se->next;
        }
    }
    
    for (i = 0; i < bv->last; i++) {
        if (bv->b[i]) {
            j = 1;
            while (j) {
                if (bv->b[i] & j)
                    n++;
                j <<= 1;
            }
        }
    }
    arr = allocate_array(n);
    /* error safety; right spot for return value too */
    push_refed_array(arr);

    if (errors_too) {
        i = 0;
        se = best_result->parallel;
        while (se) {
            svalue_t *ret = get_the_error(&se->err, se->obj);
            
            if (ret)
                assign_svalue_no_free(&arr->item[i], ret);

            se = se->next;
            i++;
        }
    }
    
    for (i = 0; i < bv->last; i++) {
        if (bv->b[i]) {
            j = 1;
            k = 0;
            while (j) {
                if (bv->b[i] & j) {
                    object_t *ob = loaded_objects[BPI * i + k];
                    n--;
                    if (ob->flags & O_DESTRUCTED) {
                        arr->item[n] = const0u;
                    } else {
                        arr->item[n].type = T_OBJECT;
                        arr->item[n].u.ob = ob;
                        add_ref(ob, "push_bitvec_as_array");
                    }
                }
                j <<= 1;
                k++;
            }
        }
    }
}

/* Belgarat: this function has been extended to handle 
 *  two more parser apply calls. direct_ and indirect_ applies are
 *  called sometimes more than once
 */
static char *prefixes[] = { "can_", "direct_", "indirect_", "do_",
  /* Belgarat: names for the second pass with filled object arguments */
  "direct_", "indirect_" };

static int make_function (char * buf, char * end, int which,
                            parse_state_t * state, int try,
                            object_t * target) {
    int index = 0, match = 0, omatch = 0;
    int on_stack = 0;
    int tok;
    
    /* try = 0: "read_about_str_from_obj"
     * try = 1: "read_word_str_word_obj"
     * try = 2: "verb_word_str_word_obj"
     * try = 3: "verb_rule"
     */

    buf = strput(buf, end, prefixes[which]);
    if (try < 2) {
        buf = strput(buf, end, parse_verb_entry->match_name);
    } else {
        buf = strput(buf, end, "verb");
        push_shared_string(parse_verb_entry->match_name);
        on_stack++;
    }

    if (try == 3) {
        buf = strput(buf, end, "_rule");
        /* leave the 0; this effectively truncates the string. */
        buf++;
        share_and_push_string(rule_string(parse_vn));
        on_stack++;
    }
    while ((tok = parse_vn->token[index++])) {
        *buf++ = '_';
        switch (tok & ~CHOOSE_MODIFIER) {
        case OBJ_TOKEN:
        case OBJ_A_TOKEN:
            buf = strput(buf, end, "obj");
            goto put_obj_value;

        case OBS_TOKEN:
        case ADD_MOD(OBS_TOKEN, VIS_ONLY_MODIFIER):
            if (omatch + 1 >= which ||
                !(matches[match].token & PLURAL_MODIFIER) || (which >= 4))
                buf = strput(buf, end, "obj");
            else
                buf = strput(buf, end, "obs");
            goto put_obj_value;
            
        case LVS_TOKEN:
        case ADD_MOD(LVS_TOKEN, VIS_ONLY_MODIFIER):
            if (omatch +1 >= which ||
                !(matches[match].token & PLURAL_MODIFIER) || (which >= 4))
                buf = strput(buf, end, "liv");
            else
                buf = strput(buf, end, "lvs");
            goto put_obj_value;

        case LIV_TOKEN:
        case LIV_A_TOKEN:
            buf = strput(buf, end, "liv");

          put_obj_value:
            omatch++;
            /* Belgarat: this part was rewritten; behaviour for `which <= 4' was
             * retained and two more calls were added. There seemed to be some kind
             * of bug in the original code, but I'm not quite sure.
             */
            if (which >= 4) {
                if (omatch == 1) {
                    push_object(loaded_objects[direct_object >= 0 ? direct_object : 0]);
                } else {
                    push_object(loaded_objects[indirect_object >= 0 ? indirect_object : 0]);
                }
            } else if (omatch == which) {
                push_object(target);
            } else if (omatch > which) {
                push_number(0);
            } else if (matches[match].token == ERROR_TOKEN) {
                push_number(0);
            } else if (matches[match].token & PLURAL_MODIFIER) {
                push_bitvec_as_array(&matches[match].val.obs, which == 3);
            } else if (matches[match].val.number < 0) {
                push_number(0);
            } else if (loaded_objects[matches[match].val.number]->flags & O_DESTRUCTED) {
                push_number(0);
            } else 
                push_object(loaded_objects[matches[match].val.number]);
            match++;
            on_stack++;
            break;
        case STR_TOKEN:
            {
                char tmp[1024];
                buf = strput(buf, end, "str");
                strput_words(tmp, EndOf(tmp), matches[match].first, matches[match].last);
                push_malloced_string(string_copy(tmp, "push_real_names"));
                match++;
                on_stack++;
            }
            break;
        case WRD_TOKEN:
            {
                char tmp[1024];
                buf = strput(buf, end, "wrd");
                strput_words(tmp, end, matches[match].first, matches[match].last);
                push_malloced_string(string_copy(tmp, "push_real_names"));
                match++;
                on_stack++;
            }
            break;
        default:
            if (!try) {
                buf = strput(buf, end, literals[-(tok + 1)]);
            } else if (try < 3) {
                buf = strput(buf, end, "word");
                push_shared_string(literals[-(tok + 1)]);
                on_stack++;
            }
        }
    }
    return on_stack;
}

#define SET_OB(x) if ((ob = (x))->flags & O_DESTRUCTED) return 0;

static int check_functions (object_t * obj, parse_state_t * state) {
    object_t *ob;
    char func[256];
    int try, ret, args;
    
    SET_OB(obj);
    for (try = 0, ret = 0; !ret && try < 8; try++) {
        if (try == 4)
            SET_OB(parse_vn->handler);
        args = make_function(func, EndOf(func), 0, state, try % 4, obj);
        args += push_real_names(try % 4, 0);
        DEBUG_P(("Trying %s ... (/%s)", func, ob->obname));
        ret = process_answer(state, apply(func, ob, args, ORIGIN_DRIVER), 0);
        if (ob->flags & O_DESTRUCTED)
            return 0;
        if (ret == -3)
            return 0;
    }
    if (!ret) {
        if (state->num_errors == best_num_errors) {
            DEBUG_P(("Nothing matched and we have a better match"));
            DEBUG_DEC;
            return 0;
        }
        if (state->num_errors++ == 0)
            make_error_message(0, &current_error_info);
    }
    return 1;
}

static void clear_parallel_errors (saved_error_t ** par) {
    saved_error_t *se, *next;

    for (se = *par; se; se = next) {
        next = se->next;
        free_parser_error(&se->err);
        FREE(se);
    }
    *par = 0;
}

static int use_last_parallel_error (parse_state_t * state) {
    if (!parallel_error_info.error_type) 
        return 0;
    if (state->num_errors++ == 0) {
        free_parser_error(&current_error_info);
        current_error_info = parallel_error_info;
        parallel_error_info.error_type = 0;
    }
    return 1;
}

static int save_last_parallel_error (int ob) {
    saved_error_t *n;
    
    if (!parallel_error_info.error_type)
        return 0;
    n = ALLOCATE(saved_error_t, TAG_PARSER, "save_last_parallel_error");
    n->next = parallel_errors;
    n->obj = ob;
    n->err = parallel_error_info;
    parallel_errors = n;
    parallel_error_info.error_type = 0;
    return 1;
}

static int parallel_check_functions (object_t * obj, 
                                       parse_state_t * state,
                                       int which) {
    object_t *ob;
    char func[256];
    int try, ret, args;

    free_parser_error(&parallel_error_info);
    SET_OB(obj);
    for (try = 0, ret = 0; !ret && try < 8; try++) {
        if (try == 4)
            SET_OB(parse_vn->handler);
        args = make_function(func, EndOf(func), which, state, try % 4, obj);
        args += push_real_names(try % 4, which);
        DEBUG_P(("Trying %s ... (/%s)", func, ob->obname));
        ret = parallel_process_answer(state, apply(func, ob, args, ORIGIN_DRIVER), which);
        if (ob->flags & O_DESTRUCTED)
            return 0;
    }
    if (!ret) {
        if (state->num_errors == 0)
            make_error_message(0, &parallel_error_info);
        return 0;
    }
    return ret == 1;
}

static void singular_check_functions (int which, parse_state_t * state,
                                        match_t * m) {
    bitvec_t *bv = &m->val.obs;
    int i, k, ambig = 0, match;
    unsigned int j;
    int ordinal = m->ordinal;
    int ord2 = m->ordinal;
    int has_ordinal = (m->ordinal != 0);
    int was_error = 0;
    
    for (i = 0; i < bv->last; i++) {
        if (bv->b[i]) {
            j = 1;
            k = 0;
            while (j) {
                if (bv->b[i] & j) {
                    int ret = parallel_check_functions(loaded_objects[BPI * i + k], state, which);
                    if (ret) {
                        if (has_ordinal) {
                            ord2--;
                            if (ordinal < 0 || --ordinal == 0) {
                                if (ordinal == -2)
                                    state->num_errors--;
                                if (use_last_parallel_error(state)) {
                                    m->token = ERROR_TOKEN;
                                    if (ordinal != -1)
                                        return;
                                    ordinal = -2;
                                } else {
                                    m->val.number = BPI * i + k;
                                    return;
                                }
                            }
                        } else {
                            if (!ambig++) {
                                if (use_last_parallel_error(state)) {
                                    was_error = 1;
                                    m->token = ERROR_TOKEN;
                                } else {
                                    was_error = 0;
                                    match = BPI * i + k;
                                }
                                if (m->token & CHOOSE_MODIFIER) {
                                    if (match >= 0) m->val.number = match;
                                    return;
                                }
                            } else {
                                match = -1;
                            }
                        }
                    } else {
                        if (has_ordinal && (ordinal == -1 || --ord2 == 0)) {
                            free_parser_error(&second_parallel_error_info);
                            second_parallel_error_info = parallel_error_info;
                            parallel_error_info.error_type = 0;
                        }
                        /* not a valid object */
                        bv->b[i] &= ~j;
                    }
                }
                j <<= 1;
                k++;
            }
        }
    }
    if (!has_ordinal) {
        if (ambig == 1) {
            m->val.number = match;
            return;
        }
        if (was_error)
            state->num_errors--;
        
        m->token = ERROR_TOKEN;
        if (ambig == 0 && use_last_parallel_error(state))
            return;
        if (state->num_errors++ == 0) {
            free_parser_error(&current_error_info);

            current_error_info.error_type = ERR_AMBIG;
            bitvec_copy(&current_error_info.err.obs, &m->val.obs);
            return;
        }
    } else {
        if (ordinal == -2) return;
        
        m->token = ERROR_TOKEN;
        if (state->num_errors++ == 0) {
            free_parser_error(&current_error_info);
            if (ord2 <= 0) {
                current_error_info = second_parallel_error_info;
                second_parallel_error_info.error_type = 0;
            } else {
                /* Didn't find enough; signal an ordinal error */
                current_error_info.error_type = ERR_ORDINAL;
                current_error_info.err.ord_error = bitvec_count(bv);
            }
        }
    }
    return;
}

static void plural_check_functions (int which, parse_state_t * state,
                                      match_t * m) {
    bitvec_t *bv = &m->val.obs;
    int i, k;
    unsigned int j;
    int found_one = 0;
    
    for (i = 0; i < bv->last; i++) {
        if (bv->b[i]) {
            j = 1;
            k = 0;
            while (j) {
                if (bv->b[i] & j) {
                    int ret = parallel_check_functions(loaded_objects[BPI * i + k], state, which);
                    if (!ret || save_last_parallel_error(BPI * i + k))
                        bv->b[i] &= ~j;
                    else
                        found_one = 1;
                }
                j <<= 1;
                k++;
            }
        }
    }
    if (!found_one && use_last_parallel_error(state))
        m->token = ERROR_TOKEN;
}

/* Belgarat: The next functions are used for tests and error
 * reporting in the second pass of parsing when all parameters are exactly
 * filled (including the object ones)
 */
static int cache_last_parallel_error(parser_error_t *storage) {
    if (!parallel_error_info.error_type) 
        return 0;
    free_parser_error(storage);
    *storage = parallel_error_info;
    parallel_error_info.error_type = 0;
    return 1;
}

static int use_cached_parallel_error (parse_state_t * state,
                                        parser_error_t* err) {
    if (!err->error_type) 
        return 0;
    if (state->num_errors++ == 0) {
        free_parser_error(&current_error_info);
        current_error_info = *err;
        err->error_type = 0;
    }
    return 1;
}

static void dependent_check_functions (int which, parse_state_t * state,
                                         match_t * m) {
    bitvec_t *bv = &m->val.obs;
    int i, k;
    unsigned int j;
    int found_one = 0;
    parser_error_t errinfo;
    
    errinfo.error_type = 0;
    for (i = 0; i < bv->last; i++) {
        if (bv->b[i]) {
            j = 1;
            k = 0;
            while (j) {
                if (bv->b[i] & j) {
                    int ret = parallel_check_functions(loaded_objects[BPI * i + k], state, which);
                    if (!ret || cache_last_parallel_error(&errinfo))
                        bv->b[i] &= ~j;
                    else {
                        if (!found_one) m->val.number = BPI * i + k;
                        found_one = 1;
                    }
                }
                j <<= 1;
                k++;
            }
        }
    }
    if (!found_one && (
        use_cached_parallel_error(state, &errinfo) || 
        use_last_parallel_error(state)) ) {
        m->token = ERROR_TOKEN;
    }
    /* Translate me!
     * Nepotrebuji, aby se mi v pameti flakaly errorove hlasky, ktere
     * se ani nepouziji.
     */
    free_parser_error(&errinfo);
}

#define CHECK_DIRECT_OK           1
#define CHECK_INDIRECT_OK         2
#define CHECK_ERROR_RELATION      4
#define CHECK_RELATION_COMPLETE   8

/* Translate me!
 * Z funkce vypadne DIRECT_OK, kdyz je kombinace schvalena directem,
 * INDIRECT_OK, kdyz je kombinace schvalena indirectem
 * a ERROR_RELATION kdyz dojde k nejake (libovolne) chybe
 * Jinymi slovy, je li zaroven DIRECT_OK a INDIRECT_OK,
 * je to platna kombinace a ERROR_RELATION urcuje, zda doslo k chybe
 * Jestlize jedno z DIRECT a INDIRECT schazi, je vztah neuplny
 * a nebude zaznamenan
 */

/* Belgarat:
 * This function checks if two object can be used together in two-object rule.
 * The function returns DIRECT_OK, when only direct object agrees to the action,
 * IDIRECT_OK if only indirect object agrees or combination of both, if both
 * objects agree (in this case flag CHECK_RELATION_COMPLETE is set as well).
 * If it returns ERROR_RELATION, there's some error (that should be saved for
 * further processing).
 */
static int check_one_relation(parse_state_t * state, int direct_first, 
                              parser_error_t *errinfo) 
{
    int res;
    int ob;
    parser_error_t err;
    
    res = 0;
    err.error_type = 0;
    ob = direct_first ? direct_object : indirect_object;
    res = parallel_check_functions(
        loaded_objects[direct_first ? direct_object : indirect_object], state, 
        direct_first ? 4 : 5);
    if (!res) {
        /* The object doesn't exist, returned 0 or "#error". This object
         * can not be used for the rule.
         */
        return CHECK_ERROR_RELATION;
    }
    res |= direct_first ? CHECK_DIRECT_OK : CHECK_INDIRECT_OK;
    if (cache_last_parallel_error(&err)) {
        /* The object could be used in general, but not at this time.
         * mark the rule with error flag so it won't be used for action,
         * but continue the processing.
         * If both objects returns "error message", then the objects are
         * valid for the rule, but they cannot be used right now.
         */
        res |= CHECK_ERROR_RELATION;
    }
    /* checking the other object in the pair */
    res = parallel_check_functions(
        loaded_objects[direct_first ? indirect_object: direct_object], state, 
        direct_first ? 5 : 4);
    if (!res) {
        /* the same as above, but for the other object */
        if (err.error_type) {
            *errinfo = err;
        }
        return res | CHECK_ERROR_RELATION;
    }
    res |= direct_first ? CHECK_INDIRECT_OK : CHECK_DIRECT_OK;
    /* don't cache the new error if the previous check returned eror too */
    if (!err.error_type && cache_last_parallel_error(&err)) {
        /* The same as above - the object can be used for the action,
         * but not right now. Mark the pair as error and keep the error 
         * cached
         */
        res |= CHECK_ERROR_RELATION;
    }
    /* mark the relation as complete. The relation is complete even if some
     * error has occured. In general the relation is complete when both objects
     * can operate together, but not now (they returned string error
     * description without # as the first character)
     */
    res |= CHECK_RELATION_COMPLETE;
    if (err.error_type)
        *errinfo = err;
    return res;
}

/* Belgarat:
 * This function checks if the objects in two-object rule can really 
 * operate together. It checks each object that matched the first OBJ token
 * with each object that matched the second OBJ.
 */
static void check_object_relations (parse_state_t * state) {
    int i, direct = -1, indirect = -1;
    int use_indirect;
    bitvec_t *dir_objs, *indir_objs;
    int j, k, l, m, n, ret;
    int found_direct = -1, found_indirect = -1;
    int direct_unique, indirect_unique;
    int found_something;
    int direct_ordinal, finished = 0;
    bitvec_t indirects, directs;
    parser_error_t err;
    
    bitvec_zero(&directs);
    bitvec_zero(&indirects);
    err.error_type = 0;
    for (i = 0; i < state->num_matches; i++) {
        if (matches[i].token & OBJ_A_TOKEN) {
            if (direct < 0) direct = i; else indirect = i;
        } else if (matches[i].token == ERROR_TOKEN) return;
    }
    if (matches[indirect].ordinal) {
        /* if the indirect object is used with ordinal number, choose only
         * that single indirect object. Imagine how it would be confusing if
         * you enter "get apple from the second cask" and the REAL second
         * cask would be skipped, because there is no apple there... without
         * error message.
         */
        int ord;
        bitvec_t *bv = &matches[indirect].val.obs;

        ord = matches[indirect].ordinal;
        if (ord > 0) i = 0;
        for (; i < bv->last; i++) {
            if (bv->b[i]) {
                j = 1, k = 0;
                while(j) {
                    if (bv->b[i] & j) {
                        /* found some object, decrement counter */
                        if (!--ord) {
                            bitvec_zero(bv);
                            bitvec_set(bv, use_indirect = BPI * i + k);
                            break;
                        }
                    }
                    j <<= 1, k++;
                }
            }
        }
        if (ord) {
            /* there's less indirect objs than specified --> error */
            matches[indirect].token = ERROR_TOKEN;
            if (state->num_errors++ == 0) {
                free_parser_error(&current_error_info);
                current_error_info.error_type = ERR_ORDINAL;
                current_error_info.err.ord_error = bitvec_count(bv) + 1;
            }
            return; 
        }
    } else { 
        /* check if there's no SO much possibilities. if so, error out. */
        int direct_count = bitvec_count(&directs);
        int indirect_count = bitvec_count(&indirects);
        /* if you want to increase performance, reduce the number in the if
         * below.  if it will be at least 20, the players should not complain
         * too much 
         */
        if (direct_count * indirect_count >= 80) {
            state->num_errors++;
            free_parser_error(&current_error_info);
            current_error_info.error_type = ERR_MANY_PATHS;
            return;
        }
    }
    dir_objs = &matches[direct].val.obs;
    indir_objs = &matches[indirect].val.obs;
    
    direct_unique = !(matches[direct].token & PLURAL_MODIFIER);
    indirect_unique = !(matches[indirect].token & PLURAL_MODIFIER);
    
    if (!(direct_ordinal = matches[direct].ordinal)) direct_ordinal = -1;
    
    for (i = 0; !finished && i < dir_objs->last; i++) if (dir_objs->b[i]) {
        for(j = 1, k = 0; j ; j <<=1, k++) if (dir_objs->b[i] & j) {
            /* found direct object.
             * I have to test it against either use_indirect or against
             * all indirect objects I have.
             */
            direct_object = BPI * i + k;
            found_something = 0;
            for (l = 0; l < indir_objs->last; l++) if (indir_objs->b[l]) {
                for (m = 1, n = 0; m ; m <<= 1, n++) if (indir_objs->b[l] & m) {
                    /* found direct/indirect object combination.
                     * test if that can succeed.
                     * if yes, remember that indirect obj.
                     */
                    indirect_object = BPI * l + n;
                    /* indirect unique --> check indirect_ first. */
                    ret = check_one_relation(state, indirect_unique, &err);
                    
                    /* If it is not complete relation, I can do nothing
                     * because both the direct and indirect objects have to agree
                     * on the relation.
                     */
                    if (!(ret & CHECK_RELATION_COMPLETE)) continue;
                    
                    /* from now on, direct and indirect object are OK */
                    if (indirect_unique && found_indirect >= 0) {
                        if (found_indirect != indirect_object) {
                            matches[indirect].token = ERROR_TOKEN;
                            if (state->num_errors++ == 0) {
                                free_parser_error(&current_error_info);
                                
                                current_error_info.error_type = ERR_AMBIG;
                                bitvec_copy(&current_error_info.err.obs, 
                                            &matches[indirect].val.obs);
                            }
                            free_parser_error(&err);
                            return;
                        }
                    }
                    if (direct_ordinal > 0) direct_ordinal--;
                    
                    if (direct_ordinal <= 0 && direct_unique && found_direct >= 0) {
                        if (found_direct != direct_object) {
                            matches[indirect].token = ERROR_TOKEN;
                            if (state->num_errors++ == 0) {
                                free_parser_error(&current_error_info);
                                
                                current_error_info.error_type = ERR_AMBIG;
                                bitvec_copy(&current_error_info.err.obs, 
                                            &matches[direct].val.obs);
                            }
                            free_parser_error(&err);
                            return;
                        }
                    }
                    /* If there's no error, mark direct and indirect object
                     * in the tables. Note that insufficient directs that have
                     * been found result in no error, so indirect object is
                     * marked in case there's some ambiguity
                     */
                    if (!(ret & CHECK_ERROR_RELATION)) {
                        if (direct_ordinal <= 0) {
                            bitvec_set(&directs, direct_object);
                        }
                        bitvec_set(&indirects, indirect_object);
                    }
                    /* regardles of error state mark found direct/indirect pair
                     * for possible future ambiguity checks
                     */
                    if (direct_ordinal <= 0)
                        found_direct = direct_object;
                    found_indirect = indirect_object;
                }
            }
            /* end of processing of direct object */
            if (found_direct && (!direct_ordinal || 
                                 (direct_unique && (matches[direct].token & CHOOSE_MODIFIER)))) {
                finished = 1;
                break;
            }
        }
    }
    /* all searched, now evaluate the results. */
    bitvec_copy(dir_objs, &directs);
    bitvec_copy(indir_objs, &indirects);
    if (direct_unique) matches[direct].val.number = found_direct; 
    else matches[direct].val.number = 0; 
    if (indirect_unique) matches[indirect].val.number = found_indirect;
    else matches[indirect].val.number = 0;
    
    if (found_direct < 0) {
        if (use_cached_parallel_error(state, &err) ||
            use_last_parallel_error(state)) 
            matches[direct].token = ERROR_TOKEN;
    } else if (found_indirect < 0) {
        if (use_cached_parallel_error(state, &err) ||
            use_last_parallel_error(state)) 
            matches[indirect].token = ERROR_TOKEN;
    }
    free_parser_error(&err);
}

static void we_are_finished (parse_state_t * state) {
    char func[256];
    char *p;
    int which, mtch;
    int try, args;
    
    DEBUG_INC;
    DEBUG_P(("we_are_finished"));
    
    if (found_level < 2) found_level = 2;
    
    /* ignore it if we already have somethign better */
    if (best_match >= parse_vn->weight) {
        DEBUG_P(("Have a better match; aborting ..."));
        DEBUG_DEC;
        return;
    }
    if (state->num_errors) {
        if (state->num_errors > best_num_errors) {
            DEBUG_DEC;
            return;
        }
        if (state->num_errors == best_num_errors
            && parse_vn->weight < best_error_match) {
            DEBUG_DEC;
            return;
        }
    }
    
    if (!check_functions(parse_user, state)) {
        DEBUG_DEC;
        return;
    }
    
    clear_parallel_errors(&parallel_errors);
    which = 1;
    for (which = 1, mtch = 0; which < 3 && mtch < state->num_matches; mtch++) {
        int tok = matches[mtch].token;
        
        if (tok == ERROR_TOKEN) {
            which++; /* Is this right if the ERROR_TOKEN
                        was actually a string? */
            continue;
        }
        if (!(tok & OBJ_A_TOKEN))
            continue;
        
        if (tok & PLURAL_MODIFIER)
            plural_check_functions(which, state, &matches[mtch]);
        else if (state->num_objs == 2)
            dependent_check_functions(which, state, &matches[mtch]);
        else
            singular_check_functions(which, state, &matches[mtch]);
        which++;
    }
    
    if (state->num_objs == 2 && !state->num_errors) {
        check_object_relations(state);
    }
    if (state->num_errors) {
        int weight = parse_vn->weight;
        
        if (current_error_info.error_type == ERR_THERE_IS_NO) {
            /* ERR_THERE_IS_NO is basically a STR in place of an OBJ,
             * so is weighted far too highly.  Give it approximately
             * the same weight as a STR.
             */
            weight = 1;
        }
        
        if (state->num_errors == best_num_errors &&
            weight <= best_error_match) {
            DEBUG_P(("Have better match; aborting ..."));
            DEBUG_DEC;
            return;
        }
        free_parser_error(&best_error_info);
        best_error_info = current_error_info;
        current_error_info.error_type = 0;
        best_num_errors = state->num_errors;
        best_error_match = weight;
    } else {
        best_match = parse_vn->weight;
        if (best_result) free_parse_result(best_result);
        best_result = ALLOCATE(parse_result_t, TAG_PARSER, "we_are_finished");
        clear_result(best_result);
        if (parse_vn->handler->flags & O_DESTRUCTED) {
            DEBUG_DEC;
            return;
        }
        best_result->ob = parse_vn->handler;
        best_result->parallel = parallel_errors;
        parallel_errors = 0;
        add_ref(parse_vn->handler, "best_result");
        for (try = 0; try < 4; try++) {
            args = make_function(func, EndOf(func), 3, state, try, 0);
            args += push_real_names(try, 3);
            best_result->res[try].func = string_copy(func, "best_result");
            best_result->res[try].num = args;
            if (args) {
                p = (char *)(best_result->res[try].args = CALLOCATE(args,
                                       svalue_t, TAG_PARSER, "best_result"));
                memcpy(p, (char *)(sp - args + 1), args * sizeof(svalue_t));
                sp -= args;
            }
        }
        DEBUG_P(("Saving successful match: %s (%s)", best_result->res[0].func,
                 best_result->ob->obname));
    }
    DEBUG_DEC;
}

static void do_the_call (void) {
    int i, n;
    object_t *ob = best_result->ob;

    for (i = 0; i < 4; i++) {
        if (ob->flags & O_DESTRUCTED) return;
        n = best_result->res[i].num;
        if (n) {
            CHECK_STACK_OVERFLOW(n);
            memcpy((char *)(sp + 1), best_result->res[i].args, n*sizeof(svalue_t));
            /*
             * Make sure we haven't dumped any dested obs onto the stack;
             * this also updates sp.
             */
            while (n--) {
                if ((++sp)->type == T_OBJECT && (sp->u.ob->flags & O_DESTRUCTED)) {
                    free_object(&sp->u.ob, "do_the_call");
                    *sp = const0u;
                }
            }
            FREE(best_result->res[i].args);
        }
        best_result->res[i].args = 0;
        DEBUG_P(("Calling %s ...", best_result->res[i].func));
        if (apply(best_result->res[i].func, ob,
                  best_result->res[i].num, ORIGIN_DRIVER)) return;
    }
    error("Parse accepted, but no do_* function found in object /%s!\n",
          ob->obname);
}

static void parse_rule (parse_state_t * state) {
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
        switch (tok & ~CHOOSE_MODIFIER) {
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
            if (!best_match && !best_error_match) {
                start = state->word_index++;
                while (state->word_index <= num_words) {
                    local_state = *state;
                    free_parser_error(&current_error_info);
                    mp = add_match(&local_state, ERROR_TOKEN,
                                   start, state->word_index - 1);
                    current_error_info.error_type = ERR_THERE_IS_NO;
                    current_error_info.err.str_problem.start = start;
                    current_error_info.err.str_problem.end = state->word_index - 1;
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
            DEBUG_DEC;
            return;
        default:
            if (literals[-(tok + 1)] == words[state->word_index].string) {
                state->word_index++;
                DEBUG_P(("Matched literal: %s", literals[-(tok + 1)]));
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
                        
                        while (literals[-(tok + 1)] != words[state->word_index++].string) {
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
                            free_parser_error(&current_error_info);
                            current_error_info.error_type = ERR_THERE_IS_NO;
                            current_error_info.err.str_problem.start = last->first;
                            current_error_info.err.str_problem.end = state->word_index - 1;
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

static int check_literal (int lit, int start) {
    DEBUG_PP(("check_literal: %s", literals[lit]));

    while (start < num_words) {
        if (literals[lit] == words[start++].string) {
            DEBUG_PP(("yes"));
            return start;
        }
    }
    DEBUG_PP(("no"));
    return 0;
}

static void parse_rules (void) {
    int pos;
    parse_state_t local_state;

    parse_vn = parse_verb_entry->node;
    while (parse_vn) {
        DEBUG_PP(("Rule: %s", rule_string(parse_vn)));
        if ((!parse_restricted || parse_vn->handler == parse_restricted)
            && (best_match <= parse_vn->weight))  {
            pos = 0;
            if ((parse_vn->lit[0] == -1 ||
                 (pos = check_literal(parse_vn->lit[0], 1)))
                && (parse_vn->lit[1] == -1 ||
                    check_literal(parse_vn->lit[1], pos))) {
                DEBUG_P(("Trying rule: %s", rule_string(parse_vn)));

                local_state.tok_index = 0;
                local_state.word_index = 1;
                local_state.num_matches = 0;
                local_state.num_errors = 0;
                local_state.num_objs = 0;
                parse_rule(&local_state);
            }
        }
        parse_vn = parse_vn->next;
    }
}

static void reset_error (void) {
    best_match = 0;
    best_error_match = 0;
    best_num_errors = 5732; /* Yes.  Exactly 5,732 errors.  Don't ask. */
    free_parser_error(&current_error_info);
    free_parser_error(&best_error_info);
}

static void parse_recurse (char ** iwords, char ** ostart, char ** oend) {
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
#if defined(DEBUG) || defined(PARSE_DEBUG)
        if (debug_parse_depth) {
            char dbuf[1024];
            char *end = EndOf(dbuf);
            int i;
            char *p;
            p = strput(dbuf, end, "Trying interpretation: ");
            for (i = 0; i < num_words; i++) {
                p = strput(p, end, words[i].string);
                p = strput(p, end, ":");
            }
            DEBUG_P((dbuf));
        }
#endif
        parse_rules();
    }
}

static void parse_sentence (char * input) {
    char *starts[MAX_WORDS_PER_LINE];
    char *orig_starts[MAX_WORDS_PER_LINE];
    char *orig_ends[MAX_WORDS_PER_LINE];
    char buf[MAX_WORD_LENGTH], *p, *start;
    unsigned char c, *inp;
    char *end = EndOf(buf) - 1; /* space for zero */
    int n = 0;
    int i;
    int flag;
    
    reset_error();
    free_words();
    p = start = buf;
    flag = 0;
    inp = input;
    while (*inp && (uisspace(*inp) || isignore(*inp)))
        inp++;
    orig_starts[0] = inp;
    
    while ((c = *inp++)) {
        if (isignore(c)) continue;
        if (uisupper(c))
            c = tolower(c);
        if (iskeep(c) && p < end) {
            if (!flag)
                flag = 1;
            *p++ = c;
            if (p == end) break; /* truncate */
        } else {
            /* whitespace or punctuation */
            if (!uisspace(c))
                while (*inp && !iskeep(*inp) && !uisspace(*inp))
                    inp++;
            else
                inp--;

            if (flag) {
                flag = 0;
                *p++ = 0;
                orig_ends[n] = inp - 1; /* points to where c was */
                starts[n++] = start;
                if (n == MAX_WORDS_PER_LINE)
                    return; /* too many words */
                start = p;
                while (*inp && uisspace(*inp))
                    inp++;
                orig_starts[n] = inp;
                if (p == end)
                    break; /* truncate */
            } else {
                while (*inp && uisspace(*inp))
                    inp++;
            }
        }
    }
    if (flag) {
        *p++ = 0;
        orig_ends[n] = inp - 2;
        starts[n++] = start;
        if (n == MAX_WORDS_PER_LINE)
            return; /* too many words */
    } else {
        if (n)
            orig_ends[n - 1] = inp - 2;
        else
            orig_ends[0] = inp - 2;
    }
    if (p > end) {
        starts[n] = end;
    } else {
        starts[n] = p;
        *p = 0;
    }

    /* find an interpretation, first word must be shared (verb) */
    for (i = 1; i <= n; i++) {
        char *vb = findstring(buf);
        verb_t *ve;
        
        if (vb) {
            ve = verbs[DO_HASH(vb, VERB_HASH_SIZE)];
            while (ve) {
                if (ve->real_name == vb) {
                    if (ve->flags & VB_IS_SYN)
                        parse_verb_entry = ((verb_syn_t *)ve)->real;
                    else
                        parse_verb_entry = ve;

                    words[0].string = vb;
                    words[0].type = 0;
                    
                    if (found_level < 1) found_level = 1;
                    if (!objects_loaded && 
                        (parse_verb_entry->flags & VB_HAS_OBJ)) 
                        load_objects();
                    num_words = 1;
                    words[0].start = orig_starts[0];
                    words[0].end = orig_ends[i-1];
                    parse_recurse(&starts[i], &orig_starts[i], &orig_ends[i]);
                }
                ve = ve->next;
            }
        }
        starts[i][-1] = ' ';
    }
}

static svalue_t * get_the_error (parser_error_t * err, int obj) {
    int tmp = err->error_type;
    static svalue_t hack = { T_NUMBER };

    err->error_type = 0;
    push_number(tmp);
    if (obj == -1 || (loaded_objects[obj]->flags & O_DESTRUCTED))
        push_undefined();
    else
        push_object(loaded_objects[obj]);
    
    switch (tmp) {
    case ERR_IS_NOT:
    case ERR_NOT_LIVING:
    case ERR_NOT_ACCESSIBLE:
        push_shared_string(err->err.noun->name);
        push_number(get_single(&err->err.noun->pv.noun) == -1);
        return apply_master_ob(APPLY_PARSER_ERROR_MESSAGE, 4);
    case ERR_AMBIG:
        push_bitvec_as_array(&err->err.obs, 0);
        return apply_master_ob(APPLY_PARSER_ERROR_MESSAGE, 3);
    case ERR_ORDINAL:
        push_number(err->err.ord_error);
        return apply_master_ob(APPLY_PARSER_ERROR_MESSAGE, 3);
    case ERR_THERE_IS_NO:
        push_words(err->err.str_problem.start,
                   err->err.str_problem.end);
        return apply_master_ob(APPLY_PARSER_ERROR_MESSAGE, 3);
    case ERR_ALLOCATED:
        push_malloced_string(err->err.str);
        return apply_master_ob(APPLY_PARSER_ERROR_MESSAGE, 3);
    case ERR_BAD_MULTIPLE:
        return apply_master_ob(APPLY_PARSER_ERROR_MESSAGE, 2);
    case ERR_MANY_PATHS:
  return apply_master_ob(APPLY_PARSER_ERROR_MESSAGE, 5);
    default:
        pop_stack();
        sp--;
        hack.u.number = -found_level;
        return &hack;
    }
}

void f_parse_sentence (void) {
    if (!current_object->pinfo)
        error("/%s is not known by the parser.  Call parse_init() first.\n",
              current_object->obname);

    //if (pi)
      //  error("Illegal to call parse_sentence() recursively.\n");
    
    /* may not be done in case of an error, or in case of tail recursion.
     * if we are called tail recursively, we don't need this any more.
     */
    if (best_result) {
        free_parse_result(best_result);
        best_result = 0;
    }

    if (st_num_arg == 4) 
        parse_nicks = (sp--)->u.map;
    
    if (st_num_arg >= 3)
        parse_env = (sp--)->u.arr;

    if (st_num_arg >= 2 && (sp--)->u.number) {
#if defined(DEBUG) || defined(PARSE_DEBUG)
        debug_parse_depth = 1;
        if ((sp + 1)->u.number > 1)
            debug_parse_verbose = 1;
        else
            debug_parse_verbose = 0;
    } else {
        debug_parse_depth = 0;
#else
        error("Parser debugging not enabled. (compile with -DDEBUG or -DPARSE_DEBUG).\n");
#endif
    }

    STACK_INC;
    sp->type = T_ERROR_HANDLER;
    sp->u.error_handler = free_parse_globals;

    parse_user = current_object;
    pi = current_object->pinfo;
    parse_restricted = 0;
    found_level = 0;
    parse_sentence((sp-1)->u.string);

    if (best_match) {
        sp--; /* pop the error handler */
        free_parse_globals();

        do_the_call();
        free_string_svalue(sp);
        put_number(1);
    } else {
        svalue_t *ret = get_the_error(&best_error_info, -1);
        
        sp--; /* pop the error handler */
        free_parse_globals();
        
        free_string_svalue(sp);
        if (ret) {
            *sp = *ret;
            ret->type = T_NUMBER; /* will be freed later */
        } else
            *sp = const0;
    }

    if (best_result) {
        free_parse_result(best_result);
        best_result = 0;
    }
}

void f_parse_my_rules (void) {
    int flag = (st_num_arg == 3 ? (sp--)->u.number : 0);
    
    if (!(sp-1)->u.ob->pinfo)
        error("/%s is not known by the parser.  Call parse_init() first.\n",
              (sp-1)->u.ob->obname);
    if (!current_object->pinfo)
        error("/%s is not known by the parser.  Call parse_init() first.\n",
              current_object->obname);

    if (pi)
        error("Illegal to call parse_sentence() recursively.\n");
    
    STACK_INC;
    sp->type = T_ERROR_HANDLER;
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
                        free_object(&arr->item[n].u.ob, "parse_my_rules");
                        arr->item[n] = const0u;
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
        svalue_t *ret = get_the_error(&best_error_info, -1);
        
        sp--; /* pop the error handler */
        free_string_svalue(sp--);
        free_svalue(sp, "parse_my_rules"); /* may have been destructed */
        if (ret) {
            *sp = *ret;
            ret->type = T_NUMBER;
        } else {
            *sp = const0;
        }
    }
    free_parse_globals();
}

void f_parse_remove() {
    char *verb;
    verb_t *verb_entry;
    
    verb = SHARED_STRING(sp);
    verb_entry = verbs[DO_HASH(verb, VERB_HASH_SIZE)];
    while (verb_entry) {
        if (verb_entry->match_name == verb) {
            verb_node_t **vn = &(verb_entry->node), *old;
            while (*vn) {
                if ((*vn)->handler == current_object) {
                    old = *vn;
                    *vn = (*vn)->next;
                    FREE(old);
                } else vn = &((*vn)->next);
            }
        }
        verb_entry = verb_entry->next;
    }
    free_string_svalue(sp--);
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
    
    rule = sp->u.string;
    verb = SHARED_STRING(sp-1);
    verb_entry = 0;
    handler = current_object;
    if (!(handler->pinfo))
        error("/%s is not known by the parser.  Call parse_init() first.\n",
              handler->obname);

    /* We need the literals */
    interrogate_master();

    /* Create the rule */
    make_rule(rule, tokens, &weight);

    /* Now find a verb entry to put it in */
    if (verb) {
        verb_entry = verbs[DO_HASH(verb, VERB_HASH_SIZE)];
        while (verb_entry) {
            if (verb_entry->match_name == verb &&
                verb_entry->real_name == verb &&
                !(verb_entry->flags & VB_IS_SYN))
                break;
            verb_entry = verb_entry->next;
        }
    }
    
    if (!verb_entry) {
        if (!verb)
            verb = make_shared_string((sp-1)->u.string);
        else
            ref_string(verb);
        
        h = DO_HASH(verb, VERB_HASH_SIZE);
        verb_entry = ALLOCATE(verb_t, TAG_PARSER, "parse_add_rule");
        verb_entry->real_name = verb;
        ref_string(verb);
        verb_entry->match_name = verb;
        verb_entry->node = 0;
        verb_entry->flags = 0;
        verb_entry->next = verbs[h];
        verbs[h] = verb_entry;
    }

    /* Add a new node */
    for (i = 0, j = 0; tokens[i]; i++) {
        if (tokens[i] <= 0 && j < 2)
            lit[j++] = -(tokens[i]+1);
    }

    while (j < 2)
        lit[j++] = -1;

    verb_node = (verb_node_t *)DXALLOC(sizeof(verb_node_t) + sizeof(int)*i,
                                       TAG_PARSER, "parse_add_rule");

    verb_node->lit[0] = lit[0];
    verb_node->lit[1] = lit[1];
    for (j = 0; j <= i; j++) {
        if (tokens[j] >= OBJ_A_TOKEN)
            verb_entry->flags |= VB_HAS_OBJ;
        verb_node->token[j] = tokens[j];
    }
    verb_node->weight = weight;
    verb_node->handler = handler;
    handler->pinfo->flags |= PI_VERB_HANDLER;
    verb_node->next = verb_entry->node;
    verb_entry->node = verb_node;

    ret = apply(LIVINGS_ARE_REMOTE, handler, 0, ORIGIN_DRIVER);
    if (!IS_ZERO(ret))
        handler->pinfo->flags |= PI_REMOTE_LIVINGS;

    /* return */
    free_string_svalue(sp--);
    free_string_svalue(sp--);
}

void f_parse_add_synonym() {
    char *new_verb, *old_verb, *rule, *orig_new_verb;
    verb_t *vb;
    verb_node_t *vn, *verb_node;
    verb_t *verb_entry;
    int tokens[10];
    int weight;
    int h;
    
    if (st_num_arg == 3) {
        orig_new_verb = (sp-2)->u.string;
        new_verb = SHARED_STRING(sp-2);
        old_verb = SHARED_STRING(sp-1);
        rule = sp->u.string;
    } else {
        orig_new_verb = (sp-1)->u.string;
        new_verb = SHARED_STRING(sp-1);
        old_verb = SHARED_STRING(sp);
        rule = 0;
    }

    if (old_verb == new_verb)
        error("Verb cannot be a synonym for itself.\n");
    
    verb_entry = 0;

    if (!old_verb)
        error("%s is not a verb!\n", old_verb);

    vb = verbs[DO_HASH(old_verb, VERB_HASH_SIZE)];
    while (vb) {
        if (vb->real_name == old_verb && vb->match_name == old_verb)
            break;
        vb = vb->next;
    }
    
    if (!vb)
        error("%s is not a verb!\n", old_verb);

    verb_entry = 0;

    /* Now find a verb entry to put it in */
    if (new_verb) {
        verb_entry = verbs[DO_HASH(new_verb, VERB_HASH_SIZE)];
        while (verb_entry) {
            if (verb_entry->real_name == new_verb
                && verb_entry->match_name == old_verb) {
                if (rule) {
                    if ((verb_entry->flags & VB_IS_SYN) == 0) break;
                } else {
                    if ((verb_entry->flags & VB_IS_SYN))
                        break;
                }
            }
            verb_entry = verb_entry->next;
        }
    }

    if (!verb_entry) {
        if (!new_verb)
            new_verb = make_shared_string(orig_new_verb);
        else
            ref_string(new_verb);
        ref_string(old_verb);
        
        h = DO_HASH(new_verb, VERB_HASH_SIZE);
        verb_entry = ALLOCATE(verb_t, TAG_PARSER, "parse_add_rule");
        verb_entry->real_name = new_verb;
        verb_entry->match_name = old_verb;
        verb_entry->node = 0;
        verb_entry->flags = 0;
        verb_entry->next = verbs[h];
        verbs[h] = verb_entry;
    }

    if (rule) {
        int i;
        
        /* Create the rule */
        make_rule(rule, tokens, &weight);

        /*check that the rule we are shadowing exists, and check it's handler*/
        for (vn = vb->node; vn; vn = vn->next) {
            for (i = 0; tokens[i]; i++) {
                if (vn->token[i] != tokens[i]) break;
            }
            if (!tokens[i] && !vn->token[i]) break; /* match */
        }
        if (!vn) error("No such rule defined.\n");
        if (vn->handler != current_object) error("Rule owned by different object.\n");
        
        verb_node = (verb_node_t *)DXALLOC(sizeof(verb_node_t) + sizeof(int)*i,
                                           TAG_PARSER, "parse_add_rule");
        memcpy(verb_node, vn, sizeof(verb_node_t) + sizeof(int)*i);
        for (i = 0; vn->token[i]; i++)
            if (vn->token[i] >= OBJ_A_TOKEN) {
                verb_entry->flags |= VB_HAS_OBJ;
                break;
            }
        
        verb_node->next = verb_entry->node;
        verb_entry->node = verb_node;
    } else {
        verb_syn_t *syn = (verb_syn_t*)verb_entry;
        syn->flags = VB_IS_SYN | (vb->flags & VB_HAS_OBJ);
        syn->real = vb;
    }

    if (st_num_arg == 3) free_string_svalue(sp--);
    free_string_svalue(sp--);
    free_string_svalue(sp--);
}

void f_parse_dump (void)
{
    int i;
    outbuffer_t ob;

    outbuf_zero(&ob);
    for (i = 0; i < VERB_HASH_SIZE; i++) {
        verb_t *v;
        
        for (v = verbs[i]; v; v = v->next) {
            verb_node_t *vn = v->node;
            if (v->real_name == v->match_name)
                outbuf_addv(&ob, "Verb %s:\n", v->real_name);
            else
                outbuf_addv(&ob, "Verb %s (%s):\n", v->real_name, v->match_name);
            if (v->flags & VB_IS_SYN) {
                outbuf_addv(&ob, "  Synonym for: %s\n", ((verb_syn_t *)v)->real->real_name);
                continue;
            }
            while (vn) {
                outbuf_addv(&ob, "  (/%s) %s\n", vn->handler->obname, rule_string(vn));
                vn = vn->next;
            }
        }
    }
    outbuf_push(&ob);
}
