// Mudlib copy of src/include/pcre_flags.h. Keep the two files in sync.
#ifndef SRC_INCLUDE_PCRE_FLAGS_H_
#define SRC_INCLUDE_PCRE_FLAGS_H_

#define PCRE_DEFAULT 0

#define PCRE_I (1 << 16)  // PCRE2_CASELESS
#define PCRE_M (1 << 17)  // PCRE2_MULTILINE
#define PCRE_S (1 << 18)  // PCRE2_DOTALL
#define PCRE_U (1 << 19)  // PCRE2_UNGREEDY
#define PCRE_X (1 << 20)  // PCRE2_EXTENDED
#define PCRE_A (1 << 21)  // PCRE2_ANCHORED
#define PCRE_NO_UCP (1 << 22)
#define PCRE_ENDANCHOR (1 << 23)
#define PCRE_NO_AUTO_CAPTURE (1 << 24)
#define PCRE_FIRSTLINE (1 << 25)
#define PCRE_DUPNAMES (1 << 26)
#define PCRE_NOTEMPTY (1 << 27)
#define PCRE_NOTBOL (1 << 28)
#define PCRE_NOTEOL (1 << 29)
#define PCRE_PARTIAL (1 << 30)

#endif  // SRC_INCLUDE_PCRE_FLAGS_H_
