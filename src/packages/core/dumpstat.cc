#include "base/package_api.h"

#include "packages/core/file.h"  // for check_valid_path

/*
 * Write statistics about objects on file.
 */

struct SvalueSizeCtx {
  int* total;
  int* depth;
};

static int sumSizes(mapping_t* /*m*/, mapping_node_t* /*elt*/, void* /*tp*/);
static int svalue_size(svalue_t* /*v*/, int* /*depth*/);

static int sumSizes(mapping_t* /*m*/, mapping_node_t* elt, void* tp) {
  auto* ctx = static_cast<SvalueSizeCtx*>(tp);

  *ctx->total += (svalue_size(&elt->values[0], ctx->depth) + svalue_size(&elt->values[1], ctx->depth));
  *ctx->total += sizeof(mapping_node_t);
  return 0;
}

static int svalue_size(svalue_t* v, int* depth) {
  int i, total;

  switch (v->type) {
    case T_OBJECT:
    case T_REAL:
    case T_NUMBER:
      return 0;
    case T_STRING:
      return (strlen(v->u.string) + 1);
    case T_ARRAY:
    case T_CLASS:
      if (++*depth > 100) {
        return 0;
      }

      /* first svalue is stored inside the array struct */
      total = sizeof(array_t) - sizeof(svalue_t);
      for (i = 0; i < v->u.arr->size; i++) {
        total += svalue_size(&v->u.arr->item[i], depth) + sizeof(svalue_t);
      }
      (*depth)--;
      return total;
    case T_MAPPING: {
      if (++*depth > 100) {
        return 0;
      }
      total = sizeof(mapping_t);
      SvalueSizeCtx ctx{&total, depth};
      mapTraverse(v->u.map, sumSizes, &ctx);
      (*depth)--;
      return total;
    }
    case T_FUNCTION: {
      svalue_t tmp;
      tmp.type = T_ARRAY;
      tmp.u.arr = v->u.fp->hdr.args;

      if (++*depth > 100) {
        return 0;
      }

      if (tmp.u.arr) {
        total = sizeof(funptr_hdr_t) + svalue_size(&tmp, depth);
      } else {
        total = sizeof(funptr_hdr_t);
      }
      switch (v->u.fp->hdr.type) {
        case FP_EFUN:
          total += sizeof(efun_ptr_t);
          break;
        case FP_LOCAL | FP_NOT_BINDABLE:
          total += sizeof(local_ptr_t);
          break;
        case FP_SIMUL:
          total += sizeof(simul_ptr_t);
          break;
        case FP_FUNCTIONAL:
        case FP_FUNCTIONAL | FP_NOT_BINDABLE:
          total += sizeof(functional_t);
          break;
      }

      (*depth)--;
      return total;
    }
    case T_BUFFER:
      /* first byte is stored inside the buffer struct */
      return sizeof(buffer_t) + v->u.buf->size - 1;
    case T_PROMISE: {
      if (++*depth > 100) {
        return 0;
      }
      total = sizeof(promise_t) + svalue_size(&v->u.prom->result, depth);
      if (v->u.prom->reactions) {
        total += v->u.prom->reactions->size() * sizeof(promise_reaction_t);
      }
      (*depth)--;
      return total;
    }
    default:
        // some freed value or a reference (!) to one (in all my test cases
        // anyway), it will be removed by reclaim_objects later, Wodan
        // fatal("Illegal type: %d\n", v->type);
        ;
  }
  /* NOTREACHED */
  return 0;
}

int data_size(object_t* ob) {
  int total = 0, i;

  if (ob->prog) {
    for (i = 0; i < ob->prog->num_variables_total; i++) {
      int depth = 0;
      total += svalue_size(&ob->variables[i], &depth) + sizeof(svalue_t);
    }
  }
  return total;
}

void dumpstat(const char* tfn) {
  FILE* f;
  object_t* ob;
  const char* fn;
#ifdef F_SET_HIDE
  int display_hidden;
#endif

  vm::Value pathv = check_valid_path(tfn, current_object, "dumpallobj", 1);
  if (!pathv.is(T_STRING)) {
    error("Invalid path '/%s' for writing.\n", tfn);
    return;
  }
  fn = pathv->u.string;
  f = fopen(fn, "w");
  if (!f) {
    error("Unable to open '/%s' for writing.\n", fn);
    return;
  }

#ifdef F_SET_HIDE
  display_hidden = -1;
#endif
  for (ob = obj_list; ob; ob = ob->next_all) {
    int tmp;

#ifdef F_SET_HIDE
    if (ob->flags & O_HIDDEN) {
      if (display_hidden == -1) {
        display_hidden = valid_hide(current_object);
      }
      if (!display_hidden) {
        continue;
      }
    }
#endif
    /* FIXME */
    if (ob->prog && (ob->prog->ref == 1 || !(ob->flags & O_CLONE))) {
      tmp = ob->prog->total_size;
    } else {
      tmp = 0;
    }
    fprintf(f, "%-20s %zu ref %2d %s %s (%d)\n", ob->obname, tmp + data_size(ob) + sizeof(object_t),
            ob->ref, ob->flags & O_HEART_BEAT ? "HB" : "  ",
#ifndef NO_ENVIRONMENT
            ob->super ? ob->super->obname : "--",
#else
            "--",
#endif
            /* ob->cpu */ 0);
  }
  fclose(f);
}

#ifdef F_DUMPALLOBJ
void f_dumpallobj(vm::EfunArgs&, std::optional<vm::StringArg> file) {
  if (file) {
    dumpstat(file->c_str());
  } else {
    dumpstat("/OBJ_DUMP");
  }
}
#endif
