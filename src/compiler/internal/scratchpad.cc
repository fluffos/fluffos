#include "base/std.h"

#include "scratchpad.h"

#include <cstdlib>
#include <new>

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

constexpr std::size_t kChunkSize = std::size_t{64} * 1024;  // default overflow-chunk payload
constexpr std::size_t kBaseSize = std::size_t{64} * 1024;   // persistent first chunk payload

struct Chunk {
  Chunk *prev;
  std::size_t cap;   // payload capacity in bytes
  std::size_t used;  // bytes handed out
  char *data() { return reinterpret_cast<char *>(this) + sizeof(Chunk); }
};

// Persistent first chunk, never freed (reset to empty at scratch_destroy).
alignas(std::max_align_t) unsigned char base_storage[sizeof(Chunk) + kBaseSize];

Chunk *cur = nullptr;

// Warm-chunk cache: scratch_destroy parks standard-size overflow chunks
// here (up to kSpareChunks) instead of freeing them, and allocation pops
// them back. Compiles that outgrow the base chunk then reuse the SAME
// warm pages every time instead of paying malloc + cold-page faults per
// compile -- this is what lets string-growth churn compete with a
// general-purpose allocator's block recycling. Exact-fit oversize chunks
// are always freed (they'd pin arbitrary amounts of memory).
constexpr int kSpareChunks = 16;  // 16 x 64KB = 1MB cache ceiling
Chunk *spare = nullptr;
int spare_count = 0;

Chunk *base_chunk() { return reinterpret_cast<Chunk *>(base_storage); }

inline std::size_t align_up(std::size_t n, std::size_t align) {
  return (n + align - 1) & ~(align - 1);
}

}  // namespace

void *scratch_raw_allocate(std::size_t bytes, std::size_t align) {
  if (cur == nullptr) {
    Chunk *b = base_chunk();
    b->prev = nullptr;
    b->cap = kBaseSize;
    b->used = 0;
    cur = b;
  }
  std::size_t off = align_up(cur->used, align);
  if (off + bytes > cur->cap) {
    // New chunk. Its payload base is max_align_t-aligned (DMALLOC/malloc
    // guarantee + power-of-two header padding), so offset 0 satisfies
    // any `align` a container can ask for.
    Chunk *c;
    if (bytes <= kChunkSize && spare != nullptr) {
      c = spare;
      spare = c->prev;
      spare_count--;
    } else {
      std::size_t cap = bytes > kChunkSize ? bytes : kChunkSize;
      // Plain malloc, NOT DMALLOC: the spare-chunk cache deliberately
      // holds chunks across compiles (process-lifetime infrastructure),
      // and DEBUGMALLOC's shutdown sweep would report every parked chunk
      // as an unaccounted leak and fail driver-autotest.
      c = static_cast<Chunk *>(malloc(sizeof(Chunk) + cap));
      c->cap = cap;
    }
    c->prev = cur;
    c->used = 0;
    cur = c;
    off = 0;
  }
  char *p = cur->data() + off;
  cur->used = off + bytes;
  return p;
}

void scratch_destroy() {
  while (cur != nullptr && cur != base_chunk()) {
    Chunk *prev = cur->prev;
    if (cur->cap == kChunkSize && spare_count < kSpareChunks) {
      cur->prev = spare;
      spare = cur;
      spare_count++;
    } else {
      free(cur);
    }
    cur = prev;
  }
  base_chunk()->used = 0;
  if (cur == nullptr) {
    Chunk *b = base_chunk();
    b->prev = nullptr;
    b->cap = kBaseSize;
    cur = b;
  }
}

ScratchString *scratch_new_string(std::string_view sv) {
  void *mem = scratch_raw_allocate(sizeof(ScratchString), alignof(ScratchString));
  // Placement-new; deliberately never destructed. Both the object and its
  // buffer (SSO bytes inside the object for short strings -- the common
  // identifier case, one arena allocation total -- or a ScratchAllocator
  // block) are arena memory, bulk-freed at scratch_destroy.
  return new (mem) ScratchString(sv.data(), sv.size());
}
