---
title: contrib / update_pluralize
---
# update_pluralize

### NAME

    update_pluralize - add, replace or remove exceptions used by pluralize()

### SYNOPSIS

    void update_pluralize(mapping words);
    void update_pluralize(string word, string plural);
    void update_pluralize(string word);

### DESCRIPTION

[pluralize()](pluralize) checks the last word of its argument against a table
of exceptions before it falls back to its suffix rules ("-y" -> "-ies", "-us"
-> "-i", and so on). The table always holds the driver's built-in exceptions
(child -> children, goose -> geese, sheep -> sheep, ...). update_pluralize()
adds entries to it from LPC.

The entries added from LPC are kept as a separate layer on top of the
built-ins. An LPC entry for a word that also has a built-in entry wins, and
removing that LPC entry brings the built-in back.

- `update_pluralize(mapping words)` replaces the whole LPC layer with the
  mapping's word : plural pairs. Every key and value must be a string.
  `update_pluralize(([]))` removes every LPC entry and leaves only the
  built-ins.
- `update_pluralize(string word, string plural)` adds one entry, or replaces
  the LPC entry for that word.
- `update_pluralize(string word)` removes the LPC entry for that word.
  Removing a word that has no LPC entry does nothing.

Words are matched ASCII case-insensitively: A-Z are lowercased before a word
is stored or looked up, and other bytes, including non-ASCII letters, are
compared as they are. Plurals are lowercased the same way. The word and its
plural are each at most 64 bytes, and the word cannot contain a space:
pluralize() only looks up the part after the last space, so a word with one
in it could never match. An empty word or plural is an error, and so is a
mapping with two keys that lowercase to the same word but give different
plurals.

An entry is stored as "drop this many bytes from the end, then append these",
worked out from the longest prefix the word and its plural share.
octopus -> octopodes shares "octop", so it is stored as "drop 2, append
'odes'". pluralize() applies that to the word as the caller wrote it, so the
caller's capitalization of the shared part survives:

```c
update_pluralize("octopus", "octopodes");
pluralize("Octopus");                  // "Octopodes"
pluralize("a large octopus of doom");  // "large octopodes of doom"
```

Every call is first passed to the master apply
[valid_pluralize()](../../apply/master/valid_pluralize). If it denies the
call, or the master does not define it, update_pluralize() raises an error
and the table is unchanged. A mapping argument is read after
valid_pluralize() returns, so changes the master makes to it are what get
stored. If any entry is invalid, the call raises an error and the LPC layer
is left as it was.

Every pluralize() call uses the same table, including calls that conjugate a
verb (smile -> smiles), so an entry applies to verbs as well as nouns. A word
that is both gets one answer for both: adding cow -> kine for cattle also turns
the verb "cow" (to intimidate) into "kine". The built-in table already has
such words: die -> dice, foot -> feet, index -> indices, staff -> staves, and
fish -> fish.

The table lives in driver memory and is shared by every object. The entries
added from LPC are gone after the driver restarts, so a mudlib that uses them
adds them again during boot, for example from the master object's create().

### SEE ALSO

    pluralize(3), valid_pluralize(4)
