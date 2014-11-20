/*
 * Defines and types for LPC stackmachine code
 */

#ifndef _ICODE_H
#define _ICODE_H

// TODO: These icodes should become public api for vm<->compiler
#define PUSH_STRING (0 << 6)
#define PUSH_NUMBER (1 << 6)
#define PUSH_GLOBAL (2 << 6)
#define PUSH_LOCAL (3 << 6)

#define PUSH_WHAT (3 << 6)
#define PUSH_MASK (0xff ^ (PUSH_WHAT))

#define FOREACH_LEFT_GLOBAL 1
#define FOREACH_RIGHT_GLOBAL 2
#define FOREACH_REF 4
#define FOREACH_MAPPING 8

void i_generate___INIT(void);
void i_generate_node(struct parse_node_t *);
void i_generate_continue(void);
void i_generate_forward_jump(void);
void i_update_forward_jump(void);
void i_update_continues(void);
void i_branch_backwards(char, int);
void i_update_breaks(void);
void i_save_loop_info(struct parse_node_t *);
void i_restore_loop_info(void);
void i_generate_forward_branch(char);
void i_update_forward_branch(const char *);
void i_update_forward_branch_links(char, struct parse_node_t *);
void i_generate_else(void);
void i_initialize_parser(void);
void i_uninitialize_parser(void);
void i_generate_final_program(int);
void i_generate_inherited_init_call(int, short);

void optimize_icode(char *, char *, char *);
#endif
