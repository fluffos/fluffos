/*
 * ffi.h -- internal declarations for package_ffi.
 *
 * The LPC-visible type codes live in src/include/ffi.h (shipped to the
 * mudlib). This header is the driver-internal surface: the cleanup and
 * DEBUGMALLOC-marking hooks the rest of the driver calls.
 */

#ifndef PACKAGES_FFI_FFI_H_
#define PACKAGES_FFI_FFI_H_

#ifdef PACKAGE_FFI

/* Release every open library, prepared function, allocation and
 * callback -- called on driver shutdown. */
void ffi_cleanup(void);

#ifdef DEBUGMALLOC_EXTENSIONS
/* Account for live FFI state (allocations held as buffers are already
 * GC-tracked; this marks the handle tables and the callback funptrs). */
void mark_ffi(void);
#endif

#endif /* PACKAGE_FFI */

#endif /* PACKAGES_FFI_FFI_H_ */
