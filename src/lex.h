#define MLEN 4096
#define NSIZE 256
#define MAX_INSTRS 512

struct lpc_predef_s {
    char *flag;
    struct lpc_predef_s *next;
};

extern struct lpc_predef_s * lpc_predefs;
