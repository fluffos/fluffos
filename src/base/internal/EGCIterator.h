#ifndef FLUFFOS_SRC_BASE_INTERNAL_STRUTILS_CC_EGCSTRINGVIEW_H_
#define FLUFFOS_SRC_BASE_INTERNAL_STRUTILS_CC_EGCSTRINGVIEW_H_

#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <string>
#include <memory>
#include <unicode/utf8.h>
#include <unicode/uchar.h>
#include <unicode/brkiter.h>
#include <unicode/unistr.h>

// Wrapper class to create a icu EGC iterator for given string.
// can access underlying icu::BreakIterator
class EGCIterator {
 private:
  UText text_ = UTEXT_INITIALIZER;
  bool ok_ = false;
  const char* src_;
  int32_t len_;

 protected:
  std::unique_ptr<icu::BreakIterator> brk_ = nullptr;

 public:
  [[nodiscard]] bool ok() const { return ok_; }
  [[nodiscard]] const char* data() const { return src_; }
  [[nodiscard]] int32_t len() const { return len_; }
  void reset(const char* src, int32_t slen) {
    ok_ = false;

    UErrorCode status = U_ZERO_ERROR;
    utext_openUTF8(&text_, src, slen, &status);
    if (!U_SUCCESS(status)) {
      utext_close(&text_);
      return ;
    }

    status = U_ZERO_ERROR;
    brk_->setText(&text_, status);
    if (!U_SUCCESS(status)) {
      utext_close(&text_);
      return;
    }
    brk_->first();

    src_ = src;
    len_ =  slen;
    ok_ = true;
  }
  auto operator->() { return brk_.operator->(); }
  EGCIterator(const char* src, int32_t slen) {
    UErrorCode status = U_ZERO_ERROR;
    brk_.reset(icu::BreakIterator::createCharacterInstance(icu::Locale::getDefault(), status));
    if (!U_SUCCESS(status)) {
      return;
    }
    reset(src, slen);
  }
  ~EGCIterator() {
    if (this->ok()) {
      utext_close(&text_);
      brk_.reset();
    }
  }
};

#endif  // FLUFFOS_SRC_BASE_INTERNAL_STRUTILS_CC_EGCSTRINGVIEW_H_
