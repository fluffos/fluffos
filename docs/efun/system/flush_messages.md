---
layout: default
title: system / flush_messages
---

### NAME

    flush_messages - send all pending messages to a user

### SYNOPSIS

    int flush_messages();
    int flush_messages(object user);

### DESCRIPTION

    Normally, messages are queued, then sent all at once to minimize the
    number of packets required.  This efun forces all pending messages to
    be written immediately.  If no user is specified, messages for ALL users
    are flushed.
