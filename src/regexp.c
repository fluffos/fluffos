/*
 *
 * regexp.c - regular expression matching
 *
 * DESCRIPTION
 *
 *      Underneath the reformatting and comment blocks which were added to
 *      make it consistent with the rest of the code, you will find a
 *      modified version of Henry Specer's regular expression library.
 *      Henry's functions were modified to provide the minimal regular
 *      expression matching, as required by (c) 1986 by University of Toronto.
 *      Written by Henry Spencer.  Not derived from licensed software.
 *
 *      Permission is granted to anyone to use this software for any
 *      purpose on any computer system, and to redistribute it freely,
 *      subject to the following restrictions:
 *
 *      1. The author is not responsible for the consequences of use of
 *         this software, no matter how awful, even if they arise
 *         from defects in it.
 *
 *      2. The origin of this software must not be misrepresented, either
 *         by explicit claim or by omission.
 *
 *      3. Altered versions must be plainly marked as such, and must not
 *         be misrepresented as being the original software.
 *
 *
 * This version modified by Ian Phillipps to return pointer to terminating
 * NUL on substitution string. [ Temp mail address ex-igp@camcon.co.uk ]
 *
 *      Altered by amylaar to support excompatible option and the
 *      operators \< and >\ . ( 7.Sep. 1991 )
 *
 * regsub altered by amylaar to take an additional parameter specifying
 * maximum number of bytes that can be written to the memory region
 * pointed to by dest
 *
 *      Beware that some of this code is subtly aware of the way operator
 *      precedence is structured in regular expressions.  Serious changes in
 *      regular-expression syntax might require a total rethink.
 *
 * AUTHORS
 *
 *     Mark H. Colburn, NAPS International (mark@jhereg.mn.org)
 *     Henry Spencer, University of Torronto (henry@utzoo.edu)
 *
 * Sponsored by The USENIX Association for public distribution.
 *
 */

/* Headers */

#include "std.h"
#include "regexp.h"
#include "lpc_incl.h"
#include "comm.h"
#include "ed.h"

/*
 * The "internal use only" fields in regexp.h are present to pass info from
 * compile to execute that permits the execute phase to run lots faster on
 * simple cases.  They are:
 *
 * regstart     char that must begin a match; '\0' if none obvious
 * reganch      is the match anchored (at beginning-of-line only)?
 * regmust      string (pointer into program) that match must include, or NULL
 * regmlen      length of regmust string
 *
 * Regstart and reganch permit very fast decisions on suitable starting points
 * for a match, cutting down the work a lot.  Regmust permits fast rejection
 * of lines that cannot possibly match.  The regmust tests are costly enough
 * that regcomp() supplies a regmust only if the r.e. contains something
 * potentially expensive (at present, the only such thing detected is * or +
 * at the start of the r.e., which can involve a lot of backup).  Regmlen is
 * supplied because the test in regexec() needs it and regcomp() is computing
 * it anyway.
 */

/*
 * The first byte of the regexp internal "program" is actually this magic
 * number; the start node begins in the second byte.
 */
#define MAGIC   0234

/*
 * Structure for regexp "program".  This is essentially a linear encoding
 * of a nondeterministic finite-state machine (aka syntax charts or
 * "railroad normal form" in parsing technology).  Each node is an opcode
 * plus a "nxt" pointer, possibly plus an operand.  "Nxt" pointers of
 * all nodes except BRANCH implement concatenation; a "nxt" pointer with
 * a BRANCH on both ends of it is connecting two alternatives.  (Here we
 * have one of the subtle syntax dependencies:  an individual BRANCH (as
 * opposed to a collection of them) is never concatenated with anything
 * because of operator precedence.)  The operand of some types of node is
 * a literal string; for others, it is a node leading into a sub-FSM.  In
 * particular, the operand of a BRANCH node is the first node of the branch.
 * (NB this is *not* a tree structure:  the tail of the branch connects
 * to the thing following the set of BRANCHes.)  The opcodes are:
 */

/* definition   number  opnd?   meaning */
#define END     0               /* no   End of program. */
#define BOL     1               /* no   Match "" at beginning of line. */
#define EOL     2               /* no   Match "" at end of line. */
#define ANY     3               /* no   Match any one character. */
#define ANYOF   4               /* str  Match any character in this string. */
#define ANYBUT  5               /* str  Match any character not in this
                                 * string. */
#define BRANCH  6               /* node Match this alternative, or the nxt... */
#define BACK    7               /* no   Match "", "nxt" ptr points backward. */
#define EXACTLY 8               /* str  Match this string. */
#define NOTHING 9               /* no   Match empty string. */
#define STAR    10              /* node Match this (simple) thing 0 or more
                                 * times. */
#define PLUS    11              /* node Match this (simple) thing 1 or more
                                 * times. */
#define WORDSTART 12            /* node matching a start of a word          */
#define WORDEND 13              /* node matching an end of a word           */
#define OPEN    20              /* no   Mark this point in input as start of
                                 * #n. */
 /* OPEN+1 is number 1, etc. */
#define CLOSE   30              /* no   Analogous to OPEN. */

/*
 * Opcode notes:
 *
 * BRANCH       The set of branches constituting a single choice are hooked
 *              together with their "nxt" pointers, since precedence prevents
 *              anything being concatenated to any individual branch.  The
 *              "nxt" pointer of the last BRANCH in a choice points to the
 *              thing following the whole choice.  This is also where the
 *              final "nxt" pointer of each individual branch points; each
 *              branch starts with the operand node of a BRANCH node.
 *
 * BACK         Normal "nxt" pointers all implicitly point forward; BACK
 *              exists to make loop structures possible.
 *
 * STAR,PLUS    '?', and complex '*' and '+', are implemented as circular
 *              BRANCH structures using BACK.  Simple cases (one character
 *              per match) are implemented with STAR and PLUS for speed and
 *              to minimize recursive * plunges.
 *
 * OPEN,CLOSE   ...are numbered at compile time.
 */

/*
 * A node is one char of opcode followed by two chars of "nxt" pointer.
 * "Nxt" pointers are stored as two 8-bit pieces, high order first.  The
 * value is a positive offset from the opcode of the node containing it.
 * An operand, if any, simply follows the node.  (Note that much of the
 * code generation knows about this implicit relationship.)
 *
 * Using two bytes for the "nxt" pointer is vast overkill for most things,
 * but allows patterns to get big without disasters.
 */
#define OP(p)   (*(p))
/* stralloc.h */
#undef NEXT
#define NEXT(p) (((*((p)+1)&0377)<<8) + (*((p)+2)&0377))
#define OPERAND(p)      ((p) + 3)

/*
 * Utility definitions.
 */

#define SPECIAL 0x100
#define LBRAC   ('('|SPECIAL)
#define RBRAC   (')'|SPECIAL)
#define ASTERIX ('*'|SPECIAL)
#define PLUSS   ('+'|SPECIAL)
#define QMARK   ('?'|SPECIAL)
#define OR_OP   ('|'|SPECIAL)
#define DOLLAR  ('$'|SPECIAL)
#define DOT     ('.'|SPECIAL)
#define CARET   ('^'|SPECIAL)
#define LSQBRAC ('['|SPECIAL)
#define RSQBRAC (']'|SPECIAL)
#define LSHBRAC ('<'|SPECIAL)
#define RSHBRAC ('>'|SPECIAL)

#define FAIL(m) { regerror(m); return(NULL); }
#define ISMULT(c)       ((c) == ASTERIX || (c) == PLUSS || (c) == QMARK)
#define META    "^$.[()|?+*\\"

/* Linux seems to define CHARBITS to 8 in values.h; not sure what system
 * defines CHARBITS in a form we can use or how to tell (check in configure?)
 * Just use 8 bits for now.
 */
#if 0
#if defined(linux)
#  ifndef CHARBITS
#    define CHARBITS      0xff
#  endif
#  define UCHARAT(p)    (*(unsigned char *)(p))
#else
#  ifndef CHARBITS
#    define CHARBITS      0xff
#    define       UCHARAT(p)      (*(unsigned char *)(p))
#  else
#    define       UCHARAT(p)      (*(p)&CHARBITS)
#  endif
#endif
#else
#  undef CHARBITS
#  define CHARBITS 0xff
#  define UCHARAT(p)      (*(unsigned char *)(p))
#endif

#define ISWORDPART(c) ( isalnum((unsigned char)c) || (c) == '_' )

/*
 * Flags to be passed up and down.
 */
#define HASWIDTH        01      /* Known never to match null string. */
#define SIMPLE          02      /* Simple enough to be STAR/PLUS operand. */
#define SPSTART         04      /* Starts with * or +. */
#define WORST           0       /* Worst case. */

/*
 * Global work variables for regcomp().
 */
int regexp_user;
const char *regexp_error;

static short *regparse;         /* Input-scan pointer. */
static int regnpar;             /* () count. */
static char regdummy;
static char *regcode;           /* Code-emit pointer; &regdummy = don't. */
static int regsize;             /* Code size. */

/*
 * Forward declarations for regcomp()'s friends.
 */
#ifndef STATIC
#define STATIC  static
#endif
STATIC char *reg (int, int *);
STATIC char *regbranch (int *);
STATIC char *regpiece (int *);
STATIC char *regatom (int *);
STATIC char *regnode (char);
STATIC char *regnext (char *);
STATIC void regc (char);
STATIC void reginsert (char, char *);
STATIC void regtail (char *, char *);
STATIC void regoptail (char *, char *);

#ifdef STRCSPN
STATIC int strcspn();
#endif

STATIC void regerror (const char * s) {
    switch (regexp_user) {
    case ED_REGEXP:
        ED_OUTPUTV(ED_DEST, "ed: regular expression error: %s", s);
        break;
    case EFUN_REGEXP:
        regexp_error = s;
        break;
    }
}

/*
 - regcomp - compile a regular expression into internal code
 *
 * We can't allocate space until we know how big the compiled form will be,
 * but we can't compile it (and thus know how big it is) until we've got a
 * place to put the code.  So we cheat:  we compile it twice, once with code
 * generation turned off and size counting turned on, and once "for real".
 * This also means that we don't allocate space until we are sure that the
 * thing really will compile successfully, and we never have to move the
 * code and thus invalidate pointers into it.  (Note that it has to be in
 * one piece because FREE() must be able to free it all.)
 *
 * Beware that the optimization-preparation code in here knows about some
 * of the structure of the compiled regexp.
 */
regexp *regcomp (unsigned char * exp,
                   int excompat)       /* \( \) operators like in unix ex */
{
    register regexp *r;
    register unsigned char *scan;
    register char *longest;
    register int len;
    int flags;
    short *exp2, *dest, c;

    if (!exp)
        FAIL("NULL argument\n");

    exp2 = (short *)
        DXALLOC((strlen((char *)exp) + 1) * (sizeof(short[8]) / sizeof(char[8])), 
                TAG_TEMPORARY, "regcomp: 1");
    for (scan = exp, dest = exp2; (c = *scan++);) {
        switch (c) {
        case '(':
        case ')':
            *dest++ = excompat ? c : c | SPECIAL;
            break;
        case '.':
        case '*':
        case '+':
        case '?':
        case '|':
        case '$':
        case '^':
        case '[':
        case ']':
            *dest++ = c | SPECIAL;
            break;
        case '\\':
            switch (c = *scan++) {
            case 0:
                FREE(exp2);
                FAIL("Regular expression cannot end with '\\'.  Use \"\\\\\".\n");
                break;
            case '(':
            case ')':
                *dest++ = excompat ? c | SPECIAL : c;
                break;
            case '<':
            case '>':
                *dest++ = c | SPECIAL;
                break;
            case '{':
            case '}':
                FREE(exp2);
                FAIL("sorry, unimplemented operator\n");
            case 'b':
                *dest++ = '\b';
                break;
            case 't':
                *dest++ = '\t';
                break;
            case 'r':
                *dest++ = '\r';
                break;
            default:
                *dest++ = c;
            }
            break;
        default:
            *dest++ = c;
        }
    }
    *dest = 0;
    /* First pass: determine size, legality. */
    regparse = exp2;
    regnpar = 1;
    regsize = 0L;
    regcode = &regdummy;
    regc((char) MAGIC);
    if (reg(0, &flags) == (char *) NULL) {
        FREE(exp2);
        return ((regexp *) NULL);
    }

    /* Small enough for pointer-storage convention? */
    if (regsize >= 32767L)      /* Probably could be 65535L. */
    {
        FREE(exp2);
        FAIL("regexp too big\n");
    }

    /* Allocate space. */
    r = (regexp *) DXALLOC(sizeof(regexp) + (unsigned) regsize, 
                           TAG_TEMPORARY, "regcomp: 2");
    if (r == (regexp *) NULL) {
        FREE(exp2);
        FAIL("out of space\n");
    }

    /* Second pass: emit code. */
    regparse = exp2;
    regnpar = 1;
    regcode = (char *)(r->program);
    regc((char) MAGIC);
    if (reg(0, &flags) == NULL) {
        FREE(exp2);
        FREE(r);
        return ((regexp *) NULL);
    }

    /* Dig out information for optimizations. */
    r->regstart = '\0';         /* Worst-case defaults. */
    r->reganch = 0;
    r->regmust = NULL;
    r->regmlen = 0;
    scan = (unsigned char *)(r->program + 1);   /* First BRANCH. */
    if (OP(regnext((char *)scan)) == END) {     /* Only one top-level choice. */
        scan = OPERAND(scan);

        /* Starting-point info. */
        if (OP(scan) == EXACTLY)
            r->regstart = *OPERAND(scan);
        else if (OP(scan) == BOL)
            r->reganch++;

        /*
         * If there's something expensive in the r.e., find the longest
         * literal string that must appear and make it the regmust.  Resolve
         * ties in favor of later strings, since the regstart check works
         * with the beginning of the r.e. and avoiding duplication
         * strengthens checking.  Not a strong reason, but sufficient in the
         * absence of others.
         */
        if (flags & SPSTART) {
            longest = NULL;
            len = 0;
            for (; scan != NULL; scan = (unsigned char *)regnext((char *)scan)) {
                char *tmp = (char *)OPERAND(scan);
                int tlen;
                if (OP(scan) == EXACTLY && (tlen = strlen(tmp)) >= len) {
                    longest = tmp;
                    len = tlen;
                }
            }
            r->regmust = longest;
            r->regmlen = len;
        }
    }
    FREE((char *) exp2);
    return (r);
}

/*
 - reg - regular expression, i.e. main body or parenthesized thing
 *
 * Caller must absorb opening parenthesis.
 *
 * Combining parenthesis handling with the base level of regular expression
 * is a trifle forced, but the need to tie the tails of the branches to what
 * follows makes it hard to avoid.
 */
static char *reg (int paren, /* Parenthesized? */
                         int * flagp)
{
    register char *ret;
    register char *br;
    register char *ender;
    register int parno = 0;
    int flags;

    *flagp = HASWIDTH;          /* Tentatively. */

    /* Make an OPEN node, if parenthesized. */
    if (paren) {
        if (regnpar >= NSUBEXP)
            FAIL("too many ()\n");
        parno = regnpar;
        regnpar++;
        ret = regnode(OPEN + parno);
    } else
        ret = (char *) NULL;

    /* Pick up the branches, linking them together. */
    br = regbranch(&flags);
    if (br == (char *) NULL)
        return ((char *) NULL);
    if (ret != (char *) NULL)
        regtail(ret, br);       /* OPEN -> first. */
    else
        ret = br;
    if (!(flags & HASWIDTH))
        *flagp &= ~HASWIDTH;
    *flagp |= flags & SPSTART;
    while (*regparse == OR_OP) {
        regparse++;
        br = regbranch(&flags);
        if (br == (char *) NULL)
            return ((char *) NULL);
        regtail(ret, br);       /* BRANCH -> BRANCH. */
        if (!(flags & HASWIDTH))
            *flagp &= ~HASWIDTH;
        *flagp |= flags & SPSTART;
    }

    /* Make a closing node, and hook it on the end. */
    ender = regnode((paren) ? CLOSE + parno : END);
    regtail(ret, ender);

    /* Hook the tails of the branches to the closing node. */
    for (br = ret; br != (char *) NULL; br = regnext(br))
        regoptail(br, ender);

    /* Check for proper termination. */
    if (paren && *regparse++ != RBRAC) {
        FAIL("unmatched ()\n");
    } else if (!paren && *regparse != '\0') {
        if (*regparse == RBRAC) {
            FAIL("unmatched ()\n");
        } else
            FAIL("junk on end\n");/* "Can't happen". */
        /* NOTREACHED */
    }
    return (ret);
}

/*
 - regbranch - one alternative of an | operator
 *
 * Implements the concatenation operator.
 */
static char *regbranch (int * flagp)
{
    register char *ret;
    register char *chain;
    register char *latest;
    int flags;

    *flagp = WORST;             /* Tentatively. */

    ret = regnode(BRANCH);
    chain = (char *) NULL;
    while (*regparse != '\0' && *regparse != OR_OP && *regparse != RBRAC) {
        latest = regpiece(&flags);
        if (latest == (char *) NULL)
            return ((char *) NULL);
        *flagp |= flags & HASWIDTH;
        if (chain == (char *) NULL)     /* First piece. */
            *flagp |= flags & SPSTART;
        else
            regtail(chain, latest);
        chain = latest;
    }
    if (chain == (char *) NULL) /* Loop ran zero times. */
        regnode(NOTHING);

    return (ret);
}

/*
 - regpiece - something followed by possible [*+?]
 *
 * Note that the branching code sequence used for ? and the general cases of
 * * and + are somewhat optimized: they use the same NOTHING node as both the
 * endmarker for their branch list and the body of the last branch.  It might
 * seem that this node could be dispensed with entirely, but the endmarker
 * role is not redundant.
 */
static char *regpiece (int * flagp)
{
    register char *ret;
    register short op;
    register char *nxt;
    int flags;

    ret = regatom(&flags);
    if (ret == (char *) NULL)
        return ((char *) NULL);

    op = *regparse;
    if (!ISMULT(op)) {
        *flagp = flags;
        return (ret);
    }
    if (!(flags & HASWIDTH) && op != QMARK)
        FAIL("*+ operand could be empty\n");
    *flagp = (op != PLUSS) ? (WORST | SPSTART) : (WORST | HASWIDTH);

    if (op == ASTERIX && (flags & SIMPLE))
        reginsert(STAR, ret);
    else if (op == ASTERIX) {
        /* Emit x* as (x&|), where & means "self". */
        reginsert(BRANCH, ret); /* Either x */
        regoptail(ret, regnode(BACK));  /* and loop */
        regoptail(ret, ret);    /* back */
        regtail(ret, regnode(BRANCH));  /* or */
        regtail(ret, regnode(NOTHING)); /* null. */
    } else if (op == PLUSS && (flags & SIMPLE))
        reginsert(PLUS, ret);
    else if (op == PLUSS) {
        /* Emit x+ as x(&|), where & means "self". */
        nxt = regnode(BRANCH);  /* Either */
        regtail(ret, nxt);
        regtail(regnode(BACK), ret);    /* loop back */
        regtail(nxt, regnode(BRANCH));  /* or */
        regtail(ret, regnode(NOTHING)); /* null. */
    } else if (op == QMARK) {
        /* Emit x? as (x|) */
        reginsert(BRANCH, ret); /* Either x */
        regtail(ret, regnode(BRANCH));  /* or */
        nxt = regnode(NOTHING); /* null. */
        regtail(ret, nxt);
        regoptail(ret, nxt);
    }
    regparse++;
    if (ISMULT(*regparse))
        FAIL("nested *?+\n");

    return (ret);
}

/*
 - regatom - the lowest level
 *
 * Optimization:  gobbles an entire sequence of ordinary characters so that
 * it can turn them into a single node, which is smaller to store and
 * faster to run.
 */
static char *regatom (int * flagp)
{
    register char *ret;
    int flags;

    *flagp = WORST;             /* Tentatively. */

    switch (*regparse++) {
    case CARET:
        ret = regnode(BOL);
        break;
    case DOLLAR:
        ret = regnode(EOL);
        break;
    case DOT:
        ret = regnode(ANY);
        *flagp |= HASWIDTH | SIMPLE;
        break;
    case LSHBRAC:
        ret = regnode(WORDSTART);
        break;
    case RSHBRAC:
        ret = regnode(WORDEND);
        break;
    case LSQBRAC:{
            register int class;
            register int classend;

            if (*regparse == CARET) {   /* Complement of range. */
                ret = regnode(ANYBUT);
                regparse++;
            } else
                ret = regnode(ANYOF);
            if (*regparse == RSQBRAC || *regparse == '-')
                regc(*regparse++);
            while (*regparse != '\0' && *regparse != RSQBRAC) {
                if (*regparse == '-') {
                    regparse++;
                    if (*regparse == RSQBRAC || *regparse == '\0')
                        regc('-');
                    else {
                        class = (CHARBITS & *(regparse - 2)) + 1;
                        classend = (CHARBITS & *(regparse));
                        if (class > classend + 1)
                            FAIL("invalid [] range\n");
                        for (; class <= classend; class++)
                            regc(class);
                        regparse++;
                    }
                } else
                    regc(*regparse++);
            }
            regc('\0');
            if (*regparse != RSQBRAC)
                FAIL("unmatched []\n");
            regparse++;
            *flagp |= HASWIDTH | SIMPLE;
        }
        break;
    case LBRAC:
        ret = reg(1, &flags);
        if (ret == (char *) NULL)
            return ((char *) NULL);
        *flagp |= flags & (HASWIDTH | SPSTART);
        break;
    case '\0':
    case OR_OP:
    case RBRAC:
        FAIL("internal urp\n"); /* Supposed to be caught earlier. */
        break;
    case ASTERIX:
        FAIL("* follows nothing\n");
        break;
    case PLUSS:
        FAIL("+ follows nothing\n");
        break;
    case QMARK:
        FAIL("? follows nothing\n");
        break;
    default:{
            register int len;
            register short ender;

            regparse--;
            for (len = 0; regparse[len] &&
             !(regparse[len] & SPECIAL) && regparse[len] != RSQBRAC; len++);
            if (len <= 0) {
                FAIL("unexpected ]\n");
            }
            ender = *(regparse + len);
            if (len > 1 && ISMULT(ender))
                len--;          /* Back off clear of ?+* operand. */
            *flagp |= HASWIDTH;
            if (len == 1)
                *flagp |= SIMPLE;
            ret = regnode(EXACTLY);
            while (len > 0) {
                regc(*regparse++);
                len--;
            }
            regc('\0');
        }
        break;
    }

    return (ret);
}

/*
 - regnode - emit a node
 */
static char *regnode (char op)
{
    register char *ret;
    register char *ptr;

    ret = regcode;
    if (ret == &regdummy) {
        regsize += 3;
        return (ret);
    }
    ptr = ret;
    *ptr++ = op;
    *ptr++ = '\0';              /* Null "nxt" pointer. */
    *ptr++ = '\0';
    regcode = ptr;

    return (ret);
}

/*
 - regc - emit (if appropriate) a byte of code
 */
static void regc (char b)
{
    if (regcode != &regdummy)
        *regcode++ = b;
    else
        regsize++;
}

/*
 - reginsert - insert an operator in front of already-emitted operand
 *
 * Means relocating the operand.
 */
static void reginsert (char op, char * opnd)
{
    register char *src;
    register char *dst;
    register char *place;

    if (regcode == &regdummy) {
        regsize += 3;
        return;
    }
    src = regcode;
    regcode += 3;
    dst = regcode;
    while (src > opnd)
        *--dst = *--src;

    place = opnd;               /* Op node, where operand used to be. */
    *place++ = op;
    *place++ = '\0';
    *place++ = '\0';
}

/*
 - regtail - set the next-pointer at the end of a node chain
 */
static void regtail (char * p, char * val)
{
    register char *scan;
    register char *temp;
    register int offset;

    if (p == &regdummy)
        return;

    /* Find last node. */
    scan = p;
    for (;;) {
        temp = regnext(scan);
        if (temp == (char *) NULL)
            break;
        scan = temp;
    }

    if (OP(scan) == BACK)
        offset = scan - val;
    else
        offset = val - scan;
    *(scan + 1) = (offset >> 8) & 0377;
    *(scan + 2) = offset & 0377;
}

/*
 - regoptail - regtail on operand of first argument; nop if operandless
 */
static void regoptail (char * p, char * val)
{
    /* "Operandless" and "op != BRANCH" are synonymous in practice. */
    if (p == (char *) NULL || p == &regdummy || OP(p) != BRANCH)
        return;
    regtail(OPERAND(p), val);
}

/*
 * regexec and friends
 */

/*
 * Global work variables for regexec().
 */
static const char *reginput;    /* String-input pointer. */
static const char *regbol;      /* Beginning of input, for ^ check. */
static const char **regstartp;  /* Pointer to startp array. */
static const char **regendp;    /* Ditto for endp. */

/*
 * Forwards.
 */
STATIC int regtry (regexp *, const char *);
STATIC int regmatch (char *);
STATIC int regrepeat (char *);

#ifdef DEBUG
int regnarrate = 0;
void regdump (regexp *);
STATIC char *regprop (char *);

#endif

/*
 - regexec - match a regexp against a string
 */
int regexec (register regexp * prog, register const char * string)
{
    register const char *s;

    /* Be paranoid... */
    if (prog == (regexp *) NULL || string == (char *) NULL) {
        regerror("NULL parameter\n");
        return (0);
    }
    /* Check validity of program. */
    if (UCHARAT(prog->program) != MAGIC) {
        regerror("corrupted program\n");
        return (0);
    }
    /* If there is a "must appear" string, look for it. */
    if (prog->regmust != (char *) NULL) {
        s = string;
        while ((s = strchr(s, prog->regmust[0])) != (char *) NULL) {
            if (strncmp(s, prog->regmust, prog->regmlen) == 0)
                break;          /* Found it. */
            s++;
        }
        if (s == (char *) NULL) /* Not present. */
            return (0);
    }
    /* Mark beginning of line for ^ . */
    regbol = string;

    /* Simplest case:  anchored match need be tried only once. */
    if (prog->reganch)
        return (regtry(prog, string));

    /* Messy cases:  unanchored match. */
    s = string;
    if (prog->regstart != '\0')
        /* We know what char it must start with. */
        while ((s = strchr(s, prog->regstart)) != (char *) NULL) {
            if (regtry(prog, s))
                return (1);
            s++;
        }
    else
        /* We don't -- general case. */
        do {
            if (regtry(prog, s))
                return (1);
        } while (*s++ != '\0');

    /* Failure. */
    return (0);
}

/*
 - regtry - try match at specific point
 */
static int regtry (regexp * prog, const char * string)
{
    register int i;
    register const char **sp;
    register const char **ep;

    reginput = string;
    regstartp = prog->startp;
    regendp = prog->endp;

    sp = prog->startp;
    ep = prog->endp;
    for (i = NSUBEXP; i > 0; i--) {
        *sp++ = (char *) NULL;
        *ep++ = (char *) NULL;
    }
    if (regmatch(prog->program + 1)) {
        prog->startp[0] = string;
        prog->endp[0] = reginput;
        return (1);
    } else
        return (0);
}

/*
 - regmatch - main matching routine
 *
 * Conceptually the strategy is simple:  check to see whether the current
 * node matches, call self recursively to see whether the rest matches,
 * and then act accordingly.  In practice we make some effort to avoid
 * recursion, in particular by going through "ordinary" nodes (that don't
 * need to know whether the rest of the match failed) by a loop instead of
 * by recursion.
 */
static int regmatch (char * prog)
{
    register char *scan;        /* Current node. */
    char *nxt;                  /* nxt node. */

    scan = prog;
#ifdef DEBUG
    if (scan != (char *) NULL && regnarrate)
        debug_message("%s(\n", regprop(scan));
#endif
    while (scan != (char *) NULL) {
#ifdef DEBUG
        if (regnarrate)
            debug_message("%s...\n", regprop(scan));
#endif
        nxt = regnext(scan);

        switch (OP(scan)) {
        case BOL:
            if (reginput != regbol)
                return (0);
            break;
        case EOL:
            if (*reginput != '\0')
                return (0);
            break;
        case ANY:
            if (*reginput == '\0')
                return (0);
            reginput++;
            break;
        case WORDSTART:
            if (reginput == regbol)
                break;
            if (*reginput == '\0' ||
                ISWORDPART(*(reginput - 1)) || !ISWORDPART(*reginput))
                return (0);
            break;
        case WORDEND:
            if (*reginput == '\0')
                break;
            if (reginput == regbol ||
                !ISWORDPART(*(reginput - 1)) || ISWORDPART(*reginput))
                return (0);
            break;
        case EXACTLY:{
                register int len;
                register char *opnd;

                opnd = OPERAND(scan);
                /* Inline the first character, for speed. */
                if (*opnd != *reginput)
                    return (0);
                len = strlen(opnd);
                if (len > 1 && strncmp(opnd, reginput, len) != 0)
                    return (0);
                reginput += len;
            }
            break;
        case ANYOF:
            if (*reginput == '\0' ||
                strchr(OPERAND(scan), *reginput) == (char *) NULL)
                return (0);
            reginput++;
            break;
        case ANYBUT:
            if (*reginput == '\0' ||
                strchr(OPERAND(scan), *reginput) != (char *) NULL)
                return (0);
            reginput++;
            break;
        case NOTHING:
            break;
        case BACK:
            break;
        case OPEN + 1:
        case OPEN + 2:
        case OPEN + 3:
        case OPEN + 4:
        case OPEN + 5:
        case OPEN + 6:
        case OPEN + 7:
        case OPEN + 8:
        case OPEN + 9:{
                register int no;
                register const char *save;

                no = OP(scan) - OPEN;
                save = reginput;

                if (regmatch(nxt)) {
                    /*
                     * Don't set startp if some later invocation of the same
                     * parentheses already has.
                     */
                    if (regstartp[no] == (char *) NULL)
                        regstartp[no] = save;
                    return (1);
                } else
                    return (0);
            }
            break;
        case CLOSE + 1:
        case CLOSE + 2:
        case CLOSE + 3:
        case CLOSE + 4:
        case CLOSE + 5:
        case CLOSE + 6:
        case CLOSE + 7:
        case CLOSE + 8:
        case CLOSE + 9:{
                register int no;
                register const char *save;

                no = OP(scan) - CLOSE;
                save = reginput;

                if (regmatch(nxt)) {
                    /*
                     * Don't set endp if some later invocation of the same
                     * parentheses already has.
                     */
                    if (regendp[no] == (char *) NULL)
                        regendp[no] = save;
                    return (1);
                } else
                    return (0);
            }
            break;
        case BRANCH:{
                register const char *save;

                if (OP(nxt) != BRANCH)  /* No choice. */
                    nxt = OPERAND(scan);        /* Avoid recursion. */
                else {
                    do {
                        save = reginput;
                        if (regmatch(OPERAND(scan)))
                            return (1);
                        reginput = save;
                        scan = regnext(scan);
                    } while (scan != (char *) NULL && OP(scan) == BRANCH);
                    return (0);
                    /* NOTREACHED */
                }
            }
            break;
        case STAR:
        case PLUS:{
                register char nextch;
                register int no;
                register const char *save;
                register int minimum;

                /*
                 * Lookahead to avoid useless match attempts when we know
                 * what character comes next.
                 */
                nextch = '\0';
                if (OP(nxt) == EXACTLY)
                    nextch = *OPERAND(nxt);
                minimum = (OP(scan) == STAR) ? 0 : 1;
                save = reginput;
                no = regrepeat(OPERAND(scan));
                while (no >= minimum) {
                    /* If it could work, try it. */
                    if (nextch == '\0' || *reginput == nextch)
                        if (regmatch(nxt))
                            return (1);
                    /* Couldn't or didn't -- back up. */
                    no--;
                    reginput = save + no;
                }
                return (0);
            }
            break;
        case END:
            return (1);         /* Success! */
            break;
        default:
            regerror("memory corruption\n");
            return (0);
            break;
        }

        scan = nxt;
    }

    /*
     * We get here only if there's trouble -- normally "case END" is the
     * terminating point.
     */
    regerror("corrupted pointers\n");
    return (0);
}

/*
 - regrepeat - repeatedly match something simple, report how many
 */
static int regrepeat (char * p)
{
    register int count = 0;
    register const char *scan;
    register char *opnd;

    scan = reginput;
    opnd = OPERAND(p);
    switch (OP(p)) {
    case ANY:
        count = strlen(scan);
        scan += count;
        break;
    case EXACTLY:
        while (*opnd == *scan) {
            count++;
            scan++;
        }
        break;
    case ANYOF:
        while (*scan != '\0' && strchr(opnd, *scan) != (char *) NULL) {
            count++;
            scan++;
        }
        break;
    case ANYBUT:
        while (*scan != '\0' && strchr(opnd, *scan) == (char *) NULL) {
            count++;
            scan++;
        }
        break;
    default:                    /* Oh dear.  Called inappropriately. */
        regerror("internal foulup\n");
        count = 0;              /* Best compromise. */
        break;
    }
    reginput = scan;

    return (count);
}


/*
 - regnext - dig the "nxt" pointer out of a node
 */
static char *regnext (register char * p)
{
    register int offset;

    if (p == &regdummy)
        return ((char *) NULL);

    offset = NEXT(p);
    if (offset == 0)
        return ((char *) NULL);

    if (OP(p) == BACK)
        return (p - offset);
    else
        return (p + offset);
}

#ifdef DEBUG

STATIC char *regprop (char *);

/*
 - regdump - dump a regexp onto stdout in vaguely comprehensible form
 */
void regdump (regexp * r)
{
    register char *s;
    register char op = EXACTLY; /* Arbitrary non-END op. */
    register char *nxt;

#ifdef _AIX
    extern char *strchr();

#endif                          /* _AIX */

    s = r->program + 1;
    while (op != END) {         /* While that wasn't END last time... */
        op = OP(s);
        printf("%2d%s", (s - r->program), regprop(s));    /* Where, what. */
        nxt = regnext(s);
        if (nxt == (char *) NULL)       /* nxt ptr. */
            printf("(0)");
        else
            printf("(%d)", ((s - r->program) + (nxt - s)));
        s += 3;
        if (op == ANYOF || op == ANYBUT || op == EXACTLY) {
            /* Literal string, where present. */
            while (*s != '\0') {
                putchar(*s);
                s++;
            }
            s++;
        }
        putchar('\n');
    }

    /* Header fields of interest. */
    if (r->regstart != '\0')
        printf("start `%c' ", r->regstart);
    if (r->reganch)
        printf("anchored ");
    if (r->regmust != (char *) NULL)
        printf("must have \"%s\"", r->regmust);
    printf("\n");
}

/*
 - regprop - printable representation of opcode
 */
static char *regprop (char * op)
{
    register char *p;
    static char buf[50];

    strcpy(buf, ":");

    switch (OP(op)) {
    case BOL:
        p = "BOL";
        break;
    case EOL:
        p = "EOL";
        break;
    case ANY:
        p = "ANY";
        break;
    case ANYOF:
        p = "ANYOF";
        break;
    case ANYBUT:
        p = "ANYBUT";
        break;
    case BRANCH:
        p = "BRANCH";
        break;
    case EXACTLY:
        p = "EXACTLY";
        break;
    case NOTHING:
        p = "NOTHING";
        break;
    case BACK:
        p = "BACK";
        break;
    case END:
        p = "END";
        break;
    case WORDSTART:
        p = "WORDSTART";
        break;
    case WORDEND:
        p = "WORDEND";
        break;
    case OPEN + 1:
    case OPEN + 2:
    case OPEN + 3:
    case OPEN + 4:
    case OPEN + 5:
    case OPEN + 6:
    case OPEN + 7:
    case OPEN + 8:
    case OPEN + 9:
        sprintf(buf + strlen(buf), "OPEN%d", OP(op) - OPEN);
        p = (char *) NULL;
        break;
    case CLOSE + 1:
    case CLOSE + 2:
    case CLOSE + 3:
    case CLOSE + 4:
    case CLOSE + 5:
    case CLOSE + 6:
    case CLOSE + 7:
    case CLOSE + 8:
    case CLOSE + 9:
        sprintf(buf + strlen(buf), "CLOSE%d", OP(op) - CLOSE);
        p = (char *) NULL;
        break;
    case STAR:
        p = "STAR";
        break;
    case PLUS:
        p = "PLUS";
        break;
    default:
        p = (char *) NULL;
        regerror("corrupted opcode\n");
        break;
    }
    if (p != (char *) NULL)
        strcat(buf, p);
    return (buf);
}
#endif

/*
 * The following is provided for those people who do not have strcspn() in
 * their C libraries.  They should get off their butts and do something
 * about it; at least one public-domain implementation of those (highly
 * useful) string routines has been published on Usenet.
 */
#ifdef STRCSPN
/*
 * strcspn - find length of initial segment of s1 consisting entirely
 * of characters not from s2
 */
static int strcspn (char * s1, char * s2)
{
    register char *scan1;
    register char *scan2;
    register int count;

    count = 0;
    for (scan1 = s1; *scan1 != '\0'; scan1++) {
        for (scan2 = s2; *scan2 != '\0';)       /* ++ moved down. */
            if (*scan1 == *scan2++)
                return (count);
        count++;
    }
    return (count);
}
#endif


/*
 - regsub - perform substitutions after a regexp match
 */
char *regsub (regexp * prog, char * source, char * dest, int n)
{
    register char *src;
    register char *dst;
    register char c;
    register int no;
    register int len;

    if (prog == (regexp *) NULL ||
        source == (char *) NULL || dest == (char *) NULL) {
        regerror("NULL parm to regsub\n");
        return NULL;
    }
    if (UCHARAT(prog->program) != MAGIC) {
        regerror("damaged regexp fed to regsub\n");
        return NULL;
    }
    src = source;
    dst = dest;
    while ((c = *src++) != '\0') {
        if (c == '&')
            no = 0;
        else if (c == '\\' && '0' <= *src && *src <= '9')
            no = *src++ - '0';
        else
            no = -1;

        if (no < 0) {           /* Ordinary character. */
            if (c == '\\' && (*src == '\\' || *src == '&'))
                c = *src++;
            if (--n < 0) {      /* amylaar */
                regerror("line too long\n");
                return NULL;
            }
            *dst++ = c;
        } else if (prog->startp[no] != (char *) NULL &&
                   prog->endp[no] != (char *) NULL) {
            len = prog->endp[no] - prog->startp[no];
            if ((n -= len) < 0) {       /* amylaar */
                regerror("line too long\n");
                return NULL;
            }
            strncpy(dst, prog->startp[no], len);
            dst += len;
            if (len != 0 && *(dst - 1) == '\0') {       /* strncpy hit NUL. */
                regerror("damaged match string\n");
                return NULL;
            }
        }
    }
    if (--n < 0) {              /* amylaar */
        regerror("line too long\n");
        return NULL;
    }
    *dst = '\0';
    return dst;
}
