#ifndef STRUTILS_H
#define STRUTILS_H

#include <unicode/utf8.h>
#include <unicode/uchar.h>
#include <unicode/brkiter.h>
#include <unicode/unistr.h>
#include <unicode/ucnv.h>

#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <string>

#include "base/internal/EGCIterator.h"

// --------------------------------------------------------------------------
/// @brief removes given characters from beginning of string
///
/// @param str   string to be trimmed
/// @param chars characters to be removed, defaults to whitespace
///
/// @return returns trimmed string
// --------------------------------------------------------------------------
inline std::string& ltrim(std::string&& str, const std::string& chars = "\t\n\v\f\r ") {
  str.erase(0, str.find_first_not_of(chars));
  return str;
}

inline std::string ltrim(const std::string& str, const std::string& chars = "\t\n\v\f\r ") {
  std::string ret{str};

  ret.erase(0, str.find_first_not_of(chars));
  return ret;
}

// --------------------------------------------------------------------------
/// @brief removes given characters from end of string
///
/// @param str   string to be trimmed
/// @param chars characters to be removed, defaults to whitespace
///
/// @return returns trimmed string
// --------------------------------------------------------------------------
inline std::string& rtrim(std::string&& str, const std::string& chars = "\t\n\v\f\r ") {
  str.erase(str.find_last_not_of(chars) + 1);
  return str;
}

inline std::string rtrim(const std::string& str, const std::string& chars = "\t\n\v\f\r ") {
  std::string ret{str};

  ret.erase(str.find_last_not_of(chars) + 1);
  return ret;
}

// --------------------------------------------------------------------------
/// @brief removes given chars from beginning and end of string
///
/// @param str   string to be trimmed
/// @param chars characters to be removed, defaults to whitespace
///
/// @return returns trimmed string
// --------------------------------------------------------------------------
inline std::string& trim(std::string&& str, const std::string& chars = "\t\n\v\f\r ") {
  return ltrim(rtrim(str, chars), chars);
}

inline std::string trim(const std::string& str, const std::string& chars = "\t\n\v\f\r ") {
  return ltrim(rtrim(str, chars), chars);
}

// --------------------------------------------------------------------------
/// @brief checks beginning of string1 for presens of string2
///
/// @param str   string to be examined
/// @param start string to be searched
///
/// @return returns true if str begins with startm false otherwise
// --------------------------------------------------------------------------
inline bool starts_with(const std::string& str, const std::string& start) {
  return str.compare(0, start.length(), start) == 0;
}

// --------------------------------------------------------------------------
/// @brief checks end of string 1 for presens of string 2
///
/// @param str   string to be examined
/// @param end   string to be searched
///
/// @return returns true if str begins with startm false otherwise
// --------------------------------------------------------------------------
inline bool ends_with(const std::string& str, const std::string& end) {
  if (str.length() < end.length()) return false;
  return str.compare(str.length() - end.length(), end.length(), end) == 0;
}

inline void ReplaceStringInPlace(std::string& subject, const std::string& search,
                                 const std::string& replace) {
  size_t pos = 0;
  while ((pos = subject.find(search, pos)) != std::string::npos) {
    subject.replace(pos, search.length(), replace);
    pos += replace.length();
  }
}

// a smarter subclass that remembers current location and attempt to do
// relative movement to improve speed. but offer no access to underlying break iterator.
class EGCSmartIterator : public EGCIterator {
 public:
  EGCSmartIterator(const char* src, int32_t slen) : EGCIterator(src, slen) {}
  size_t count() {
    if (count_ == -1) {
      // ASCII: one cluster per byte, so the count is the byte length. This is
      // the hot one -- sizeof(str) on a mudlib string lands here.
      //
      // Leave the cursor at the end, exactly where the ICU walk below leaves
      // it. No current caller counts and then keeps iterating (they all use
      // the value, or re-seek with index_to_offset first), but letting the
      // two paths disagree about cursor state would make a later count() +
      // next() silently mean different things on ASCII and non-ASCII input.
      if (is_ascii()) {
        count_ = len();
        current_idx_ = count_;
        ascii_pos_ = len();
        return count_;
      }
      count_ = 0;
      brk_->first();
      while (brk_->next() != icu::BreakIterator::DONE) ++count_;
      current_idx_ = count_;
    }
    return count_;
  }
  int32_t index_to_offset(int32_t index) {
    // ASCII: EGC index == byte offset. Boundaries are 0..len, a negative index
    // counts back from the end, and anything outside that range is DONE --
    // matching what the ICU walk below returns for the same input.
    if (is_ascii()) {
      int32_t off = index >= 0 ? index : len() + index;
      if (off < 0 || off > len()) return icu::BreakIterator::DONE;
      ascii_pos_ = off;
      current_idx_ = index;
      return off;
    }
    if (index == 0) {
      current_idx_ = 0;
      return brk_->first();
    }
    if (index == -1) {
      current_idx_ = -1;
      brk_->last();
      return brk_->previous();
    }
    if (index > 0) {  // forward search
      if (current_idx_ < 0 || (current_idx_ >= 0 && current_idx_ > 2 * index)) {
        current_idx_ = 0;
        brk_->first();
      }
    } else {  // reverse search
      if (current_idx_ >= 0 || (current_idx_ < 0 && current_idx_ < 2 * index)) {
        current_idx_ = -1;
        brk_->last();
        brk_->previous();
      }
    }
    auto oldpos = brk_->current();
    auto pos = brk_->next(index - current_idx_);
    if (pos == icu::BreakIterator::DONE) {
      brk_->isBoundary(oldpos);  // reset back
    } else {
      current_idx_ = index;
    }
    return pos;
  }
  int32_t post_index_to_offset(int32_t index) {
    auto pos = index_to_offset(index);
    if (pos < 0) return pos;
    if (is_ascii()) {
      // Mirrors the ICU pair below: next() then previous(). Off the end,
      // next() reports DONE and the following previous() still steps the
      // cursor back one boundary -- reproduce that side effect exactly.
      if (pos >= len()) {
        ascii_pos_ = len() > 0 ? len() - 1 : 0;
        return icu::BreakIterator::DONE;
      }
      return pos + 1;  // cursor stays at pos, as next()+previous() leaves it
    }
    pos = brk_->next();
    brk_->previous();
    return pos;
  }
  int32_t first() {
    current_idx_ = 0;
    if (is_ascii()) {
      ascii_pos_ = 0;
      return 0;
    }
    return brk_->first();
  }
  int32_t last() {
    current_idx_ = -1;
    if (is_ascii()) {
      ascii_pos_ = len();
      return len();
    }
    return brk_->last();
  }
  int32_t next() {
    if (is_ascii()) {
      if (ascii_pos_ >= len()) return icu::BreakIterator::DONE;  // cursor unchanged
      current_idx_++;
      return ++ascii_pos_;
    }
    auto oldpos = brk_->current();
    auto pos = brk_->next();
    if (pos == icu::BreakIterator::DONE) {
      brk_->isBoundary(oldpos);  // reset
    } else {
      current_idx_++;
    }
    return pos;
  }

 private:
  int32_t current_idx_ = 0;
  int32_t count_ = -1;
  int32_t ascii_pos_ = 0;  // byte offset cursor, ASCII fast path only
};

// Is this string pure ASCII (and CR-free), i.e. does byte offset equal
// grapheme-cluster index throughout?
//
// Pass counted=true only for a STRING_MALLOC / STRING_SHARED string, i.e.
// one that has a malloc_block_t/block_t header: those memoize the answer in
// the header, so repeated queries on the same string are O(1) -- which is
// what turns the common `for (i = 0; i < sizeof(s); i++)` from O(n^2) into
// O(n). A STRING_CONSTANT has no header and is rescanned each time.
// See MSTR_ASCII_* in stralloc.h.
//
// Takes primitives rather than an svalue_t deliberately: base/ must not
// depend on VM types (see AGENTS.md 12).
bool u8_string_is_ascii_cached(const char* str, int32_t len, bool counted);

// Check string s is valid utf8
bool u8_validate(char**);
bool u8_validate(const char*);
bool u8_validate(const uint8_t*, size_t);
UChar32 u8_egc_index_as_single_codepoint(const char*, int32_t, int32_t);
void u8_copy_and_replace_codepoint_at(EGCSmartIterator& iter, char* dst, int32_t index, UChar32 c);
int32_t u8_offset_to_egc_index(EGCIterator& iter, int32_t offset);
int32_t u8_strncpy(uint8_t*, const uint8_t*, const int32_t);
size_t u8_truncate(const uint8_t*, size_t);
// Return display width for string piece, len could be -1 for NULL terminated string.
size_t u8_width(const char* src, int len);
void u8_truncate_below_width(const char* src, size_t len, size_t max_width, bool break_for_line,
                             bool always_break_before_newline, size_t* out_len, size_t* out_width);
std::string u8_sanitize(std::string_view src);
// Length (0-3) of the incomplete-but-valid UTF-8 sequence prefix at the end
// of buf.  Streaming input should hold those bytes back until the rest of
// the character arrives, instead of sanitizing them into U+FFFD.
size_t u8_incomplete_tail(std::string_view buf);
int32_t u8_egc_find_as_offset(EGCIterator& iter, const char* needle, size_t needle_len,
                              bool reverse);

std::vector<std::string_view> u8_egc_split(const char* src, int32_t slen);
std::string u8_convert_encoding(UConverter* trans, const char* data, int len);
#endif  // STRUTILS_H
