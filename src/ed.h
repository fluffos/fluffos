#ifndef ED_H
#define ED_H

#include "regexp.h"

/* define this if you don't like the ending dollar signs in ed, in n-mode */
#undef NO_END_DOLLAR_SIGN
/*
 *	#defines for non-printing ASCII characters
 */
#define NUL	0x00		/* ^@ */
#define EOS	0x00		/* end of string */
#define SOH	0x01		/* ^A */
#define STX	0x02		/* ^B */
#define ETX	0x03		/* ^C */
#define EOT	0x04		/* ^D */
#define ENQ	0x05		/* ^E */
#define ACK	0x06		/* ^F */
#define BEL	0x07		/* ^G */
#define BS	0x08		/* ^H */
#define HT	0x09		/* ^I */
#define LF	0x0a		/* ^J */
#define NL	'\n'
#define VT	0x0b		/* ^K */
#define FF	0x0c		/* ^L */
#define CR	0x0d		/* ^M */
#define SO	0x0e		/* ^N */
#define SI	0x0f		/* ^O */
#define DLE	0x10		/* ^P */
#define DC1	0x11		/* ^Q */
#define DC2	0x12		/* ^R */
#define DC3	0x13		/* ^S */
#define DC4	0x14		/* ^T */
#define NAK	0x15		/* ^U */
#define SYN	0x16		/* ^V */
#define ETB	0x17		/* ^W */
#define CAN	0x18		/* ^X */
#define EM	0x19		/* ^Y */
#define SUB	0x1a		/* ^Z */
#define ESC	0x1b		/* ^[ */
#define FS	0x1c		/* ^\ */
#define GS	0x1d		/* ^] */
#define US	0x1f		/* ^_ */
#define SP	0x20		/* space */
#define DEL	0x7f		/* DEL */
#define ESCAPE  '\\'

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

#define BAD_LINE_RANGE  (ERR-7)
#define BAD_LINE_NUMBER (ERR-8)
#define SYNTAX_ERROR    (ERR-9)
#define RANGE_ILLEGAL   (ERR-10)
#define RESTRICTED      (ERR-11)
#define LINE_OR_RANGE_ILL (ERR-12)
#define FILE_NAME_ERROR (ERR-13)
#define MARK_A_TO_Z     (ERR-14)
#define SUB_BAD_PATTERN (ERR-15)
#define SUB_BAD_REPLACEMENT (ERR-16)
#define BAD_DESTINATION (ERR-17)
#define END_OF_FILE     (ERR-18)

#define	BUFFER_SIZE	2048	/* stream-buffer size:  == 1 hd cluster */

#define LINFREE		1	/* entry not in use */
#define LGLOB		2	/* line marked global */

#define MAXLINE		512	/* max number of chars per line */
#define MAXPAT		256	/* max number of chars per replacemnt pattern */
#define MAXFNAME 	256	/* max file name size */

struct line {
    int l_stat;			/* empty, mark */
    struct line *l_prev;
    struct line *l_next;
    char l_buff[1];
};
typedef struct line LINE;

struct ed_buffer {
    int diag;			/* diagnostic-output? flag */
    int truncflg;		/* truncate long line flag */
    int nonascii;		/* count of non-ascii chars read */
    int nullchar;		/* count of null chars read */
    int truncated;		/* count of lines truncated */
    char fname[MAXFNAME];
    int fchanged;		/* file-changed? flag */
    int nofname;
    int mark['z' - 'a' + 1];
    regexp *oldpat;

    LINE Line0;
    int CurLn;
    LINE *CurPtr;		/* CurLn and CurPtr must be kept in step */
    int LastLn;
    int Line1, Line2, nlines;
    int flags;
    int appending;
    int moring;			/* used for the wait line of help */
#ifdef OLD_ED
    char *exit_fn;		/* Function to be called when user exits */
    char *write_fn;             /* Function to be called when user writes */
    struct object *exit_ob;	/* in this object */
#else
    struct object *owner;
    struct ed_buffer *next_ed_buf;
#endif
    int shiftwidth;
    int leading_blanks;
    int cur_autoindent;
    int restricted;		/* restricted access ed */
};

/*
 * ed.c
 */
void ed_start PROT((char *, char *, char *, int, struct object *));
void ed_cmd PROT((char *));
void save_ed_buffer PROT((void));
void regerror PROT((char *));

#ifndef OLD_ED
char *object_ed_cmd PROT((struct object *, char *));
char *object_ed_start PROT((struct object *, char *, int));
int object_ed_mode PROT((struct object *));
void object_save_ed_buffer PROT((struct object *));
struct ed_buffer *find_ed_buffer PROT((struct object *));
#endif

#endif
