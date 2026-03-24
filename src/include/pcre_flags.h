// PCRE efun flags (kept in high bits to avoid legacy flag collisions)
#ifndef SRC_INCLUDE_PCRE_FLAGS_H_
#define SRC_INCLUDE_PCRE_FLAGS_H_

#define PCRE_DEFAULT 0

// Compile-time options
#define PCRE_I (1 << 16)  // PCRE_CASELESS
#define PCRE_M (1 << 17)  // PCRE_MULTILINE
#define PCRE_S (1 << 18)  // PCRE_DOTALL
#define PCRE_U (1 << 19)  // PCRE_UNGREEDY
#define PCRE_X (1 << 20)  // PCRE_EXTENDED

// Exec-time option
#define PCRE_A (1 << 21)  // PCRE_ANCHORED

#endif  // SRC_INCLUDE_PCRE_FLAGS_H_
