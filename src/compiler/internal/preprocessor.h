#pragma once

#include <memory>
#include <string>

#include "compiler/internal/LexStream.h"

/**
 * LpcPreprocessor — standalone LPC C-preprocessor stage.
 *
 * Design: operates entirely on std::string internally.
 *   - Reads the full source via LexStream into a std::string
 *   - Walks with a cursor, processing directives
 *   - Emits preprocessed text into another std::string
 *
 * Handles:
 *   - #define / #undef  (object-like and function-like macros)
 *   - #if / #ifdef / #ifndef / #elif / #else / #endif
 *   - Line splicing (backslash-newline)
 *   - // and / * ... * / comments (stripped / collapsed to a space)
 *   - #include — emitted as a comment (driver lexer_utils.cc handles real inclusion)
 *   - Macro expansion (object-like and function-like, with recursion guard)
 *
 * Usage:
 *   std::string result = LpcPreprocessor(stream, file).preprocess();
 *   std::istringstream ss(result);
 *   start_new_file(std::make_unique<IStreamLexStream>(ss));
 */
 class LpcPreprocessor {
  public:
   LpcPreprocessor(std::unique_ptr<LexStream> stream, const char *filename);
   LpcPreprocessor(std::string source, const char *filename);
   ~LpcPreprocessor();
 
   // Run the preprocessor and return the fully preprocessed source text.
   std::string preprocess();
 
   // Get preprocessor errors
   const std::vector<std::string>& errors() const;

 private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

