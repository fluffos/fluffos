#ifndef PREPROCESS_H
#define PREPROCESS_H

static void add_define PROT((char *, int, char *));
static void handle_elif();
static void handle_else();
static void handle_endif();
static int cond_get_exp PROT((int));
static void handle_cond PROT((int));

defn_t *lookup_define PROT((char *));

#endif
