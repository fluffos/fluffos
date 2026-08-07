#ifndef FLUFFOS_SRC_BASE_INTERNAL_STRUTILS_CC_EGCSTRINGVIEW_H_
#define FLUFFOS_SRC_BASE_INTERNAL_STRUTILS_CC_EGCSTRINGVIEW_H_

#include <algorithm>
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
  [[nodiscard]] const char* data() const { return src_; }
  [[nodiscard]] int32_t len() const { return len_; }
  void reset(const char* src, int32_t slen) {
    ok_ = false;
    icu_ready_ = false;
    src_ = src;
    len_ = slen;

    ascii_ = all_ascii(src, slen);
    if (ascii_) {
      // Pure ASCII is always well-formed UTF-8; defer ICU until something
      // actually asks for the underlying break iterator.
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
