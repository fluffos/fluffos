void replace_programs();

struct replace_ob {
    struct object *ob;
    struct program *new_prog;
    unsigned short var_offset;
    struct replace_ob *next;
};


extern struct replace_ob *obj_list_replace;
