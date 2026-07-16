// Reference-loop (cycle) introspection efuns: has_cycle(), find_cycles(),
// break_cycles().
//
// The VM reclaims compound values by pure reference counting with no cycle
// collector, so a structure that reaches itself leaks permanently once the
// last outside reference is dropped (see docs/concepts/general/
// reference_loops.md). These efuns give the mudlib tools to find such loops
// and to break them safely.
//
// All three share one traversal: an ITERATIVE depth-first search (explicit
// heap stack -- mudlib data can nest arbitrarily deep, so no C-stack
// recursion and no MAX_SAVE_SVALUE_DEPTH cap) over the value graph, with the
// classic white/grey/black coloring. An edge whose target is grey (still
// open on the current DFS path) is a back-edge; a directed graph is acyclic
// iff its DFS finds no back-edges, so breaking exactly the back-edges is the
// minimal edit that makes a value loop-free while leaving all other sharing
// (DAG edges to already-finished nodes) untouched.
//
// Traversed edges: array/class items, mapping keys AND values, and a
// function pointer's captured argument list (fp->hdr.args). Objects are
// deliberately leaves: a loop routed through an object's global variables is
// the destruct()-managed kind (destruct2() zeroes the variable block), and a
// value walk has no business reading other objects' variables.
// NOTE: this edge set must stay in sync with each_child() in
// src/packages/develop/checkmemory.cc (md_scan_orphaned_cycles), or these
// efuns and the Debug orphan scan will disagree about what a loop is.
//
// The walk itself never mutates the graph and never frees anything, so the
// raw pointers used as color-map keys stay valid for the whole walk.
// break_cycles() records the slots to clear during the walk and applies them
// in a post-pass; see the ordering comments in f_break_cycles().

#include "base/package_api.h"

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

#if defined(F_HAS_CYCLE) || defined(F_FIND_CYCLES) || defined(F_BREAK_CYCLES)

namespace {

enum : char { COLOR_GREY = 1, COLOR_BLACK = 2 };

enum WalkMode { WALK_DETECT, WALK_FIND, WALK_BREAK };

// Which kind of slot a back-edge sits in decides how break_cycles() clears
// it: item/value slots are overwritten with 0 in place; a back-edge in
// mapping-KEY position cannot be overwritten (the node is hashed by the
// key's pointer, and a second zero key could collide), so the whole node is
// deleted instead; a back-edge on the funptr->args edge (possible because
// bind() SHARES the args array between the old and new funptr) has no
// svalue slot at all, so the funptr's args list is replaced with a
// zero-filled one of the same size.
enum SlotKind { SLOT_ITEM, SLOT_MAP_VALUE, SLOT_MAP_KEY, SLOT_FP_ARGS };

struct PendingFix {
  SlotKind kind;
  svalue_t container;  // owns a reference until the post-pass is done
  svalue_t* slot;      // ITEM/MAP_VALUE: slot to zero; MAP_KEY: &node->values[0];
                       // FP_ARGS: null (container is the owning funptr)
};

struct Frame {
  unsigned short type;  // T_ARRAY, T_CLASS, T_MAPPING, or T_FUNCTION
  union {
    array_t* arr;
    mapping_t* map;
    funptr_t* fp;
    void* ptr;
  } u;
  int idx;               // next item (array/class), current bucket (mapping),
                         // 0 = args pending / 1 = done (function)
  mapping_node_t* node;  // current node within the bucket (mapping)
  int phase;             // 0 = key next, 1 = value next (mapping)
};

struct WalkResult {
  bool found = false;
  std::vector<std::string> paths;  // WALK_FIND
  std::vector<PendingFix> fixes;   // WALK_BREAK
};

void* compound_ptr(const svalue_t* sv) {
  switch (sv->type) {
    case T_ARRAY:
    case T_CLASS:
      return reinterpret_cast<void*>(sv->u.arr);
    case T_MAPPING:
      return reinterpret_cast<void*>(sv->u.map);
    case T_FUNCTION:
      return reinterpret_cast<void*>(sv->u.fp);
  }
  return nullptr;
}

// Human-readable rendering of a mapping key for find_cycles() paths.
std::string render_key(const svalue_t* key) {
  switch (key->type) {
    case T_NUMBER:
      return std::to_string(key->u.number);
    case T_REAL: {
      char buf[64];
      snprintf(buf, sizeof(buf), "%g", key->u.real);
      return buf;
    }
    case T_STRING: {
      size_t len = SVALUE_STRLEN(key);
      if (len > 32) {
        len = u8_truncate(reinterpret_cast<const uint8_t*>(key->u.string), 32);
      }
      return "\"" + std::string(key->u.string, len) + "\"";
    }
    case T_OBJECT:
      return std::string("OBJ(") + key->u.ob->obname + ")";
    case T_ARRAY:
      return "<array>";
    case T_CLASS:
      return "<class>";
    case T_MAPPING:
      return "<mapping>";
    case T_FUNCTION:
      return "<function>";
    default:
      return "<...>";
  }
}

Frame make_frame(unsigned short type, void* ptr) {
  Frame f{};
  f.type = type;
  f.u.ptr = ptr;
  if (type == T_MAPPING) {
    f.node = reinterpret_cast<mapping_t*>(ptr)->table[0];
  }
  return f;
}

void cycle_walk(svalue_t* root, WalkMode mode, WalkResult* res) {
  void* rptr = compound_ptr(root);
  if (rptr == nullptr) {
    return;
  }

  std::unordered_map<void*, char> color;
  std::vector<Frame> stack;
  std::vector<std::string> labels;  // edge label per open frame below the root

  color[rptr] = COLOR_GREY;
  stack.push_back(make_frame(root->type, rptr));

  // Examine one outgoing edge of the frame on top of the stack. `slot` is
  // null only for the funptr->args edge (kind SLOT_FP_ARGS), which is not an
  // svalue slot; it CAN be a back-edge target, because bind() shares the
  // args array between the old and the new funptr (f_bind in
  // packages/core/efuns_main.cc bumps args->ref instead of copying).
  auto handle_edge = [&](unsigned short ttype, void* tptr, svalue_t* slot, SlotKind kind,
                         std::string&& label) {
    auto ins = color.try_emplace(tptr, COLOR_GREY);
    if (ins.second) {  // white: tree edge, descend
      Frame child = make_frame(ttype, tptr);
      stack.push_back(child);  // may invalidate refs into `stack`
      if (mode == WALK_FIND) {
        labels.push_back(std::move(label));
      }
      return;
    }
    if (ins.first->second == COLOR_GREY) {  // back-edge: a cycle
      res->found = true;
      if (mode == WALK_FIND) {
        std::string path;
        for (const auto& l : labels) {
          path += l;
        }
        path += label;
        res->paths.push_back(std::move(path));
      } else if (mode == WALK_BREAK) {
        PendingFix fix;
        fix.kind = kind;
        fix.slot = slot;
        // Hold a reference on the slot's container (for SLOT_FP_ARGS, the
        // owning funptr) so the post-pass can touch it no matter what
        // earlier fixes deallocated.
        const Frame& owner = stack.back();
        svalue_t tmp;
        tmp.type = owner.type;
        tmp.subtype = 0;
        tmp.u.arr = reinterpret_cast<array_t*>(owner.u.ptr);
        assign_svalue_no_free(&fix.container, &tmp);
        res->fixes.push_back(fix);
      }
    }
    // black: already fully explored, nothing new reachable
  };

  while (!stack.empty()) {
    if (mode == WALK_DETECT && res->found) {
      return;
    }

    Frame& f = stack.back();
    svalue_t* slot = nullptr;
    SlotKind kind = SLOT_ITEM;
    std::string label;
    bool have_edge = false;

    switch (f.type) {
      case T_ARRAY:
      case T_CLASS: {
        array_t* arr = f.u.arr;
        if (f.idx < arr->size) {
          slot = &arr->item[f.idx];
          kind = SLOT_ITEM;
          if (mode == WALK_FIND) {
            label = (f.type == T_ARRAY) ? "[" + std::to_string(f.idx) + "]"
                                        : "." + std::to_string(f.idx);
          }
          f.idx++;
          have_edge = true;
        }
        break;
      }
      case T_MAPPING: {
        mapping_t* map = f.u.map;
        // table_size is a mask: valid buckets are 0 .. table_size inclusive
        while (f.node == nullptr && f.idx < static_cast<int>(map->table_size)) {
          f.idx++;
          f.node = map->table[f.idx];
        }
        if (f.node != nullptr) {
          if (f.phase == 0) {
            slot = &f.node->values[0];
            kind = SLOT_MAP_KEY;
            if (mode == WALK_FIND) {
              label = "[key " + render_key(slot) + "]";
            }
            f.phase = 1;
          } else {
            slot = &f.node->values[1];
            kind = SLOT_MAP_VALUE;
            if (mode == WALK_FIND) {
              label = "[" + render_key(&f.node->values[0]) + "]";
            }
            f.node = f.node->next;
            f.phase = 0;
          }
          have_edge = true;
        }
        break;
      }
      case T_FUNCTION: {
        funptr_t* fp = f.u.fp;
        if (f.idx == 0) {
          f.idx = 1;
          if (fp->hdr.args != nullptr) {
            handle_edge(T_ARRAY, reinterpret_cast<void*>(fp->hdr.args), nullptr, SLOT_FP_ARGS,
                        mode == WALK_FIND ? std::string("(args)") : std::string());
            continue;  // `f` may be a stale reference now
          }
        }
        break;
      }
    }

    if (!have_edge) {  // frame exhausted: close it
      color[f.u.ptr] = COLOR_BLACK;
      stack.pop_back();
      if (mode == WALK_FIND && !stack.empty()) {
        labels.pop_back();  // the root frame never pushed a label
      }
      continue;
    }

    void* tptr = compound_ptr(slot);
    if (tptr != nullptr) {
      handle_edge(slot->type, tptr, slot, kind, std::move(label));
      // `f` may be a stale reference now; loop back around
    }
  }
}

}  // namespace

#endif  // any of the three efuns

#ifdef F_HAS_CYCLE
void f_has_cycle() {
  WalkResult res;
  cycle_walk(sp, WALK_DETECT, &res);
  free_svalue(sp, "f_has_cycle");
  put_number(res.found ? 1 : 0);
}
#endif

#ifdef F_FIND_CYCLES
void f_find_cycles() {
  WalkResult res;
  cycle_walk(sp, WALK_FIND, &res);
  // May error("Illegal array size") if there are more back-edges than
  // max_array_size; at this point nothing is held, so that unwinds cleanly
  // (the std::vectors are reclaimed by their destructors).
  array_t* ret = allocate_empty_array(static_cast<int>(res.paths.size()));
  for (size_t i = 0; i < res.paths.size(); i++) {
    ret->item[i].type = T_STRING;
    ret->item[i].subtype = STRING_MALLOC;
    ret->item[i].u.string = string_copy(res.paths[i].c_str(), "f_find_cycles");
  }
  free_svalue(sp, "f_find_cycles");
  put_array(ret);
}
#endif

#ifdef F_BREAK_CYCLES
void f_break_cycles() {
  WalkResult res;
  cycle_walk(sp, WALK_BREAK, &res);

  // The temporary container references taken during the walk are released
  // by RAII, not a trailing loop: allocate_array() below can error() (LPC
  // can shrink __MAX_ARRAY_SIZE__ at runtime via set_config(), making a
  // same-size replacement illegal), and an unwind that skipped a manual
  // release loop would leak every held reference (AGENTS.md section 4).
  struct ContainerRelease {
    std::vector<PendingFix>* fixes;
    ~ContainerRelease() {
      for (auto& fix : *fixes) {
        free_svalue(&fix.container, "f_break_cycles");
      }
    }
  } release_guard{&res.fixes};

  // Apply the recorded fixes. Order matters:
  //
  // 1. Overwrite item/value back-edge slots with 0 first, and detach the
  //    args list of any funptr whose args EDGE is the back-edge (possible
  //    via bind()'s args sharing) -- replaced with a zero-filled list of
  //    the same size so the funptr keeps its call arity. Neither can ever
  //    cascade a deallocation: every back-edge target is an ancestor on the
  //    walk's tree path, and that path -- made of tree edges, which we never
  //    break -- anchors it to the root the caller still holds on the VM
  //    stack.
  LPC_INT broken = 0;
  for (auto& fix : res.fixes) {
    if (fix.kind == SLOT_FP_ARGS) {
      funptr_t* fp = fix.container.u.fp;
      array_t* old = fp->hdr.args;
      if (old != nullptr) {
        fp->hdr.args = allocate_array(old->size);
        free_array(old);
      }
      broken++;
    } else if (fix.kind != SLOT_MAP_KEY) {
      free_svalue(fix.slot, "f_break_cycles");
      *fix.slot = const0;
      broken++;
    }
  }
  // 2. Then delete mapping nodes whose KEY is the back-edge. Deleting a node
  //    frees its value subtree, which CAN cascade; every other pending fix
  //    holds its own reference on its container (taken during the walk), so
  //    the deletions are order-independent and never touch freed memory.
  //    (Same caveat as map_delete(): a `foreach (k, ref v in m)` loop
  //    variable in an OUTER frame aimed at the deleted node's value slot is
  //    left dangling -- a pre-existing hazard of node deletion during an
  //    unlocked mapping foreach, not specific to this efun.)
  for (auto& fix : res.fixes) {
    if (fix.kind == SLOT_MAP_KEY) {
      mapping_delete(fix.container.u.map, fix.slot);
      broken++;
    }
  }

  free_svalue(sp, "f_break_cycles");
  put_number(broken);
}
#endif
