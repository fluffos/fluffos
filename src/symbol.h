#ifndef DB_H_J7ASBHTE
#define DB_H_J7ASBHTE

#define OP_SYMBOL_INC (1)
#define OP_SYMBOL_DEFINE (2)
#define OP_SYMBOL_VAR (3)
#define OP_SYMBOL_FUNC (4)
#define OP_SYMBOL_NEW (5)
#define OP_SYMBOL_POP (6)
#define OP_SYMBOL_FREE (7)

void symbol_enable(int op);

void symbol_start(const char *filename);

void symbol_end();

void symbol_record(int op, const char *file, int line, const char *detail);

#endif /* end of include guard: DB_H_J7ASBHTE */
