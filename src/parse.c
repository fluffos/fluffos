/*

  Pattern Parser package for LPmud

  Ver 3.1

  If you have questions or complaints about this code please refer them
  to jna@cd.chalmers.se

*/

#include "std.h"
#include "parse.h"
#include "master.h"
#include "add_action.h"

/*****************************************************

  This is the parser used by the efun parse_command

*/
/*

  General documentation:

  parse_command() is one of the most complex efun in LPmud to use. It takes
  some effort to learn and use, but when mastered, very powerful constructs
  can be implemented.

  Basically parse_command() is a hotted sscanf operating on word basis. It
  works similar to sscanf in that it takes a pattern and a variable set of
  destination arguments. It is together with sscanf the only efun to use
  pass by reference for other variables than arrays.

  *** NOTE: Effective June 22, 1994 ... parse_command() and sscanf()
            no longer receive an lvalue list before being called.
            The destination arguments are simply placed (in reverse
            order) into a preallocated stack frame; upon return the
            driver will perform the necessary assignments and clean
            up the stack.

  To make the efun useful it must have a certain support from the mudlib,
  there is a set of functions that it needs to call to get relevant
  information before it can parse in a sensible manner.

  In earlier versions it used the normal id() lfun in the LPC objects to
  find out if a given object was identified by a certain string. This was
  highly inefficient as it could result in hundreds or maybe thousands of
  calls when very long commands were parsed.

  The new version relies on the LPC objects to give it three lists of 'names'.

       1 - The normal singular names.
       2 - The plural forms of the names.
       3 - The acknowledged adjectives of the object.

  These are fetched by calls to the functions:

       1 - string *parse_command_id_list();
       2 - string *parse_command_plural_id_list();
       3 - string *parse_command_adjectiv_id_list();

  The only really needed list is the first. If the second does not exist
  than the efun will try to create one from the singular list. For
  grammatical reasons it does not always succeed in a perfect way. This is
  especially true when the 'names' are not single words but phrases.

  The third is very nice to have because it makes constructs like
  'get all the little blue ones' possible.

  Apart from these functions that should exist in all objects, and which
  are therefore best put in /std/object.c there is also a set of functions
  needed in /secure/master.c These are not absolutely necessary but they
  give extra power to the efun.

  Basically these /secure/master.c lfuns are there to give default values
  for the lists of names fetched from each object.

  The names in these lists are applicable to any and all objects, the first
  three are identical to the lfun's in the objects:

       string *parse_command_id_list()
                - Would normally return: ({ "one", "thing" })

       string *parse_command_plural_id_list()
                - Would normally return: ({ "ones", "things", "them" })

       string *parse_command_adjectiv_id_list()
                - Would normally return ({ "iffish" })

  The last two are the default list of the prepositions and a single so called
  'all' word.

       string *parse_command_prepos_list()
                 - Would normally return: ({ "in", "on", "under" })

       string parse_command_all_word()
                 - Would normally return: "all"

  IF you want to use a different language than English but still want the
  default pluralform maker to work, you need to replace parse.c with the
  following file:

#if 0
    * Language configured parse.c
    *
    #define PARSE_FOREIGN

    char *parse_to_plural(str)
        char *str;
    {

        * Your own plural converter for your language *

    }

      * The numberwords below should be replaced for the new language *

    static char *ord1[] = {"", "first", "second", "third", "fourth", "fifth",
                           "sixth", "seventh", "eighth", "ninth", "tenth",
                           "eleventh", "twelfth", "thirteenth", "fourteenth",
                           "fifteenth", "sixteenth", "seventeenth",
                           "eighteenth","nineteenth"};

    static char *ord10[] = {"", "", "twenty","thirty","forty","fifty","sixty",
                            "seventy", "eighty","ninety"};

    static char *sord10[] = {"", "", "twentieth", "thirtieth", "fortieth",
                             "fiftieth", "sixtieth","seventieth", "eightieth",
                             "ninetieth"};

    static char *num1[] = {"", "one","two","three","four","five","six",
                           "seven","eight","nine","ten",
                           "eleven","twelve","thirteen","fourteen","fifteen",
                           "sixteen", "seventeen","eighteen","nineteen"};

    static char *num10[] = {"", "", "twenty","thirty","forty","fifty","sixty",
                           "seventy", "eighty","ninety"};

    #include "parse_english.c"      * This parse.c file *

#endif

  When all these things are defined parse_command() works best and most
  efficient. What follows is the docs for how to use it from LPC:


  Doc for LPC function

int parse_command(string, object/object*, string, destargs...)

                        Returns 1 if pattern matches

        string          Given command

        object*         if arr
        object                  array holding the accessible objects
                        if ob
                                object from which to recurse and create
                                the list of accessible objects, normally
                                ob = environment(this_player())
        string          Parsepattern as list of words and formats:
                        Example string = " 'get' / 'take' %i "
                        Syntax:
                                'word'          obligatory text
                                [word]          optional text
                                /               Alternative marker
                                %o              Single item, object
                                %l              Living objects
                                %s              Any text
                                %w              Any word
                                %p              One of a list (prepositions)
                                %i              Any items
                                %d              Number 0- or tx(0-99)

        destargs        This is the list of result variables as in sscanf
                        One variable is needed for each %_
                        The return types of different %_ is:
                        %o      Returns an object
                        %s      Returns a string of words
                        %w      Returns a string of one word
                        %p      Can on entry hold a list of word in array
                                or an empty variable
                                Returns:
                                   if empty variable: a string
                                   if array: array[0]=matched word
                        %i      Returns a special array on the form:
                                [0] = (int) +(wanted) -(order) 0(all)
                                [1..n] (object) Objectpointers
                        %l      Returns a special array on the form:
                                [0] = (int) +(wanted) -(order) 0(all)
                                [1..n] (object) Objectpointers
                                                These are only living objects.
                        %d      Returns a number

  The only types of % that uses all the loaded information from the objects
  are %i and %l. These are in fact identical except that %l filters out
  all nonliving objects from the list of objects before trying to parse.

  The return values of %i and %l is also the most complex. They return an
  array consisting of first a number and then all possible objects matching.
  As the typical string matched by %i/%l looks like: 'three red roses',
  'all nasty bugs' or 'second blue sword' the number indicates which
  of these numerical constructs was matched:

         if numeral >0 then three, four, five etc were matched
         if numeral <0 then second, twentyfirst etc were matched
         if numeral==0 then 'all' or a generic plural form such as 'apples'
                            were matched.

  NOTE!
       The efun makes no semantic implication on the given numeral. It does
       not matter if 'all apples' or 'second apple' is given. A %i will
       return ALL possible objects matching in the array. It is up to the
       caller to decide what 'second' means in a given context.

       Also when given an object and not an explicit array of objects the
       entire recursive inventory of the given object is searched. It is up
       to the caller to decide which of the objects are actually visible
       meaning that 'second' might not at all mean the second object in
       the returned array of objects.
                        
Example:

 if (parse_command("spray car",environment(this_player()),
                      " 'spray' / 'paint' [paint] %i ",items))
 {
      If the pattern matched then items holds a return array as described
        under 'destargs' %i above.

 }

 BUGS / Features
:

 Patterns of type: "%s %w %i"
   Might not work as one would expect. %w will always succeed so the arg
   corresponding to %s will always be empty.

 Patterns of the type: 'word' and [word]
   The 'word' can not contain spaces. It must be a single word. This is so
   because the pattern is exploded on " " (space) and a pattern element can
   therefore not contain spaces.

*/

/* Some useful string macros
*/
#define EQ(x,y) (strcmp(x,y)==0)
#define EQN(x,y) (strncmp(x,y,strlen(x))==0)
#define EMPTY(x) (strcmp(x,"")==0)

/* Global arrays for 'caching' of ids

   The main 'parse' routine stores these on call, making the entire
   parse_command() reentrant.
*/
typedef struct parse_global_s {
    struct parse_global_s *next;

    array_t *Id_list;
    array_t *Pluid_list;
    array_t *Adjid_list;
    array_t *Id_list_d;         /* From master */
    array_t *Pluid_list_d;      /* From master */
    array_t *Adjid_list_d;      /* From master */
    array_t *Prepos_list;       /* From master */
    char *Allword;              /* From master */

    array_t *warr;
    array_t *patarr;
    array_t *obarr;
} parse_global_t;

static parse_global_t *globals = 0;

#define gId_list       (globals->Id_list)
#define gPluid_list    (globals->Pluid_list)
#define gAdjid_list    (globals->Adjid_list)
#define gId_list_d     (globals->Id_list_d)
#define gPluid_list_d  (globals->Pluid_list_d)
#define gAdjid_list_d  (globals->Adjid_list_d)
#define gPrepos_list   (globals->Prepos_list)
#define gAllword       (globals->Allword)
#define parse_warr     (globals->warr)
#define parse_patarr   (globals->patarr)
#define parse_obarr    (globals->obarr)

static void load_lpc_info (int, object_t *);
static void parse_clean_up (void);
static void push_parse_globals (void);
static void pop_parse_globals (void);
static void store_value (svalue_t *, int, int, svalue_t *);
static void store_words_slice (svalue_t *, int, int, 
                                    array_t *, int, int);
static svalue_t *sub_parse (array_t *, array_t *, int *, 
                                 array_t *, int *, int *, svalue_t *);
static svalue_t *one_parse (array_t *, const char *, array_t *, 
                                 int *, int *, svalue_t *);
static svalue_t *number_parse (array_t *, array_t *, int *, int *);
static svalue_t *item_parse (array_t *, array_t *, int *, int *);
#ifndef NO_ADD_ACTION
static svalue_t *living_parse (array_t *, array_t *, int *, int *);
#endif
static svalue_t *single_parse (array_t *, array_t *, int *, int *);
static svalue_t *prepos_parse (array_t *, int *, int *, svalue_t *);
static int match_object (int, array_t *, int *, int *);
static int find_string (const char *, array_t *, int *);
static int check_adjectiv (int, array_t *, int, int);
static int member_string (const char *, array_t *);
static const char *parse_to_plural (const char *);
static const char *parse_one_plural (const char *);

/*
 * Function name:       load_lpc_info
 * Description:         Loads relevant information from a given object.
 *                      This is the ids, plural ids and adjectiv ids. This
 *                      is the only calls to LPC objects other than the
 *                      master object that occur within the efun
 *                      parse_command().
 * Arguments:           ix: Index in the array
 *                      ob: The object to call for information.
 */
static void
load_lpc_info (int ix, object_t * ob)
{
    array_t *tmp, *sing;
    svalue_t *ret;
    int il, make_plural = 0;
    const char *str;

    if (!ob || ob->flags & O_DESTRUCTED)
        return;

    if (gPluid_list &&
        gPluid_list->size > ix &&
        gPluid_list->item[ix].type == T_NUMBER &&
        gPluid_list->item[ix].u.number == 0) {
        ret = apply(APPLY_QGET_PLURID, ob, 0, ORIGIN_DRIVER);
        if (ret && ret->type == T_ARRAY)
            assign_svalue_no_free(&gPluid_list->item[ix], ret);
        else {
            make_plural = 1;
            gPluid_list->item[ix].u.number = 1;
        }
    }
    if (gId_list &&
        gId_list->size > ix &&
        gId_list->item[ix].type == T_NUMBER &&
        gId_list->item[ix].u.number == 0 &&
        !(ob->flags & O_DESTRUCTED) ) {
        ret = apply(APPLY_QGET_ID, ob, 0, ORIGIN_DRIVER);
        if (ret && ret->type == T_ARRAY) {
            assign_svalue_no_free(&gId_list->item[ix], ret);
            if (make_plural) {
                tmp = allocate_array(ret->u.arr->size);
                sing = ret->u.arr;

                for (il = 0; il < tmp->size; il++) {
                    if (sing->item[il].type == T_STRING) {
                        str = parse_to_plural(sing->item[il].u.string);
                        tmp->item[il].type = T_STRING;
                        tmp->item[il].subtype = STRING_MALLOC;
                        tmp->item[il].u.string = str;
                    }
                }
                gPluid_list->item[ix].type = T_ARRAY;
                gPluid_list->item[ix].u.arr = tmp;
            }
        } else {
            gId_list->item[ix].u.number = 1;
        }
    }
    if (gAdjid_list &&
        gAdjid_list->size > ix &&
        gAdjid_list->item[ix].type == T_NUMBER &&
        gAdjid_list->item[ix].u.number == 0 &&
        !(ob->flags & O_DESTRUCTED)) {
        ret = apply(APPLY_QGET_ADJID, ob, 0, ORIGIN_DRIVER);
        if (ret && ret->type == T_ARRAY)
            assign_svalue_no_free(&gAdjid_list->item[ix], ret);
        else
            gAdjid_list->item[ix].u.number = 1;
    }
}

/* Main function, called from interpret.c (or eoperators.c)
*/

/* Some leak prevention: */
static void parse_clean_up() {
    parse_global_t *pg;

    pg = globals;
    globals = pg->next;

    if (pg->Id_list)
        free_array(pg->Id_list);
    if (pg->Pluid_list)
        free_array(pg->Pluid_list);
    if (pg->Adjid_list)
        free_array(pg->Adjid_list);
    if (pg->Id_list_d)
        free_array(pg->Id_list_d);
    if (pg->Pluid_list_d)
        free_array(pg->Pluid_list_d);
    if (pg->Adjid_list_d)
        free_array(pg->Adjid_list_d);
    if (pg->Prepos_list)
        free_array(pg->Prepos_list);
    if (pg->Allword)
        FREE(pg->Allword);
    if (pg->warr)
        free_array(pg->warr);
    if (pg->patarr)
        free_array(pg->patarr);
    if (pg->obarr)
        free_array(pg->obarr);
    FREE(pg);
}

static void push_parse_globals() {
    parse_global_t *pg;

    STACK_INC;
    sp->type = T_ERROR_HANDLER;
    sp->u.error_handler = parse_clean_up;

    pg = ALLOCATE(parse_global_t, TAG_TEMPORARY, "push_parse_globals");
    pg->next = globals;
    globals = pg;

    pg->Id_list = 0;
    pg->Pluid_list = 0;
    pg->Adjid_list = 0;
    pg->Id_list_d = 0;
    pg->Pluid_list_d = 0;
    pg->Adjid_list_d = 0;
    pg->Prepos_list = 0;
    pg->Allword = 0;
    pg->warr = 0;
    pg->patarr = 0;
    pg->obarr = 0;
}

static void pop_parse_globals() {
    sp--;  /* remove error handler */
    parse_clean_up();
}

/* all the routines below return a pointer to this which should be copied
   immediately ... */
static svalue_t parse_ret;

/*
 * Function name:       parse
 * Description:         The main function for the efun: parse_command()
 *                      It parses a given command using a given pattern and
 *                      a set of objects (see args below). For details
 *                      see LPC documentation of the efun.
 * Arguments:           cmd: The command to parse
 *                      ob_or_array: A list of objects or one object from
 *                                   which to make a list of objects by
 *                                   using the objects deep_inventory
 *                      pattern: The given parse pattern somewhat like sscanf
 *                               but with different %-codes, see efun docs.
 *                      stack_args: Pointer to destination arguments.
 *                      num_arg: Number of destination arguments.
 * Returns:             True if command matched pattern.
 */
int
parse (const char *        cmd,            /* Command to parse */
         svalue_t *    ob_or_array,    /* Object or array of objects */
         const char *        pattern,        /* Special parsing pattern */
         svalue_t *    stack_args,     /* Pointer to lvalue args on
                                         * stack */
         int           num_arg)
{
    int pix, cix, six, fail, fword, ocix, fpix;
    svalue_t *pval;
    array_t *obarr;

    /*
     * Pattern and commands can not be empty
     */
    if (!*cmd || !*pattern)
        return 0;

    push_parse_globals();

    /* Array of words in command */
    parse_warr = explode_string(cmd, strlen(cmd), " ", 1);
    
    /* Array of pattern elements */
    parse_patarr = explode_string(pattern, strlen(pattern), " ", 1);

    /*
     * Explode can return '0'.
     */
    if (!parse_warr)
        parse_warr = allocate_array(0);
    if (!parse_patarr)
        parse_patarr = allocate_array(0);

    /* note: obarr is only put in parse_obarr if it needs freeing */
    if (ob_or_array->type == T_ARRAY)
        obarr = ob_or_array->u.arr;
#ifndef NO_ENVIRONMENT
    else if (ob_or_array->type == T_OBJECT) {
        /* 1 == ob + deepinv */
        parse_obarr = obarr = deep_inventory(ob_or_array->u.ob, 1, NULL);
    }
#endif
    else 
        error("Bad second argument to parse_command()\n");

    check_for_destr(obarr);

    gId_list = allocate_array(obarr->size);
    gPluid_list = allocate_array(obarr->size);
    gAdjid_list = allocate_array(obarr->size);

    /*
     * Get the default ids of 'general references' from master object
     */
    pval = apply(APPLY_QGET_ID, master_ob, 0, ORIGIN_DRIVER);
    if (pval && pval->type == T_ARRAY) {
        gId_list_d = pval->u.arr;
        pval->u.arr->ref++;     /* Otherwise next sapply will free it */
    }

    pval = apply(APPLY_QGET_PLURID, master_ob, 0, ORIGIN_DRIVER);
    if (pval && pval->type == T_ARRAY) {
        gPluid_list_d = pval->u.arr;
        pval->u.arr->ref++;     /* Otherwise next sapply will free it */
    }

    pval = apply(APPLY_QGET_ADJID, master_ob, 0, ORIGIN_DRIVER);
    if (pval && pval->type == T_ARRAY) {
        gAdjid_list_d = pval->u.arr;
        pval->u.arr->ref++;     /* Otherwise next sapply will free it */
    }

    pval = apply_master_ob(APPLY_QGET_PREPOS, 0);
    if (pval && pval->type == T_ARRAY) {
        gPrepos_list = pval->u.arr;
        pval->u.arr->ref++;     /* Otherwise next sapply will free it */
    }

    pval = apply_master_ob(APPLY_QGET_ALLWORD, 0);
    if (pval && pval->type == T_STRING)
        gAllword = alloc_cstring(pval->u.string, "parse");

    /*
     * Loop through the pattern. Handle %s but not '/'
     */
    for (six = 0, cix = 0, pix = 0; pix < parse_patarr->size; pix++) {
        pval = 0;               /* The 'fill-in' value */
        fail = 0;               /* 1 if match failed */

        if (EQ(parse_patarr->item[pix].u.string, "%s")) {
            /*
             * We are at end of pattern, scrap up the remaining words and put
             * them in the fill-in value.
             */
            if (pix == (parse_patarr->size - 1)) {
                store_words_slice(stack_args, six++, num_arg,
                                  parse_warr, cix, parse_warr->size - 1);
                cix = parse_warr->size;
            } else
                /*
                 * There is something after %s, try to parse with the next
                 * pattern. Begin with the current word and step one word for
                 * each fail, until match or end of words.
                 */
            {
                ocix = fword = cix;     /* Current word */
                fpix = ++pix;   /* pix == next pattern */
                do {
                    fail = 0;
                    /*
                     * Parse the following pattern, fill-in values:
                     * stack_args[six] = result of %s stack_args[six + 1] =
                     * result of following pattern, if it is a fill-in
                     * pattern
                     */
                    pval = sub_parse(obarr, parse_patarr, &pix,
                                     parse_warr, &cix,
                                     &fail, ((six + 1) < num_arg) ?
                                     &stack_args[six + 1] : 0);
                    if (fail) {
                        cix = ++ocix;
                        pix = fpix;
                    }
                } while (fail && (cix < parse_warr->size));

                /*
                 * We found something mathing the pattern after %s. First
                 * stack_args[six + 1] = result of match Then stack_args[six]
                 * = the skipped words before match
                 */
                if (!fail) {
                    if (pval) { /* A match with a value fill in param */
                        store_value(stack_args, six + 1, num_arg, pval);
                        store_words_slice(stack_args, six, num_arg,
                                          parse_warr, fword, ocix - 1);
                        six += 2;
                    } else {    /* A match with a non value ie 'word' */
                        store_words_slice(stack_args, six++, num_arg,
                                          parse_warr, fword, ocix - 1);
                    }
                    pval = 0;
                }
            }
        }
        /*
         * The pattern was not %s, parse the pattern if it is not '/', a '/'
         * here is skipped. If match, put in fill-in value.
         */
        else if (!EQ(parse_patarr->item[pix].u.string, "/")) {
            pval = sub_parse(obarr, parse_patarr, &pix, 
                             parse_warr, &cix, &fail,
                             (six < num_arg) ? &stack_args[six] : 0);
            if (!fail && pval)
                store_value(stack_args, six++, num_arg, pval);
        }
        /*
         * Terminate parsing if no match
         */
        if (fail)
            break;
    }

    /*
     * Also fail when there is words left to parse and pattern exhausted
     */
    if (cix < parse_warr->size)
        fail = 1;

    pop_parse_globals();

    return !fail;
}

static void
store_value (svalue_t * sp, int pos, int num, svalue_t * what)
{
    svalue_t *ret;

    if (pos >= num) {
        free_svalue(what, "store_value");
    } else {
        ret = sp + num - pos - 1;
        free_svalue(ret, "store_value"); /* is this necessary? */
        *ret = *what;
    }
}

/*
 * Function name:       slice_words
 * Description:         Gives an imploded string of words from an array
 * Arguments:           warr: array of words
 *                      from: First word to use
 *                      to:   Last word to use
 * Returns:             A pointer to a static svalue now containing string.
 */
static void
store_words_slice (svalue_t * sp, int pos, int num, array_t * warr, int from, int to)
{
    svalue_t *ret;
    array_t *slice;

    if (pos >= num)
        return;

    ret = sp + num - pos -1;
    ret->type = T_STRING;

    if (from <= to) {
        warr->ref++;
        slice = slice_array(warr, from, to);

        if (slice->size) {
            ret->subtype = STRING_MALLOC;
            ret->u.string = implode_string(slice, " ", 1);
            free_array(slice);
            return;
        }
        free_array(slice);
    }

    ret->subtype = STRING_CONSTANT;
    ret->u.string = "";
}

/*
 * Function name:       sub_parse
 * Description:         Parses a array of words against a pattern. Gives
 *                      result as an svalue. Sets fail if parsing fails and
 *                      updates pointers in pattern and word arrays. It
 *                      handles alternate patterns but not "%s"
 */
static svalue_t *
       sub_parse (array_t * obarr, array_t * patarr, int * pix_in, array_t * warr, int * cix_in, int * fail, svalue_t * sp)
{
    int cix, pix, subfail;
    svalue_t *pval;

    /*
     * Fail if we have a pattern left but no words to parse
     */
    if (*cix_in == warr->size) {
        *fail = 1;
        return 0;
    }
    cix = *cix_in;
    pix = *pix_in;
    subfail = 0;

    pval = one_parse(obarr, patarr->item[pix].u.string,
                     warr, &cix, &subfail, sp);

    while (subfail) {
        pix++;
        cix = *cix_in;

        /*
         * Find the next alternative pattern, consecutive '/' are skipped
         */
        while ((pix < patarr->size) && (EQ(patarr->item[pix].u.string, "/"))) {
            subfail = 0;
            pix++;
        }

        if (!subfail && (pix < patarr->size)) {
            pval = one_parse(obarr, patarr->item[pix].u.string, warr, &cix,
                             &subfail, sp);
        } else {
            *fail = 1;
            *pix_in = pix - 1;
            return 0;
        }
    }

    /*
     * If there is alternatives left after the mathing pattern, skip them
     */
    if ((pix + 1 < patarr->size) && (EQ(patarr->item[pix + 1].u.string, "/"))) {
        while ((pix + 1 < patarr->size) &&
               (EQ(patarr->item[pix + 1].u.string, "/"))) {
            pix += 2;
        }
        if (pix >= patarr->size)
            pix = patarr->size - 1;
    }
    *cix_in = cix;
    *pix_in = pix;
    *fail = 0;
    return pval;
}

/*
 * Function name:       one_parse
 * Description:         Checks one parse pattern to see if match. Consumes
 *                      needed number of words from warr.
 * Arguments:           obarr: Vector of objects relevant to parse
 *                      pat: The pattern to match against.
 *                      warr: Vector of words in the command to parse
 *                      cix_in: Current word in commandword array
 *                      fail: Fail flag if parse did not match
 *                      prep_param: Only used on %p (see prepos_parse)
 * Returns:             svalue holding result of parse.
 */
static svalue_t *
one_parse (array_t * obarr, const char * pat, array_t * warr, int * cix_in, int * fail, svalue_t * prep_param)
{
    char ch;
    svalue_t *pval;
    const char *str1, *str2;

    /*
     * Fail if we have a pattern left but no words to parse
     */
    if (*cix_in >= warr->size) {
        *fail = 1;
        return 0;
    }
    ch = pat[0];
    if (ch == '%') {
        ch = ((uisupper(pat[1])) ? tolower(pat[1]) : pat[1]);
    }
    pval = 0;

    switch (ch) {
    case 'i':
        pval = item_parse(obarr, warr, cix_in, fail);
        break;

#ifndef NO_ADD_ACTION
    case 'l':
        pval = living_parse(obarr, warr, cix_in, fail);
        break;
#endif

    case 's':
        *fail = 0;              /* This is double %s in pattern, skip it */
        break;

    case 'w':
        parse_ret.type = T_STRING;
        parse_ret.subtype = STRING_SHARED;
        parse_ret.u.string = make_shared_string(warr->item[*cix_in].u.string);
        pval = &parse_ret;
        (*cix_in)++;
        *fail = 0;
        break;

    case 'o':
        pval = single_parse(obarr, warr, cix_in, fail);
        break;

    case 'p':
        pval = prepos_parse(warr, cix_in, fail, prep_param);
        break;

    case 'd':
        pval = number_parse(obarr, warr, cix_in, fail);
        break;

    case '\'':
        str1 = &pat[1];
        str2 = warr->item[*cix_in].u.string;
        if ((strncmp(str1, str2, strlen(str1) - 1) == 0) &&
            (strlen(str1) == strlen(str2) + 1)) {
            *fail = 0;
            (*cix_in)++;
        } else
            *fail = 1;
        break;

    case '[':
        str1 = &pat[1];
        str2 = warr->item[*cix_in].u.string;
        if ((strncmp(str1, str2, strlen(str1) - 1) == 0) &&
            (strlen(str1) == strlen(str2) + 1)) {
            (*cix_in)++;
        }
        *fail = 0;
        break;

    default:
        *fail = 0;              /* Skip invalid patterns */
    }
    return pval;
}

/*
   We normally define these, see initial documentation (top of file)
*/
#ifndef PARSE_FOREIGN

static const char *ord1[] =
{"", "first", "second", "third", "fourth", "fifth",
 "sixth", "seventh", "eighth", "ninth", "tenth",
 "eleventh", "twelfth", "thirteenth", "fourteenth",
 "fifteenth", "sixteenth", "seventeenth",
 "eighteenth", "nineteenth"};

static const char *ord10[] =
{"", "", "twenty", "thirty", "forty", "fifty", "sixty",
 "seventy", "eighty", "ninety"};

static const char *sord10[] =
{"", "", "twentieth", "thirtieth", "fortieth",
 "fiftieth", "sixtieth", "seventieth", "eightieth",
 "ninetieth"};

static const char *num1[] =
{"", "one", "two", "three", "four", "five", "six",
 "seven", "eight", "nine", "ten",
 "eleven", "twelve", "thirteen", "fourteen", "fifteen",
 "sixteen", "seventeen", "eighteen", "nineteen"};

static const char *num10[] =
{"", "", "twenty", "thirty", "forty", "fifty", "sixty",
 "seventy", "eighty", "ninety"};

#endif

/*
 * Function name:       number_parse
 * Description:         Tries to interpret the word in wvec as a numeral
 *                      descriptor and returns the result on the form:
 *                      ret.type == T_NUMBER
 *                          num == 0, 'zero', '0', gAllword
 *                          num > 0, one, two, three etc or numbers given
 *                          num < 0, first, second,third etc given
 * Arguments:           obarr: Vector of objects relevant to parse
 *                      warr: Vector of words in the command to parse
 *                      cix_in: Current word in commandword array
 *                      fail: Fail flag if parse did not match
 * Returns:             svalue holding result of parse.
 */
static svalue_t *
number_parse (array_t * obarr, array_t * warr, int * cix_in, int * fail)
{
    int cix, ten, ones, num;
    char buf[100];

    cix = *cix_in;
    *fail = 0;

    if (sscanf(warr->item[cix].u.string, "%d", &num)) {
        if (num >= 0) {
            (*cix_in)++;
            parse_ret.type = T_NUMBER;
            parse_ret.u.number = num;
            return &parse_ret;
        }
        *fail = 1;
        return 0;               /* Only nonnegative numbers */
    }
    if (gAllword && (strcmp(warr->item[cix].u.string, gAllword) == 0)) {
        (*cix_in)++;
        parse_ret.type = T_NUMBER;
        parse_ret.u.number = 0;
        return &parse_ret;
    }
    /* This next double loop is incredibly stupid. -Beek */
    for (ten = 0; ten < 10; ten++)
        for (ones = 0; ones < 10; ones++) {
            sprintf(buf, "%s%s", num10[ten],
                    (ten > 1) ? num1[ones] : num1[ten * 10 + ones]);
            if (EQ(buf, warr->item[cix].u.string)) {
                (*cix_in)++;
                parse_ret.type = T_NUMBER;
                parse_ret.u.number = ten * 10 + ones;
                return &parse_ret;
            }
        }

    /* this one too */
    for (ten = 0; ten < 10; ten++)
        for (ones = 0; ones < 10; ones++) {
            sprintf(buf, "%s%s", (ones) ? ord10[ten] : sord10[ten],
                    (ten > 1) ? ord1[ones] : ord1[ten * 10 + ones]);
            if (EQ(buf, warr->item[cix].u.string)) {
                (*cix_in)++;
                parse_ret.type = T_NUMBER;
                parse_ret.u.number = -(ten * 10 + ones);
                return &parse_ret;
            }
        }

    *fail = 1;
    return 0;
}

/*
 * Function name:       item_parse
 * Description:         Tries to match as many objects in obvec as possible
 *                      onto the description given in commandarray warr.
 *                      Also finds numeral description if one exist and returns
 *                      that as first element in array:
 *                      ret[0].type == T_NUMBER
 *                          num == 0, 'all' or 'general plural given'
 *                          num > 0, one, two, three etc given
 *                          num < 0, first, second,third etc given
 *                      ret[1-n] == Selected objectpointers from obarr
 * Arguments:           obarr: Vector of objects relevant to parse
 *                      warr: Vector of words in the command to parse
 *                      cix_in: Current word in commandword array
 *                      fail: Fail flag if parse did not match
 * Returns:             svalue holding result of parse.
 */
static svalue_t *
       item_parse (array_t * obarr, array_t * warr, int * cix_in, int * fail)
{
    array_t *tmp, *ret;
    svalue_t *pval;
    int cix, tix, obix, plur_flag, max_cix, match_all;

    tmp = allocate_array(obarr->size + 1);
    /* in case of errors */
    push_refed_array(tmp);
    
    if ((pval = number_parse(obarr, warr, cix_in, fail)))
        tmp->item[0] = *pval;

    if (pval) {
        match_all = (pval->u.number == 0);
        plur_flag = (match_all || pval->u.number > 1);
    } else {
        plur_flag = 0;
        match_all = 0;
    }

    for (max_cix = *cix_in, tix = 1, obix = 0; obix < obarr->size; obix++) {
        *fail = 0;
        cix = *cix_in;
        if (obarr->item[obix].type != T_OBJECT)
            continue;
        if (cix == warr->size && match_all) {
            assign_svalue_no_free(&tmp->item[tix++], &obarr->item[obix]);
            continue;
        }
        load_lpc_info(obix, obarr->item[obix].u.ob);

        if (match_object(obix, warr, &cix, &plur_flag)) {
            assign_svalue_no_free(&tmp->item[tix++], &obarr->item[obix]);
            max_cix = (max_cix < cix) ? cix : max_cix;
        }
    }
    sp--; /* remove the copy.  No errors occured */

    if (tix < 2) {
        *fail = 1;
        free_array(tmp);
        if (pval)
            (*cix_in)--;
        return 0;
    } else {
        if (*cix_in < warr->size)
            *cix_in = max_cix + 1;
        if (!pval) {
            tmp->item[0].type = T_NUMBER;
            tmp->item[0].u.number = plur_flag ? 0 : 1;
        }
        ret = slice_array(tmp, 0, tix - 1);
    }

    parse_ret.type = T_ARRAY;
    parse_ret.u.arr = ret;
    return &parse_ret;
}

/*
 * Function name:       living_parse
 * Description:         Tries to match as many living objects in obvec as
 *                      possible onto the description given in the command-
 *                      array warr.
 *                      Also finds numeral description if one exist and returns
 *                      that as first element in array:
 *                      ret[0].type == T_NUMBER
 *                          num == 0, 'all' or 'general plural given'
 *                          num > 0, one, two, three etc given
 *                          num < 0, first, second,third etc given
 *                      ret[1-n] == Selected objectpointers from obarr
 *                      If not found in obarr a find_player and
 *                      lastly a find_living is done. These will return an
 *                      objecttype svalue.
 * Arguments:           obarr: Vector of objects relevant to parse
 *                      warr: Vector of words in the command to parse
 *                      cix_in: Current word in commandword array
 *                      fail: Fail flag if parse did not match
 * Returns:             svalue holding result of parse.
 */
#ifndef NO_ADD_ACTION
static svalue_t *
       living_parse (array_t * obarr, array_t * warr, int * cix_in, int * fail)
{
    array_t *live;
    svalue_t *pval;
    object_t *ob;
    int obix, tix;

    live = allocate_array(obarr->size);
    /* in case of errors */
    push_refed_array(live);
    tix = 0;
    *fail = 0;

    for (obix = 0; obix < obarr->size; obix++)
        if (obarr->item[obix].u.ob->flags & O_ENABLE_COMMANDS)
            assign_svalue_no_free(&live->item[tix++], &obarr->item[obix]);

    if (tix) {
        pval = item_parse(live, warr, cix_in, fail);
        if (pval) {
            sp--;
            free_array(live);
            return pval;
        }
    }
    sp--;
    free_array(live);

    /*
     * find_player
     */
    ob = find_living_object(warr->item[*cix_in].u.string, 1);
    if (!ob)
        /*
         * find_living
         */
        ob = find_living_object(warr->item[*cix_in].u.string, 0);

    if (ob) {
        parse_ret.type = T_OBJECT;
        parse_ret.u.ob = ob;
        add_ref(ob, "living_parse");
        (*cix_in)++;
        return &parse_ret;
    }
    *fail = 1;
    return 0;
}
#endif

/*
 * Function name:       single_parse
 * Description:         Finds the first object in obvec fitting the description
 *                      in commandarray warr. Gives this as an objectpointer.
 * Arguments:           obarr: Vector of objects relevant to parse
 *                      warr: Vector of words in the command to parse
 *                      cix_in: Current word in commandword array
 *                      fail: Fail flag if parse did not match
 * Returns:             svalue holding result of parse.
 */
static svalue_t *
       single_parse (array_t * obarr, array_t * warr, int * cix_in, int * fail)
{
    int cix, obix, plur_flag;

    for (obix = 0; obix < obarr->size; obix++) {
        *fail = 0;
        cix = *cix_in;
        if (obarr->item[obix].type == T_OBJECT)
            load_lpc_info(obix, obarr->item[obix].u.ob);
        plur_flag = 0;
        if (match_object(obix, warr, &cix, &plur_flag)) {
            *cix_in = cix + 1;
            assign_svalue_no_free(&parse_ret, &obarr->item[obix]);
            return &parse_ret;
        }
    }
    *fail = 1;
    return 0;
}

/*
 * Function name:       prepos_parse
 * Description:         This is a general sentencelist matcher with some hard-
 *                      coded prepositions as the default list. The list is
 *                      sent as a parameter which will be replaced in the
 *                      destination values. If no list is given the return
 *                      value on match with the hardcoded prepositions will be
 *                      string. If a list is given, the list will be returned
 *                      with the matched sentence swapped to the first element.
 * Arguments:           warr: Vector of words in the command to parse
 *                      cix_in: Current word in commandword array
 *                      fail: Fail flag if parse did not match
 *                      prepos: Pointer to svalue holding prepos parameter.
 * Returns:             svalue holding result of parse.
 */
static svalue_t *
prepos_parse (array_t * warr, int * cix_in, int * fail, svalue_t * prepos)
{
    array_t *parr, *tarr;
    const char *tmp;
    int pix, tix;

    if ((!prepos) || (prepos->type != T_ARRAY)) {
        parr = gPrepos_list;
    } else {
        parr = prepos->u.arr;
    }

    for (pix = 0; pix < parr->size; pix++) {
        if (parr->item[pix].type != T_STRING)
            continue;

        tmp = parr->item[pix].u.string;
        if (!strchr(tmp, ' ')) {
            if (EQ(tmp, warr->item[*cix_in].u.string)) {
                (*cix_in)++;
                break;
            }
        } else {
            tarr = explode_string(tmp, strlen(tmp), " ", 1);
            for (tix = 0; tix < tarr->size; tix++) {
                if ((*cix_in + tix >= warr->size) ||
                    (!EQ(warr->item[*cix_in + tix].u.string, tarr->item[tix].u.string)))
                    break;
            }
            if ((tix = (tix == tarr->size) ? 1 : 0))
                (*cix_in) += tarr->size;
            free_array(tarr);
            if (tix)
                break;
        }
    }

    if (pix == parr->size) {
        parse_ret.type = T_NUMBER;
        parse_ret.u.number = 0;
        *fail = 1;
    } else if (parr != gPrepos_list) {
        parse_ret = parr->item[0];
        parr->item[0] = parr->item[pix];
        parr->item[pix] = parse_ret;
        *fail = 0;
        assign_svalue_no_free(&parse_ret, prepos);
    } else {
        parse_ret.type = T_STRING;
        parse_ret.subtype = STRING_MALLOC;
        parse_ret.u.string = string_copy(parr->item[pix].u.string, "parse");
        *fail = 0;
    }

    return &parse_ret;
}

/*
 * Function name:       match_object
 * Description:         Tests if a given object matches the description as
 *                      given in the commandarray warr.
 * Arguments:           obix: Index in id arrays for this object.
 *                      warr: Vector of words in the command to parse
 *                      cix_in: Current word in commandword array
 *                      plur: This arg gets set if the noun was on pluralform
 * Returns:             True if object matches.
 */
static int
match_object (int obix, array_t * warr, int * cix_in, int * plur)
{
    array_t *ids;
    int il, pos, cplur, old_cix;
    const char *str;

    for (cplur = (*plur * 2); cplur < 4; cplur++) {
        switch (cplur) {
        case 0:
            if (!gId_list_d)
                continue;
            ids = gId_list_d;
            break;

        case 1:
            if (!gId_list ||
                gId_list->size <= obix ||
                gId_list->item[obix].type != T_ARRAY)
                continue;
            ids = gId_list->item[obix].u.arr;
            break;

        case 2:
            if (!gPluid_list_d)
                continue;
            ids = gPluid_list_d;
            break;

        case 3:
            if (!gPluid_list ||
                gPluid_list->size <= obix ||
                gPluid_list->item[obix].type != T_ARRAY)
                continue;
            ids = gPluid_list->item[obix].u.arr;
            break;

        default:
            ids = 0;

        }

        for (il = 0; il < ids->size; il++) {
            if (ids->item[il].type == T_STRING) {
                str = ids->item[il].u.string;   /* A given id of the object */
                old_cix = *cix_in;
                if ((pos = find_string(str, warr, cix_in)) >= 0) {
                    if (pos == old_cix) {
                        if (cplur > 1)
                            *plur = 1;
                        return 1;
                    } else if (check_adjectiv(obix, warr, old_cix, pos - 1)) {
                        if (cplur > 1)
                            *plur = 1;
                        return 1;
                    }
                }
                *cix_in = old_cix;
            }
        }
    }
    return 0;
}

/*
 * Function name:       find_string
 * Description:         Finds out if a given string exist within an
 *                      array of words.
 * Arguments:           str: String of some words
 *                      warr: Array of words
 *                      cix_in: Startpos in word array
 * Returns:             Pos in array if string found or -1
 */
static int
find_string (const char * str, array_t * warr, int * cix_in)
{
    int fpos;
    const char *p1, *p2;
    array_t *split;

    for (; *cix_in < warr->size; (*cix_in)++) {
        p1 = warr->item[*cix_in].u.string;
        if (p1[0] != str[0])
            continue;

        if (strcmp(p1, str) == 0)       /* str was one word and we found it */
            return *cix_in;

        if (!(p2 = strchr(str, ' ')))
            continue;

        /*
         * If str was multi word we need to make som special checks
         */
        if (*cix_in == (warr->size - 1))
            continue;

        split = explode_string(str, strlen(str), " ", 1);

        /*
         * warr->size - *cix_in ==      
         * 2: One extra word 
         * 3: Two extra words
         */
        if (!split || (split->size > (warr->size - *cix_in))) {
            if (split)
                free_array(split);
            continue;
        }
        fpos = *cix_in;
        for (; (*cix_in - fpos) < split->size; (*cix_in)++) {
            if (strcmp(split->item[*cix_in - fpos].u.string,
                       warr->item[*cix_in].u.string))
                break;
        }
        if ((*cix_in - fpos) == split->size) {
            if (split)
                free_array(split);
            return fpos;
        }

        if (split)
            free_array(split);
        *cix_in = fpos;
    }
    return -1;
}

/*
 * Function name:       check_adjectiv
 * Description:         Checks a word to see if it fits as adjectiv of an
 *                      object.
 * Arguments:           obix: The index in the global id arrays
 *                      warr: The command words
 *                      from: #1 cmdword to test
 *                      to:   last cmdword to test
 * Returns:             True if a match is made.
 */
static int
check_adjectiv (int obix, array_t * warr, int from, int to)
{
    int il, back, sum, fail;
    char *adstr;
    array_t *ids;

    if (gAdjid_list->item[obix].type == T_ARRAY)
        ids = gAdjid_list->item[obix].u.arr;
    else
        ids = 0;

    for (sum = 0, fail = 0, il = from; il <= to; il++) {
        sum += strlen(warr->item[il].u.string) + 1;
        if ((member_string(warr->item[il].u.string, ids) < 0) &&
            (member_string(warr->item[il].u.string, gAdjid_list_d) < 0)) {
            fail = 1;
        }
    }

    /*
     * Simple case: all adjs were single word
     */
    if (!fail)
        return 1;

    if (from == to)
        return 0;

    adstr = (char *)DXALLOC(sum, TAG_TEMPORARY, "check_adjectiv");

    /*
     * If we now have: "adj1 adj2 adj3 ... adjN"
     * We must test in order: "adj1 adj2 adj3 .... adjN-1 adjN"
                              "adj1 adj2 adj3 .... adjN-1"
                              "adj1 adj2 adj3 ...." 
                              ....
     * if match for adj1 .. adj3 continue with:
     *                        "adj4 adj5 .... adjN-1 adjN"
     *                        "adj4 adj5 .... adjN-1"
     *                        "adj4 adj5 ...."
     *                        .....
     */
    for (il = from; il <= to;) {/* adj1 .. adjN */
        for (back = to; back >= il; back--) {   /* back from adjN to adj[il] */
            /*
             * Create teststring with "adj[il] .. adj[back]"
             */
            adstr[0] = 0;
            for (sum = il; sum <= back; sum++) {        /* test "adj[il] ..
                                                         * adj[back] */
                if (sum > il)
                    strcat(adstr, " ");
                strcat(adstr, warr->item[sum].u.string);
            }
            if ((member_string(adstr, ids) < 0) &&
                (member_string(adstr, gAdjid_list_d) < 0))
                continue;
            else {
                il = back + 1;  /* Match "adj[il] adj[il+1] .. adj[back]" */
                back = to;
                break;
            }
        }
        if (back < to) {
            FREE(adstr);        /* adj[il] does not match at all => no match */
            return 0;
        }
    }
    FREE(adstr);
    return 1;
}

/*
 * Function name:       member_string
 * Description:         Checks if a string is a member of an array.
 * Arguments:           str: The string to search for
 *                      sarr: array of strings
 * Returns:             Pos if found else -1.
 */
static int
member_string (const char * str, array_t * sarr)
{
    int il;

    if (!sarr)
        return -1;

    for (il = 0; il < sarr->size; il++) {
        if (sarr->item[il].type != T_STRING)
            continue;

        if (strcmp(sarr->item[il].u.string, str) == 0)
            return il;
    }
    return -1;
}

#ifndef PARSE_FOREIGN
/*
 * Function name:       parse_to_plural
 * Description:         Change a sentence in singular form to a sentence
 *                      in pluralform.
 * Arguments:           str: The sentence to change
 * Returns:             Sentence in plural form.
 */
static const char *
     parse_to_plural (const char * str)
{
    array_t *words;
    const char *sp;
    int il, changed;

    if (!(strchr(str, ' ')))
        return string_copy(parse_one_plural(str), "parse_to_plural");

    words = explode_string(str, strlen(str), " ", 1);

    for (changed = 0, il = 1; il < words->size; il++) {
        if ((EQ(words->item[il].u.string, "of")) ||
            (il + 1 == words->size)) {
            sp = parse_one_plural(words->item[il - 1].u.string);
            if (sp != words->item[il - 1].u.string) {
                free_svalue(&words->item[il - 1], "parse_to_plural");
                words->item[il - 1].type = T_STRING;
                words->item[il - 1].subtype = STRING_MALLOC;
                words->item[il - 1].u.string = string_copy(sp, 
                                                           "parse_to_plural");
                changed = 1;
            }
        }
    }
    if (!changed) {
        free_array(words);
        return string_copy(str, "parse_to_plural");
    }
    sp = implode_string(words, " ", 1);
    free_array(words);
    return sp;
}

/*
 * Function name:       parse_one_plural
 * Description:         Change a noun in singularform to a noun
 *                      in pluralform.
 * Arguments:           str: The sentence to change
 * Returns:             Word in plural form.
 */
static const char *
     parse_one_plural (const char * str)
{
    char ch, ch2, ch3;
    int sl;
    static char pbuf[100];      /* Words > 100 letters? In Wales maybe... */

    sl = strlen(str) - 1;
    if ((sl < 2) || (sl > 90))
        return str;

    ch = str[sl];
    ch2 = str[sl - 1];
    ch3 = sl > 2 ? str[sl - 2] : '\0';
    strcpy(pbuf, str);
    pbuf[sl] = 0;

    switch (ch) {
    case 'e':
        if (ch2 == 'f') {
            pbuf[sl - 1] = 0;
            return strcat(pbuf, "ves");
        }
    case 'f':
        return strcat(pbuf, "ves");
    case 'h':
        if (ch2 == 's' || ch2 == 'c')
            return strcat(pbuf, "hes");
    case 's':
        return strcat(pbuf, "ses");
    case 'x':
        if (EQ(str, "ox"))
            return "oxen";
        else
            return strcat(pbuf, "xes");
    case 'y':
        if ((ch2 != 'a' && ch2 != 'e' && ch2 != 'i' && ch2 != 'o' &&
             ch2 != 'u') || (ch2 == 'u' && ch3 == 'q'))
            return strcat(pbuf, "ies");
    }

    if (EQ(str, "corpse"))
        return "corpses";
    if (EQ(str, "tooth"))
        return "teeth";
    if (EQ(str, "foot"))
        return "feet";
    if (EQ(str, "man"))
        return "men";
    if (EQ(str, "woman"))
        return "women";
    if (EQ(str, "child"))
        return "children";
    if (EQ(str, "sheep"))
        return "sheep";
    if (EQ(str, "goose"))
        return "geese";
    if (EQ(str, "mouse"))
        return "mice";
    if (EQ(str, "deer"))
        return "deer";
    if (EQ(str, "moose"))
        return "moose";

    pbuf[sl] = ch;
    return strcat(pbuf, "s");
}
#endif

/*

   End of Parser

***************************************************************/
