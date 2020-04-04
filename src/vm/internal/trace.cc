#include "base/std.h"

#include "vm/internal/trace.h"

#include "vm/internal/base/interpret.h"
#include "vm/internal/base/machine.h"

// FIXME: for svalue_to_string
#include "packages/core/sprintf.h"

// Dump a stack trace at current location.

/* The end of a static buffer */
#define EndOf(x) (x + sizeof(x) / sizeof(x[0]))

namespace {

void get_trace_details(const program_t *prog, long findex, char **fname, int *na, int *nl) {
  function_t *cfp = &prog->function_table[findex];

  *fname = cfp->funcname;
  *na = cfp->num_arg;
  *nl = cfp->num_local;
}

void dump_trace_line(const char *fname, const char *pname, const char *const obname, char *where) {
  char line[256];
  char *end = EndOf(line);
  char *p;

  p = strput(line, end, "Object: ");
  if (obname[0] != '<' && p < end) {
    *p++ = '/';
  }
  p = strput(p, end, obname);
  p = strput(p, end, ", Program: ");
  if (pname[0] != '<' && p < end) {
    *p++ = '/';
  }
  p = strput(p, end, pname);
  p = strput(p, end, "\n   in ");
  p = strput(p, end, fname);
  p = strput(p, end, "() at ");
  p = strput(p, end, where);
  p = strput(p, end, "\n");
  debug_message(line);
}

}  // namespace

const char *dump_trace(int how) {
  control_stack_t *p;
  const char *ret = nullptr;
  int num_arg = -1, num_local = -1;

  int i;

  if (current_prog == nullptr) {
    return nullptr;
  }
  if (csp < &control_stack[0]) {
    return nullptr;
  }
  if (how) {
    last_instructions();
  }
  debug_message("--- trace ---\n");
  for (p = &control_stack[0]; p <= csp; p++) {
    struct program_t *trace_prog;
    struct object_t *trace_obj;
    char *trace_pc;
    struct svalue_t *trace_fp;

    if (p == csp) {
      trace_prog = current_prog;
      trace_obj = current_object;
      trace_pc = pc;
      trace_fp = fp;
    } else {
      trace_prog = p[1].prog;
      trace_obj = p[1].ob;
      trace_pc = p[1].pc;
      trace_fp = p[1].fp;
    }

    switch (p[0].framekind & FRAME_MASK) {
      case FRAME_FUNCTION: {
        char *fname;
        get_trace_details(trace_prog, p[0].fr.table_index, &fname, &num_arg, &num_local);
        dump_trace_line(fname, trace_prog->filename, trace_obj->obname,
                        get_line_number(trace_pc, trace_prog));
        if (strcmp(fname, "heart_beat") == 0) {
          ret = p->ob ? p->ob->obname : nullptr;
        }
        break;
      }
      case FRAME_FUNP: {
        outbuffer_t tmpbuf;
        svalue_t tmpval;

        tmpbuf.real_size = 0;
        tmpbuf.buffer = nullptr;

        tmpval.type = T_FUNCTION;
        tmpval.u.fp = p[0].fr.funp;

        svalue_to_string(&tmpval, &tmpbuf, 0, 0, 0);

        dump_trace_line(tmpbuf.buffer, trace_prog->filename, trace_obj->obname,
                        get_line_number(trace_pc, trace_prog));

        FREE_MSTR(tmpbuf.buffer);
        num_arg = p[0].fr.funp->f.functional.num_arg;
        num_local = p[0].fr.funp->f.functional.num_local;
      } break;
      case FRAME_FAKE:
        dump_trace_line("<fake>", trace_prog->filename, trace_obj->obname,
                        get_line_number(trace_pc, trace_prog));
        num_arg = -1;
        break;
      case FRAME_CATCH:
        dump_trace_line("<catch>", trace_prog->filename, trace_obj->obname,
                        get_line_number(trace_pc, trace_prog));
        num_arg = -1;
        break;
      default:
#ifdef DEBUG
        fatal("unknown type of frame\n");
#else
        debug_message("unknown type of frame\n");
#endif
        break;
    }
    if (num_arg > 0) {
      debug_message("arguments: [");
      for (i = 0; i < num_arg; i++) {
        outbuffer_t outbuf;

        if (i) {
          debug_message(", ");
        }
        outbuf_zero(&outbuf);
        svalue_to_string(&trace_fp[i], &outbuf, 0, 0, 0);
        /* don't need to fix length here */
        debug_message("%s", outbuf.buffer);
        FREE_MSTR(outbuf.buffer);
      }
      debug_message("]\n");
    }
    if (num_local > 0 && num_arg != -1) {
      struct svalue_t *ptr = trace_fp + num_arg;
      debug_message("locals: [");
      for (i = 0; i < num_local; i++) {
        outbuffer_t outbuf;

        if (i) {
          debug_message(", ");
        }
        outbuf_zero(&outbuf);
        svalue_to_string(&ptr[i], &outbuf, 0, 0, 0);
        /* no need to fix length */
        debug_message("%s", outbuf.buffer);
        FREE_MSTR(outbuf.buffer);
      }
      debug_message("]\n");
    }
  }
  debug_message("--- end trace ---\n");
  return ret;
}

array_t *get_svalue_trace() {
  control_stack_t *p;
  array_t *v;
  mapping_t *m;
  const char *file;
  int line;
  char *fname;
  int num_arg, num_local = -1;

  svalue_t *ptr;
  int i;

  if (current_prog == nullptr) {
    return &the_null_array;
  }
  if (csp < &control_stack[0]) {
    return &the_null_array;
  }
  v = allocate_empty_array((csp - &control_stack[0]) + 1);
  for (p = &control_stack[0]; p < csp; p++) {
    m = allocate_mapping(6);
    switch (p[0].framekind & FRAME_MASK) {
      case FRAME_FUNCTION:
        get_trace_details(p[1].prog, p[0].fr.table_index, &fname, &num_arg, &num_local);
        add_mapping_string(m, "function", fname);
        break;
      case FRAME_CATCH:
        add_mapping_string(m, "function", "CATCH");
        num_arg = -1;
        break;
      case FRAME_FAKE:
        add_mapping_string(m, "function", "<fake>");
        num_arg = -1;
        break;
      case FRAME_FUNP: {
        outbuffer_t tmpbuf;
        svalue_t tmpval;

        tmpbuf.real_size = 0;
        tmpbuf.buffer = nullptr;

        tmpval.type = T_FUNCTION;
        tmpval.u.fp = p[0].fr.funp;

        svalue_to_string(&tmpval, &tmpbuf, 0, 0, 0);

        add_mapping_string(m, "function", tmpbuf.buffer);
        FREE_MSTR(tmpbuf.buffer);
        num_arg = p[0].fr.funp->f.functional.num_arg;
        num_local = p[0].fr.funp->f.functional.num_local;
      } break;
#ifdef DEBUG
      default:
        fatal("unknown type of frame\n");
#endif
    }
    add_mapping_malloced_string(m, "program", add_slash(p[1].prog->filename));
    add_mapping_object(m, "object", p[1].ob);
    get_explicit_line_number_info(p[1].pc, p[1].prog, &file, &line);
    add_mapping_malloced_string(m, "file", add_slash(file));
    add_mapping_pair(m, "line", line);
    if (num_arg != -1) {
      array_t *v2;

      ptr = p[1].fp;
      v2 = allocate_empty_array(num_arg);
      for (i = 0; i < num_arg; i++) {
        assign_svalue_no_free(&v2->item[i], &ptr[i]);
      }
      add_mapping_array(m, "arguments", v2);
      v2->ref--;
    }
    if (num_local > 0 && num_arg != -1) {
      array_t *v2;

      ptr = p[1].fp + num_arg;
      v2 = allocate_empty_array(num_local);
      for (i = 0; i < num_local; i++) {
        assign_svalue_no_free(&v2->item[i], &ptr[i]);
      }
      add_mapping_array(m, "locals", v2);
      v2->ref--;
    }
    v->item[(p - &control_stack[0])].type = T_MAPPING;
    v->item[(p - &control_stack[0])].u.map = m;
  }
  m = allocate_mapping(6);
  switch (p[0].framekind & FRAME_MASK) {
    case FRAME_FUNCTION:
      get_trace_details(current_prog, p[0].fr.table_index, &fname, &num_arg, &num_local);
      add_mapping_string(m, "function", fname);
      break;
    case FRAME_CATCH:
      add_mapping_string(m, "function", "CATCH");
      num_arg = -1;
      break;
    case FRAME_FAKE:
      add_mapping_string(m, "function", "<fake>");
      num_arg = -1;
      break;
    case FRAME_FUNP: {
      outbuffer_t tmpbuf;
      svalue_t tmpval;

      tmpbuf.real_size = 0;
      tmpbuf.buffer = nullptr;

      tmpval.type = T_FUNCTION;
      tmpval.u.fp = p[0].fr.funp;

      svalue_to_string(&tmpval, &tmpbuf, 0, 0, 0);
      add_mapping_string(m, "function", tmpbuf.buffer);
      FREE_MSTR(tmpbuf.buffer);
      num_arg = p[0].fr.funp->f.functional.num_arg;
      num_local = p[0].fr.funp->f.functional.num_local;
    } break;
  }
  add_mapping_malloced_string(m, "program", add_slash(current_prog->filename));
  add_mapping_object(m, "object", current_object);
  get_line_number_info(&file, &line);
  add_mapping_malloced_string(m, "file", add_slash(file));
  add_mapping_pair(m, "line", line);
  if (num_arg > 0) {
    array_t *v2;

    v2 = allocate_empty_array(num_arg);
    for (i = 0; i < num_arg; i++) {
      assign_svalue_no_free(&v2->item[i], &fp[i]);
    }
    add_mapping_array(m, "arguments", v2);
    v2->ref--;
  }
  if (num_local > 0 && num_arg != -1) {
    array_t *v2;

    v2 = allocate_empty_array(num_local);
    for (i = 0; i < num_local; i++) {
      assign_svalue_no_free(&v2->item[i], &fp[i + num_arg]);
    }
    add_mapping_array(m, "locals", v2);
    v2->ref--;
  }
  v->item[(csp - &control_stack[0])].type = T_MAPPING;
  v->item[(csp - &control_stack[0])].u.map = m;
  /* return a reference zero array */
  v->ref--;
  return v;
}
