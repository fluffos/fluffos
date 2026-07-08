---
layout: doc
title: contrib / roulette_wheel
---
# roulette_wheel

### NAME

    roulette_wheel() - weighted-random pick of a mapping key

### SYNOPSIS

    mixed roulette_wheel( mapping weights );

### DESCRIPTION

    Given a mapping whose values are non-negative integer weights,
    returns one of its keys chosen at random with probability
    proportional to that key's weight ("roulette-wheel" selection).
    Errors on an empty mapping or a negative/non-integer weight.

### SEE ALSO

    random(3)
