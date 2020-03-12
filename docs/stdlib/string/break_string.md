---
layout: default
title: stdlib / break_string
---

### NAME

    break_string


### SYNOPSIS

    string break_string(string str)
    string break_string(string str, int width)
    string break_string(string str, int width, string indent)
    string break_string(string str, int width, int space)
    string break_string(string str, int width, string indent, int flags)
    string break_string(string str, int width, int space, int flags)


### ARGUMENTS

    str - string to wrap
    width - optional: maximum line length (default 78)
    indent - optional: String that appears before each broken line
    space - optional: number of spaces before each broken line
    flags - optional: this allows the behavior of break_string ()
             to change; for possible flags see point 'description'

DESCRIPTION
============

    In the first form, the string 'str' is changed by inserting "\n"
    broken that with a subsequent output line no longer as 'width' characters.
    Possibly existing "\n" in 'str' are removed beforehand.

    If you also add a string 'indent', this will be used to  set each of
    the wrapped lines.

    Similarly, when specifying the number 'space', a string with 'space'
    characters in front of each broken line.

    In addition, there are the following optional flags, which can be combined as desired
    can:

           BS_LEAVE_MY_LFS - "\n" that already exist in the text are retained
           BS_SINGLE_SPACE - double spaces and spaces after line
                               breaks are removed
           BS_BLOCK        - the text is formatted in justification
           BS_NO_PARINDENT - for justification with specified line breaks
                               (BS_BLOCK | BS_LEAVE_MY_LFS) lines after "\ n"
                               usually started with a space.
                               To insert the leading space
                               suppress, BS_NO_PARINDENT must be specified
           BS_INDENT_ONCE - the first line of the text is preceded by
                               output 'indent'; get all the following lines
                               preceded by an empty string
           BS_PREPEND_INDENT - the Ident is prepended to the text if the
                               Indent + Text is longer than one line. The text
                               is indented by a space, which is done using
                               BS_NO_PARINDENT can be prevented.

RETURN VALUE
=============

    The wrapped text.

    Runtime error if the indent is longer than the specified width.

EXAMPLES
=========

    write(break_string("This is a longer text. Just as an example.", 27));

       => This is a longer
          Text. Just as an example.

     write(break_string("It looks like this with indent", 30, "Wargon says: "));

       => Wargon says: It looks like
          Wargon says: this with indent

    write(break_string ("It looks like this with indent", 30, "Wargon says: ",
                       BS_INDENT_ONCE));

       => Wargon says: It looks like
                       this with indent

    write(break_string("It looks like this with spaces", 30, 2));

       =>  It looks like this with
           spaces

    write(break_string("I want it\nbut pre-formatted!",60,
                       "Wargon says: ", BS_LEAVE_MY_LFS));

       => Wargon says: I want it
          Wargon says: but pre-formatted!

    write(break_string("I want it\nbut pre-formatted!",30,
                       "Wargon says: ", BS_LEAVE_MY_LFS | BS_PREPEND_INDENT));

       => Wargon says:
           I want it
           but pre-formatted!


### AUTHOR

    senderwiederholung
