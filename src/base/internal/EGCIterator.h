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
class EGCIterator {
 private:
  bool ok_ = false;
  const char* src_;
  int32_t len_;

  static BreakIteratorPool* pool() {
    static BreakIteratorPool pool(32);
    return &pool;
  }

 protected:
  BreakIteratorPool::item_type brk_;

 public:
  [[nodiscard]] bool ok() const { return ok_; }
  [[nodiscard]] const char* data() const { return src_; }
  [[nodiscard]] int32_t len() const { return len_; }
  void reset(const char* src, int32_t slen) {
    ok_ = false;

    UText text = UTEXT_INITIALIZER;
    UErrorCode status = U_ZERO_ERROR;
    utext_openUTF8(&text, src, slen, &status);
    if (!U_SUCCESS(status)) {
      utext_close(&text);
      return;
    }

    status = U_ZERO_ERROR;
    brk_->setText(&text, status);  // copies text
    if (!U_SUCCESS(status)) {
      utext_close(&text);
      return;
    }
    // no longer needed.
    utext_close(&text);

    brk_->first();

    src_ = src;
    len_ = slen;
    ok_ = true;
  }
  auto operator->() { return brk_.operator->(); }

  static BreakIteratorPool::item_type GetBreakIterator() { return pool()->accquire(); }

  EGCIterator(const char* src, int32_t slen) : src_(src), len_(slen), brk_(GetBreakIterator()) {
    reset(src, slen);
  }

  virtual ~EGCIterator() {}
};
#endif  // FLUFFOS_SRC_BASE_INTERNAL_STRUTILS_CC_EGCSTRINGVIEW_H_
