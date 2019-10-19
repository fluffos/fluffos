---
layout: default
title: mudlib / set_light
---

### NAME

    set_light() - update or query an object's light level

### SYNOPSIS

    int set_light( int light_level_adjustment );

### DESCRIPTION

    Passing  <light_level_adjustment>  as  0  queries  the object's current
    light level.  A positive number will increase the light level, while  a
    negative number will decrease the light level.

