---
layout: default
title: arrays / allocate
---

### NAME

    allocate() - allocate an array

### SYNOPSIS

<?prettify?>
<pre class="prettyprint lang-c">
varargs mixed *allocate( int size, void | mixed value );
</pre>

### DESCRIPTION

Allocate  an  array of <size> elements.  The number of elements must be>= 0 and not bigger than a system maximum (usually ~10000).   All  elements are initialized to 0 by default.

If the optional second argument is provided, the values are initialized to that argument, unless the second argument
 is a functional.  In  that case, the functional will be evaluated for each array element, with the first argument
  passed to the functional being the number of  the  array element.

Example:

<?prettify?>
<pre class="prettyprint">
allocate( 5, (: $1 + 3 :) ) => ({ 3, 4, 5, 6, 7 })
</pre>

### SEE ALSO

    sizeof(3), allocate_mapping(3)

