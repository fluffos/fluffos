/* Mudlib-facing constants for the hook_* efuns.
 * Copy into the mudlib include path, or #include from the driver's
 * installed include/ directory. See docs/concepts/general/hooks.md. */

#ifndef LPC_HOOKS_H
#define LPC_HOOKS_H

/* Exactly one kind bit must be set in the flags argument to hook_attach().
 * For event points (object:create / object:destruct) kind 0 means "the
 * point's kind". */
#define HOOK_BEFORE 0
#define HOOK_DECIDE 0x01
#define HOOK_AFTER 0x02
#define HOOK_AROUND 0x04

#define HOOK_FAIL_CLOSED 0x10 /* with HOOK_DECIDE: a skip/error is a refuse */
/* Reserved — hook_attach() rejects these until the driver implements them. */
#define HOOK_DEFERRED 0x20
#define HOOK_OVERRIDE_NOMASK 0x40

#endif /* LPC_HOOKS_H */
