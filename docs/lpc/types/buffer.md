---
title: types / buffer
---
# buffer

'buffer' is a cross between the LPC array type and the LPC string type.
'buffer' is intended as a way to conveniently manipulate binary data.
'buffer' is not zero-terminated (that is, it has an associated length). A
'buffer' is an array of bytes that is implemented using one byte per
element. Like arrays and mappings, buffers are reference types.

Create a buffer with `allocate_buffer(size)` (bytes start at 0), or read one
from a file with `read_buffer(file_name, ...)` (same args as `read_bytes`);
write with `int write_buffer(string file, int start, mixed source)`.
`sizeof(buf)` returns the byte count and `bufferp(buf)` tests the type.
Buffers concatenate with `buf = buf1 + buf2` and `buf += buf1`. The socket
efuns accept and return the 'buffer' type (`STREAM_BINARY` (3) and
`DATAGRAM_BINARY` (4) modes).

### Indexing and ranges

Bytes read as ints in the range 0..255, and every write path (`=`, `++`,
`--`, `+=`, `-=`) checks that the resulting value stays in that range —
assigning a negative value or anything above 255 raises a catchable error
and leaves the byte unchanged. Forward, reverse, and open-ended indexes and
ranges all work, and a range is also assignable — the buffer resizes when
the replacement has a different length:

```c
buffer buf = allocate_buffer(4);

buf[0] = 200;          // write a byte
int x = buf[0];        // 200 -- always unsigned
int y = buf[<1];       // last byte

buffer part = buf[1..2];    // sub-buffer (a copy)
part = buf[2..];            // open-ended
part = buf[<3..<2];         // reverse indexes

buf[1..2] = allocate_buffer(5);   // range assignment; buf grows to 7 bytes
```

### Converting strings and arrays

Strings and arrays of ints **promote** to buffers wherever a buffer is
expected in an assignment context — a string contributes its raw UTF-8
bytes, and an array contributes one byte per element (every item must be
an int 0..255 or the conversion errors, leaving the target unchanged).
The explicit form is the [to_buffer](../../efun/buffers/to_buffer) efun:

```c
buffer buf = "abc";           // declaration initializer
buf = ({ 1, 2, 255 });        // assignment
buf += "é";                   // append the 2 UTF-8 bytes 0xC3 0xA9
buf = buf + ({ 0 });          // + works too
buf[0..1] = "AB";             // and buffer range assignment
buf = to_buffer("abc");       // explicit conversion
buf = ({ 300 });              // error: item out of 0..255
```

### foreach

Buffers iterate like arrays: each byte is delivered as an int 0..255. A
`ref` loop variable mutates the bytes in place (buffers are reference
types):

```c
buffer buf = allocate_buffer(3);

foreach (int b in buf) {       // read each byte
    write(sprintf("%d\n", b));
}

foreach (int ref b in buf) {   // modify in place
    b += 1;                    // errors if the result leaves 0..255
}
```

### See also

`types/array`, `constructs/ref`
