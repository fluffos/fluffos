// PCRE efun flags (kept in high bits to avoid legacy flag collisions).
// Shared with mudlibs: copy this header into the mudlib include dir.
//
// Integer bits are the compatibility surface. Every pcre_* efun also accepts
// a mapping of named PCRE2 options in the same argument slot — that is how
// options that do not fit in a 32-bit LPC int (and substitute / match-limit
// controls) are exposed. See docs/efun/pcre/pcre_config.md.
#ifndef SRC_INCLUDE_PCRE_FLAGS_H_
#define SRC_INCLUDE_PCRE_FLAGS_H_

#define PCRE_DEFAULT 0

// Compile-time options (same bits as the PCRE1-era flags)
#define PCRE_I (1 << 16)  // PCRE2_CASELESS
#define PCRE_M (1 << 17)  // PCRE2_MULTILINE
#define PCRE_S (1 << 18)  // PCRE2_DOTALL
#define PCRE_U (1 << 19)  // PCRE2_UNGREEDY
#define PCRE_X (1 << 20)  // PCRE2_EXTENDED

// Match-time
#define PCRE_A (1 << 21)  // PCRE2_ANCHORED

// PCRE2 is the library. Every compile already uses UTF-8, Unicode character
// properties (UCP: \w \d \s \b and \p{...}), and JIT when the library was
// built with it. PCRE_NO_UCP restores ASCII-only \w \d \s \b.
#define PCRE_NO_UCP (1 << 22)           // clear PCRE2_UCP
#define PCRE_ENDANCHOR (1 << 23)        // PCRE2_ENDANCHORED
#define PCRE_NO_AUTO_CAPTURE (1 << 24)  // PCRE2_NO_AUTO_CAPTURE
#define PCRE_FIRSTLINE (1 << 25)        // PCRE2_FIRSTLINE
#define PCRE_DUPNAMES (1 << 26)         // PCRE2_DUPNAMES
#define PCRE_NOTEMPTY (1 << 27)         // PCRE2_NOTEMPTY
#define PCRE_NOTBOL (1 << 28)           // PCRE2_NOTBOL
#define PCRE_NOTEOL (1 << 29)           // PCRE2_NOTEOL
#define PCRE_PARTIAL (1 << 30)          // PCRE2_PARTIAL_SOFT

#endif  // SRC_INCLUDE_PCRE_FLAGS_H_
