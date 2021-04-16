---
layout: default
title: functions / defer
---

### NAME

    defer() - execute function after current function ends

### SYNOPSIS

    void defer(function f)

### DESCRIPTION

    Call function pointer *f when current function ends (even if it 
    was due to a runtime error).
     
    For example: 
    
    void create()
    {
      ::create();
      
      defer( (: enable_commands :) );
    }
    
    The effect the defer() function has is it will cause the 
    enable_commands() efun to be called after the execution of the
    create() function ends.  The argument passed to defer() can be
    any function type.

