/*
 * options.h: this file contains options that used to be compile time options
 *            in the driver.  the options have been removed from the driver
 *            and are now to be simulated in LPC as part of the mudlib if they
 *            are really wanted/needed.  removal of options from the driver is
 *            a currently a work in progress, so those options that are still
 *            part of the driver (and relevant here) are commented out.
 */

#ifndef _MUDLIB_OPTIONS_H_
#define _MUDLIB_OPTIONS_H_

/****************************************************************************
 *                          COMPATIBILITY                                   *
 *                         ---------------                                  *
 * The MudOS driver has evolved quite a bit over the years.  These defines  *
 * are mainly to preserve old behavior in case people didn't want to        *
 * rewrite the relevant portions of their code.                             *
 *									    *
 * In most cases, code which needs these defines should be rewritten when   *
 * possible.  The 'Compat status' field is designed to give an idea how     *
 * likely it is that support for that option will be removed in the near    *
 * future.  Certain options are fairly easy to work around, and double      *
 * the size of the associated code, as well as the maintenance workload,    *
 * and can make the code significantly more complex or harder to read, so   *
 * supporting them indefinitely is impractical.                             *
 *                                                                          *
 * WARNING: If you are using software designed to run with the MudOS driver *
 *          it may assume certain settings of these options.  Check the     *
 *          instructions for details.                                       *
 ****************************************************************************/

/* NONINTERACTIVE_STDERR_WRITE: if defined, all writes/tells/etc to
 *   noninteractive objects will be written to stderr prefixed with a ']'
 *   (old behavior).
 *
 * Compat status: Easy to support, and also on the "It's a bug!  No, it's
 * a feature!" religious war list.
 */
/*#undef NONINTERACTIVE_STDERR_WRITE*/

/* NO_LIGHT: define this to disable the set_light() and driver maintenance
 *   of light levels in objects.  You can simulate it via LPC if you want...
 *
 * Compat status: Very dated, easy to simulate, and gross.
 */
/*#define NO_LIGHT*/

/* NO_ADD_ACTION: define this to remove add_action, commands, livings, etc.
 * process_input() then becomes the only way to deal with player input. 
 *
 * Compat status: next to impossible to simulate, hard to replace, and 
 * very, very widely used.
 */
/*#undef NO_ADD_ACTION*/

/* NO_SNOOP: disables the snoop() efun and all related functionality.
 */
/*#undef NO_SNOOP*/

/* NO_ENVIRONMENT: define this to remove the handling of object containment
 * relationships by the driver 
 *
 * Compat status: hard to simulate efficiently, and very widely used.
 */
/*#undef NO_ENVIRONMENT*/

/* NO_WIZARDS: for historical reasons, MudOS used to keep track of who
 * is and isn't a wizard.  Defining this removes that completely.
 * If this is defined, the wizardp() and related efuns don't exist.
 *
 * Also note that if it is not defined, then non-wizards are always put
 * in restricted mode when ed() is used, regardless of the setting of
 * the restrict parameter.
 *
 * Compat status: easy to simulate and dated.
 */
#define NO_WIZARDS

/* OLD_ED: ed() efun backwards compatible with the old version.  The new
 * version requires/allows a mudlib front end.
 *
 * Compat status: Easily simulated.
 */
#define OLD_ED

/****************************************************************************
 *                           MISCELLANEOUS                                  *
 *                          ---------------                                 *
 * Various options that affect the way the driver behaves.                  *
 *                                                                          *
 * WARNING: If you are using software designed to run with the MudOS driver *
 *          it may assume certain settings of these options.  Check the     *
 *          instructions for details.                                       *
 ****************************************************************************/

/* NO_RESETS: completely disable the periodic calling of reset() */
/*#undef NO_RESETS*/

/* LAZY_RESETS: if this is defined, an object will only have reset()
 *   called in it when it is touched via call_other() or move_object()
 *   (assuming enough time has passed since the last reset).  If LAZY_RESETS
 *   is #undef'd, then reset() will be called as always (which guaranteed that
 *   reset would always be called at least once).  The advantage of lazy
 *   resets is that reset doesn't get called in an object that is touched
 *   once and never again (which can save memory since some objects won't get
 *   reloaded that otherwise would).
 */
/*#undef LAZY_RESETS*/

/* NO_ANSI: define if you wish to disallow users from typing in commands that
 *   contain ANSI escape sequences.  Defining NO_ANSI causes all escapes
 *   (ASCII 27) to be replaced with a space ' ' before the string is passed
 *   to the action routines added with add_action.
 *
 * STRIP_BEFORE_PROCESS_INPUT allows the location where the stripping is 
 * done to be controlled.  If it is defined, then process_input() doesn't
 * see ANSI characters either; if it is undefined ESC chars can be processed
 * by process_input(), but are stripped before add_actions are called.
 * Note that if NO_ADD_ACTION is defined, then #define NO_ANSI without
 * #define STRIP_BEFORE_PROCESS_INPUT is the same as #undef NO_ANSI.
 *
 * If you anticipate problems with users intentionally typing in ANSI codes
 * to make your terminal flash, etc define this.
 */
/*#define NO_ANSI*/
/*#define STRIP_BEFORE_PROCESS_INPUT*/

/* THIS_PLAYER_IN_CALL_OUT: define this if you wish this_player() to be
 *   usable from within call_out() callbacks.
 */
#define THIS_PLAYER_IN_CALL_OUT

/* CALLOUT_HANDLES: If this is defined, call_out() returns an integer, which
 * can be passed to remove_call_out() or find_call_out().  Removing call_outs
 * by name is still allowed, but is significantly less efficient, and also
 * doesn't work for function pointers.  This option adds 4 bytes overhead
 * per callout to keep track of the handle.
 */
#define CALLOUT_HANDLES

/* FLUSH_OUTPUT_IMMEDIATELY: Causes output to be written to sockets
 * immediately after being generated.  Useful for debugging.  
 */
/*#undef FLUSH_OUTPUT_IMMEDIATELY*/

/* PRIVS: define this if you want object privileges.  Your mudlib must
 *   explicitly make use of this functionality to be useful.  Defining this
 *   this will increase the size of the object structure by 4 bytes (8 bytes
 *   on the DEC Alpha) and will add a new master apply during object creation
 *   to "privs_file".  In general, privileges can be used to increase the
 *   granularity of security beyond the current root uid mechanism.
 *
 * [NOTE: for those who'd rather do such things at the mudlib level, look at
 *  the inherits() efun and the 'valid_object' apply to master.]
 */
/*#undef PRIVS*/

/* INTERACTIVE_CATCH_TELL: define this if you want catch_tell called on
 *   interactives as well as NPCs.  If this is defined, user.c will need a
 *   catch_tell(msg) method that calls receive(msg);
 */
/*#undef INTERACTIVE_CATCH_TELL*/

/* RESTRICTED_ED: define this if you want restricted ed mode enabled.
 */
#define RESTRICTED_ED

/* SNOOP_SHADOWED: define this if you want snoop to report what is
 *   sent to the player even in the event that the player's catch_tell() is
 *   shadowed and the player may not be seeing what is being sent.  Messages
 *   of this sort will be prefixed with $$.
 */
/*#undef SNOOP_SHADOWED*/

/* RECEIVE_SNOOP: define this if you want snoop text to be sent to
 *   the receive_snoop() function in the snooper object (instead of being
 *   sent directly via add_message()).  This is useful if you want to
 *   build a smart client that does something different with snoop messages.
 */
/*#undef RECEIVE_SNOOP*/

/****************************************************************************
 *                            UID PACKAGE                                   *
 *                            -----------                                   *
 * UIDS are the basis for some mudlib security systems.  Basically, they're *
 * preserved for backwards compatibility, as several ways of breaking       *
 * almost any system which relies on them are known.  (No, it's not a flaw  *
 * of uids; only that b/c of the ease with which LPC objects can call       *
 * each other, it's far too easy to leave holes)                            *
 *                                                                          *
 * If you don't care about security, the first option is probably what you  *
 * want.                                                                    *
 ****************************************************************************/

/*
 * PACKAGE_UIDS: define this if you want a driver that does use uids.
 *
 */
/*#undef PACKAGE_UIDS*/

/* AUTO_SETEUID: when an object is created it's euid is automatically set to
 *   the equivalent of seteuid(getuid(this_object())).  undef AUTO_SETEUID
 *   if you would rather have the euid of the created object be set to 0.
 */
/*#undef AUTO_SETEUID*/

/* AUTO_TRUST_BACKBONE: define this if you want objects with the backbone
 *   uid to automatically be trusted and to have their euid set to the uid of
 *   the object that forced the object's creation.
 */
/*#define AUTO_TRUST_BACKBONE*/

/*************************************************************************
 *                       FOR EXPERIENCED USERS                           *
 *                      -----------------------                          *
 * Most of these options will probably be of no interest to many users.  *
 *************************************************************************/

/* HEARTBEAT_INTERVAL: define heartbeat interval in microseconds (us).
 *   1,000,000 us = 1 second.  The value of this macro specifies
 *   the frequency with which the heart_beat method will be called in
 *   those LPC objects which have called set_heart_beat(1).
 *
 * [NOTE: if ualarm() isn't available, alarm() is used instead.  Since
 *  alarm() requires its argument in units of a second, we map 1 - 1,000,000 us
 *  to an actual interval of one (1) second and 1,000,001 - 2,000,000 maps to
 *  an actual interval of two (2) seconds, etc.]
 */
#define HEARTBEAT_INTERVAL 2000000

/* 
 * CALLOUT_CYCLE_SIZE: This is the number of slots in the call_out list.
 * It should be approximately the average number of active call_outs, or
 * a few times smaller.  It should also be a power of 2, and also be relatively
 * prime to any common call_out lengths.  If all this is too confusing, 32
 * isn't a bad number :-)
 */
#define CALLOUT_CYCLE_SIZE 32

/* HEART_BEAT_CHUNK: The number of heart_beat chunks allocated at a time.
 * A large number wastes memory as some will be sitting around unused, while
 * a small one wastes more CPU reallocating when it needs to grow.  Default
 * to a medium value.
 */
#define HEART_BEAT_CHUNK      32

#endif
