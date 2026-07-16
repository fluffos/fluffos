/*
 * ffi.h -- type codes for the FFI package (package_ffi).
 *
 * Copy this file to your mudlib include dir. The codes describe the C
 * type of an argument or return value to ffi_prepare()/ffi_call() and
 * the scalar type for ffi_read()/ffi_write(). See docs/driver/ffi.md.
 *
 * The boundary is BYTES: LPC strings are UTF-8-native and are NEVER
 * marshalled implicitly to C char*. All pointer/data crosses as a
 * `buffer`; encode with string_encode()/decode with string_decode().
 */

#ifndef _FFI_H_
#define _FFI_H_

#define FFI_VOID 0
#define FFI_INT8 1
#define FFI_UINT8 2
#define FFI_INT16 3
#define FFI_UINT16 4
#define FFI_INT32 5
#define FFI_UINT32 6
#define FFI_INT64 7
#define FFI_UINT64 8
#define FFI_FLOAT 9  /* C float  <-> LPC float */
#define FFI_DOUBLE 10 /* C double <-> LPC float */
/*
 * A pointer. As an ARGUMENT: pass a buffer; the native pointer is the
 * buffer's bytes (whatever encoding you put there). As a RETURN: an
 * integer holding the raw address -- copy from it with ffi_peek().
 */
#define FFI_POINTER 11

/* Native C int/long widths (resolved to fixed sizes at build time). */
#define FFI_INT FFI_INT32
#define FFI_UINT FFI_UINT32
#define FFI_LONG FFI_INT64
#define FFI_ULONG FFI_UINT64

#endif /* _FFI_H_ */
