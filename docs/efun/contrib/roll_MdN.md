---
layout: default
title: contrib / roll_MdN.pre
---

### NAME

    roll_MdN() - dice roll generator where you specify the number of dice and
    the number of sides to each die, with bonus if supplied.

### SYNOPSIS

    int roll_MdN(int rolls, int sides, int bonus : default = 0)

### DESCRIPTION

    rolls `sides`-sided die `rolls` number of times and returns the sum
    of all rolls

    if `bonus` is supplied, will add the bonus and return the result.

### EXAMPLE

    // roll a 1d4
    roll_MdN(1, 4)

    // roll 2d6
    roll_MdN(2, 6)

    // roll 1d10 and add 15
    roll_MdN(1, 10, 15)
