/* Sample header for tools/ffi/test.py -- exercises the generator's C
 * subset: scalar/pointer/string params, void return, structs, and the
 * unsupported forms it must skip. */

extern double sqrt(double x);
extern double pow(double base, double exp);
int abs(int n);
size_t strlen(const char *s);
void qsort(void *base, size_t nmemb, size_t size, void *compar);

/* Unsupported: variadic and a raw function-pointer parameter -- both
 * must be reported and skipped, never mis-bound. */
int printf(const char *fmt, ...);
void set_handler(int (*cb)(int));

struct Point {
    int x;
    int y;
};

struct Mixed {
    char tag;
    double value;
    short flags;
};
