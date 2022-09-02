---
layout: default
title: buffers / buffer_transcode
---

### NAME

    buffer_transcode() - transcode a buffer from one encoding to another

### SYNOPSIS

    buffer buffer_transcode(buffer src,
                            string from_encoding,
                            string to_encoding);

### DESCRIPTION

    Transcode given buffer from encoding 'from_encoding' to encoding
    'to_encoding'.

### SEE ALSO

    string_encode, string_decode
