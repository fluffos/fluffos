/* 39% */
/*
 *  ed - standard editor
 *  ~~
 *	Authors: Brian Beattie, Kees Bot, and others
 *
 * Copyright 1987 Brian Beattie Rights Reserved.
 * Permission to copy or distribute granted under the following conditions:
 * 1). No charge may be made other than reasonable charges for reproduction.
 * 2). This notice must remain intact.
 * 3). No further restrictions may be added.
 * 4). Except meaningless ones.
 *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 *  TurboC mods and cleanup 8/17/88 RAMontante.
 *  Further information (posting headers, etc.) at end of file.
 *  RE stuff replaced with Spencerian version, sundry other bugfix+speedups
 *  Ian Phillipps. Version incremented to "5".
 * _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
 *  Changed the program to use more of the #define's that are at the top of
 *  this file.  Modified prntln() to print out a string instead of each
 *  individual character to make it easier to see if snooping a wizard who
 *  is in the editor.  Got rid of putcntl() because of the change to strings
 *  instead of characters, and made a define with the name of putcntl()
 *  Incremented version to "6".
 *  Scott Grosch / Belgarath    08/10/91
 * _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _
 *
 *  ********--->> INDENTATION ONLINE !!!! <<----------****************
 *  Indentation added by Ted Gaunt (aka Qixx) paradox@mcs.anl.gov
 *  help files added by Ted Gaunt
 *  '^' added by Ted Gaunt
 *  Note: this version compatible with v.3.0.34  (and probably all others too)
 *  but i've only tested it on v.3 please mail me if it works on your mud
 *  and if you like it!
 */

/* A quick fix that hopefully does the job! -- Buddha */
#define PROMPT ":"

int	version = 6;	/* used only in the "set" function, for i.d. */

#include <stdio.h>
#include <string.h>
#ifdef __STDC__
#include <memory.h>
#endif
#include <sys/types.h>  /* need for netinet */
#include <ctype.h>
/* Regexp is Henry Spencer's package. WARNING: regsub is modified to return
 * a pointer to the \0 after the destination string, and this program refers
 * to the "private" reganch field in the struct regexp.
 */
#include "config.h"
#include "lint.h"
#include "regexp.h"
#include "interpret.h"
#include "object.h"
#include "exec.h"
#include "comm.h"

#ifdef ED /* define ED in options.h if you want the ed() efun */

/* define this if you don't like the ending dollar signs in ed, in n-mode */
#define NO_END_DOLLAR_SIGN
/*
 *	#defines for non-printing ASCII characters
 */
#define NUL	0x00	/* ^@ */
#define EOS	0x00	/* end of string */
#define SOH	0x01	/* ^A */
#define STX	0x02	/* ^B */
#define ETX	0x03	/* ^C */
#define EOT	0x04	/* ^D */
#define ENQ	0x05	/* ^E */
#define ACK	0x06	/* ^F */
#define BEL	0x07	/* ^G */
#define BS	0x08	/* ^H */
#define HT	0x09	/* ^I */
#define LF	0x0a	/* ^J */
#define NL	'\n'
#define VT	0x0b	/* ^K */
#define FF	0x0c	/* ^L */
#define CR	0x0d	/* ^M */
#define SO	0x0e	/* ^N */
#define SI	0x0f	/* ^O */
#define DLE	0x10	/* ^P */
#define DC1	0x11	/* ^Q */
#define DC2	0x12	/* ^R */
#define DC3	0x13	/* ^S */
#define DC4	0x14	/* ^T */
#define NAK	0x15	/* ^U */
#define SYN	0x16	/* ^V */
#define ETB	0x17	/* ^W */
#define CAN	0x18	/* ^X */
#define EM	0x19	/* ^Y */
#define SUB	0x1a	/* ^Z */
#define ESC	0x1b	/* ^[ */
#define FS	0x1c	/* ^\ */
#define GS	0x1d	/* ^] */
/*#define RS	0x1e	   ^^ */
#define US	0x1f	/* ^_ */
#define SP	0x20	/* space */
#define DEL	0x7f	/* DEL*/
#define ESCAPE  '\\'

#define TAB '\t'		/* added by Qixx for indentation */
#define LB '{'
#define RB '}'
#define LC '('
#define RC ')'
#define LS '['
#define RS ']'
#define PP '\"'
#define EOL '\0'


#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif
#define ERR		-2
#define FATAL		(ERR-1)
#define CHANGED		(ERR-2)
#define SET_FAIL	(ERR-3)
#define SUB_FAIL	(ERR-4)
#define MEM_FAIL	(ERR-5)
#define UNRECOG_COMMAND (ERR-6)


#define	BUFFER_SIZE	2048	/* stream-buffer size:  == 1 hd cluster */

#define LINFREE	1	/* entry not in use */
#define LGLOB	2       /* line marked global */

#define MAXLINE	512	/* max number of chars per line */
#define MAXPAT	256	/* max number of chars per replacement pattern */
#define MAXFNAME 256	/* max file name size */


/**  Global variables  **/

extern struct program *current_prog;
extern struct object *master_ob;

int EdErr = 0;

struct	line {
  int		l_stat;		/* empty, mark */
  struct line	*l_prev;
  struct line	*l_next;
  char		l_buff[1];
};
typedef struct line	LINE;

extern struct object *command_giver;
void set_prompt PROT((char *));

#ifndef toupper
extern int toupper PROT((int));
#endif

int doprnt PROT((int, int));
int ins PROT((char *));
int deflt PROT((int, int));
static int strip_buff PROT((int line,char *buff2));
static void print_help PROT((int arg));
static void print_help2 PROT((void));
static void count_blanks PROT((int line));
static void _count_blanks PROT((char *str, int blanks));

#define P_INTERACTIVE (command_giver->interactive)
#define P_NET_DEAD (P_INTERACTIVE->net_dead)
#define P_DIAG		(P_INTERACTIVE->ed_buffer->diag)
#define P_TRUNCFLG	(P_INTERACTIVE->ed_buffer->truncflg)
#define P_NONASCII	(P_INTERACTIVE->ed_buffer->nonascii)
#define P_NULLCHAR	(P_INTERACTIVE->ed_buffer->nullchar)
#define P_TRUNCATED	(P_INTERACTIVE->ed_buffer->truncated)
#define P_FNAME		(P_INTERACTIVE->ed_buffer->fname)
#define P_FCHANGED	(P_INTERACTIVE->ed_buffer->fchanged)
#define P_NOFNAME	(P_INTERACTIVE->ed_buffer->nofname)
#define P_MARK		(P_INTERACTIVE->ed_buffer->mark)
#define P_OLDPAT	(P_INTERACTIVE->ed_buffer->oldpat)
#define P_LINE0		(P_INTERACTIVE->ed_buffer->Line0)
#define P_LINE0		(P_INTERACTIVE->ed_buffer->Line0)
#define P_CURLN		(P_INTERACTIVE->ed_buffer->CurLn)
#define P_CURPTR	(P_INTERACTIVE->ed_buffer->CurPtr)
#define P_LASTLN	(P_INTERACTIVE->ed_buffer->LastLn)
#define P_LINE1		(P_INTERACTIVE->ed_buffer->Line1)
#define P_LINE2		(P_INTERACTIVE->ed_buffer->Line2)
#define P_NLINES	(P_INTERACTIVE->ed_buffer->nlines)
#define P_SHIFTWIDTH	(P_INTERACTIVE->ed_buffer->shiftwidth)
/* shiftwidth is meant to be a 4-bit-value that can be packed into an int
   along with flags, therefore masks 0x1 ... 0x8 are reserved.           */
#define P_FLAGS 	(P_INTERACTIVE->ed_buffer->flags)
#define NFLG_MASK	0x0010
#define P_NFLG		( P_FLAGS & NFLG_MASK )
#define LFLG_MASK	0x0020
#define P_LFLG		( P_FLAGS & LFLG_MASK )
#define PFLG_MASK	0x0040
#define P_PFLG		( P_FLAGS & PFLG_MASK )
#define EIGHTBIT_MASK	0x0080
#define P_EIGHTBIT	( P_FLAGS & EIGHTBIT_MASK )
#define AUTOINDFLG_MASK	0x0100
#define P_AUTOINDFLG	( P_FLAGS & AUTOINDFLG_MASK )
#define EXCOMPAT_MASK	0x0200
#define P_EXCOMPAT	( P_FLAGS & EXCOMPAT_MASK )
#define DPRINT_MASK     0x0400
#define P_DPRINT        ( P_FLAGS & DPRINT_MASK )
#define SHIFTWIDTH_MASK	0x000f
#define ALL_FLAGS_MASK	0x03f0
#define ED_BUFFER       (command_giver->interactive->ed_buffer)
#define P_APPENDING	(ED_BUFFER->appending)
#define P_MORE		(ED_BUFFER->moring)
#define P_LEADBLANKS	(ED_BUFFER->leading_blanks)
#define P_CUR_AUTOIND   (ED_BUFFER->cur_autoindent)
#define P_RESTRICT	(command_giver->interactive->ed_buffer->restricted)


char	inlin[MAXLINE];
char	*inptr;		/* tty input buffer */
struct ed_buffer {
  int	diag;		/* diagnostic-output? flag */
  int	truncflg;	/* truncate long line flag */
  int	nonascii;	/* count of non-ascii chars read */
  int	nullchar;	/* count of null chars read */
  int	truncated;	/* count of lines truncated */
  char	fname[MAXFNAME];
  int	fchanged;	/* file-changed? flag */
  int	nofname;
  int	mark['z'-'a'+1];
  regexp	*oldpat;
	
  LINE	Line0;
  int	CurLn;
  LINE	*CurPtr;	/* CurLn and CurPtr must be kept in step */
  int	LastLn;
  int	Line1, Line2, nlines;
  int	flags;
  int	appending;
  int     moring;         /* used for the wait line of help */
  char    *exit_fn;	/* Function to be called when user exits */
  struct object *exit_ob; /* in this object */
  int	shiftwidth;
  int	leading_blanks;
  int	cur_autoindent;
  int     restricted;    /* restricted access ed */
};

struct tbl {
  char	*t_str;
  int	t_and_mask;
  int	t_or_mask;
} *t, tbl[] = {
  {"number",	~FALSE,		NFLG_MASK},
  {"nonumber",	~NFLG_MASK,	FALSE},
  {"list",		~FALSE,		LFLG_MASK},
  {"nolist",	~LFLG_MASK,	FALSE},
  {"print",	~FALSE, 	PFLG_MASK},
  {"noprint",	~PFLG_MASK,	FALSE},
  {"eightbit",	~FALSE,		EIGHTBIT_MASK},
  {"noeightbit",	~EIGHTBIT_MASK,	FALSE},
  {"autoindent",	~FALSE,		AUTOINDFLG_MASK},
  {"noautoindent",	~AUTOINDFLG_MASK, FALSE},
  {"excompatible", ~FALSE,		EXCOMPAT_MASK},
  {"noexcompatible",~EXCOMPAT_MASK,FALSE},
  {"dprint",	~FALSE,		DPRINT_MASK},
  {"nodprint",	~DPRINT_MASK,	FALSE},
};


/*-------------------------------------------------------------------------*/

#ifndef _AIX
extern	char	*strcpy(), *strncpy();
#endif
extern	char	*xalloc();
extern	LINE	*getptr();
extern	char	*gettxt();
extern	char	*gettxtl();
extern	char	*catsub();
extern	void	prntln(), putcntl(), error();
regexp	*optpat();


/*________  Macros  ________________________________________________________*/

#ifndef max
#  define max(a,b)	((a) > (b) ? (a) : (b))
#endif

#ifndef min
#  define min(a,b)	((a) < (b) ? (a) : (b))
#endif

#define nextln(l)	((l)+1 > P_LASTLN ? 0 : (l)+1)
#define prevln(l)	((l)-1 < 0 ? P_LASTLN : (l)-1)

#define gettxtl(lin)	((lin)->l_buff)
#define gettxt(num)	(gettxtl( getptr(num) ))

#define getnextptr(p)	((p)->l_next)
#define getprevptr(p)	((p)->l_prev)

#define setCurLn( lin )	( P_CURPTR = getptr( P_CURLN = (lin) ) )
#define nextCurLn()	( P_CURLN = nextln(P_CURLN), P_CURPTR = getnextptr( P_CURPTR ) )
#define prevCurLn()	( P_CURLN = prevln(P_CURLN), P_CURPTR = getprevptr( P_CURPTR ) )

#define clrbuf()	del(1, P_LASTLN)

#define	Skip_White_Space	{while (*inptr==SP || *inptr==HT) inptr++;}

#define relink(a, x, y, b) { (x)->l_prev = (a); (y)->l_next = (b); }


/*________  functions  ______________________________________________________*/


/*	append.c	*/


int append(line, glob)
int	line, glob;
{
  if(glob)
    return(ERR);
  setCurLn( line );
  P_APPENDING = 1;
  if(P_NFLG)
    add_message("%6d. ",P_CURLN+1);
  if (P_CUR_AUTOIND)
    add_message("%*s",P_LEADBLANKS,"");
  set_prompt("*\b");
  return 0;
}

int more_append(str)
     char *str;
{
  if(str[0] == '.' && str[1] == '\0') {
    P_APPENDING = 0;
    set_prompt(":");
    return(0);
  }
  if(P_NFLG)
    add_message("%6d. ",P_CURLN+2);
  if ( P_CUR_AUTOIND )
    {
      int i;
      int less_indent_flag = 0;
      
      while ( *str=='\004' || *str == '\013' )
	{
	  str++;
	  P_LEADBLANKS-=P_SHIFTWIDTH;
	  if ( P_LEADBLANKS < 0 ) P_LEADBLANKS=0;
	  less_indent_flag=1;
	}
      for ( i=0; i < P_LEADBLANKS; ) inlin[i++]=' ';
      strncpy(inlin+P_LEADBLANKS,str,MAXLINE-P_LEADBLANKS);
      inlin[MAXLINE-1]='\0';
      _count_blanks(inlin,0);
      add_message("%*s",P_LEADBLANKS,"");
      if ( !*str && less_indent_flag ) return 0;
      str=inlin;
    }
  if( ins(str) < 0)
    return( MEM_FAIL );
  return 0;
}

static void count_blanks(line)
     int line;
{
  _count_blanks(gettxtl(getptr(line)), 0);
}

static void _count_blanks(str,blanks)
     char *str;
     int blanks;
{
  for ( ; *str; str++ )
    {
      if ( *str == ' ' ) blanks++;
      else if ( *str == '\t' ) blanks += 8 - blanks % 8 ;
      else break;
    }
  P_LEADBLANKS = blanks<MAXLINE ? blanks : MAXLINE ;
}

/*	ckglob.c	*/

int ckglob()
{
  regexp	*glbpat;
  char	c, delim, *lin;
  int	num;
  LINE	*ptr;

  c = *inptr;
  
  if(c != 'g' && c != 'v')
    return(0);
  if (deflt(1, P_LASTLN) < 0)
    return(ERR);

  delim = *++inptr;
  if(delim <= ' ')
    return(ERR);

  glbpat = optpat();
  if(*inptr == delim)
    inptr++;
  ptr = getptr(1);
  for (num=1; num<=P_LASTLN; num++) {
    ptr->l_stat &= ~LGLOB;
    if (P_LINE1 <= num && num <= P_LINE2) {
      /* we might have got a NULL pointer if the
	 supplied pattern was invalid		   */
      if (glbpat) {
	lin = gettxtl(ptr);
	if(regexec(glbpat, lin )) {
	  if (c=='g') ptr->l_stat |= LGLOB;
	} else {
	  if (c=='v') ptr->l_stat |= LGLOB;
	}
      }
      ptr = getnextptr(ptr);
    }
  }
  return(1);
}


/*  deflt.c
 *	Set P_LINE1 & P_LINE2 (the command-range delimiters) if the file is
 *	empty; Test whether they have valid values.
 */

int deflt(def1, def2)
     int def1, def2;
{
  if(P_NLINES == 0) {
    P_LINE1 = def1;
    P_LINE2 = def2;
  }
  return ( (P_LINE1>P_LINE2 || P_LINE1<=0) ? ERR : 0 );
}


/*	del.c	*/

/* One of the calls to this function tests its return value for an error
 * condition.  But del doesn't return any error value, and it isn't obvious
 * to me what errors might be detectable/reportable.  To silence a warning
 * message, I've added a constant return statement. -- RAM
 * ... It could check to<=P_LASTLN ... igp
 */

int del(from, to)
     int	from, to;
{
  LINE	*first, *last, *next, *tmp;
  
  if(from < 1)
    from = 1;
  first = getprevptr( getptr( from ) );
  last = getnextptr( getptr( to ) );
  next = first->l_next;
  while(next != last && next != &P_LINE0) {
    tmp = next->l_next;
    FREE((char *)next);
    next = tmp;
  }
  relink(first, last, first, last);
  P_LASTLN -= (to - from)+1;
  setCurLn( prevln(from) );
  return(0);
}


int dolst(line1, line2)
     int line1, line2;
{
  int oldflags=P_FLAGS, p;
  
  P_FLAGS |= LFLG_MASK;
  p = doprnt(line1, line2);
  P_FLAGS = oldflags;
  return p;
}


/*	esc.c
 * Map escape sequences into their equivalent symbols.  Returns the
 * correct ASCII character.  If no escape prefix is present then s
 * is untouched and *s is returned, otherwise **s is advanced to point
 * at the escaped character and the translated character is returned.
 */
int esc(s)
     char	**s;
{
  register int	rval;
  
  if (**s != ESCAPE) {
    rval = **s;
  } else {
    (*s)++;
    switch(islower(**s) ? toupper(**s) : **s) {
    case '\000':
      rval = ESCAPE;	break;
    case 'S':
      rval = ' ';	break;
    case 'N':
      rval = '\n';	break;
    case 'T':
      rval = '\t';	break;
    case 'B':
      rval = '\b';	break;
    case 'R':
      rval = '\r';	break;
    default:
      rval = **s;	break;
    }
  }
  return (rval);
}


/*	doprnt.c	*/

int doprnt(from, to)
     int	from, to;
{
  from = (from < 1) ? 1 : from;
  to = (to > P_LASTLN) ? P_LASTLN : to;
  
  if(to != 0) {
    setCurLn( from );
    while( P_CURLN <= to ) {
      prntln( gettxtl( P_CURPTR ), P_LFLG, (P_NFLG ? P_CURLN : 0));
      if(P_NET_DEAD || (P_CURLN == to))
	break;
      nextCurLn();
    }
  }
  return(0);
}

static void
free_ed_buffer()
{
  clrbuf();
  if (ED_BUFFER->exit_fn) {
     /* make this "safe" */
    safe_apply(ED_BUFFER->exit_fn,
      ED_BUFFER->exit_ob, 0);
    FREE(ED_BUFFER->exit_fn);
    free_object(ED_BUFFER->exit_ob, "ed EOF");
    FREE((char *)ED_BUFFER);
    ED_BUFFER= 0;
    set_prompt("> ");
    return;
  }
  FREE((char *)ED_BUFFER);
  ED_BUFFER = 0;
  add_message("Exit from ed.\n");
  set_prompt("> ");
  return;
}

#define putcntl(X) *line++ = '^'; *line++ = (X) ? ((*str&31)|'@') : '?'

void prntln(str, vflg, len) 
     char	*str;
     int 	vflg, len;
{
  char *line, start[MAXLINE+2]; 

  line = start;
  if (len) add_message("%6d  ", len); /* made 8 chars wide */
  while (*str && *str != NL) {
    if ((line - start) > MAXLINE) {
      free_ed_buffer();
      error("Bad file format. Ed can't handle lines longer than %d.\n",MAXLINE);
    }
    if (*str < ' ' || *str >= DEL) {
      switch (*str) {
      case '\t':
  /* didn't see a reason to make tabs a special case like this -- Raistlin */
	*line++ = *str;
	break;
      case DEL:
	putcntl(0);
	break;
      default:
	putcntl(1);
	break;
      }
    } else *line++ = *str;
    str++;
  }
#ifndef NO_END_DOLLAR_SIGN
  if (vflg) *line++ = '$';
#endif
  *line = EOS;
  add_message("%s\n", start);
}

/*	egets.c	*/

int egets(str,size,stream)
     char	*str;
     int	size;
     FILE	*stream;
{
  int	c, count;
  char	*cp;

  for(count = 0, cp = str; size > count;) {
    c = getc(stream);
    if(c == EOF) {
      *cp = EOS;
      if(count)
	add_message("[Incomplete last line]\n");
      return(count);
    }
    else if(c == NL) {
      *cp = EOS;
      return(++count);
    }
    else if (c == 0)
      P_NULLCHAR++;	/* count nulls */
    else {
      if(c > 127) {
	if(!P_EIGHTBIT)		/* if not saving eighth bit */
	  c = c&127;	/* strip eigth bit */
	P_NONASCII++;		/* count it */
      }
      *cp++ = c;	/* not null, keep it */
      count++;
    }
  }
  str[count-1] = EOS;
  if(c != NL) {
    add_message("truncating line\n");
    P_TRUNCATED++;
    while((c = getc(stream)) != EOF)
      if(c == NL)
	break;
  }
  return(count);
}  /* egets */


int doread(lin, fname)
     int	lin;
     char	*fname;
{
  FILE	*fp;
  int	err;
  unsigned int	bytes;
  unsigned int	lines;
  static char	str[MAXLINE];

  err = 0;
  P_NONASCII = P_NULLCHAR = P_TRUNCATED = 0;

  if (P_DIAG) add_message("\"%s\" ",fname);
  if( (fp = fopen(fname, "r")) == NULL ) {
    add_message(" isn't readable.\n");
    return( ERR );
  }
  setCurLn( lin );
  for(lines = 0, bytes = 0;(err = egets(str,MAXLINE,fp)) > 0;) {
    bytes += err;
    if(ins(str) < 0) {
      err = MEM_FAIL;
      break;
    }
    lines++;
  }
  fclose(fp);
  if(err < 0)
    return(err);
  if (P_DIAG) {
    add_message("%u lines %u bytes",lines,bytes);
    if(P_NONASCII)
      add_message(" [%d non-ascii]",P_NONASCII);
    if(P_NULLCHAR)
      add_message(" [%d nul]",P_NULLCHAR);
    if(P_TRUNCATED)
      add_message(" [%d lines truncated]",P_TRUNCATED);
    add_message("\n");
  }
  return( err );
}  /* doread */


int dowrite(from, to, fname, apflg)
     int	from, to;
     char	*fname;
     int	apflg;
{
  FILE	*fp;
  int	lin, err;
  unsigned int	lines;
  unsigned int	bytes;
  char	*str;
  LINE	*lptr;

  err = 0;
  lines = bytes = 0;
  
  if(!P_RESTRICT)
    add_message("\"%s\" ",fname);
  if((fp = fopen(fname,(apflg?"a":"w"))) == NULL) {
    if(!P_RESTRICT)
      add_message(" can't be opened for writing!\n");
    else
      add_message("Couldn't open file for writing!\n");
    return( ERR );
  }

  lptr = getptr(from);
  for(lin = from; lin <= to; lin++) {
    str = lptr->l_buff;
    lines++;
    bytes += strlen(str) + 1;	/* str + '\n' */
    if(fputs(str, fp) == EOF) {
      add_message("file write error\n");
      err++;
      break;
    }
    fputc('\n', fp);
    lptr = lptr->l_next;
  }
  if(!P_RESTRICT)
    add_message("%u lines %lu bytes\n",lines,bytes);
  fclose(fp);
  return( err );
}  /* dowrite */


/*	find.c	*/

int find(pat, dir)
     regexp	*pat;
     int	dir;
{
  int	i, num;
  LINE	*lin;

  if (!pat) return (ERR);
  dir ? nextCurLn() : prevCurLn() ;
  num = P_CURLN;
  lin = P_CURPTR;
  for(i=0; i<P_LASTLN; i++ ) {
    if(regexec( pat, gettxtl( lin ) ))
      return(num);
    if (EdErr) { EdErr = 0; break; }
    if( dir )
      num = nextln(num), lin = getnextptr(lin);
    else
      num = prevln(num), lin = getprevptr(lin);
  }
/* Put us back to where we came from */
  dir ? prevCurLn() : nextCurLn();
  return ( ERR );
} /* find() */

/*	getfn.c	*/

char *getfn(writeflg)
     int writeflg;
{
  static char	file[MAXFNAME];
  char	*cp;
  char *file2;
  struct svalue *ret;
    
  if(*inptr == NL) {
    P_NOFNAME=TRUE;
    file[0] = '/';
    strcpy(file+1, P_FNAME);
  } else {
    P_NOFNAME=FALSE;
    Skip_White_Space;
	
    cp = file;
    while(*inptr && *inptr != NL && *inptr != SP && *inptr != HT)
      *cp++ = *inptr++;
    *cp = '\0';
	
  }
  if(strlen(file) == 0) {
    add_message("Bad file name.\n");
    return( NULL );
  }
  if (file[0] != '/') {
    push_string(file, STRING_MALLOC);
    ret = apply_master_ob("make_path_absolute", 1);
    if (ret == 0 || ret->type != T_STRING)
      return NULL;
    strncpy(file, ret->u.string, sizeof file - 1);
  }
  file2 = check_valid_path(file, command_giver,
			   "ed_start", writeflg);
  if (!file2)
    return( NULL );
  strncpy(file, file2, MAXFNAME-1);
  file[MAXFNAME-1] = 0;

  if(strlen(file) == 0) {
    add_message("no file name\n");
    return(NULL);
  }
  return( file );
}  /* getfn */


int getnum(first)
     int first;
{
  regexp	*srchpat;
  int	num;
  char	c;

  Skip_White_Space;

  if(*inptr >= '0' && *inptr <= '9') {	/* line number */
    for(num = 0; *inptr >= '0' && *inptr <= '9'; ++inptr) {
      num = (num * 10) + (*inptr - '0');
    }
    return num;
  }

  switch(c = *inptr) {
  case '.':
    inptr++;
    return (P_CURLN);

  case '$':
    inptr++;
    return (P_LASTLN);

  case '/':
  case '?':
    srchpat = optpat();
    if(*inptr == c)
      inptr++;
    return(find(srchpat,c == '/'?1:0));
	
  case '-':
  case '+':
    return(first ? P_CURLN : 1);

  case '\'':
    inptr++;
    if (*inptr < 'a' || *inptr > 'z')
      return(EOF);
    return P_MARK[ *inptr++ - 'a' ];

  default:
    return ( first ? EOF : 1 );	/* unknown address */
  }
}  /* getnum */


/*  getone.c
 *	Parse a number (or arithmetic expression) off the command line.
 */
#define FIRST 1
#define NOTFIRST 0

int getone()
{
  int	c, i, num;

  if((num = getnum(FIRST)) >= 0) {
    for (;;) {
      Skip_White_Space;
      if(*inptr != '+' && *inptr != '-')
	break;	/* exit infinite loop */
      
      c = *inptr++;
      if((i = getnum(NOTFIRST)) < 0)
	return ( i );
      if(c == '+')
	num += i;
      else
	num -= i;
    }
  }
  return ( num>P_LASTLN ? ERR : num );
}  /* getone */


int getlst()
{
  int	num;
  
  P_LINE2 = 0;
  for(P_NLINES = 0; (num = getone()) >= 0;)
    {
      P_LINE1 = P_LINE2;
      P_LINE2 = num;
      P_NLINES++;
      if(*inptr != ',' && *inptr != ';')
	break;
      if(*inptr == ';')
	setCurLn( num );
      inptr++;
    }
  P_NLINES = min(P_NLINES, 2);
  if(P_NLINES == 0)
    P_LINE2 = P_CURLN;
  if(P_NLINES <= 1)
    P_LINE1 = P_LINE2;
  
  return ( (num == ERR) ? num : P_NLINES );
}  /* getlst */


/*	getptr.c	*/

LINE *getptr(num)
     int	num;
{
  LINE	*ptr;
  int	j;
  
  if (2*num>P_LASTLN && num<=P_LASTLN) {	/* high line numbers */
    ptr = P_LINE0.l_prev;
    for (j = P_LASTLN; j>num; j--)
      ptr = ptr->l_prev;
  } else {				/* low line numbers */
    ptr = &P_LINE0;
    for(j = 0; j < num; j++)
      ptr = ptr->l_next;
  }
  return(ptr);
}


/*	getrhs.c	*/

int getrhs(sub)
     char	*sub;
{
  char delim = *inptr++;
  char *outmax = sub + MAXPAT;
  if( delim == NL || *inptr == NL)	/* check for eol */
    return( ERR );
  while( *inptr != delim && *inptr != NL ) {
    if ( sub > outmax )
      return ERR;
    if ( *inptr == ESCAPE ) {
      switch ( *++inptr ) {
      case 'r':
	*sub++ = '\r';
	inptr++;
	break;
      case 'n':
	*sub++ = '\n';
	inptr++;
	break;
      case 'b':
	*sub++ = '\b';
	inptr++;
	break;
      case 't':
	*sub++ = '\t';
	inptr++;
	break;
      case '0': {
	int i=3;
	*sub = 0;
	do {
	  if (*++inptr<'0' || *inptr >'7')
	    break;
	  *sub = (*sub<<3) | (*inptr-'0');
	} while (--i!=0);
	sub++;
      } break;
      case '&':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      case '\\':
	*sub++ = ESCAPE; /* fall through */
      default:
	*sub++ = *inptr;
	if ( *inptr != NL )
	  inptr++;
      }
    }
    else *sub++ = *inptr++;
  }
  *sub = '\0';
  
  inptr++;		/* skip over delimter */
  Skip_White_Space;
  if(*inptr == 'g') {
    inptr++;
    return( 1 );
  }
  return( 0 );
}

/*	ins.c	*/

int ins(str)
     char	*str;
{
  char	*cp;
  LINE	*new, *nxt;
  int	len;
  
  do {
    for ( cp = str; *cp && *cp != NL; cp++ )
      ;
    len = cp - str;
    /* cp now points to end of first or only line */
    
    if((new = (LINE *)DXALLOC(sizeof(LINE)+len, 27, "ins: new")) == NULL)
      return( MEM_FAIL ); 	/* no memory */
    
    new->l_stat=0;
    strncpy(new->l_buff,str,len);	/* build new line */
    new->l_buff[len] = EOS;
    nxt = getnextptr(P_CURPTR);	/* get next line */
    relink(P_CURPTR, new, new, nxt);	/* add to linked list */
    relink(new, nxt, P_CURPTR, new);
    P_LASTLN++;
    P_CURLN++;
    P_CURPTR = new;
    str = cp + 1;
  }
  while( *cp != EOS );
  return 1;
}


/*	join.c	*/

int join(first, last)
     int first, last;
{
  char buf[MAXLINE];
  char *cp=buf, *str;
  LINE *lin;
  int num;
  
  if (first<=0 || first>last || last>P_LASTLN)
    return(ERR);
  if (first==last) {
    setCurLn( first );
    return 0;
  }
  lin = getptr(first);
  for (num=first; num<=last; num++) {
    str=gettxtl(lin);
    while ( *str ) {
      if (cp >= buf + MAXLINE-1 ) {
	add_message("line too long\n");
	return(ERR);
      }
      *cp++ = *str++;
    }
    lin = getnextptr(lin);
  }
  *cp = EOS;
  del(first, last);
  if( ins(buf) < 0 )
    return MEM_FAIL;
  P_FCHANGED = TRUE;
  return 0;
}


/*  move.c
 *	Unlink the block of lines from P_LINE1 to P_LINE2, and relink them
 *	after line "num".
 */

int move(num)
     int	num;
{
  int	range;
  LINE	*before, *first, *last, *after;
  
  if( P_LINE1 <= num && num <= P_LINE2 )
    return( ERR );
  range = P_LINE2 - P_LINE1 + 1;
  before = getptr(prevln(P_LINE1));
  first = getptr(P_LINE1);
  last = getptr(P_LINE2);
  after = getptr(nextln(P_LINE2));
  
  relink(before, after, before, after);
  P_LASTLN -= range;	/* per AST's posted patch 2/2/88 */
  if (num > P_LINE1)
    num -= range;
  
  before = getptr(num);
  after = getptr(nextln(num));
  relink(before, first, last, after);
  relink(last, after, before, first);
  P_LASTLN += range;	/* per AST's posted patch 2/2/88 */
  setCurLn( num + range );
  return( 1 );
}


int transfer(num)
     int num;
{
  int mid, lin, ntrans;

  if (P_LINE1<=0 || P_LINE1>P_LINE2)
    return(ERR);
  
  mid= num<P_LINE2 ? num : P_LINE2;
  
  setCurLn( num );
  ntrans=0;
  
  for (lin=P_LINE1; lin<=mid; lin++) {
    if( ins(gettxt(lin)) < 0 )
      return MEM_FAIL;
    ntrans++;
  }
  lin+=ntrans;
  P_LINE2+=ntrans;
  
  for ( ; lin <= P_LINE2; lin += 2 ) {
    if( ins(gettxt(lin)) < 0 )
      return MEM_FAIL;
    P_LINE2++;
  }
  return(1);
}


/*	optpat.c	*/

regexp *optpat()
{
  char	delim, str[MAXPAT], *cp;

  delim = *inptr++;
  if (delim == NL)
    return P_OLDPAT;
  cp = str;
  while(*inptr != delim && *inptr != NL && *inptr != EOS && cp < str + MAXPAT - 1) {
    if(*inptr == ESCAPE && inptr[1] != NL)
      *cp++ = *inptr++;
    *cp++ = *inptr++;
  }

  *cp = EOS;
  if(*str == EOS)
    return(P_OLDPAT);
  if(P_OLDPAT)
    FREE((char *)P_OLDPAT);
  return P_OLDPAT = regcomp(str,P_EXCOMPAT);
}

int set()
{
  char	word[16];
  int	i;
  struct tbl *limit;

  if(*(++inptr) != 't') {
    if(*inptr != SP && *inptr != HT && *inptr != NL)
      return(ERR);
  } else
    inptr++;

  if ( (*inptr == NL))
    {
      add_message("ed version %d.%d\n", version/100, version%100);
      limit = tbl + (sizeof(tbl) / sizeof(struct tbl));
      for(t = tbl; t < limit; t+=2) {
	add_message(	"%s:%s ", t->t_str, 
		    P_FLAGS & t->t_or_mask ?"on":"off");
      }
      add_message("\nshiftwidth:%d\n",P_SHIFTWIDTH);
      return(0);
    }

  Skip_White_Space;
  for(i = 0; *inptr != SP && *inptr != HT && *inptr != NL;) {
    if (i == sizeof word - 1) {
      add_message("Too long argument to 'set'!\n");
      return 0;
    }
    word[i++] = *inptr++;
  }
  word[i] = EOS;
  limit = tbl + (sizeof(tbl) / sizeof(struct tbl));
  for(t = tbl; t < limit; t++) {
    if(strcmp(word,t->t_str) == 0) {
      P_FLAGS = (P_FLAGS & t->t_and_mask) | t->t_or_mask;
      return(0);
    }

  }
  if ( !strcmp(word,"save") ) {
    struct svalue *ret;
    if(P_RESTRICT)
      return(SET_FAIL);
    push_object(command_giver);
    push_number( P_SHIFTWIDTH | P_FLAGS );
    ret = apply_master_ob("save_ed_setup",2);
    if ( ret && ret->type==T_NUMBER && ret->u.number > 0 )
      return 0;
  }
  if ( !strcmp(word,"shiftwidth") ) {
    Skip_White_Space;
    if ( isdigit(*inptr) ) {
      P_SHIFTWIDTH = *inptr-'0';
      return 0;
    }
  }
  return SET_FAIL;
}

#ifndef relink
void relink(a, x, y, b)
     LINE	*a, *x, *y, *b;
{
  x->l_prev = a;
  y->l_next = b;
}
#endif


void set_ed_buf()
{
  relink(&P_LINE0, &P_LINE0, &P_LINE0, &P_LINE0);
  P_CURLN = P_LASTLN = 0;
  P_CURPTR = &P_LINE0;
}


/*	subst.c	*/

int subst(pat, sub, gflg, pflag)
     regexp	*pat;
     char	*sub;
     int	gflg, pflag;
{
  int	nchngd = 0;
  char	*txtptr;
  char	*new, *old, buf[MAXLINE];
  int	space;			/* amylaar */
  int	still_running = 1;
  LINE	*lastline = getptr( P_LINE2 );

  if(P_LINE1 <= 0)
    return( SUB_FAIL );
  nchngd = 0;			/* reset count of lines changed */

  for( setCurLn( prevln( P_LINE1 ) ); still_running; ) {
    nextCurLn();
    new = buf;
    space = MAXLINE;		/* amylaar */
    if ( P_CURPTR == lastline )
      still_running = 0;
    if ( regexec( pat, txtptr = gettxtl( P_CURPTR ) ) ) {
      do
	{
	  /* Copy leading text */
	  int diff = pat->startp[0] - txtptr;
	  if ( (space-=diff) < 0 ) /* amylaar */
	    return SUB_FAIL;
	  strncpy( new, txtptr, diff );
	  new += diff;
	  /* Do substitution */
	  old = new;
	  new = regsub( pat, sub, new, space);
	  if (!new || (space-= new-old) < 0) /* amylaar */
	    return SUB_FAIL;
	  if (txtptr == pat->endp[0]) { /* amylaar :
					   prevent infinite loop */
	    if ( !*txtptr ) break;
	    if (--space < 0) return SUB_FAIL;
	    *new++ = *txtptr++;
	  } else
	    txtptr = pat->endp[0];
	}
      while( gflg && !pat->reganch && regexec( pat, txtptr ));
      
      /* Copy trailing chars */
      /* amylaar : always check for enough space left
       * BEFORE altering memory
       */
      if ( (space-= strlen(txtptr)+1 ) < 0 )
	return SUB_FAIL;
      strcpy(new, txtptr);
      del(P_CURLN,P_CURLN);
      if( ins(buf) < 0 )
	return MEM_FAIL;
      nchngd++;
      if(pflag)
	doprnt(P_CURLN, P_CURLN);
    }
  }
  return (( nchngd == 0 && !gflg ) ? SUB_FAIL : nchngd);
}

#define MAX_INDENT 40
int indent[MAX_INDENT];
char codes[MAX_INDENT];
int ind_occur[MAX_INDENT];
int str_on;
int indent_level;
int temp_indent;
int indent_error;

static int
indent_code()
{
  int from,to,current;
  char *inlip;
  /* static char	locti[MAXLINE]; */
  /* static char idented[MAXLINE]; */
  from=1;
  to= P_LASTLN;
    
  indent_level=0;
  str_on=0;
  temp_indent=0;
  indent_error=0;
  for (current=0;current<MAX_INDENT;current++) {
    indent[current]=0;
    codes[current]=SP;
    ind_occur[current]=0;}
  P_FCHANGED = TRUE;
  for (current=from;current<=to;current++) 
    {
      setCurLn(current);
      inlip=gettxtl( P_CURPTR );
      strip_buff(current,inlip);
      if (indent_error) return (ERR);
    }
  return 0;
}

static int
strip_buff(line,buff2)
     int line;
     char *buff2;
{
  int i;
  int i2;
  int flag, flagnotif;
  static char buff[MAXLINE];
  for (i=0;i<(indent_level+temp_indent)*3;i++) buff[i]=SP;
  i=(indent_level+temp_indent)*3;
  flag=flagnotif=0;
  temp_indent=0;
  for (i2=0;i2<MAXLINE;i2++) {
    switch (buff2[i2]) {
    case PP:
      if (buff2[i2-1]!='\\') str_on=!str_on;
      flag=1;
      buff[i++]=buff2[i2];
      break;
    case EOL:
      if (buff2[i2-1]!='\\') {
	if (str_on) {add_message("Detected a unterminated string on line %d\n",line);indent_error=1;}
	str_on=0;
		
      }
      buff[i++]=MAXLINE;
      i2=1000;
      break;
    case SP:
      if (flag) buff[i++]=buff2[i2];
      break;
    case TAB:
      if (str_on) buff[i++]=buff2[i2];
      break;
    case NL:
      flag=0;
      if (buff2[i2-1]!='\\') {
	if (str_on) {
	  add_message("Detected a unterminated string on line %d\n",line);
	  indent_error=1;}
	str_on=0;
      }
      buff[i++]=buff2[i2];
      break;
    case LB:
      flag=1;
      if (!str_on) {
	codes[++indent_level]=buff[i++]=buff2[i2];
	ind_occur[indent_level]=line;
	if (indent_level==1) indent[1]=3;
      } else buff[i++]=buff2[i2];
      flagnotif=1;
      temp_indent=0;
      break;
    case ';':
      flag=1;
      buff[i++]=buff2[i2];
      flagnotif=1;
      temp_indent=0;
      break;
    case LC:
    case LS:
      flag=1;
      if (!str_on) {
	codes[++indent_level]=buff[i++]=buff2[i2];
	ind_occur[indent_level]=line;
	indent[indent_level]=i;
      } else buff[i++]=buff2[i2];
      break;
    case RB:
      if (!str_on) {
	if (LB!=codes[indent_level]) {
	  add_message("Mismatched brackets, '%c' on line %d\n",
		      buff2[i2],line);
	  add_message("-which doesn't match '%c' on line %d\n",
		      codes[indent_level],ind_occur[indent_level]);
	  indent_error=1;}
	indent_level--;
	if (indent_level<0) {
	  indent_level=0;}
	if (!flag && i) i=indent[indent_level];
	buff[i++]=buff2[i2];
      } else buff[i++]=buff2[i2];
      flag=1;
      /*	    flagnotif=1; */
      break;
    case RC:
      if (!str_on) {
	if (LC!=codes[indent_level]) {
	  add_message("Mismatched brackets, '%c' on line %d\n",
		      buff2[i2],line);
	  add_message("-which doesn't match '%c' on line %d\n",
		      codes[indent_level],ind_occur[indent_level]);
	  indent_error=1;}
	indent_level--;
	if (indent_level<0) {
	  indent_level=0;}
	if (!flag && i) i=indent[indent_level];
	buff[i++]=buff2[i2];
      } else buff[i++]=buff2[i2];
      flag=1;
      /*	    flagnotif=1;*/
      break;
    case RS:
      if (!str_on) {
	if (LS!=codes[indent_level]) {
	  add_message("Mismatched brackets, '%c' on line %d\n",
		      buff2[i2],line);
	  add_message("-which doesn't match '%c' on line %d\n",
		      codes[indent_level],ind_occur[indent_level]);
	  indent_error=1;}
	indent_level--;
	if (indent_level<0) {
	  indent_level=0;}
	if (!flag && i) i=indent[indent_level];
	buff[i++]=buff2[i2];
      } else buff[i++]=buff2[i2];
      flag=1;
      /*	    flagnotif=1;*/
      break;
    case '#':
      if (!flag) i=0;
      flag=1;
      buff[i++]=buff2[i2];
      break;
    case 'i':
      if (!str_on && buff2[i2+1]=='f') {temp_indent=1; flagnotif=0;}
      if (flagnotif) {temp_indent=0; flagnotif=0;}
      flag=1;
      buff[i++]=buff2[i2];
      break;
    case 'e':
      if (!str_on && (i2+10)<MAXLINE)
	if (buff2[i2+1]=='l' && buff2[i2+2]=='s' && buff2[i2+3]=='e') 
	  {temp_indent=1; flagnotif=0; indent[indent_level+1]=i+3;}
      /*drop through*/
    default:
      if (flagnotif) {temp_indent=0; flagnotif=0;}
      flag=1;
      buff[i++]=buff2[i2];
      break;
    }
  }
  del(line,line);
  ins(buff);
  return 1;
}

/*  docmd.c
 *	Perform the command specified in the input buffer, as pointed to
 *	by inptr.  Actually, this finds the command letter first.
 */

int docmd(glob)
     int	glob;
{
  static char	rhs[MAXPAT];
  regexp	*subpat;
  int	c, err, line3;
  int	apflg, pflag, gflag;
  int	nchng;
  char	*fptr;

  pflag = FALSE;
  Skip_White_Space;

  c = *inptr++;
  switch(c) {
  case NL:
    if( P_NLINES == 0 && (P_LINE2 = nextln(P_CURLN)) == 0 )
      return(ERR);
    setCurLn( P_LINE2 );
    return (1);

  case '=':
    add_message("%d\n",P_LINE2);
    break;

  case 'o':
  case 'a':
  case 'A':
    P_CUR_AUTOIND = c=='a' ? P_AUTOINDFLG : !P_AUTOINDFLG;
    if(*inptr != NL || P_NLINES > 1)
      return(ERR);

    if ( P_CUR_AUTOIND ) count_blanks(P_LINE1);
    if(append(P_LINE1, glob) < 0)
      return(ERR);
    P_FCHANGED = TRUE;
    break;

  case 'c':
    if(*inptr != NL)
      return(ERR);

    if(deflt(P_CURLN, P_CURLN) < 0)
      return(ERR);

    P_CUR_AUTOIND = P_AUTOINDFLG;
    if ( P_AUTOINDFLG ) count_blanks(P_LINE1);
    if(del(P_LINE1, P_LINE2) < 0)
      return(ERR);
    if(append(P_CURLN, glob) < 0)
      return(ERR);
    P_FCHANGED = TRUE;
    break;

  case 'd':
    if(*inptr != NL)
      return(ERR);

    if(deflt(P_CURLN, P_CURLN) < 0)
      return(ERR);

    if(del(P_LINE1, P_LINE2) < 0)
      return(ERR);
    if(nextln(P_CURLN) != 0)
      nextCurLn();
    if (P_DPRINT)
      doprnt(P_CURLN, P_CURLN);
    P_FCHANGED = TRUE;
    break;

  case 'e':
    if(P_RESTRICT)
      return(ERR);
    if(P_NLINES > 0)
      return(ERR);
    if(P_FCHANGED)
      return CHANGED;
    /*FALL THROUGH*/
  case 'E':
    if(P_RESTRICT)
      return(ERR);
    if(P_NLINES > 0)
      return(ERR);

    if(*inptr != ' ' && *inptr != HT && *inptr != NL)
      return(ERR);

    if((fptr = getfn(0)) == NULL)
      return(ERR);

    clrbuf();
    (void)doread(0, fptr);

    strcpy(P_FNAME, fptr);
    P_FCHANGED = FALSE;
    break;

  case 'f':
    if(P_RESTRICT)
      return(ERR);
    if(P_NLINES > 0)
      return(ERR);

    if(*inptr != ' ' && *inptr != HT && *inptr != NL)
      return(ERR);

    fptr = getfn(0);

    if (P_NOFNAME)
      add_message("%s\n", P_FNAME);
    else {
      if(fptr == NULL) return(ERR);
      strcpy(P_FNAME, fptr);
    }
    break;

  case 'O':
  case 'i':
    if(*inptr != NL || P_NLINES > 1)
      return(ERR);

    P_CUR_AUTOIND = P_AUTOINDFLG;
    if ( P_AUTOINDFLG ) count_blanks(P_LINE1);
    if(append(prevln(P_LINE1), glob) < 0)
      return(ERR);
    P_FCHANGED = TRUE;
    break;

  case 'j':
    if (*inptr != NL || deflt(P_CURLN, P_CURLN+1)<0)
      return(ERR);

    if (join(P_LINE1, P_LINE2) < 0)
      return(ERR);
    break;

  case 'k':
    Skip_White_Space;

    if (*inptr < 'a' || *inptr > 'z')
      return ERR;
    c= *inptr++;

    if(*inptr != ' ' && *inptr != HT && *inptr != NL)
      return(ERR);

    P_MARK[c-'a'] = P_LINE1;
    break;

  case 'l':
    if(*inptr != NL)
      return(ERR);
    if(deflt(P_CURLN,P_CURLN) < 0)
      return(ERR);
    if (dolst(P_LINE1,P_LINE2) < 0)
      return(ERR);
    break;

  case 'm':
    if((line3 = getone()) < 0)
      return(ERR);
    if(deflt(P_CURLN,P_CURLN) < 0)
      return(ERR);
    if(move(line3) < 0)
      return(ERR);
    P_FCHANGED = TRUE;
    break;
  case 'n':
    if(*inptr != NL)
      return(ERR);
    if (P_NFLG)
      P_FLAGS &= ~NFLG_MASK;
    else 
      P_FLAGS |= NFLG_MASK;
    P_DIAG=!P_DIAG;
    add_message(	"number %s, list %s\n",
		P_NFLG?"on":"off", P_LFLG?"on":"off");
    break;
 		
  case 'I':
    if(P_NLINES > 0)
      return(ERR);
    if(*inptr != NL)
      return(ERR);
    add_message("Indenting entire code...\n");
    if (indent_code())
      add_message("Indention halted.\n");
    else 
      add_message("Done indenting.\n");
    break;
 
  case 'H':
  case 'h': 
    print_help(*(inptr++));
    break;
 
  case 'P':
  case 'p':
    if(*inptr != NL)
      return(ERR);
    if(deflt(P_CURLN,P_CURLN) < 0)
      return(ERR);
    if(doprnt(P_LINE1,P_LINE2) < 0)
      return(ERR);
    break;

  case 'q':
    if (P_FCHANGED)
      return CHANGED;
    /*FALL THROUGH*/
  case 'Q':
    clrbuf();
    if(*inptr == NL && P_NLINES == 0 && !glob)
      return(EOF);
    else
      return(ERR);

  case 'r':
    if(P_RESTRICT)
      return(ERR);
    if(P_NLINES > 1)
      return(ERR);

    if(P_NLINES == 0)		/* The original code tested */
      P_LINE2 = P_LASTLN;	/*	if(P_NLINES = 0)    */
    /* which looks wrong.  RAM  */

    if(*inptr != ' ' && *inptr != HT && *inptr != NL)
      return(ERR);

    if((fptr = getfn(0)) == NULL)
      return(ERR);

    if((err = doread(P_LINE2, fptr)) < 0)
      return(err);
    P_FCHANGED = TRUE;
    break;

  case 's':
    if(*inptr == 'e')
      return(set());
    Skip_White_Space;
    if((subpat = optpat()) == NULL)
      return(ERR);
    if((gflag = getrhs(rhs)) < 0)
      return(ERR);
    if(*inptr == 'p')
      pflag++;
    if(deflt(P_CURLN, P_CURLN) < 0)
      return(ERR);
    if((nchng = subst(subpat, rhs, gflag, pflag)) < 0)
      return(ERR);
    if(nchng)
      P_FCHANGED = TRUE;
    if ( nchng==1 && P_PFLG ) {
      if(doprnt(P_CURLN, P_CURLN) < 0)
	return(ERR);
    }
    break;

  case 't':
    if((line3 = getone()) < 0)
      return(ERR);
    if(deflt(P_CURLN,P_CURLN) < 0)
      return(ERR);
    if(transfer(line3) < 0)
      return(ERR);
    P_FCHANGED = TRUE;
    break;

  case 'W':
  case 'w':
    apflg = (c=='W');

    if(P_RESTRICT)
      return(ERR);
    if(*inptr != ' ' && *inptr != HT && *inptr != NL)
      return(ERR);

    if((fptr = getfn(1)) == NULL)
      return(ERR);

    if(deflt(1, P_LASTLN) < 0)
      return(ERR);
    if(dowrite(P_LINE1, P_LINE2, fptr, apflg) < 0)
      return(ERR);
    P_FCHANGED = FALSE;
    break;

  case 'x':
    if(*inptr == NL && P_NLINES == 0 && !glob) {
      if((fptr = getfn(1)) == NULL)
	return(ERR);
      if(dowrite(1, P_LASTLN, fptr, 0) >= 0)
	{
	  clrbuf();
	  return(EOF);
	}
    }
    return(ERR);

  case 'z':
    if(deflt(P_CURLN,P_CURLN) < 0)
      return(ERR);

    switch(*inptr) {
    case '-':
      if(doprnt(P_LINE1-21,P_LINE1) < 0)
	return(ERR);
      break;

    case '.':
      if(doprnt(P_LINE1-11,P_LINE1+10) < 0)
	return(ERR);
      break;

    case '+':
    case '\n':
      if(doprnt(P_LINE1,P_LINE1+21) < 0)
	return(ERR);
      break;
    }
    break;

  case 'Z':
    if(deflt(P_CURLN,P_CURLN) < 0)
      return(ERR);
	
    switch(*inptr) {
    case '-':
      if(doprnt(P_LINE1-41,P_LINE1) < 0)
	return(ERR);
      break;
	    
    case '.':
      if(doprnt(P_LINE1-21,P_LINE1+20) < 0)
	return(ERR);
      break;
	    
    case '+':
    case '\n':
      if(doprnt(P_LINE1,P_LINE1+41) < 0)
	return(ERR);
      break;
    }
    break;	
  default:
    return(UNRECOG_COMMAND);
  }

  return (0);
}				/* docmd */


/*	doglob.c	*/
int doglob()
{
  int	lin, status;
  char	*cmd;
  LINE	*ptr;

  cmd = inptr;

  for (;;) {
    ptr = getptr(1);
    for (lin=1; lin<=P_LASTLN; lin++) {
      if (ptr->l_stat & LGLOB)
	break;
      ptr = getnextptr(ptr);
    }
    if (lin > P_LASTLN)
      break;

    ptr->l_stat &= ~LGLOB;
    P_CURLN = lin; P_CURPTR = ptr;
    inptr = cmd;
    if((status = getlst()) < 0)
      return(status);
    if((status = docmd(1)) < 0)
      return(status);
  }
  return(P_CURLN);
}				/* doglob */


/*
 * Start the editor. Because several users can edit simultaneously,
 * they will each need a separate editor data block.
 *
 * If an exit_fn and exit_ob is given, then call exit_ob->exit_fn at
 * exit of editor. The purpose is to make it possible for external LPC
 * code to maintain a list of locked files.
 */
void
ed_start(file_arg, exit_fn, restricted, exit_ob)
     char *file_arg;
     char *exit_fn;
     int restricted;
     struct object *exit_ob;
{
  struct svalue *setup;

  if (!command_giver->interactive)
    error("Tried to start an ed session on a non-interative user.\n");
  if (ED_BUFFER)
    error("Tried to start an ed session, when already active.\n");
  ED_BUFFER = (struct ed_buffer *)
		DXALLOC(sizeof(struct ed_buffer), 28, "ed_start: ED_BUFFER");
  memset((char *)command_giver->interactive->ed_buffer, '\0',
	 sizeof (struct ed_buffer));
  ED_BUFFER->truncflg = 1;
  ED_BUFFER->flags |= EIGHTBIT_MASK;
  ED_BUFFER->shiftwidth= 4;
  push_object(command_giver);
  setup = apply_master_ob("retrieve_ed_setup",1);
  if ( setup && setup->type==T_NUMBER && setup->u.number ) {
    ED_BUFFER->flags      = setup->u.number & ALL_FLAGS_MASK;
    ED_BUFFER->shiftwidth = setup->u.number & SHIFTWIDTH_MASK;
  }
  ED_BUFFER->CurPtr =
    &command_giver->interactive->ed_buffer->Line0;

#ifdef RESTRICTED_ED
  if (command_giver->flags & O_IS_WIZARD) {
    P_RESTRICT = 0;
  } else {
    P_RESTRICT = 1;
  }
#endif				/* RESTRICTED_ED */
  if (restricted) {
     P_RESTRICT = 1;
  }

  if (exit_fn) {
    ED_BUFFER->exit_fn = string_copy(exit_fn);
    exit_ob->ref ++ ;
  } else {
    ED_BUFFER->exit_fn = 0;
  }
  ED_BUFFER->exit_ob = exit_ob;
  set_ed_buf();

  /*
   * Check for read on startup, since the buffer is read in. But don't
   * check for write, since we may want to change the file name.
   * When in compatibility mode, we assume that the test of valid read
   * is done by the caller of ed().
   */
  if(file_arg
     && (file_arg =
	 check_valid_path(file_arg, command_giver,
			  "ed_start", 0))
     && !doread(0, file_arg)) {
    setCurLn( 1 );
  }
  if (file_arg) {
    strncpy(P_FNAME, file_arg, MAXFNAME-1);
    P_FNAME[MAXFNAME-1] = 0;
  } else {
    add_message("No file.\n");
  }
  set_prompt(":");
  return;
}

void
ed_cmd(str)
     char *str;
{
  int status;

  if (P_MORE) {
    print_help2();
    return;
  }
  if (P_APPENDING) {
    more_append(str);
    return;
  }
  if (strlen(str) < MAXLINE)
    strcat(str, "\n");
	
  strncpy(inlin, str, MAXLINE-1);
  inlin[MAXLINE-1] = 0;
  inptr = inlin;
  if(getlst() >= 0)
    if((status = ckglob()) != 0) {
      if(status >= 0 && (status = doglob()) >= 0) {
	setCurLn( status );
	return;
      }
    } else {
      if((status = docmd(0)) >= 0) {
	if(status == 1)
	  doprnt(P_CURLN, P_CURLN);
	return;
      }
    }
  switch (status) {
  case EOF:
    free_ed_buffer();
    return;
  case FATAL:
    if (ED_BUFFER->exit_fn) {
      FREE(ED_BUFFER->exit_fn);
      free_object(ED_BUFFER->exit_ob, "ed FATAL");
    }
    FREE((char *)ED_BUFFER);
    ED_BUFFER= 0;
    add_message("FATAL ERROR\n");
    set_prompt(PROMPT);
    return;
  case CHANGED:
    add_message("File has been changed.\n");
    break;
  case SET_FAIL:
    add_message("`set' command failed.\n");
    break;
  case SUB_FAIL:
    add_message("string substitution failed.\n");
    break;
  case MEM_FAIL:
    add_message("Out of memory: text may have been lost.\n" );
    break;
  case UNRECOG_COMMAND:
    add_message("Unrecognized command.\n");
    break;
  default:
    add_message("Failed command.\n");
    /*  Unrecognized or failed command (this  */
    /*  is SOOOO much better than "?" :-)	  */
  }
}


void
save_ed_buffer()
{
  struct svalue *stmp;
  char *fname;

  push_string(P_FNAME, STRING_SHARED);
  stmp = apply_master_ob("get_ed_buffer_save_file_name",1);
  if (stmp) {
    if (stmp->type == T_STRING) {
      fname = stmp->u.string;
      if (*fname == '/') fname++;
      dowrite(1, P_LASTLN, fname , 0);
    }
    free_svalue(stmp);
  }
  free_ed_buffer();
}

static void
print_help(arg)
     int arg;
{
  switch (arg) {
  case 'I':
    add_message("       Automatic Indentation (V 1.0)\n");
    add_message("------------------------------------\n");
    add_message("           by Qixx [Update: 7/10/91]\n");
    add_message("\nBy using the command 'I', a program is run which will\n");
    add_message("automatically indent all lines in your code.  As this is\n");
    add_message("being done, the program will also search for some basic\n");
    add_message("errors (which don't show up good during compiling) such as\n");
    add_message("Unterminated String, Mismatched Brackets and Parentheses,\n");
    add_message("and indented code is easy to understand and debug, since if\n");
    add_message("your brackets are off -- the code will LOOK wrong. Please\n");
    add_message("mail me at gaunt@mcs.anl.gov with any pieces of code which\n");
    add_message("don't get indented properly.\n");
    break;
  case 'n':
    add_message("Command: n   Usage: n\n");
    add_message("This command toggles the internal flag which will cause line\n");
    add_message("numbers to be printed whenever a line is listed.\n");
    break;
  case 'a':
    add_message("Command: a   Usage: a\n");
    add_message("Append causes the editor to enter input mode, inserting all text\n");
    add_message("starting AFTER the current line. Use a '.' on a blank line to exit\n");
    add_message("this mode.\n");
    break;
  case 'A':
    add_message("Command: A   Usage: A\n\
Like the 'a' command, but uses inverse autoindent mode.\n");
    break;
  case 'i':
    add_message("Command: i   Usage: i\n");
    add_message("Insert causes the editor to enter input mode, inserting all text\n");
    add_message("starting BEFORE the current line. Use a '.' on a blank line to exit\n");
    add_message("this mode.\n");
    break;
  case 'c':
    add_message("Command: c   Usage: c\n");
    add_message("Change command causes the current line to be wiped from memory.\n");
    add_message("The editor enters input mode and all text is inserted where the previous\n");
    add_message("line existed.\n");
    break;
  case 'd':
    add_message("Command: d   Usage: d  or [range]d\n");
    add_message("Deletes the current line unless preceeded with a range of lines,\n");
    add_message("then the entire range will be deleted.\n");
    break;
#ifndef RESTRICTED_ED
  case 'e':
    add_message("Commmand: e  Usage: e filename\n");
    add_message("Causes the current file to be wiped from memory, and the new file\n");
    add_message("to be loaded in.\n");
    break;      
  case 'E':
    add_message("Commmand: E  Usage: E filename\n");
    add_message("Causes the current file to be wiped from memory, and the new file\n");
    add_message("to be loaded in.  Different from 'e' in the fact that it will wipe\n");
    add_message("the current file even if there are unsaved modifications.\n");
    break;
  case 'f':
    add_message("Command: f  Usage: f  or f filename\n");
    add_message("Display or set the current filename.   If  filename is given as \nan argument, the file (f) command changes the current filename to\nfilename; otherwise, it prints  the current filename.\n");
    break;
#endif				/* RESTRICTED_ED mode */
  case 'g':
    add_message("Command: g  Usage: g/re/p\n");
    add_message("Search in all lines for expression 're', and print\n");
    add_message("every match. Command 'l' can also be given\n");
    add_message("Unlike in unix ed, you can also supply a range of lines\n");
    add_message("to search in\n");
    add_message("Compare with command 'v'.\n");
    break;
  case 'h':
    add_message("Command: h    Usage:  h  or hc (where c is a command)\n");
    add_message("Help files added by Qixx.\n");
    break;
  case 'j':
    add_message("Command: j    Usage: j or [range]j\n");
    add_message("Join Lines. Remove the NEWLINE character  from  between the  two\naddressed lines.  The defaults are the current line and the line\nfollowing.  If exactly one address is given,  this  command does\nnothing.  The joined line is the resulting current line.\n");
    break;
  case 'k':
    add_message("Command: k   Usage: kc  (where c is a character)\n");
    add_message("Mark the addressed line with the name c,  a  lower-case\nletter.   The  address-form,  'c,  addresses  the  line\nmarked by c.  k accepts one address; the default is the\ncurrent line.  The current line is left unchanged.\n");
    break;
  case 'l':
    add_message("Command: l   Usage: l  or  [range]l\n");
    add_message("List the current line or a range of lines in an unambiguous\nway such that non-printing characters are represented as\nsymbols (specifically New-Lines).\n");
    break;
  case 'm':
    add_message("Command: m   Usage: mADDRESS or [range]mADDRESS\n");
    add_message("Move the current line (or range of lines if specified) to a\nlocation just after the specified ADDRESS.  Address 0 is the\nbeginning of the file and the default destination is the\ncurrent line.\n");
    break;
  case 'p':
    add_message("Command: p    Usage: p  or  [range]p\n");
    add_message("Print the current line (or range of lines if specified) to the\nscreen. See the command 'n' if line numbering is desired.\n");
    break;
  case 'q':
    add_message("Command: q    Usage: q\n");
    add_message("Quit the editor. Note that you can't quit this way if there\nare any unsaved changes.  See 'w' for writing changes to file.\n");
    break;
  case 'Q':
    add_message("Command: Q    Usage: Q\n");
    add_message("Force Quit.  Quit the editor even if the buffer contains unsaved\nmodifications.\n");
    break;
#ifndef RESTRICTED_ED
  case 'r':
    add_message("Command: r    Usage: r filename\n");
    add_message("Reads the given filename into the current buffer starting\nat the current line.\n");
    break;
#endif				/* RESTRICTED_ED */
  case 't':
    add_message("Command: t   Usage: tADDRESS or [range]tADDRESS\n");
    add_message("Transpose a copy of the current line (or range of lines if specified)\nto a location just after the specified ADDRESS.  Address 0 is the\nbeginning of the file and the default destination\nis the current line.\n");
    break;
  case 'v':
    add_message("Command: v   Usage: v/re/p\n");
    add_message("Search in all lines without expression 're', and print\n");
    add_message("every match. Other commands than 'p' can also be given\n");
    add_message("Compare with command 'g'.\n");
    break;
  case 'z':
    add_message("Command: z   Usage: z  or  z-  or z.\n");
    add_message("Displays 20 lines starting at the current line.\nIf the command is 'z.' then 20 lines are displayed being\ncentered on the current line. The command 'z-' displays\nthe 20 lines before the current line.\n");
    break;
  case 'Z':
    add_message("Command: Z   Usage: Z  or  Z-  or Z.\n");
    add_message("Displays 40 lines starting at the current line.\nIf the command is 'Z.' then 40 lines are displayed being\ncentered on the current line. The command 'Z-' displays\nthe 40 lines before the current line.\n");
    break;
  case 'x':
    add_message("Command: x   Usage: x\n");
    add_message("Save file under the current name, and then exit from ed.\n");
    break;
  case 's':
    if ( *inptr=='e' && *(inptr+1)=='t' ) {
      add_message("\
Without arguments: show current settings.\n\
'set save' will preserve the current settings for subsequent invocations of ed.\n\
Options:\n\
\n\
number	   will print line numbers before printing or inserting a lines\n\
list	   will print control characters in p(rint) and z command like in l(ist)\n\
print	   will show current line after a single substitution\n\
eightbit\n\
autoindent will preserve current indentation while entering text.\n\
	   use ^D or ^K to get back one step back to the right.\n\
excompatible will exchange the meaning of \\( and ( as well as \\) and )\n\
dprint     will print out the current line after deleting text.\n\
\n\
An option can be cleared by prepending it with 'no' in the set command, e.g.\n\
'set nolist' to turn off the list option.\n\
\n\
set shiftwidth <digit> will store <digit> in the shiftwidth variable, which\n\
determines how much blanks are removed from the current indentation when\n\
typing ^D or ^K in the autoindent mode.\n");
    } 
    break;
    /*is there anyone who wants to add an exact description for the 's' command? */
#ifndef RESTRICTED_ED	
  case 'w':
  case 'W':
  case '/':
  case '?':
    add_message("Sorry no help yet for this command. Try again later.\n");
    break;
#endif				/* RESTRICTED_ED */
  default:
    add_message("       Help for Ed  (V 2.0)\n");
    add_message("---------------------------------\n");
    add_message("     by Qixx [Update: 7/10/91]\n");
    add_message("\n\nCommands\n--------\n");
    add_message("/\tsearch forward for pattern\n");
    add_message("?\tsearch backward for a pattern\n");
    /* add_message("^\tglobal search and print for pattern\n"); */
    add_message("=\tshow current line number\n");
    add_message("a\tappend text starting after this line\n");
    add_message("A\tlike 'a' but with inverse autoindent mode\n");
    add_message("c\tchange current line, query for replacement text\n");
    add_message("d\tdelete line(s)\n");
#ifdef RESTRICTED_ED
    if(!P_RESTRICT) {
      add_message("e\treplace this file with another file\n");
      add_message("E\tsame as 'e' but works if file has been modified\n");
      add_message("f\tshow/change current file name\n");
    }
#endif				/* restricted ed */
    add_message("g\tSearch and execute command on any matching line.\n");
    add_message("h\thelp file (display this message)\n");
    add_message("i\tinsert text starting before this line\n");
    add_message("I\tindent the entire code (Qixx version 1.0)\n");
    add_message("\n--Return to continue--");
    P_MORE=1;
    break;
  }
}

static void
print_help2()
{
  P_MORE=0;
  add_message("j\tjoin lines together\n");
  add_message("k\tmark this line with a character - later referenced as 'a\n");
  add_message("l\tline line(s) with control characters displayed\n");
  add_message("m\tmove line(s) to specified line\n");
  add_message("n\ttoggle line numbering\n");
  add_message("p\tprint line(s) in range\n");
  add_message("q\tquit editor\n");
  add_message("Q\tquit editor even if file modified and not saved\n\
r\tread file into editor at end of file or behind the given line\n");
  add_message("s\tsearch and replace\n");
  add_message("set\tquery, change or save option settings\n");
  add_message("t\tmove copy of line(s) to specified line\n");
  add_message("v\tSearch and execute command on any non-matching line.\n");
  add_message("x\tsave file and quit\n");
#ifdef RESTRICTED_ED
  if(!P_RESTRICT) {
    add_message("w\twrite to current file (or specified file)\n");
    add_message("W\tlike the 'w' command but appends instead\n");
  }
#endif				/* restricted ed */
  add_message("z\tdisplay 20 lines, possible args are . + -\n");
  add_message("Z\tdisplay 40 lines, possible args are . + -\n");
  add_message("\nFor further information type 'hc' where c is the command\nthat help is desired for.\n");
}
#endif /* ED */

/* regerror.c (used by regexp.c) */
void regerror( s )
    char *s;
{
  add_message("ed: %s\n", s );
}
