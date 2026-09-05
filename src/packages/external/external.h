/*
 * external.h -- driver-internal surface for package_external.
 *
 * LPC-visible efuns live in external.spec. This header is the hooks the
 * rest of the driver calls: owner-destruct cleanup, DEBUGMALLOC marking,
 * and shutdown cleanup for external_create() handles.
 */

#ifndef PACKAGES_EXTERNAL_EXTERNAL_H_
#define PACKAGES_EXTERNAL_EXTERNAL_H_

#ifdef PACKAGE_EXTERNAL

/* Abort every handle owned by `ob` -- called from destruct_object(). */
void external_owner_destructed(struct object_t* ob);

/* Drop every handle -- called on driver shutdown, before promise_cleanup()
 * / clear_tick_events(). */
void external_cleanup(void);

#ifdef DEBUGMALLOC_EXTENSIONS
void mark_external(void);
#endif

#endif /* PACKAGE_EXTERNAL */

#endif /* PACKAGES_EXTERNAL_EXTERNAL_H_ */
