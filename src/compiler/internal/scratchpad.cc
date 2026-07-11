#include "base/std.h"

#include "scratchpad.h"

#include <new>

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
Chunk* chunks[kMaxRetained];
int chunk_count = 0;
int cur_index = 0;
Chunk* overflow = nullptr;      // intrusive list of this cycle's overflow chunks
Chunk* cur_overflow = nullptr;  // active overflow chunk (null = bump the array)
Chunk* active = nullptr;        // THE bump target (cache of cur_overflow ?: chunks[cur_index])

// Observability counters (scratch_stats / scratchpad_status).
std::size_t cycle_bytes = 0;
std::size_t peak_cycle_bytes = 0;
std::size_t chunk_mallocs = 0;
std::size_t reset_count = 0;

void ensure_init() {
  if (chunk_count == 0) {
    Chunk* b = reinterpret_cast<Chunk*>(base_storage);
    b->cap = chunk_payload < kBaseSize ? chunk_payload : kBaseSize;
    b->used = 0;
    b->next_overflow = nullptr;
    chunks[0] = b;
    chunk_count = 1;
    cur_index = 0;
    active = b;
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
  chunk_mallocs++;
  return c;
}

}  // namespace

void* scratch_raw_allocate(std::size_t bytes, std::size_t align) {
  ensure_init();
  Chunk* c = active;
  std::size_t off = align_up(c->used, align);
  if (off + bytes > c->cap) {
    if (bytes > chunk_payload) {
      // Oversize: its own exact-fit overflow chunk, filled completely.
      // The active bump chunk is left untouched for later requests.
      Chunk* big = new_chunk(bytes);
      big->next_overflow = overflow;
      overflow = big;
      big->used = bytes;
      cycle_bytes += bytes;
      return big->data();
    }
    if (cur_overflow == nullptr && cur_index + 1 < chunk_count) {
      c = chunks[++cur_index];  // warm reuse: next retained chunk
      c->used = 0;
    } else if (cur_overflow == nullptr && chunk_count < kMaxRetained) {
      c = chunks[chunk_count] = new_chunk(chunk_payload);
      cur_index = chunk_count++;
    } else {
      // Retained ceiling reached: bump from overflow chunks for the rest
      // of this cycle (freed at reset).
      c = new_chunk(chunk_payload);
      c->next_overflow = overflow;
      overflow = c;
      cur_overflow = c;
    }
    active = c;
    off = align_up(c->used, align);
  }
  char* p = c->data() + off;
  c->used = off + bytes;
  cycle_bytes += bytes;
  return p;
}

void scratch_destroy() {
  ensure_init();
  while (overflow != nullptr) {
    Chunk* next = overflow->next_overflow;
    FREE(overflow);
    overflow = next;
  }
  cur_overflow = nullptr;
  cur_index = 0;
  chunks[0]->used = 0;
  active = chunks[0];
  if (cycle_bytes > peak_cycle_bytes) peak_cycle_bytes = cycle_bytes;
  cycle_bytes = 0;
  reset_count++;
}

void scratch_set_chunk_size_for_testing(std::size_t payload) {
  // Test/bench knob: shrink chunks to force worst-case behavior
  // (constant advances, oversize spills). Retained chunks of the OLD
  // size are dropped so the new geometry applies uniformly; chunk 0's
  // capacity is clamped to its static storage.
  scratch_destroy();
  for (int i = 1; i < chunk_count; i++) FREE(chunks[i]);
  chunk_count = 0;
  chunk_payload = payload;
  ensure_init();
  active = chunks[0];
}

ScratchStats scratch_stats() {
  return ScratchStats{cycle_bytes, peak_cycle_bytes, chunk_mallocs, reset_count, chunk_count};
}

uint64_t scratchpad_status(outbuffer_t* out, int verbose) {
  std::size_t retained = 0;
  for (int i = 0; i < chunk_count; i++) retained += sizeof(Chunk) + chunks[i]->cap;
  // chunk 0 is static storage, not heap.
  std::size_t heap_retained = retained - (chunk_count > 0 ? sizeof(Chunk) + chunks[0]->cap : 0);
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
