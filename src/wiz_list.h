struct wiz_list {
    char *name;
    int length;
    struct wiz_list *next;
    int moves;
    int cost;
    int heart_beats;
    int total_worth;
    int size_array;		/* Total size of this wizards arrays. */
    int errors;
    int objects;
    /*
     * The following values are all used to store the last error
     * message.
     */
    char *file_name;
    char *error_message;
    int line_number;
};

extern struct wiz_list *add_name PROT((char *));
struct value;
extern void save_wiz_file(), load_wiz_file(), wiz_decay();
