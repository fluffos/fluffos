#ifdef LPC_TO_C
void
     C_CALL_OTHER(svalue * ret, svalue * s0, svalue * s1, int num_arg);

void
     C_SSCANF(svalue * ret, svalue * s0, svalue * s1, int num_arg);

void
     C_PUSH_LVALUE(struct svalue * s0);

void
     C_UNDEFINED(struct svalue * ret);

void
     C_AGGREGATE(struct svalue * ret, int num);

void
     C_ASSOC(struct svalue * ret, int num);

void
     C_CALL(struct svalue * ret, unsigned short numargs, unsigned short func_index);

void
     C_PROG_STRING(svalue * ret, int string_number);

void
     C_STRING(svalue * ret, char *p, int type);

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
     C_NUMBER(svalue * ret, int n);

void
     C_REAL(svalue * ret, double r);

void
     C_BUFFER(svalue * ret, struct buffer * b);

void
     C_MAPPING(svalue * ret, struct mapping * m);

void
     C_MALLOCED_STRING(svalue * ret, char *p);

void
     C_CONSTANT_STRING(svalue * ret, char *p);

void
     C_VECTOR(svalue * ret, struct vector * v);

void eval_opcode(int instruction,
		      svalue * ret,
		      svalue * s0, svalue * s1);

#endif
