---
title: master / valid_pluralize
---
# valid_pluralize

### NAME

    valid_pluralize - controls the use of the update_pluralize() efun

### SYNOPSIS

    int valid_pluralize(object caller, mixed what, string plural);

### DESCRIPTION

The driver calls this apply on the master object every time
[update_pluralize()](../../efun/contrib/update_pluralize) is called, before it
changes the exception table that [pluralize()](../../efun/contrib/pluralize)
uses.

- `caller` - the object that called update_pluralize().
- `what` - the efun's first argument: the mapping, or the word.
- `plural` - the plural being added. It is undefined in the mapping form and
  in the single-argument remove form, so `undefinedp(plural)` with a string
  `what` means a removal.

Return a nonzero value to allow the change. Return 0 to deny it; the efun
then raises the error "Master object denied permission to
update_pluralize()." and the table is unchanged.

If the master object does not define valid_pluralize(), every call to
update_pluralize() is denied. Calls made before the master object has loaded
are allowed.

When `what` is a mapping, it is the caller's mapping itself, not a copy. The
efun reads it after this apply returns, so entries the master adds, changes or
removes are what get stored.

### SEE ALSO

    update_pluralize(3), pluralize(3)
