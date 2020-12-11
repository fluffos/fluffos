---
layout: default
title: Bug Report
---

### Where To File

Please creates a [New Issue on GitHub](https://github.com/fluffos/fluffos/issues?direction=desc&milestone=none&sort=popularity&state=open).

### Bug Report content

Please be sure to include these in your bug report.

- The version number, driver will output this on start.

- The full output, the driver will at least print out some backtrace or error message. Include those.

- If you can, try to narrow down to a small LPC program that reproduce the
  problem easily. If you can not, Try using Valgrind method first.

## GDB

If you have met an crash, the driver should automatically print out an list of backtrace, but sometime that doesn't really contain enough information.

If you want to catch the crash, try running driver under GDB directly

```shell
$ gdb --args driver <argumnets>

then in GDB prompt
> handle SIGPIPE nostop noprint pass
> run
```

and when you met an crash, do this

```shell
> bt
> info locals
```

and paste the result to your issue!

### Sanitizer

Most of the crashing bug is actually caused by previous silent memory corruption, which it is very hard to detect.

The currently preferred way to detect any sort of memory corruption is to use Sanitizer, that way you catch the
 problem when it happens, not when it causes other problems. However it mostly only works under Linux.

Here is how you should generate a bug report with Sanitizer.

Build driver in sanitizer enabled mode

```shell
$ cmake .. -DENABLE_SANITIZER=ON
```
Launch driver as usual

```shell
$ ./driver <args>
```

Login to your lib as usual, do something fishy.

(it will be slow, that is okay) You may also have to relax your `maximum eval cost` setting, if necessary. When
 Valgrind halts and prints out a backtrace with `Invalid read of size 1`, or `Invalid write of size 1`, save the entire stack trace.
