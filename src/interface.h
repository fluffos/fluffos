typedef struct {
    char *fname;
    void (**jump_table) ();
} interface_item_t;

extern interface_item_t interface[];
