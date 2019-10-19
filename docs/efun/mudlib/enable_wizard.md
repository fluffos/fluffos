---
layout: default
title: mudlib / enable_wizard
---

### NAME

    enable_wizard() - give wizard priveleges to an object

### SYNOPSIS

    void enable_wizard( void );

### DESRIPTION

    Any  interactive object that calls enable_wizard() will cause wizardp()
    to return true if called on that object.  enable_wizard()  gives  three
    privileges to the interactive object in question:

    1.  ability to use restricted modes of ed when the RESTRICTED_ED option
        is compiled into the driver.

    2.  privilege of receiving descriptive runtime error messages.

    3.  privilege of using the trace(3) and traceprefix(3) efuns.

### SEE ALSO

    disable_wizard(3), wizardp(3)

