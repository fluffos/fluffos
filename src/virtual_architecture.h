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
