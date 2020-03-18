---
layout: default
title: interactive / terminal_type
---

### NAME

    terminal_type - inform the mudlib of the user's terminal type

### SYNOPSIS

    void terminal_type( string term );

### DESCRIPTION

    This apply is called on the interactive object with term set to the terminal type for the user, as reported by telnet negotiation.  If the user's client never responds (it's not telnet, for example) this will never be called.
