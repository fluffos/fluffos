---
layout: default
title: driver / call_into_vm.md
---

Version: master

Author: sunyucong@gmail.com
Date: 2017-08-29

Driver code are consisted of roughly 2 parts.

1. VM related code. This is the LPC vm and assoicated stack machine. All EFUN functions also belongs to this part.
2. Non VM code, this mostly incldue communication related stuff.

To execute LPC, Non VM code needs to do some preparations:
1. Push control frame to control frame stack.
2. Push svalue_t arguments into stack
3. set PC and various variables for VM code.
4. call execute_instruction(pc)
5. deal with various error condition, if no error, use the value and reset everything.

Normally, Non VM code *should not* go through this process, as there is currently several helper functions that most of
the driver code uses.

1. safe_apply()
2. safe_call_function_pointer()
3. ...


However, several parts of driver can't use these functions, instead they are using
1. apply()
2. call_direct()
3. call_program()

For these code, the only correct way of doing this is documented here.
'''
    // use push_number() , push_malloc_string() etc to push arguments into stack
    num_arg = X; // MUST remember how many arguments were pushed.

    // setup error context
    error_context_t econ;
    save_context(&econ);

    try {
    ret = call_function_pointer(funp, num_arg);
    } catch (const char *) {
    restore_context(&econ);
    /* condition was restored to where it was when we came in */
    pop_n_elems(num_arg);
    ret = nullptr;
    }
    pop_context(&econ);
'''

(unfinished)
LPC VM use C++ Exception to handle *any* error encountered during LPC execution. This could come from following non-exausted list

1. error generated from 'throw()'' in LPC code.
2. 'error()' function from EFUN implementations.
