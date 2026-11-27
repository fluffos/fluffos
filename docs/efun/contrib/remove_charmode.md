---
title: contrib / remove_charmode.pre
---
# remove_charmode

### NAME

    remove_charmode() - switches interactive object into linemode

### SYNOPSIS

    int remove_charmode(object);

### DESCRIPTION

    switches interactive object into linemode: the client is told to stop
    sending one keystroke at a time (telnet), and input already typed ahead
    is kept and read as line input from then on (a backspace in it erases
    the character before it, as it would have in line mode). If a whole
    line is already waiting, it is processed as the next command.

    returns -1 if argument missing
    -2 in case of error (object destroyed or not interactive)
    current input_to flags with char_mode unset

### SEE ALSO

    query_charmode(3), input_to(3), get_char(3)
