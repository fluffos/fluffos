---
layout: default
title: strings / reg_assoc
---

### NAME

    reg_assoc() - A regular pattern substring extractor

### SYNOPSIS

    mixed  *reg_assoc(string  str,  string *pat_arr, mixed *tok_arr, void |
    mixed def);

### DESCRIPTION

    reg_assoc takes a string and explodes it into substrings  matching  the
    regular expression pattern-strings given in pat_arr and associates them
    with tokens given in tok_arr. If def (default 0) is given, it is  asso‐
    ciated  with  a  non-match. The return value is an array of two arrays,
    the 1st being an array of the form

         ({ non-match1, match1, non-match2, match2, ...,
            non-match n, match n, non-match n+1 })

    and the 2nd holds the tokens corresponding to the matches in order

         ({ def, token corresponding to match1, ....,  def,           token
    corresponding to match n, def }).

    pat_arr  and  tok_arr  must  be  of  the same sizes, the ith element in
    tok_arr being the corresponding token to the ith  element  of  pat_arr.
    pat_arr can only hold strings.

    If pat_arr (and hence tok_arr) has size 0 then the return value is sim‐
    ply ({ ({ str }), ({ def }) }).

### EXAMPLE

    #define STRING_PAT "\"(\\\\.|[^\\\"])*\""
    #define NUM_PAT "[0-9]+"

    #define F_STRING 1
    #define F_NUM 2

    reg_assoc("Blah \"blah\" test 203 hhh j 308 \"bacdcd\b\"acb",
      ({ STRING_PAT, NUM_PAT }), ({ F_STRING, F_NUM }), "no-match")

    will return
    ({ ({ "Blah ", "\"blah\"", " test ", "203", " hhh j ", "308", " ",
          "\"bacdcd\b\"", "acb" }),
       ({ "no-match", F_STRING, "no-match", F_NUM, "no-match", F_NUM,
          "no-match", F_STRING, "no-match" }) })

