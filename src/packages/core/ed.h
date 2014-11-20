#ifndef ED_H
#define ED_H

/* define this if you don't like the ending dollar signs in ed, in n-mode */
#undef NO_END_DOLLAR_SIGN
/*
 *      #defines for non-printing ASCII characters
 */
#define NUL 0x00 /* ^@ */
#define EOS 0x00 /* end of string */
#define SOH 0x01 /* ^A */
#define STX 0x02 /* ^B */
#define ETX 0x03 /* ^C */
#define EOT 0x04 /* ^D */
#define ENQ 0x05 /* ^E */
#define ACK 0x06 /* ^F */
#define BEL 0x07 /* ^G */
#define BS 0x08  /* ^H */
#define HT 0x09  /* ^I */
#define LF 0x0a  /* ^J */
#define NL '\n'
#define VT 0x0b  /* ^K */
#define FF 0x0c  /* ^L */
#define CR 0x0d  /* ^M */
#define SO 0x0e  /* ^N */
#define SI 0x0f  /* ^O */
#define DLE 0x10 /* ^P */
#define DC1 0x11 /* ^Q */
#define DC2 0x12 /* ^R */
#define DC3 0x13 /* ^S */
#define DC4 0x14 /* ^T */
#define NAK 0x15 /* ^U */
#define SYN 0x16 /* ^V */
#define ETB 0x17 /* ^W */
#define CAN 0x18 /* ^X */
#define EM 0x19  /* ^Y */
#define SUB 0x1a /* ^Z */
#define ESC 0x1b /* ^[ */
#define FS 0x1c  /* ^\ */
#define GS 0x1d  /* ^] */
#define US 0x1f  /* ^_ */
#define SP 0x20  /* space */
#define DEL 0x7f /* DEL */
#define ESCAPE '\\'

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#define EDERR -2
#define FATAL (EDERR - 1)
#define CHANGED (EDERR - 2)
#define SET_FAIL (EDERR - 3)
#define SUB_FAIL (EDERR - 4)
#define MEM_FAIL (EDERR - 5)
#define UNRECOG_COMMAND (EDERR - 6)

#define BAD_LINE_RANGE (EDERR - 7)
#define BAD_LINE_NUMBER (EDERR - 8)
#define SYNTAX_ERROR (EDERR - 9)
#define RANGE_ILLEGAL (EDERR - 10)
#define IS_RESTRICTED (EDERR - 11)
#define LINE_OR_RANGE_ILL (EDERR - 12)
#define FILE_NAME_ERROR (EDERR - 13)
#define MARK_A_TO_Z (EDERR - 14)
#define SUB_BAD_PATTERN (EDERR - 15)
#define SUB_BAD_REPLACEMENT (EDERR - 16)
#define BAD_DESTINATION (EDERR - 17)
#define END_OF_FILE (EDERR - 18)
#define SEARCH_FAILED (EDERR - 19)
#define NO_LINE_RANGE (EDERR - 20)

// This one hopefully holds a screen's data for the 'z' command.
// Even if bytes per line is actually 80, this is still 50-ish lines.
#define BUFFER_SIZE 4096

#define LGLOB 2 /* line marked global */

#define ED_MAXLINE 2047 /* max number of chars per line */
#define MAXPAT 256      /* max number of chars per replacemnt pattern */
#define MAXFNAME 256    /* max file name size */

typedef struct ed_line_s {
  int l_stat; /* empty, mark */
  struct ed_line_s *l_prev;
  struct ed_line_s *l_next;
  char l_buff[1];
} ed_line_t;

struct strlst {
  char *screen;
  struct strlst *next;
};

typedef struct ed_buffer_s {
  int nonascii;  /* count of non-ascii chars read */
  int nullchar;  /* count of null chars read */
  int truncated; /* count of lines truncated */
  char fname[MAXFNAME];
  int fchanged; /* file-changed? flag */
  int nofname;
  int mark['z' - 'a' + 1];
  struct regexp *oldpat;

  ed_line_t Line0;
  int CurLn;
  ed_line_t *CurPtr; /* CurLn and CurPtr must be kept in step */
  int LastLn;
  int Line1, Line2, nlines;
  int flags;
  int appending;
  int moring;             /* used for the wait line of help */
  struct strlst *helpout; /* help output linked list */
#ifdef OLD_ED
  char *exit_fn;     /* Function to be called when user exits */
  char *write_fn;    /* Function to be called when user writes */
  object_t *exit_ob; /* in this object */
#else
  object_t *owner;
  struct ed_buffer_s *next_ed_buf;
#endif
  int shiftwidth;
  int leading_blanks;
  int cur_autoindent;
  int scroll_lines;
  int restricted; /* restricted access ed */
} ed_buffer_t;

/*
 * ed.c
 */
void ed_start(const char *, const char *, const char *, int, struct object_t *, int);
void ed_cmd(char *);
void save_ed_buffer(struct object_t *);

#ifdef OLD_ED
#define ED_OUTPUT(x, y) add_message(x, y, strlen(y))
#define ED_OUTPUTV add_vmessage
#define ED_DEST command_giver
#else
#define ED_OUTPUT outbuf_add
#define ED_OUTPUTV outbuf_addv
#define ED_DEST &current_ed_results
#endif

#ifndef OLD_ED
char *object_ed_cmd(struct object_t *, const char *);
char *object_ed_start(struct object_t *, const char *, int, int);
int object_ed_mode(struct object_t *);
void object_save_ed_buffer(struct object_t *);
ed_buffer_t *find_ed_buffer(struct object_t *);
void object_ed_output(char *);
void object_ed_outputv(char *, ...);

extern outbuffer_t current_ed_results;
#endif

#endif
