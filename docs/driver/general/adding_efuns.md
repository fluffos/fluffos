---
layout: default
title: general / adding_efuns
---

This document explains how to add efunctions to FluffOS driver. Here are the
steps:

1. Creating an directory under `src/packages/` with name `mypkg`.

1. inside `src/packages/mypkg` directory, create LPC function prototype file called `mypkg.spec`.

   For example, the prototype for the cat() efun is as follows:

    `int cat(string, void|int, void|int);`

   The second and third arguments of `cat` can either be `void` or `int`. Here
   'void' means optional. Thus `cat("xyz")` is allowed as is `cat("xyz",5,8);`
   The return type of `cat()` is int.

   If a function is to allow a variable number of arguments (vararg), you can
   declare with trailing elipses:

   `void call_out(string, int, ...);`

   call_out() does not return a value.

1. Create an file `mypkg.cc` and create an `CMakeLists.txt` to compile it, add an library target called `PACKAGE_MYPKG`.

1. In your `mypkg.cc`, start by including the pkg API header,

    `#include "base/package_api.h"`

   Then add a function of the form `f_efunction_name() {}`

   For examples of how to structure the f_efunction_name function, look at the other efun already defined in
    the driver.

   In addition, you should read the file ./doc/driver/stackmachine. This file is fairly old but is still
     useful toward understanding the general structure of the driver stack machine.

   The main points to know are that the arguments passed to an efunction are stored on a stack of svalues named `sp`.
   For example, if the efunction is passed three arguments, then the first argument will be at `(sp - 2)`, the
   second at `(sp - 1)`, and the third at sp. If a function allows a variable number of arguments, then the
   num_arg parameter of the efun will be set to indicate the number of arguments actually passed (on the sp stack).

   Each efunction is responsible for leaving a single svalue on the sp stack at the time the efunction exits(even if
    the efunction is defined to return void).

   The usual strategy is to use the stack values that you need `(sp - x)`, `pop_n_elems(num_args)` and then push a
    result onto the stack.

   Note, if an efunction takes a single argument and is defined to return void, then you needn't pop anything from
    the stack or push anything on (the single argument can serve as the return value).

