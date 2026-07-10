---
title: general / FluffOS driver
---
# The FluffOS driver

The FluffOS driver is the program (written in C++) which provides the low-level
support that makes a mud possible. The driver does many things, including:

1. Accepts connections from remote machines (via a listen port) and hands each
   new connection to the master object's `connect()` apply, which returns the
   login object that the connection is attached to.

2. Provides a set of external functions (efuns) that may be called from within
   LPC objects.

3. Compiles LPC source files into a compact internal bytecode form. This happens
   the first time an object is loaded, for example via `load_object(filename)` or
   `clone_object(filename)`.

4. Interprets (executes) objects represented in the bytecode form. The two main
   ways in which code gets executed are as follows:

   a) The driver calls functions in objects based on input received from users
   (via their connection). The specific functions that get called depend on what
   associations the objects of the mud have specified between player-typed
   commands and functions (via the `add_action(function_name, command_name)`
   efun). The driver also calls functions in LPC objects at certain lifecycle
   events; these callbacks are known as *applies* (such as `create`, `init`, and
   `clean_up`).

   b) Objects can cause the driver to execute code in other objects via the
   `call_other(object, function_name, args, ...)` efun. An alternate form of the
   `call_other` efun is `object->function_name(args, ...)`.
