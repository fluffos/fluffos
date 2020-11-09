/*
 * sprintf.c v1.05 for LPMud 3.0.52
 *
 * An implementation of (s)printf() for LPC, with quite a few
 * extensions (note that as no floating point exists, some parameters
 * have slightly different meaning or restrictions to "standard"
 * (s)printf.)  Implemented by Lynscar (Sean A Reith).
 * 2/28/93: float support for MudOS added by jacques/blackthorn
 *
 * This version supports the following as modifiers:
 *  " "   pad positive integers with a space.
 *  "+"   pad positive integers with a plus sign.
 *  "-"   left adjusted within field size.
 *        NB: std (s)printf() defaults to right justification, which is
 *            unnatural in the context of a mainly string based language
 *            but has been retained for "compatability" ;)
 *  "|"   centered within field size.
 *  "="   column mode if strings are greater than field size.  this is only
 *        meaningful with strings, all other types ignore
 *        this.  columns are auto-magically word wrapped.
 *  "#"   table mode, print a list of '\n' separated 'words' in a
 *        table within the field size.  only meaningful with strings.
 *   n    specifies the field size, a '*' specifies to use the corresponding
 *        arg as the field size.  if n is prepended with a zero, then is padded
 *        zeros, else it is padded with spaces (or specified pad string).
 *  "."n  precision of n, simple strings truncate after this (if precision is
 *        greater than field size, then field size = precision), tables use
 *        precision to specify the number of columns (if precision not specified
 *        then tables calculate a best fit), all other types ignore this.
 *  ":"n  n specifies the fs _and_ the precision, if n is prepended by a zero
 *        then it is padded with zeros instead of spaces.
 *  "@"   the argument is an array.  the corresponding format_info (minus the
 *        "@") is applyed to each element of the array.
 *  "'X'" The char(s) between the single-quotes are used to pad to field
 *        size (defaults to space) (if both a zero (in front of field
 *        size) and a pad string are specified, the one specified second
 *        overrules).  NOTE:  to include "'" in the pad string, you must
 *        use "\\'" (as the backslash has to be escaped past the
 *        interpreter), similarly, to include "\\" requires "\\\\".
 * The following are the possible type specifiers.
 *  "%"   in which case no arguments are interpreted, and a "%" is inserted, and
 *        all modifiers are ignored.
 *  "O"   the argument is an LPC datatype.
 *  "s"   the argument is a string.
 *  "d"   the integer arg is printed in decimal.
 *  "i"   as d.
 *  "f"   floating point value.
 *  "c"   the integer arg is to be printed as a character.
 *  "o"   the integer arg is printed in octal.
 *  "x"   the integer arg is printed in hex.
 *  "X"   the integer arg is printed in hex (in capitals).
 */

#include "base/package_api.h"

#include "packages/core/sprintf.h"

#include <unicode/brkiter.h>

#if defined(F_SPRINTF) || defined(F_PRINTF)

typedef unsigned int format_info;

/*
 * Format of format_info:
 *   00000000 0000xxxx : argument type:
 *                              0000 : type not found yet;
 *                              0001 : error type not found;
 *                              0010 : percent sign, null argument;
 *                              0011 : LPC datatype;
 *                              0100 : string;
 *                              0101 : char;
 *                              1000 : integer;
 *                              1010 : octal;
 *                              1011 : hex;
 *                              1100 : HEX;
 *                              1101 : float;
 *   00000000 00xx0000 : justification:
 *                              00 : right;
 *                              01 : centre;
 *                              10 : left;
 *   00000000 xx000000 : positive pad char:
 *                              00 : none;
 *                              01 : ' ';
 *                              10 : '+';
 *   0000000x 00000000 : array mode?
 *   000000x0 00000000 : column mode?
 *   00000x00 00000000 : table mode?
 */

#define INFO_T 0xF
#define INFO_T_ERROR 0x1
#define INFO_T_NULL 0x2
#define INFO_T_LPC 0x3
#define INFO_T_STRING 0x4
#define INFO_T_CHAR 0x5
#define INFO_T_INT 0x8
#define INFO_T_OCT 0xA
#define INFO_T_HEX 0xB
#define INFO_T_C_HEX 0xC
#define INFO_T_FLOAT 0xD

#define INFO_J 0x30
#define INFO_J_CENTRE 0x10
#define INFO_J_LEFT 0x20

#define INFO_PP 0xC0
#define INFO_PP_SPACE 0x40
#define INFO_PP_PLUS 0x80

#define INFO_ARRAY 0x100
#define INFO_COLS 0x200
#define INFO_TABLE 0x400

#define ERR_BUFF_OVERFLOW 0x1      /* buffer overflowed */
#define ERR_TOO_FEW_ARGS 0x2       /* more arguments spec'ed than passed */
#define ERR_INVALID_STAR 0x3       /* invalid arg to * */
#define ERR_PRES_EXPECTED 0x4      /* expected precision not found */
#define ERR_INVALID_FORMAT_STR 0x5 /* error in format string */
#define ERR_INCORRECT_ARG_S 0x6    /* invalid arg to %s */
#define ERR_CST_REQUIRES_FS 0x7    /* field size not given for c/t */
#define ERR_BAD_INT_TYPE 0x8       /* bad integer type... */
#define ERR_UNDEFINED_TYPE 0x9     /* undefined type found */
#define ERR_QUOTE_EXPECTED 0xA     /* expected ' not found */
#define ERR_UNEXPECTED_EOS 0xB     /* fs terminated unexpectedly */
#define ERR_NULL_PS 0xC            /* pad string is null */
#define ERR_ARRAY_EXPECTED 0xD     /* Yep!  You guessed it. */
#define ERR_INVALID_ARG_C 0xE      /* Invalid arg to %c */
#define ERR_RECOVERY_ONLY            \
  0xF /* err msg already done...just \
       * recover */

#define ADD_CHAR(x)                                                                            \
  {                                                                                            \
    const auto max_string_length = CONFIG_INT(__MAX_STRING_LENGTH__);                          \
    if (sprintf_state->obuff.real_size == max_string_length) SPRINTF_ERROR(ERR_BUFF_OVERFLOW); \
    outbuf_addchar(&(sprintf_state->obuff), x);                                                \
  }

#define GET_NEXT_ARG                                                       \
  {                                                                        \
    if (++sprintf_state->cur_arg >= argc) SPRINTF_ERROR(ERR_TOO_FEW_ARGS); \
    carg = (argv + sprintf_state->cur_arg);                                \
  }

typedef struct {
  const char *what;
  int len;
  int width;
} pad_info_t;

typedef struct {
  const char *start;
  const char *cur;
} tab_data_t;

/* slash here means 'or' */
typedef struct ColumnSlashTable {
  union CSTData {
    const char *col;         /* column data */
    tab_data_t *tab;         /* table data */
  } d;                       /* d == data */
  unsigned short int nocols; /* number of columns in table *sigh* */
  pad_info_t *pad;
  unsigned int start;     /* starting cursor position */
  unsigned int size;      /* column/table width */
  unsigned int remainder; /* extra space needed to fill out to width */
  int pres;               /* precision */
  format_info info;       /* formatting data */
  struct ColumnSlashTable *next;
} cst; /* Columns Slash Tables */

typedef struct _sprintf_state {
  outbuffer_t obuff;
  cst *csts;
  int cur_arg;
  svalue_t clean;
  struct _sprintf_state *next;
} sprintf_state_t;

static sprintf_state_t *sprintf_state = nullptr;

static void add_space(outbuffer_t * /*outbuf*/, int indent);
static void add_justified(const char *str, int swidth, int slen, pad_info_t *pad, int fs,
                          format_info finfo, short int trailing);
static int add_column(cst **column, int trailing);
static int add_table(cst **table);

#define SPRINTF_ERROR(x) sprintf_error(x, 0)

static void pop_sprintf_state(void) {
  sprintf_state_t *state;

  state = sprintf_state;
  sprintf_state = sprintf_state->next;

  if (state->obuff.buffer) {
    FREE_MSTR(state->obuff.buffer);
  }
  while (state->csts) {
    cst *next = state->csts->next;
    if (!(state->csts->info & INFO_COLS) && state->csts->d.tab) {
      FREE(state->csts->d.tab);
    }
    FREE(state->csts);
    state->csts = next;
  }
  if (state->clean.type != T_NUMBER) {
    free_svalue(&(state->clean), "pop_sprintf_state");
  }

  FREE(state);
}

static void push_sprintf_state(void) {
  sprintf_state_t *state;

  state = reinterpret_cast<sprintf_state_t *>(
      DMALLOC(sizeof(sprintf_state_t), TAG_TEMPORARY, "push_sprintf_state"));
  outbuf_zero(&(state->obuff));
  state->csts = nullptr;
  state->cur_arg = -1;
  state->clean.type = T_NUMBER;
  state->clean.u.number = 0;
  state->next = sprintf_state;
  sprintf_state = state;
}

/* Signal an error.  Note that we call error, so this routine never returns.
 * Anything that has been allocated should be somewhere it can be found and
 * freed later.
 */
static void sprintf_error(int which, const char *premade) {
  char lbuf[2048];
  const char *err;

  switch (which) {
    case ERR_BUFF_OVERFLOW:
      err = "BUFF_SIZE overflowed...";
      break;
    case ERR_TOO_FEW_ARGS:
      err = "More arguments specified than passed.";
      break;
    case ERR_INVALID_STAR:
      err = "Incorrect argument type to *.";
      break;
    case ERR_PRES_EXPECTED:
      err = "Expected precision not found.";
      break;
    case ERR_INVALID_FORMAT_STR:
      err = "Error in format string.";
      break;
    case ERR_INCORRECT_ARG_S:
      err = "Incorrect argument to type %%s.";
      break;
    case ERR_CST_REQUIRES_FS:
      err = "Column/table mode requires a field size.";
      break;
    case ERR_BAD_INT_TYPE:
      err = "!feature - bad integer type!";
      break;
    case ERR_UNDEFINED_TYPE:
      err = "!feature - undefined type!";
      break;
    case ERR_QUOTE_EXPECTED:
      err = "Quote expected in format string.";
      break;
    case ERR_UNEXPECTED_EOS:
      err = "Unexpected end of format string.";
      break;
    case ERR_NULL_PS:
      err = "Null pad string specified.";
      break;
    case ERR_ARRAY_EXPECTED:
      err = "Array expected.";
      break;
    case ERR_INVALID_ARG_C:
      err = "Incorrect argument to type %%c, must be valid UTF8 char.";
      break;
    case ERR_RECOVERY_ONLY:
      err = premade;
      break;
    default:
      err = "undefined error in (s)printf!\n";
      break;
  }
  sprintf(lbuf, "(s)printf(): %s (arg: %d)\n", err, sprintf_state->cur_arg);
  error(lbuf);
}

static void add_space(outbuffer_t *outbuf, int indent) {
  int l;

  if ((l = outbuf_extend(outbuf, indent))) {
    memset(outbuf->buffer + outbuf->real_size, ' ', l);
    *(outbuf->buffer + outbuf->real_size + l) = 0;
    outbuf->real_size += l;
  }
}

/*
 * Converts any LPC datatype into an arbitrary string format
 * and returns a pointer to this string.
 * Scary number of parameters for a recursive function.
 */
void svalue_to_string(svalue_t *obj, outbuffer_t *outbuf, int indent, int trailing, int indent2) {
  int i;

  /* prevent an infinite recursion on self-referential structures */
  if (indent > 20) {
    outbuf_add(outbuf, "...");
    return;
  }
  if (!indent2) {
    add_space(outbuf, indent);
  }
  switch ((obj->type & ~T_FREED)) {
    case T_INVALID:
      outbuf_add(outbuf, "T_INVALID");
      break;
    case T_LVALUE:
      outbuf_add(outbuf, "lvalue: ");
      svalue_to_string(obj->u.lvalue, outbuf, indent + 2, trailing, 0);
      break;
    case T_REF:
      if (!obj->u.ref->lvalue) {
        kill_ref(obj->u.ref);
      } else {
        outbuf_add(outbuf, "ref: ");
        svalue_to_string(obj->u.ref->lvalue, outbuf, indent + 2, trailing, 0);
      }
      break;
    case T_NUMBER:
      outbuf_addv(outbuf, "%" LPC_INT_FMTSTR_P, obj->u.number);
      break;
    case T_REAL:
      outbuf_addv(outbuf, "%" LPC_FLOAT_FMTSTR_P, obj->u.real);
      break;
    case T_STRING:
      outbuf_add(outbuf, "\"");
      outbuf_add(outbuf, obj->u.string);
      outbuf_add(outbuf, "\"");
      break;
    case T_CLASS: {
      int n = obj->u.arr->size;
      outbuf_add(outbuf, "CLASS( ");
      outbuf_addv(outbuf, "%d", n);
      outbuf_add(outbuf, n == 1 ? " element\n" : " elements\n");
      for (i = 0; i < (obj->u.arr->size) - 1; i++) {
        svalue_to_string(&(obj->u.arr->item[i]), outbuf, indent + 2, 1, 0);
      }
      if (obj->u.arr->size) svalue_to_string(&(obj->u.arr->item[i]), outbuf, indent + 2, 0, 0);
      outbuf_add(outbuf, "\n");
      add_space(outbuf, indent);
      outbuf_add(outbuf, " )");
      break;
    }
    case T_ARRAY:
      if (!(obj->u.arr->size)) {
        outbuf_add(outbuf, "({ })");
      } else {
        outbuf_add(outbuf, "({ /* sizeof() == ");
        outbuf_addv(outbuf, "%d", obj->u.arr->size);
        outbuf_add(outbuf, " */\n");
        for (i = 0; i < (obj->u.arr->size) - 1; i++) {
          svalue_to_string(&(obj->u.arr->item[i]), outbuf, indent + 2, 1, 0);
        }
        svalue_to_string(&(obj->u.arr->item[i]), outbuf, indent + 2, 0, 0);
        outbuf_add(outbuf, "\n");
        add_space(outbuf, indent);
        outbuf_add(outbuf, "})");
      }
      break;
    case T_BUFFER:
      outbuf_add(outbuf, "BUFFER ( /* sizeof() == ");
      outbuf_addv(outbuf, "%d", obj->u.buf->size);
      outbuf_add(outbuf, " */\n");
      for (i = 0; i < (obj->u.buf->size) - 1; i++) {
        add_space(outbuf, indent + 2);
        outbuf_addv(outbuf, "0x%hhx,\n", (char)obj->u.buf->item[i]);
      }
      add_space(outbuf, indent + 2);
      outbuf_addv(outbuf, "0x%hhx", (char)obj->u.buf->item[i]);
      outbuf_add(outbuf, "\n");
      add_space(outbuf, indent);
      outbuf_add(outbuf, ")");
      break;
    case T_FUNCTION: {
      object_t *ob;

      outbuf_add(outbuf, "(: ");
      switch (obj->u.fp->hdr.type) {
        case FP_LOCAL | FP_NOT_BINDABLE:
          ob = obj->u.fp->hdr.owner;
          if (!ob || ob->flags & O_DESTRUCTED) {
            outbuf_add(outbuf, "0");
            break;
          }
          outbuf_add(outbuf, function_name(ob->prog, obj->u.fp->f.local.index));
          break;
        case FP_SIMUL:
          outbuf_add(outbuf, simuls[obj->u.fp->f.simul.index].func->funcname);
          break;
        case FP_FUNCTIONAL:
        case FP_FUNCTIONAL | FP_NOT_BINDABLE: {
          char buf[10];
          int n = obj->u.fp->f.functional.num_arg;

          outbuf_add(outbuf, "<code>(");
          for (i = 1; i < n; i++) {
            sprintf(buf, "$%i, ", i);
            outbuf_add(outbuf, buf);
          }
          if (n) {
            sprintf(buf, "$%i", n);
            outbuf_add(outbuf, buf);
          }
          outbuf_add(outbuf, ")");
          break;
        }
        case FP_EFUN: {
          int i;
          i = obj->u.fp->f.efun.index;
          outbuf_add(outbuf, query_instr_name(i));
          break;
        }
      }
      if (obj->u.fp->hdr.args) {
        for (i = 0; i < obj->u.fp->hdr.args->size; i++) {
          outbuf_add(outbuf, ", ");
          svalue_to_string(&(obj->u.fp->hdr.args->item[i]), outbuf, indent, 0, 0);
        }
      }
    }
      outbuf_add(outbuf, " :)");
      break;
    case T_MAPPING:
      if (!(obj->u.map->count)) {
        outbuf_add(outbuf, "([ ])");
      } else {
        outbuf_add(outbuf, "([ /* sizeof() == ");
        outbuf_addv(outbuf, "%u", obj->u.map->count);
        outbuf_add(outbuf, " */\n");
        for (i = 0; i <= obj->u.map->table_size; i++) {
          mapping_node_t *elm;

          for (elm = obj->u.map->table[i]; elm; elm = elm->next) {
            svalue_to_string(&(elm->values[0]), outbuf, indent + 2, 0, 0);
            outbuf_add(outbuf, " : ");
            svalue_to_string(&(elm->values[1]), outbuf, indent + 4, 1, 1);
          }
        }
        add_space(outbuf, indent);
        outbuf_add(outbuf, "])");
      }
      break;
    case T_OBJECT: {
      svalue_t *temp;

      if (obj->u.ob->flags & O_DESTRUCTED) {
        outbuf_addv(outbuf, "%d", 0);
        break;
      }

      outbuf_addchar(outbuf, '/');
      outbuf_add(outbuf, obj->u.ob->obname);

      if (!max_eval_error && !too_deep_error) {
        push_object(obj->u.ob);
        temp = safe_apply_master_ob(APPLY_OBJECT_NAME, 1);
        if (temp && temp != (svalue_t *)-1 && (temp->type == T_STRING)) {
          outbuf_add(outbuf, " (\"");
          outbuf_add(outbuf, temp->u.string);
          outbuf_add(outbuf, "\")");
        }
      }
      break;
    }
    default:
      outbuf_addv(outbuf, "!ERROR: GARBAGE SVALUE: %x!", obj->type);
  } /* end of switch (obj->type) */
  if (trailing) {
    outbuf_add(outbuf, ",\n");
  }
} /* end of svalue_to_string() */

static void add_pad(pad_info_t *pad, int width, bool start_from_right) {
  int output_len = width;  // default use ' ' to pad.

  // Use whitespace by default.
  if (!pad || !pad->len) {
    char *p;

    if (outbuf_extend(&(sprintf_state->obuff), output_len) < output_len) {
      SPRINTF_ERROR(ERR_BUFF_OVERFLOW);
    }

    p = sprintf_state->obuff.buffer + sprintf_state->obuff.real_size;
    sprintf_state->obuff.real_size += output_len;
    p[output_len] = 0;

    memset(p, ' ', output_len);
    return;
  }

  // Get an sanitized version of pad_string first
  std::string _pad_string;
  {
    _pad_string.reserve(pad->len);

    auto padp = pad->what;
    for (int i = 0; i < pad->len; i++) {
      char c = *padp++;
      if (c == '\\') c = *padp++;
      _pad_string.push_back(c);
    }
  }
  const auto &pad_string = _pad_string;

  std::string pad_result;
  pad_result.reserve(output_len);

  int padded_width = 0;
  while (padded_width < width) {
    if (padded_width + pad->width <= width) {
      pad_result.append(pad_string);
      padded_width += pad->width;
    } else {
      // no more room for entire pad string, try truncate
      {
        auto leftover_width = width - padded_width;
        size_t truncated_len, truncated_width;
        u8_truncate_below_width(pad_string.c_str(), pad->len, leftover_width, false, false,
                                &truncated_len, &truncated_width);
        if (truncated_len && truncated_width && truncated_width <= leftover_width) {
          pad_result.append(pad_string.substr(0, truncated_len));
          padded_width += truncated_width;
        }
      }

      // we may still ends up missing some width after truncation, fill with whitespace then.
      if (padded_width < width) {
        if (start_from_right) {
          for (auto i = 0; i < width - padded_width; i++) {
            pad_result.insert(0, " ");
          }
        } else {
          for (auto i = 0; i < width - padded_width; i++) {
            pad_result.push_back(' ');
          }
        }
        padded_width = width;
      }
    }
  }
  output_len = pad_result.length();

  if (outbuf_extend(&(sprintf_state->obuff), output_len) < output_len) {
    SPRINTF_ERROR(ERR_BUFF_OVERFLOW);
  }

  auto p = sprintf_state->obuff.buffer + sprintf_state->obuff.real_size;
  sprintf_state->obuff.real_size += output_len;
  p[output_len] = 0;

  memcpy(p, pad_result.c_str(), output_len);
}

static void add_nstr(const char *str, int len) {
  if (outbuf_extend(&(sprintf_state->obuff), len) < len) {
    SPRINTF_ERROR(ERR_BUFF_OVERFLOW);
  }
  memcpy(sprintf_state->obuff.buffer + sprintf_state->obuff.real_size, str, len);
  sprintf_state->obuff.real_size += len;
  sprintf_state->obuff.buffer[sprintf_state->obuff.real_size] = 0;
}

/*
 * Adds the string "str" to the buff after justifying it within "fs".
 * "trailing" is a flag which is set if trailing justification is to be done.
 * "str" is unmodified.  trailing is, of course, ignored in the case
 * of right justification.
 */
static void add_justified(const char *str, int swidth, int slen, pad_info_t *pad, int fs,
                          format_info finfo, short int trailing) {
  fs -= swidth;
  if (fs <= 0) {
    add_nstr(str, slen);
  } else {
    int i;
    switch (finfo & INFO_J) {
      case INFO_J_LEFT:
        add_nstr(str, slen);
        if (trailing) {
          add_pad(pad, fs, false);
        }
        break;
      case INFO_J_CENTRE:
        i = fs / 2 + fs % 2;
        add_pad(pad, i, true);
        add_nstr(str, slen);
        if (trailing) {
          add_pad(pad, fs - i, false);
        }
        break;
      case INFO_J_CENTRE | INFO_J_LEFT:
        i = fs / 2;
        add_pad(pad, i, true);
        add_nstr(str, slen);
        if (trailing) {
          add_pad(pad, fs - i, false);
        }
        break;
      default:
        /* std (s)printf defaults to right
         * justification */
        add_pad(pad, fs, true);
        add_nstr(str, slen);
    }
  }
} /* end of add_justified() */

/*
 * Adds "column" to the buffer.
 * Returns 0 is column not finished.
 * Returns 1 if column completed.
 * Returns 2 if column completed has a \n at the end.
 */
static int add_column(cst **column, int trailing) {
  int ret;
  cst *col = *column;             /* always holds (*column) */
  const char *col_d = col->d.col; /* always holds (col->d.col) */

  auto slen = strlen(col_d);
  size_t swidth;

  u8_truncate_below_width(col_d, slen, col->pres, true, true, &slen, &swidth);
  add_justified(col_d, swidth, slen, col->pad, col->size, col->info, trailing || col->next);

  col_d += slen;
  ret = 1;

  // Eat the space if break
  if (*col_d == ' ') {
    col_d++;
  } else if (*col_d == '\n') {
    // landed on an '\n'
    col_d++;
    ret = 2;
  }
  col->d.col = col_d;
  /*
   * if the next character is a NULL then take this column out of
   * the list.
   */
  if (*col_d == '\0') {
    cst *temp;

    temp = col->next;
    if (col->pad) {
      FREE(col->pad);
    }
    FREE(col);
    *column = temp;
    return ret;
  }
  return 0;
} /* end of add_column() */

/*
 * Adds "table" to the buffer.
 * Returns 0 if table not completed.
 * Returns 1 if table completed.
 */
static int add_table(cst **table) {
  int i;
  cst *tab = *table;              /* always (*table) */
  tab_data_t *tab_d = tab->d.tab; /* always tab->d.tab */
  const char *tab_di;             /* always tab->d.tab[i].cur */

  for (i = 0; i < tab->nocols && (tab_di = tab_d[i].cur); i++) {
    int done;
    int end = tab_d[i + 1].start - tab_di - 1;
    // Look for next '\n'
    for (done = 0; done != end && tab_di[done] != '\n'; done++)
      ;
    // Check if this is over width
    {
      size_t slen = done;
      size_t swidth = u8_width(tab_di, done);
      if (swidth > tab->size) {
        u8_truncate_below_width(tab_di, slen, tab->size, false, false, &slen, &swidth);
      }
      add_justified(tab_di, swidth, slen, tab->pad, tab->size, tab->info,
                    tab->pad || (i < tab->nocols - 1) || tab->next);
    }
    if (done >= end - 1) {
      tab_di = nullptr;
    } else {
      tab_di += done + 1; /* inc'ed next line ... */
    }
    tab_d[i].cur = tab_di;
  }
  if (tab->pad) {
    while (i++ < tab->nocols) {
      add_pad(tab->pad, tab->size, true);
    }
    add_pad(tab->pad, tab->remainder, false);
  }
  if (!tab_d[0].cur) {
    cst *temp;

    temp = tab->next;
    if (tab->pad) {
      FREE(tab->pad);
    }
    if (tab_d) {
      FREE(tab_d);
    }
    FREE(tab);
    *table = temp;
    return 1;
  }
  return 0;
} /* end of add_table() */

static int get_curpos() {
  char *p1, *p2;
  if (!sprintf_state->obuff.buffer) {
    return 0;
  }
  p1 = sprintf_state->obuff.buffer + sprintf_state->obuff.real_size - 1;
  p2 = p1;
  while (p2 > sprintf_state->obuff.buffer && *p2 != '\n') {
    p2--;
  }

  if (*p2 != '\n') {
    return p1 - p2 + 1;
  } else {
    return p1 - p2;
  }
}

/* We can't use a pointer to a local in a table or column, since it
 * could get overwritten by another on the same line.
 */
static pad_info_t *make_pad(pad_info_t *p) {
  pad_info_t *x;
  if (p->len == 0) {
    return nullptr;
  }
  x = reinterpret_cast<pad_info_t *>(DMALLOC(sizeof(pad_info_t), TAG_TEMPORARY, "make_pad"));
  x->what = p->what;
  x->len = p->len;
  x->width = p->width;
  return x;
}

/*
 * THE (s)printf() function.
 * It returns a pointer to it's internal buffer (or a string in the text
 * segment) thus, the string must be copied if it has to survive after
 * this function is called again, or if it's going to be modified (esp.
 * if it risks being free()ed).
 */
char *string_print_formatted(const char *format_str, int argc, svalue_t *argv) {
  format_info finfo;
  svalue_t *carg;           /* current arg */
  unsigned int nelemno = 0; /* next offset into array */
  unsigned int fpos;        /* position in format_str */
  int fs;                   /* field size */
  int pres;                 /* precision */
  pad_info_t pad;           /* fs pad string */
  unsigned int i;
  char *retvalue;
  int last;

  push_sprintf_state();
  STACK_INC;
  sp->type = T_ERROR_HANDLER;
  sp->u.error_handler = pop_sprintf_state;

  last = 0;
  for (fpos = 0; true; fpos++) {
    char c = format_str[fpos];

    if (c == '\n' || !c) {
      int column_stat = 0;

      if (last != fpos) {
        add_nstr(format_str + last, fpos - last);
        last = fpos + 1;
      } else {
        last++;
      }

      if (!sprintf_state->csts) {
        if (!c) {
          break;
        }
        ADD_CHAR('\n');
        continue;
      }
      ADD_CHAR('\n');
      while (sprintf_state->csts) {
        cst **temp;

        temp = &(sprintf_state->csts);
        while (*temp) {
          if ((*temp)->info & INFO_COLS) {
            if (*((*temp)->d.col - 1) != '\n') {
              while (*((*temp)->d.col) == ' ') {
                (*temp)->d.col++;
              }
            }
            add_pad(nullptr, (*temp)->start - get_curpos(), true);
            column_stat = add_column(temp, 0);
            if (!column_stat) {
              temp = &((*temp)->next);
            }
          } else {
            add_pad(nullptr, (*temp)->start - get_curpos(), true);
            if (!add_table(temp)) {
              temp = &((*temp)->next);
            }
          }
        } /* of while (*temp) */
        if (sprintf_state->csts || c == '\n') {
          ADD_CHAR('\n');
        }
      } /* of while (sprintf_state->csts) */
      if (column_stat == 2) {
        ADD_CHAR('\n');
      }
      if (!c) {
        break;
      }
    } else if (c == '%') {
      if (last != fpos) {
        add_nstr(format_str + last, fpos - last);
        last = fpos + 1;
      } else {
        last++;
      }
      if (format_str[fpos + 1] == '%') {
        ADD_CHAR('%');
        fpos++;
        last++;
        continue;
      }
      GET_NEXT_ARG;
      fs = 0;
      pres = 0;
      pad.len = 0;
      finfo = 0;
      for (fpos++; !(finfo & INFO_T); fpos++) {
        if (!format_str[fpos]) {
          finfo |= INFO_T_ERROR;
          break;
        }
        if (((format_str[fpos] >= '0') && (format_str[fpos] <= '9')) || (format_str[fpos] == '*')) {
          if (pres == -1) { /* then looking for pres */
            if (format_str[fpos] == '*') {
              if (carg->type != T_NUMBER) {
                SPRINTF_ERROR(ERR_INVALID_STAR);
              }
              pres = carg->u.number;
              GET_NEXT_ARG;
              continue;
            }
            pres = format_str[fpos] - '0';
            for (fpos++; (format_str[fpos] >= '0') && (format_str[fpos] <= '9'); fpos++) {
              pres = pres * 10 + format_str[fpos] - '0';
            }
            if (pres < 0) {
              pres = 0;
            }
          } else { /* then is fs (and maybe pres) */
            if ((format_str[fpos] == '0') &&
                (((format_str[fpos + 1] >= '1') && (format_str[fpos + 1] <= '9')) ||
                 (format_str[fpos + 1] == '*'))) {
              pad.what = "0";
              pad.len = 1;
              pad.width = 1;
            } else {
              if (format_str[fpos] == '*') {
                if (carg->type != T_NUMBER) {
                  SPRINTF_ERROR(ERR_INVALID_STAR);
                }
                fs = carg->u.number;
                if (fs < 0) {
                  fs = 0;
                }
                if (pres == -2) {
                  pres = fs; /* colon */
                }
                GET_NEXT_ARG;
                continue;
              }
              fs = format_str[fpos] - '0';
            }
            for (fpos++; (format_str[fpos] >= '0') && (format_str[fpos] <= '9'); fpos++) {
              fs = fs * 10 + format_str[fpos] - '0';
            }
            if (fs < 0) {
              fs = 0;
            }
            if (pres == -2) { /* colon */
              pres = fs;
            }
          }
          fpos--; /* about to get incremented */
          continue;
        }
        switch (format_str[fpos]) {
          case ' ':
            finfo |= INFO_PP_SPACE;
            break;
          case '+':
            finfo |= INFO_PP_PLUS;
            break;
          case '-':
            finfo |= INFO_J_LEFT;
            break;
          case '|':
            finfo |= INFO_J_CENTRE;
            break;
          case '@':
            finfo |= INFO_ARRAY;
            break;
          case '=':
            finfo |= INFO_COLS;
            break;
          case '#':
            finfo |= INFO_TABLE;
            break;
          case '.':
            pres = -1;
            break;
          case ':':
            pres = -2;
            break;
#ifdef DEBUG
          case '%':
            finfo |= INFO_T_NULL;
            break; /* never reached */
#endif
          case 'O':
            finfo |= INFO_T_LPC;
            break;
          case 's':
            finfo |= INFO_T_STRING;
            break;
          case 'd':
          case 'i':
            finfo |= INFO_T_INT;
            break;
          case 'f':
            finfo |= INFO_T_FLOAT;
            break;
          case 'c':
            finfo |= INFO_T_CHAR;
            break;
          case 'o':
            finfo |= INFO_T_OCT;
            break;
          case 'x':
            finfo |= INFO_T_HEX;
            break;
          case 'X':
            finfo |= INFO_T_C_HEX;
            break;
          case '\'':
            fpos++;
            pad.what = format_str + fpos;
            pad.len = 0;
            pad.width = 0;  // need to handle '\\' characters, which should be ignored.
            while (true) {
              if (!format_str[fpos]) {
                SPRINTF_ERROR(ERR_UNEXPECTED_EOS);
              }
              if (format_str[fpos] == '\\') {
                char nextc = format_str[fpos + 1];
                if (!nextc) SPRINTF_ERROR(ERR_UNEXPECTED_EOS);
                if (nextc != '\'' && nextc != '\\') {
                  sprintf_error(
                      ERR_RECOVERY_ONLY,
                      "padding string '\\' must be followed by another '\\' or single quote");
                }
                fpos += 2;
                pad.len -= 1;
                pad.width -= 1;
                continue;
              } else if (format_str[fpos] == '\'') {
                pad.len += format_str + fpos - pad.what;
                if (!pad.len) {
                  SPRINTF_ERROR(ERR_NULL_PS);
                }
                break;
              }
              fpos++;
            }
            pad.width += u8_width(pad.what, pad.len);
            if (!pad.width) {
              SPRINTF_ERROR(ERR_NULL_PS);
            }
            break;
          default:
            finfo |= INFO_T_ERROR;
        }
      } /* end of for () */
      if (pres < 0) {
        SPRINTF_ERROR(ERR_PRES_EXPECTED);
      }
      /*
       * now handle the different arg types...
       */
      if (finfo & INFO_ARRAY) {
        if (carg->type != T_ARRAY) {
          SPRINTF_ERROR(ERR_ARRAY_EXPECTED);
        }
        if (carg->u.arr->size == 0) {
          last = fpos;
          fpos--; /* 'bout to get incremented */
          continue;
        }
        carg = (argv + sprintf_state->cur_arg)->u.arr->item;
        nelemno = 1; /* next element number */
      }
      while (true) {
        if ((finfo & INFO_T) == INFO_T_LPC) {
          outbuffer_t outbuf;

          outbuf_zero(&outbuf);
          svalue_to_string(carg, &outbuf, 0, 0, 0);
          outbuf_fix(&outbuf);

          sprintf_state->clean.type = T_STRING;
          sprintf_state->clean.subtype = STRING_MALLOC;
          sprintf_state->clean.u.string = outbuf.buffer;
          carg = &(sprintf_state->clean);
          finfo ^= INFO_T_LPC;
          finfo |= INFO_T_STRING;
        }
        if ((finfo & INFO_T) == INFO_T_ERROR) {
          SPRINTF_ERROR(ERR_INVALID_FORMAT_STR);
#ifdef DEBUG
        } else if ((finfo & INFO_T) == INFO_T_NULL) {
          /* never reached... */
          debug_message("/%s: (s)printf: INFO_T_NULL.... found.\n", current_object->obname);
          ADD_CHAR('%');
#endif
        } else if ((finfo & INFO_T) == INFO_T_STRING) {
          /*
           * %s null handling added 930709 by Luke Mewburn
           * <zak@rmit.oz.au>
           */
          if (carg->type == T_NUMBER && carg->u.number == 0) {
            sprintf_state->clean.type = T_STRING;
            sprintf_state->clean.subtype = STRING_MALLOC;
            sprintf_state->clean.u.string = string_copy("0", "sprintf NULL");
            carg = &(sprintf_state->clean);
          } else if (carg->type != T_STRING) {
            SPRINTF_ERROR(ERR_INCORRECT_ARG_S);
          }
          if ((finfo & INFO_COLS) || (finfo & INFO_TABLE)) {
            cst **temp;

            if (!fs) {
              SPRINTF_ERROR(ERR_CST_REQUIRES_FS);
            }

            temp = &(sprintf_state->csts);
            while (*temp) {
              temp = &((*temp)->next);
            }
            if (finfo & INFO_COLS) {
              int tmp;
              if (pres > fs) {
                fs = pres;
              }
              *temp =
                  reinterpret_cast<cst *>(DMALLOC(sizeof(cst), TAG_TEMPORARY, "string_print: 3"));
              (*temp)->next = nullptr;
              (*temp)->d.col = carg->u.string;
              (*temp)->pad = make_pad(&pad);
              (*temp)->size = fs;
              (*temp)->pres = (pres) ? pres : fs;
              (*temp)->info = finfo;
              (*temp)->start = get_curpos();

              tmp = ((format_str[fpos] != '\n') && (format_str[fpos] != '\0')) ||
                    ((finfo & INFO_ARRAY) &&
                     (nelemno < (argv + sprintf_state->cur_arg)->u.arr->size));
              tmp = add_column(temp, tmp);
              if (tmp == 2 && !format_str[fpos]) {
                ADD_CHAR('\n');
              }
            } else { /* (finfo & INFO_TABLE) */
              unsigned int n, items_per_column, max_width;
              const char *p1, *p2;

#define TABLE carg->u.string
              (*temp) =
                  reinterpret_cast<cst *>(DMALLOC(sizeof(cst), TAG_TEMPORARY, "string_print: 4"));
              (*temp)->d.tab = nullptr;
              (*temp)->pad = make_pad(&pad);
              (*temp)->info = finfo;
              (*temp)->start = get_curpos();
              (*temp)->next = nullptr;
              max_width = 0;
              n = 1;

              p2 = p1 = TABLE;
              // calculate number of items, also find out max width for each column.
              while (*p1) {
                if (*p1 == '\n') {
                  if (!pres) {
                    auto new_max_width = u8_width(p2, p1 - p2);
                    if (new_max_width > max_width) {
                      max_width = new_max_width;
                    }
                  }
                  p1++;
                  if (*(p2 = p1)) {
                    n++;
                  }
                } else {
                  p1++;
                }
              }
              /* the null terminated word */
              if (!pres) {
                auto new_max_width = u8_width(p2, p1 - p2);
                if (new_max_width > max_width) {
                  max_width = new_max_width;
                }
              }
              if (!pres) {
                pres = fs / (max_width + 2); /* at least two
                                              * separating spaces */
                if (!pres) {
                  pres = 1;
                }

                /* This moves some entries from the right side
                 * of the table to fill out the last line,
                 * which makes the table look a bit nicer.
                 * E.g.
                 * (n=13,p=6)      (l=3,p=5)
                 * X X X X X X     X X X X X
                 * X X X X X X  -> X X X X X
                 * X               X X X X
                 *
                 */
                items_per_column = (n - 1) / pres + 1;
                if (n > pres && n % pres) {
                  pres -= (pres - n % pres) / items_per_column;
                }
              } else {
                items_per_column = (n - 1) / pres + 1;
              }
              (*temp)->size = fs / pres;
              (*temp)->remainder = fs % pres;
              if (n < pres) {
                /* If we have fewer elements than columns,
                 * pretend we are dealing with a smaller
                 * table.
                 */
                (*temp)->remainder += (pres - n) * ((*temp)->size);
                pres = n;
              }

              (*temp)->d.tab = reinterpret_cast<tab_data_t *>(
                  DCALLOC(pres + 1, sizeof(tab_data_t), TAG_TEMPORARY, "string_print: 5"));
              (*temp)->nocols = pres; /* heavy sigh */
              (*temp)->d.tab[0].start = TABLE;
              if (pres == 1) {
                (*temp)->d.tab[1].start = TABLE + SVALUE_STRLEN(carg) + 1;
              } else {
                i = 1; /* the next column number */
                n = 0; /* the current "word" number in this
                        * column */

                p1 = TABLE;
                while (*p1) {
                  if (*p1++ == '\n' && ++n >= items_per_column) {
                    (*temp)->d.tab[i++].start = p1;
                    n = 0;
                  }
                }
                for (; i <= pres; i++) {
                  (*temp)->d.tab[i].start = ++p1;
                }
              }
              for (i = 0; i < pres; i++) {
                (*temp)->d.tab[i].cur = (*temp)->d.tab[i].start;
              }

              add_table(temp);
            }
          } else { /* not column or table */
            size_t slen = SVALUE_STRLEN(carg);
            size_t swidth;
            if (pres) {
              u8_truncate_below_width(carg->u.string, slen, pres, false, false, &slen, &swidth);
            } else {
              swidth = u8_width(carg->u.string, -1);
            }
            add_justified(carg->u.string, swidth, slen, &pad, fs, finfo,
                          (((format_str[fpos] != '\n') && (format_str[fpos] != '\0')) ||
                           ((finfo & INFO_ARRAY) &&
                            (nelemno < (argv + sprintf_state->cur_arg)->u.arr->size))) ||
                              slen == 0 || (slen && carg->u.string[slen - 1] != '\n'));
          }
        } else if ((finfo & INFO_T) == INFO_T_CHAR) {
          if (carg->type != T_NUMBER) {
            SPRINTF_ERROR(ERR_INVALID_ARG_C);
          }
          /* write UTF8 codepoint */
          char temp[4 + 1] = {0};
          UChar32 c = carg->u.number;

          if (c == 0 || !u_isdefined(c)) {
            SPRINTF_ERROR(ERR_INVALID_ARG_C);
          }
          size_t offset = 0;
          U8_APPEND_UNSAFE(temp, offset, c);
          temp[4] = 0;

          int tmpl = strlen(temp);
          int swidth = u8_width(temp, -1);
          add_justified(
              temp, swidth, tmpl, &pad, fs, finfo,
              (((format_str[fpos] != '\n') && (format_str[fpos] != '\0')) ||
               ((finfo & INFO_ARRAY) && (nelemno < (argv + sprintf_state->cur_arg)->u.arr->size))));
        } else if (finfo & INFO_T_INT) {
          /* one of the integer
           * types */
          char cheat[40];
          char temp[400];

          *cheat = '%';
          i = 1;
          switch (finfo & INFO_PP) {
            case INFO_PP_SPACE:
              cheat[i++] = ' ';
              break;
            case INFO_PP_PLUS:
              cheat[i++] = '+';
              break;
          }
          if (pres) {
            cheat[i++] = '.';
            if (pres >= sizeof(temp)) {
              sprintf(cheat + i, "%zu", sizeof(temp) - 1);
            } else {
              sprintf(cheat + i, "%d", pres);
            }

            i += strlen(cheat + i);
          }

          switch (finfo & INFO_T) {
            case INFO_T_INT:
              /* make sure to print out at least 64bits. */
              cheat[i++] = 'l';
              cheat[i++] = 'l';
              cheat[i++] = 'd';
              break;
            case INFO_T_FLOAT:
              /* make sure to print out at least 64bits. */
              cheat[i++] = 'l';
              cheat[i++] = 'f';
              break;
            case INFO_T_OCT:
              cheat[i++] = 'l';
              cheat[i++] = 'l';
              cheat[i++] = 'o';
              break;
            case INFO_T_HEX:
              cheat[i++] = 'l';
              cheat[i++] = 'l';
              cheat[i++] = 'x';
              break;
            case INFO_T_C_HEX:
              cheat[i++] = 'l';
              cheat[i++] = 'l';
              cheat[i++] = 'X';
              break;
            default:
              SPRINTF_ERROR(ERR_BAD_INT_TYPE);
          }
          if ((cheat[i - 1] == 'f' && carg->type != T_REAL) ||
              (cheat[i - 1] != 'f' && carg->type != T_NUMBER)) {
#ifdef RETURN_ERROR_MESSAGES
            sprintf(buff,
                    "ERROR: (s)printf(): Incorrect argument type to %%%c. "
                    "(arg: %u)\n",
                    cheat[i - 1], sprintf_state->cur_arg);
            debug_message("Program /%s File: %s: %s", current_prog->name, get_line_number_if_any(),
                          buff);
            debug_message("%s", buff);
            if (current_object) {
              debug_message("program: /%s, object: %s, file: %s\n",
                            current_prog ? current_prog->name : "", current_object->name,
                            get_line_number_if_any());
            }
            ERROR(ERR_RECOVERY_ONLY);
#else
            error("ERROR: (s)printf(): Incorrect argument type to %%%c.\n", cheat[i - 1]);
#endif /* RETURN_ERROR_MESSAGES */
          }
          cheat[i] = '\0';

          if (carg->type == T_REAL) {
            sprintf(temp, cheat, carg->u.real);
          } else {
            sprintf(temp, cheat, carg->u.number);
          }
          {
            int tmpl = strlen(temp);
            int swidth = u8_width(temp, -1);

            add_justified(temp, swidth, tmpl, &pad, fs, finfo,
                          (((format_str[fpos] != '\n') && (format_str[fpos] != '\0')) ||
                           ((finfo & INFO_ARRAY) &&
                            (nelemno < (argv + sprintf_state->cur_arg)->u.arr->size))));
          }
        } else { /* type not found */
          SPRINTF_ERROR(ERR_UNDEFINED_TYPE);
        }
        if (sprintf_state->clean.type != T_NUMBER) {
          free_svalue(&(sprintf_state->clean), "string_print_formatted");
          sprintf_state->clean.type = T_NUMBER;
        }

        if (!(finfo & INFO_ARRAY)) {
          break;
        }
        if (nelemno >= (argv + sprintf_state->cur_arg)->u.arr->size) {
          break;
        }
        carg = (argv + sprintf_state->cur_arg)->u.arr->item + nelemno++;
      } /* end of while (1) */
      last = fpos;
      fpos--; /* bout to get incremented */
    }
  } /* end of for (fpos=0; 1; fpos++) */

  outbuf_fix(&sprintf_state->obuff);
  retvalue = sprintf_state->obuff.buffer;
  sprintf_state->obuff.buffer = nullptr;
  pop_stack(); /* pop off our error handler, will call pop_sprintf_state */
  return retvalue;
} /* end of string_print_formatted() */

#endif /* defined(F_SPRINTF) || defined(F_PRINTF) */
