---
title: general / next_bit
---
# next_bit

### NAME

    next_bit() - find the next set bit in a bit string

### SYNOPSIS

    int next_bit(string bitstring, int start);

### DESCRIPTION

    Returns the position of the next set (1) bit in `bitstring` that lies
    strictly after position `start`, or -1 if there is no set bit past
    `start`.

    `bitstring` is a bit string in the compact format produced and consumed
    by set_bit(), test_bit(), clear_bit() and clear_bits(): each character
    stores six bits, encoded as the character value minus ' ' (0x20), so bit
    N lives in character N / 6.

    Because the search is exclusive of `start`, passing -1 begins the walk
    before bit 0 and so finds the first set bit; feeding each result back in
    as the new `start` visits every set bit in ascending order. This makes
    next_bit() the idiomatic way to iterate the set bits of a bit string
    without scanning every bit position by hand.

### EXAMPLE

    // Walk every set bit in `bits`.
    for (int b = next_bit(bits, -1); b != -1; b = next_bit(bits, b)) {
        printf("bit %d is set\n", b);
    }

### SEE ALSO

    set_bit(3), test_bit(3), clear_bit(3), clear_bits(3)
