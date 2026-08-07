
#include "base/internal/scratchpad.h"

#include <new>

#include "base/internal/debugmalloc.h"
#include "base/internal/outbuf.h"

/*
 * Monotonic bump arena. See scratchpad.h for the contract.
 *
 * A singly-linked list of chunks (newest = `cur`, linked via `prev`).
 * Allocation bumps `cur->used`; deallocation is a no-op (header inline);
 * scratch_destroy frees every overflow chunk and resets the persistent
 * base chunk, so back-to-back small compiles never touch malloc.
 *
 * Allocations never span a chunk: a request that doesn't fit the current
 * chunk starts a new one (exact-fit when larger than the default chunk
 * payload, so one huge string doesn't strand a huge tail).
 *
 * Everything here is POD static state -- nothing has a destructor, so
 * arena-backed objects living in static/exit-scope storage can safely
 * run THEIR destructors (no-op deallocate) in any order at process exit.
 */

namespace {

// Production geometry: a 1MB buffer covers virtually every compile in a
// single chunk (the 206KB bench program uses ~700KB of transients).
constexpr std::size_t kBaseSize = std::size_t{1024} * 1024;  // persistent chunk 0
constexpr int kMaxRetained = 8;  // 8 x 1MB warm ceiling, retained across compiles

// Runtime-adjustable so tests/benchmarks can force the WORST CASE
// (e.g. 400-byte chunks: constant advances, every big string oversize).
// Read only on the spill/oversize paths plus one compare per allocation.
std::size_t chunk_payload = kBaseSize;

struct Chunk {
  std::size_t cap;       // payload capacity in bytes
  std::size_t used;      // bytes handed out
  Chunk* next_overflow;  // intrusive list link (overflow chunks only)
  std::size_t pad_;      // keeps data() max_align_t (16-byte) aligned
  char* data() { return reinterpret_cast<char*>(this) + sizeof(Chunk); }
};
static_assert(sizeof(Chunk) % 16 == 0, "chunk payload must stay 16-byte aligned");

// Deque-style chunk management (all POD statics -- no destructors, so
// arena-backed objects in exit-scope storage can deallocate in any order
// at process exit):
//
//   chunks[0..chunk_count)  standard-size chunks RETAINED across
//                           compiles; chunks[0] is the persistent static
//                           block. The bump cursor walks this array and
//                           reset is just "cursor back to slot 0" -- the
//                           tail of the array IS the warm cache, with no
//                           separate parking list. A long-lived driver
//                           reaches chunk_count == its peak demand and
//                           never mallocs again (scratch_stats proves it).
//   overflow chunks         everything past the retained ceiling and all
//                           oversize exact-fit requests; freed at every
//                           reset (retaining arbitrary amounts would pin
//                           unbounded memory). cur_overflow is the active
//                           bump target once the retained array is full.
alignas(std::max_align_t) unsigned char base_storage[sizeof(Chunk) + kBaseSize];
bool base_storage_taken = false;

}  // namespace

// All arena state. Owned by a ScratchArena, which owns its lifetime; the
// compiler only ever borrows one (see scratchpad.h).
struct ScratchArena::Impl {
  Chunk* chunks[kMaxRetained];
  int chunk_count = 0;
  int cur_index = 0;
  Chunk* overflow = nullptr;      // intrusive list of this cycle's overflow chunks
  Chunk* cur_overflow = nullptr;  // active overflow chunk (null = bump the array)
  Chunk* active = nullptr;        // THE bump target (cur_overflow ?: chunks[cur_index])

  // Observability counters (scratch_stats / scratchpad_status).
  std::size_t cycle_bytes = 0;
  std::size_t peak_cycle_bytes = 0;
  std::size_t chunk_mallocs = 0;
  std::size_t reset_count = 0;

  bool base_is_static = false;  // chunk 0 borrowed base_storage
};

namespace {

// The arena currently bound for allocation. Set only by
// ScratchArenaBinding, i.e. for the duration of one compile. When nothing
// is bound we fall back to the default arena, which keeps allocation
// working for code running outside any compile (unit tests exercising the
// lexer directly, tools) rather than crashing.
ScratchArena::Impl* bound = nullptr;

ScratchArena::Impl* current() {
  if (bound == nullptr) bound = scratch_default_arena().impl();
  return bound;
}

// Give up this arena's claim on the static block, if it holds it. EVERY path
// that drops chunk 0 must call this: base_is_static drives whether the
// teardown path FREEs chunk 0, so an arena that abandons the static block and
// later re-inits onto a heap one would otherwise carry a stale "don't free
// me" flag and leak that heap chunk. (Found exactly that way: the tiny-chunk
// test knob re-inits with chunk_count = 0, so it took the heap branch below
// while still flagged static, and LeakSanitizer caught the orphan.)
void release_base_claim(ScratchArena::Impl* a) {
  if (a->base_is_static) {
    base_storage_taken = false;
    a->base_is_static = false;
  }
}

void ensure_init(ScratchArena::Impl* a) {
  if (a->chunk_count == 0) {
    Chunk* b;
    if (!base_storage_taken) {
      // First arena to need one borrows the static block, so an ordinary
      // driver compile never mallocs a base chunk.
      base_storage_taken = true;
      b = reinterpret_cast<Chunk*>(base_storage);
      b->cap = chunk_payload < kBaseSize ? chunk_payload : kBaseSize;
      a->base_is_static = true;
    } else {
      b = reinterpret_cast<Chunk*>(
          DMALLOC(sizeof(Chunk) + chunk_payload, TAG_SCRATCHPAD, "scratchpad base chunk"));
      b->cap = chunk_payload;
      a->chunk_mallocs++;
      a->base_is_static = false;  // heap chunk 0: teardown MUST free this
    }
    b->used = 0;
    b->next_overflow = nullptr;
    a->chunks[0] = b;
    a->chunk_count = 1;
    a->cur_index = 0;
    a->active = b;
  }
}

inline std::size_t align_up(std::size_t n, std::size_t align) {
  return (n + align - 1) & ~(align - 1);
}

Chunk* new_chunk(std::size_t cap) {
  // DMALLOC with the dedicated TAG_SCRATCHPAD: chunks are visible to the
  // driver's memory accounting, and check_all_blocks whitelists the tag
  // (retained-across-compiles is by design, not a leak).
  Chunk* c =
      reinterpret_cast<Chunk*>(DMALLOC(sizeof(Chunk) + cap, TAG_SCRATCHPAD, "scratchpad chunk"));
  c->cap = cap;
  c->used = 0;
  c->next_overflow = nullptr;
  current()->chunk_mallocs++;
  return c;
}

}  // namespace

void* scratch_raw_allocate(std::size_t bytes, std::size_t align) {
  ScratchArena::Impl* a = current();
  ensure_init(a);
  Chunk* c = a->active;
  std::size_t off = align_up(c->used, align);
  if (off + bytes > c->cap) {
    if (bytes > chunk_payload) {
      // Oversize: its own exact-fit overflow chunk, filled completely.
      // The active bump chunk is left untouched for later requests.
      Chunk* big = new_chunk(bytes);
      big->next_overflow = a->overflow;
      a->overflow = big;
      big->used = bytes;
      a->cycle_bytes += bytes;
      return big->data();
    }
    if (a->cur_overflow == nullptr && a->cur_index + 1 < a->chunk_count) {
      c = a->chunks[++a->cur_index];  // warm reuse: next retained chunk
      c->used = 0;
    } else if (a->cur_overflow == nullptr && a->chunk_count < kMaxRetained) {
      c = a->chunks[a->chunk_count] = new_chunk(chunk_payload);
      a->cur_index = a->chunk_count++;
    } else {
      // Retained ceiling reached: bump from overflow chunks for the rest
      // of this cycle (freed at reset).
      c = new_chunk(chunk_payload);
      c->next_overflow = a->overflow;
      a->overflow = c;
      a->cur_overflow = c;
    }
    a->active = c;
    off = align_up(c->used, align);
  }
  char* p = c->data() + off;
  c->used = off + bytes;
  a->cycle_bytes += bytes;
  return p;
}

namespace {
void reset_impl(ScratchArena::Impl* a) {
  ensure_init(a);
  while (a->overflow != nullptr) {
    Chunk* next = a->overflow->next_overflow;
    FREE(a->overflow);
    a->overflow = next;
  }
  a->cur_overflow = nullptr;
  a->cur_index = 0;
  a->chunks[0]->used = 0;
  a->active = a->chunks[0];
  if (a->cycle_bytes > a->peak_cycle_bytes) a->peak_cycle_bytes = a->cycle_bytes;
  a->cycle_bytes = 0;
  a->reset_count++;
}

}  // namespace

// See the declaration in scratchpad.h. Process-lifetime deliberately: this
// is the arena a compile recycles when its caller does not supply one, so
// its chunk cache has to survive across compiles (that is what gets a
// long-lived driver to zero chunk mallocs in the steady state), and it is
// also what scratch_stats()/scratchpad_status() report when nothing is
// bound -- both would otherwise describe an arena that never took part in
// any compile.
ScratchArena& scratch_default_arena() {
  static ScratchArena a;
  return a;
}

void scratch_destroy() { reset_impl(current()); }

ScratchArena::ScratchArena() : impl_(new Impl()) {}

ScratchArena::~ScratchArena() {
  Impl* a = impl_.get();
  reset_impl(a);  // drop overflow chunks
  for (int i = 0; i < a->chunk_count; i++) {
    if (i == 0 && a->base_is_static) continue;  // static storage, not heap
    FREE(a->chunks[i]);
  }
  release_base_claim(a);  // let a later arena borrow the static block again
  if (bound == a) bound = nullptr;
}

void ScratchArena::reset() { reset_impl(impl_.get()); }

ScratchArenaBinding::ScratchArenaBinding(ScratchArena& arena) : prev_(bound) {
  bound = arena.impl();
}
ScratchArenaBinding::~ScratchArenaBinding() { bound = prev_; }

void scratch_set_chunk_size_for_testing(std::size_t payload) {
  // Test/bench knob: shrink chunks to force worst-case behavior
  // (constant advances, oversize spills). Retained chunks of the OLD
  // size are dropped so the new geometry applies uniformly; chunk 0's
  // capacity is clamped to its static storage.
  scratch_destroy();
  ScratchArena::Impl* a = current();
  // Same shape as ~ScratchArena: chunk 0 is only exempt from FREE when it is
  // the static block. It can be a heap chunk here (a second arena, or a
  // previous pass through this knob), and starting the loop at 1 -- correct
  // back when chunk 0 was ALWAYS static -- would silently orphan it.
  for (int i = 0; i < a->chunk_count; i++) {
    if (i == 0 && a->base_is_static) continue;
    FREE(a->chunks[i]);
  }
  // Chunk 0 goes too, so hand the static block back before re-initializing --
  // otherwise ensure_init() finds it still claimed, mallocs a heap chunk 0,
  // and the stale base_is_static flag keeps teardown from ever freeing it.
  release_base_claim(a);
  a->chunk_count = 0;
  chunk_payload = payload;
  ensure_init(a);
  a->active = a->chunks[0];
}

ScratchStats scratch_stats() {
  ScratchArena::Impl* a = current();
  return ScratchStats{a->cycle_bytes, a->peak_cycle_bytes, a->chunk_mallocs, a->reset_count,
                      a->chunk_count};
}

uint64_t scratchpad_status(outbuffer_t* out, int verbose) {
  ScratchArena::Impl* a = current();
  const int chunk_count = a->chunk_count;
  const std::size_t peak_cycle_bytes = a->peak_cycle_bytes;
  const std::size_t chunk_mallocs = a->chunk_mallocs;
  const std::size_t reset_count = a->reset_count;
  std::size_t retained = 0;
  for (int i = 0; i < chunk_count; i++) retained += sizeof(Chunk) + a->chunks[i]->cap;
  std::size_t heap_retained =
      retained - ((chunk_count > 0 && a->base_is_static) ? sizeof(Chunk) + a->chunks[0]->cap : 0);
  if (verbose == 1) {
    outbuf_add(out, "compile scratchpad:\n");
    outbuf_add(out, "-------------------------\n");
    outbuf_addv(out,
                "Retained chunks:\t\t%4d (%zu bytes, %zu heap)\nPeak compile bytes:\t%zu\n"
                "Chunk mallocs:\t\t%zu\nCompiles (resets):\t%zu\n",
                chunk_count, retained, heap_retained, peak_cycle_bytes, chunk_mallocs, reset_count);
  } else if (verbose != -1) {
    outbuf_addv(out, "compile scratchpad:\t\t\t%8zu %8zu\n", static_cast<std::size_t>(chunk_count),
                retained);
  }
  return heap_retained;
}

ScratchString* scratch_new_string(std::string_view sv) {
  void* mem = scratch_raw_allocate(sizeof(ScratchString), alignof(ScratchString));
  // Placement-new; deliberately never destructed. Both the object and its
  // buffer (SSO bytes inside the object for short strings -- the common
  // identifier case, one arena allocation total -- or a ScratchAllocator
  // block) are arena memory, bulk-freed at scratch_destroy.
  return new (mem) ScratchString(sv.data(), sv.size());
}
