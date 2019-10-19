---
layout: default
title: master / domain_file
---

### NAME

    domain_file - determine the domain for a given object

### SYNOPSIS

    string domain_file( string file );

### DESCRIPTION

    This  function  must  exist  in the master object.  It is called by the
    domain statistic functions in the driver to  determine  what  domain  a
    given  object should be associated with.  This is totally arbitrary and
    up to the mudlib designers wishes.  It should be noted that the  domain
    that  the  object  is  assigned to will receive "credit" for all of the
    objects behavior (errors, heart_beats, worth, etc).

    At startup the driver will ask  for  the  backbone  domain  by  calling
    domain_file("/").

### SEE ALSO

    author_stats(3), domain_stats(3), author_file(4)

### AUTHOR

    Wayfarer@Portals

