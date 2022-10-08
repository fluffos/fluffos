---
layout: default
title: contrib / roll_MdN.pre
---

### NAME

    roll_MdN() - dice roll generator where you specify the number of dice and
    the number of sides to each die.

### SYNOPSIS

    int roll_MdN(int rolls, int sides)

### DESCRIPTION

    rolls `sides`-sided die `rolls` number of times and returns the sum
    of all rolls

### EXAMPLE

    // roll a 1d4
    roll_MdN(1, 4)

    // roll 2d6
    roll_MdN(2, 6)
