#ifdef LPC_TO_C
void
     C_CALL_OTHER(svalue * ret, svalue * s0, svalue * s1, int num_arg);

void
     C_SSCANF(svalue * ret, svalue * s0, svalue * s1, int num_arg);

void
     C_PUSH_LVALUE(struct svalue * s0);

void
     C_UNDEFINED PROT((struct svalue *));

void
     C_AGGREGATE(struct svalue * ret, int num);

void
     C_ASSOC(struct svalue * ret, int num);

void
     C_CALL(struct svalue * ret, unsigned short numargs, unsigned short func_index);

void
     C_PROG_STRING(svalue * ret, int string_number);

void
     C_STRING PROT((svalue *, char *, int));

int
    C_IS_FALSE(svalue * s0);

int
    C_IS_TRUE(svalue * s0);

int
    C_SV_FALSE(svalue * s0);

int
    C_SV_TRUE(svalue * s0);

void
     C_OBJECT(svalue * ret, struct object * ob);

void
     C_NUMBER PROT((svalue *, int));

void
     C_REAL(svalue * ret, double r);

void
     C_BUFFER PROT((svalue *, struct buffer *));

void
     C_REFED_BUFFER PROT((svalue *, struct buffer *));

void
     C_MAPPING(svalue * ret, struct mapping * m);

void
     C_REFED_MAPPING(svalue * ret, struct mapping * m);

void
     C_MALLOCED_STRING PROT((svalue * ret, char *p));

void
     C_CONSTANT_STRING(svalue * ret, char *p);

void
     C_VECTOR(svalue * ret, struct vector * v);

void
     C_REFED_VECTOR(svalue * ret, struct vector * v);

void eval_opcode(int instruction,
		      svalue * ret,
		      svalue * s0, svalue * s1);

#endif
#define GV(x) &current_object->variables[(x)]

/* needed because of runtime errors and stack deallocation */
#define free_register(x) free_svalue(x),x->type=T_NUMBER

void eval_opcode(int, struct svalue *, struct svalue *, struct svalue *);

    struct svalue *c_index(struct svalue * vec, struct svalue * i, struct svalue * tmp);

#define CATCH_START c_catch_start()

#define CATCH_ERROR c_catch_error()

#define CATCH_END c_catch_end()

/* needed for catch */
    extern int external_error_recovery_context_exists;
    extern jmp_buf error_recovery_context;
