/*
 * Information for allocating a block that can grow dynamically
 * using realloc. That means that no pointers should be kept into such
 * an area, as it might be moved.
 */

struct mem_block {
    char *block;
    int current_size;
    int max_size;
};

#define START_BLOCK_SIZE	4096
