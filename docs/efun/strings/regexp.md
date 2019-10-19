---
layout: default
title: strings / regexp
---

### NAME

    regexp() - regular expression handler

### SYNOPSIS

    string *regexp( string *lines, string pattern, void | int flag );

### DESCRIPTION

    Typically  when  presented with an array of lines of text and a regular
    expression, regexp(3) returns an array  containing  those  lines  which
    match  the  pattern  specified  by  the regular expression. If the flag
    (default 0) has bit 2 set, then non-matches will be returned instead of
    matches.  If the flag  has bit 1 set, the array returned will be of the
    form ({ index1 + 1, match1, ..., indexn + 1, matchn }) where index1  is
    the index of 1st match/non match in the array lines.

### REGULAR EXPRESSION SYNTAX

    A  regular  expression  is zero or more branches, separated by '|'.  It
    matches anything that matches one of the branches.

    A branch is zero or more pieces, concatenated.  It matches a match  for
    the first, followed by a match for the second, etc.

    A piece is an atom possibly followed by '*', '+', or '?'.  An atom fol‐
    lowed by '*' matches a sequence of 0 or more matches of the  atom.   An
    atom  followed  by  '+'  matches a sequence of 1 or more matches of the
    atom.  An atom followed by '?' matches a match of the atom, or the null
    string.

    An  atom  is  a regular expression in parentheses (matching a match for
    the regular expression), a range (see below), '.'  (matching any single
    character), '^' (matching the null string at the beginning of the input
    string), '$' (matching the null string at the end of the input string),
    a  '\'  followed  by a single character (matching that character), or a
    single character with no other significance (matching that character).

    A range is a sequence of characters  enclosed  in  '[]'.   It  normally
    matches any single character from the sequence.  If the sequence begins
    with '^', it matches any single character not  from  the  rest  of  the
    sequence.  If two characters in the sequence are separated by '-', this
    is shorthand for the full list of ASCII characters between  them  (e.g.
    '[0-9]'  matches  any  decimal digit).  To include a literal ']' in the
    sequence, make it the first character (following a possible  '^').   To
    include a literal '-', make it the first or last character.

### AMBIGUITY

    If  a  regular  expression could match two different parts of the input
    string, it will match the one which begins earliest.  If both begin  in
    the same place but match different lengths, or match the same length in
    different ways, life gets messier, as follows.

    In general, the possibilities in a list of branches are  considered  in
    left-to-right  order,  the possibilities for '*', '+', and '?' are con‐
    sidered longest-first, nested constructs are considered from the outer‐
    most  in,  and  concatenated  constructs are considered leftmost-first.
    The match that will be chosen is the one that uses the earliest  possi‐
    bility  in the first choice that has to be made.  If there is more than
    one choice, the next will be made in the same manner  (earliest  possi‐
    bility) subject to the decision on the first choice.  And so forth.

    For  example,  '(ab|a)b*c'  could  match 'abc' in one of two ways.  The
    first choice is between 'ab' and 'a'; since 'ab' is earlier,  and  does
    lead  to  a  successful  overall match, it is chosen.  Since the 'b' is
    already spoken for, the 'b*' must match its last possibility—the  empty
    string—since it must respect the earlier choice.

    In  the particular case where no '|'s are present and there is only one
    '*', '+', or '?', the net effect is that  the  longest  possible  match
    will  be  chosen.   So  'ab*',  presented  with  'xabbbby',  will match
    'abbbb'.  Note that if 'ab*' is  tried  against  'xabyabbbz',  it  will
    match  'ab'  just  after  'x',  due  to  the begins-earliest rule.  (In
    effect, the decision on where to start the match is the first choice to
    be  made,  hence  subsequent choices must respect it even if this leads
    them to less-preferred alternatives.)

### SEE ALSO

    sscanf(3), explode(3), strsrch(3), ed(3)

