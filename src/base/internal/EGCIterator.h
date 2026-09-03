#ifndef FLUFFOS_SRC_BASE_INTERNAL_STRUTILS_CC_EGCSTRINGVIEW_H_
#define FLUFFOS_SRC_BASE_INTERNAL_STRUTILS_CC_EGCSTRINGVIEW_H_

#include <algorithm>
#include <cstdint>
#include <functional>
#include <cctype>
#include <locale>
#include <string>
#include <memory>
#include <utility>
#include <unicode/utf8.h>
#include <unicode/uchar.h>
#include <unicode/brkiter.h>
#include <unicode/unistr.h>

#include <stack>

class BreakIteratorPool {
  class BreakIteratorPoolInner {
   public:
    std::stack<std::unique_ptr<icu::BreakIterator>> stack_;
  };

  struct ReturnToPool_Deleter {
    explicit ReturnToPool_Deleter(std::weak_ptr<BreakIteratorPoolInner> origin)
        : origin_(std::move(origin)) {}
    void operator()(icu::BreakIterator* ptr) {
      auto origin = origin_.lock();
      if (origin) {
        origin->stack_.emplace(ptr);
      } else {
        delete ptr;
      }
    }

   private:
    std::weak_ptr<BreakIteratorPoolInner> origin_;
  };

 private:
  std::shared_ptr<BreakIteratorPoolInner> inner_;

  void add() {
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<icu::BreakIterator> ptr(
        icu::BreakIterator::createCharacterInstance(icu::Locale::getDefault(), status));
    if (!U_SUCCESS(status)) {
      ptr.reset();
    }
    inner_->stack_.emplace(std::move(ptr));
  }

 public:
  using item_type = std::unique_ptr<icu::BreakIterator, ReturnToPool_Deleter>;

  explicit BreakIteratorPool(int initial) : inner_(std::make_shared<BreakIteratorPoolInner>()) {
    for (int i = 0; i < initial; i++) add();
  }

  item_type accquire() {
    if (inner_->stack_.empty()) add();
    auto res = std::unique_ptr<icu::BreakIterator, ReturnToPool_Deleter>(
        inner_->stack_.top().release(), ReturnToPool_Deleter(inner_));
    inner_->stack_.pop();
    return res;
  }
};

// Wrapper class to create a icu EGC iterator for given string.
// can access underlying icu::BreakIterator
//
// Pure-ASCII fast path
// -------------------
// Driving ICU's rule-based break iterator costs on the order of 50 machine
// instructions per grapheme cluster, plus a utext_openUTF8()/setText() setup
// per string. In a mudlib the overwhelming majority of strings are pure
// ASCII, and for those the answer is trivial: every byte is exactly one
// grapheme cluster, so EGC index == byte offset and the cluster count is just
// the byte length.
//
// So reset() first scans for a byte with the high bit set. When there is
// none the string is flagged ASCII, ICU is NOT set up at all, and
// EGCSmartIterator answers every query arithmetically. ICU is still wired up
// lazily -- by ensure_icu(), via operator->() -- so code that reaches for the
// underlying icu::BreakIterator keeps working unchanged on any string.
class EGCIterator {
 private:
  bool ok_ = false;
  bool ascii_ = false;
  bool icu_ready_ = false;
  const char* src_;
  int32_t len_;

  static BreakIteratorPool* pool() {
    static BreakIteratorPool pool(32);
    return &pool;
  }

  // True when the string is pure ASCII (so trivially valid UTF-8) AND contains
  // no CR.
  //
  // The CR exclusion is load-bearing, not caution: UAX #29 rule GB3 joins
  // CR x LF into ONE grapheme cluster, so "\r\n" is 2 bytes but 1 cluster and
  // the byte-offset==index identity breaks. CR is the only ASCII byte that can
  // join with a following character this way -- pinned exhaustively over all
  // 128x128 ASCII pairs by EGCAsciiFastPath.CrLfIsTheOnlyAsciiJoin. Excluding
  // every CR (rather than just CR immediately before LF) keeps this a single
  // flat scan; a lone CR merely falls back to ICU, which is still correct.
  //
  // Auto-vectorizes to a few bytes per cycle, against ICU's ~50 instructions
  // per cluster.
  static bool all_ascii(const char* src, int32_t slen) {
    // slen < 0 is not a counted length (ICU uses -1 for NUL-terminated).
    // The empty loop would otherwise return true, and callers that then
    // did `size_t n = slen` built a ~2^32/2^64-byte view — SIGSEGV.
    if (slen < 0) return false;
    unsigned char acc = 0;
    unsigned char cr = 0;
    for (int32_t i = 0; i < slen; i++) {
      auto c = static_cast<unsigned char>(src[i]);
      acc |= c;
      cr |= static_cast<unsigned char>(c == '\r');
    }
    return (acc & 0x80u) == 0 && cr == 0;
  }

  bool setup_icu() {
    UText text = UTEXT_INITIALIZER;
    UErrorCode status = U_ZERO_ERROR;
    utext_openUTF8(&text, src_, len_, &status);
    if (!U_SUCCESS(status)) {
      utext_close(&text);
      return false;
    }

    status = U_ZERO_ERROR;
    brk_->setText(&text, status);  // copies text
    if (!U_SUCCESS(status)) {
      utext_close(&text);
      return false;
    }
    // no longer needed.
    utext_close(&text);

    brk_->first();
    icu_ready_ = true;
    return true;
  }

 protected:
  BreakIteratorPool::item_type brk_;

  // Materialize the ICU iterator for a string that skipped setup because it
  // is ASCII. Must be called before ANY use of brk_.
  bool ensure_icu() { return icu_ready_ ? true : setup_icu(); }

 public:
  [[nodiscard]] bool ok() const { return ok_; }
  [[nodiscard]] bool is_ascii() const { return ascii_; }
  // The raw predicate, WITHOUT constructing an iterator. Constructing one
  // acquires an ICU break iterator from the pool, which builds 32 of them on
  // first touch -- absurd overhead when all you want is a byte scan. Callers
  // that only need the answer (u8_string_is_ascii_cached) use this.
  static bool scan_is_ascii(const char* src, int32_t slen) { return all_ascii(src, slen); }
  [[nodiscard]] const char* data() const { return src_; }
  [[nodiscard]] int32_t len() const { return len_; }
  // Virtual so EGCSmartIterator can drop its cached count / cursor. The
  // EGCIterator constructor calls this by name (C++ does not dispatch to
  // the derived override during base construction); that is intentional.
  virtual void reset(const char* src, int32_t slen) {
    // explode() (and similar token walks) call reset() once per delimiter
    // with a suffix/prefix of the same buffer. all_ascii() is a full scan,
    // so doing it on every remaining slice is O(n²) in token count — that
    // is issue #1366: explode of 50k ASCII tokens went from ~8 ms to ~237 ms
    // after reset() started scanning. A subrange of a known-ASCII string is
    // still ASCII (the CR / high-bit exclusion is closed under substring),
    // so skip the scan when the new range sits inside the previous one.
    // icu_ready_ is always dropped: a BreakIterator setText()'d on the old
    // range must not be reused on the new one.
    const bool prev_ascii = ok_ && ascii_;
    const char* const prev_src = src_;
    const int32_t prev_len = len_;

    ok_ = false;
    icu_ready_ = false;
    ascii_ = false;
    src_ = src;
    len_ = slen;

    // Only -1 means NUL-terminated (ICU convention). Other negatives are
    // not a length: explode used to pass them after a strstr match that
    // started inside the counted range and ran past it.
    if (slen < -1) {
      return;
    }

    // Subrange test is overflow-safe on 32-bit (wasm32): check slen against
    // prev_len first so (prev_len - slen) cannot wrap, then compare the
    // pointer offset as uintptr_t rather than subtracting pointers from
    // possibly distinct objects.
    if (prev_ascii && slen >= 0 && slen <= prev_len) {
      const auto src_u = reinterpret_cast<uintptr_t>(src);
      const auto prev_u = reinterpret_cast<uintptr_t>(prev_src);
      if (src_u >= prev_u &&
          src_u - prev_u <= static_cast<uintptr_t>(prev_len) - static_cast<uintptr_t>(slen)) {
        ascii_ = true;
        ok_ = true;
        return;
      }
    }

    if (slen >= 0 && all_ascii(src, slen)) {
      // Pure ASCII is always well-formed UTF-8; defer ICU until something
      // actually asks for the underlying break iterator.
      ascii_ = true;
      ok_ = true;
      return;
    }

    ok_ = setup_icu();
  }
  auto operator->() {
    ensure_icu();
    return brk_.operator->();
  }

  static BreakIteratorPool::item_type GetBreakIterator() { return pool()->accquire(); }

  EGCIterator(const char* src, int32_t slen) : src_(src), len_(slen), brk_(GetBreakIterator()) {
    reset(src, slen);
  }

  virtual ~EGCIterator() {}
};
#endif  // FLUFFOS_SRC_BASE_INTERNAL_STRUTILS_CC_EGCSTRINGVIEW_H_
