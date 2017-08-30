/*
 * Write out current stack trace. If there is a heart_beat(), then return the
 * object that had that heart beat. pass 1 to also dump last few instructions.
 */
const char *dump_trace(int);

// dump current stack trace into a LPC array.
struct array_t *get_svalue_trace(void);
