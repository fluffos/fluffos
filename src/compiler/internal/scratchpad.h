#ifndef SCRATCHPAD_H_
#define SCRATCHPAD_H_

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

/*
 * Compile-lifetime allocation arena.
 *
 * A monotonic bump allocator over a growable chunk list: allocation is a
 * pointer bump, individual deallocation is a NO-OP, and everything is
 * bulk-freed at compile end (scratch_destroy). Two properties are
 * load-bearing:
 *
 *   1. Token string VALUES on Bison's value stack are `ScratchString *`
 *      (YYSTYPE is a C union and cannot own the string object itself);
 *      scratch_new_string() places the object in the arena, so the
 *      pointer is valid until scratch_destroy with no per-token
 *      ownership plumbing. (Shared strings on the value stack use the
 *      union's separate `shared_string` member -- the two lifetimes are
 *      deliberately distinguished by type.)
 *   2. A compile aborts via error()'s longjmp/exception unwind from
 *      arbitrary depths; the bulk free is what makes every such path
 *      leak-free without per-allocation ownership plumbing.
 *
 * Usage rules:
 *   - Build every TRANSIENT compile string as a ScratchString (and
 *     transient containers as ScratchVector); materialize a value-stack
 *     token with scratch_new_string().
 *   - Anything that outlives the compile (macro table, predefines,
 *     Diagnostics, program data) must NOT live here -- copy out to its
 *     own storage at the boundary.
 *   - An arena-backed object stored in memory that survives the compile
 *     (e.g. the scanner context's accumulators) must be re-initialized
 *     at the start of the next compile before any use -- its buffer died
 *     with the arena. Destructing/assigning such a stale object is safe
 *     (deallocation is a no-op), using its contents is not.
 *
 * Monotonicity trade-off, deliberate: container growth (allocate new,
 * copy, "free" old) leaves the old block behind until scratch_destroy.
 * That churn is bounded (strings are capped by DEFMAX; the arena resets
 * every compile) and buys a no-branch allocation path, no per-allocation
 * bookkeeping, and no destruction-order hazards for exit-scope
 * arena-backed objects.
 */

// Bump-allocate `bytes` with `align` alignment (power of two).
void *scratch_raw_allocate(std::size_t bytes, std::size_t align);
// No-op (monotonic arena); exists so allocator-aware containers work.
inline void scratch_raw_deallocate(void * /*p*/) noexcept {}

// Stateless allocator over the one compile arena: allocator-aware
// containers construct with no argument.
template <class T>
struct ScratchAllocator {
  using value_type = T;
  ScratchAllocator() noexcept = default;
  template <class U>
  ScratchAllocator(const ScratchAllocator<U> &) noexcept {}
  T *allocate(std::size_t n) {
    return static_cast<T *>(scratch_raw_allocate(n * sizeof(T), alignof(T)));
  }
  void deallocate(T *p, std::size_t /*n*/) noexcept { scratch_raw_deallocate(p); }
  template <class U>
  bool operator==(const ScratchAllocator<U> &) const noexcept {
    return true;
  }
  template <class U>
  bool operator!=(const ScratchAllocator<U> &) const noexcept {
    return false;
  }
};

// Arena-backed std::string / std::vector. Grow, concatenate, and destruct
// like their std:: counterparts; their storage lives in the arena and is
// bulk-freed at compile end.
using ScratchString = std::basic_string<char, std::char_traits<char>, ScratchAllocator<char>>;
template <class T>
using ScratchVector = std::vector<T, ScratchAllocator<T>>;

// Arena-place a ScratchString holding a copy of `sv` and return a pointer
// to it. The object (and its buffer) live in the arena and are NEVER
// individually destructed -- bulk-freed at scratch_destroy. This is what
// the Bison value stack's `string` member holds for string tokens.
ScratchString *scratch_new_string(std::string_view sv);

// Free every arena allocation (bulk reset at compile end). The first
// chunk is persistent, and standard-size chunks are RETAINED in a
// deque-style array whose tail is the warm cache (reset = cursor back to
// slot 0), so a long-lived driver reaches a steady state where compiles
// perform NO chunk mallocs at all -- observable via scratch_stats().
void scratch_destroy();

// Steady-state observability: after warmup, `chunk_mallocs` must stop
// growing across compiles (the warm cache absorbs every request) -- the
// multi-round benchmark and leak hunts key off this.
struct ScratchStats {
  std::size_t cycle_bytes;       // bytes bump-allocated since the last reset
  std::size_t peak_cycle_bytes;  // largest completed compile's bytes
  std::size_t chunk_mallocs;     // lifetime count of chunk allocations
  std::size_t resets;            // lifetime count of scratch_destroy() calls
  int retained_chunks;           // standard chunks retained across compiles
};
ScratchStats scratch_stats();

// mud_status() reporting line(s); returns retained heap bytes.
#include <cstdint>
struct outbuffer_t;
uint64_t scratchpad_status(struct outbuffer_t *out, int verbose);

// Test/bench knob: force a chunk payload size (e.g. 400 bytes) to observe
// worst-case behavior; drops retained chunks and resets the arena.
void scratch_set_chunk_size_for_testing(std::size_t payload);

#endif
