#ifndef SWAP_H
#define SWAP_H

/*
 * swap.c
 */
int swap PROT((struct object *));
int swap_line_numbers PROT((struct program *));
void load_ob_from_swap PROT((struct object *));
void load_line_numbers PROT((struct program *));
void remove_swap_file PROT((struct object *));
void unlink_swap_file PROT((void));
void remove_line_swap PROT((struct program *));
int locate_in PROT((struct program *));
int locate_out PROT((struct program *));
void print_swap_stats PROT((void));

#endif
