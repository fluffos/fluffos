/*
 * external.h -- driver-internal surface for package_external.
 *
 * LPC-visible efuns live in external.spec. This header is the hooks the
 * rest of the driver calls: socket read/close for the promise form of
 * external_start(), DEBUGMALLOC marking, and shutdown cleanup.
 */

#ifndef PACKAGES_EXTERNAL_EXTERNAL_H_
#define PACKAGES_EXTERNAL_EXTERNAL_H_

#ifdef PACKAGE_EXTERNAL

/* Called from the efun-socket STREAM read path. Returns 1 if `fd` is a
 * promise-form external_start() job (the data was consumed; do not call
 * the LPC read callback). */
int external_promise_take_read(int fd, const char* data, int len);

/* Called from socket_close() for every close of an LPC socket. `aborted`
 * is 1 when the close is not a normal EOF (object destruct, shutdown,
 * explicit force-close without SC_DO_CALLBACK): the promise is rejected.
 * A normal close marks I/O complete; the promise fulfills with
 * ({ output, exit_code }) once waitpid has also reported. */
void external_promise_closed(int fd, int aborted);

/* Drop every in-flight promise job -- called on driver shutdown, before
 * promise_cleanup() / clear_tick_events(). */
void external_cleanup(void);

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_external(void);
#endif

#endif /* PACKAGE_EXTERNAL */

#endif /* PACKAGES_EXTERNAL_EXTERNAL_H_ */
