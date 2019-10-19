---
layout: default
title: interactive / write_prompt
---

### NAME

    write_prompt - called when the parser wants a prompt to be written.

### SYNOPSIS

    void write_prompt( void );

### DESCRIPTION

    If  write_prompt  is present in the player object, the driver will call
    it whenever the default prompt would normally be printed.   The  driver
    will not call write_prompt when the player is in input_to or ed.

### AUTHOR

    Truilkan@TMI

