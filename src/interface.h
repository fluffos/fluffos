#include "interpret.h"

struct interface_item {
    char *fname;
    void (**jump_table) (struct svalue *);
};

extern struct interface_item interface[];
