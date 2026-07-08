/*
 * package_ffi -- foreign function interface for LPC.
 *
 * Loads native shared libraries and calls C functions with signatures
 * described at runtime (via libffi), including LPC function pointers as
 * C callbacks. All pointer/byte data crosses as `buffer` -- LPC strings
 * are UTF-8-native and never implicitly marshalled to char*. Every load
 * and prepare is gated by the master apply valid_ffi().
 *
 * See docs/driver/ffi-plan.md and the LPC header <ffi.h>.
 */

/* --- library & symbol --------------------------------------------- */
int ffi_load(string);
void ffi_unload(int);
int ffi_symbol(int, string);

/* --- describe & call ---------------------------------------------- */
int ffi_prepare(int, string, int, int *);
mixed ffi_call(int, mixed *);

/* --- native memory ------------------------------------------------ */
buffer ffi_alloc(int);
void ffi_free(buffer);
int ffi_sizeof(int);
buffer ffi_peek(int, int);
int ffi_address(buffer);

/* --- typed peek/poke into a buffer at an offset ------------------- */
mixed ffi_read(buffer, int, int);
void ffi_write(buffer, int, int, mixed);

/* --- struct layout ------------------------------------------------ */
mixed *ffi_struct_layout(int *);

/* --- LPC function pointer as a C callback -------------------------- */
int ffi_callback(function, int, int *);
int ffi_callback_addr(int);
void ffi_callback_free(int);

/* --- introspection ------------------------------------------------ */
string ffi_error();
mapping ffi_status();
