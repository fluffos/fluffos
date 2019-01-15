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

### Valgrind

Most of the crashing bug is actually caused by previous silent memory corruption, which it is very hard to detect normally.

The correct way to detect any sort of memory corruption is to use Valgrind, that way you catch the problem when it happens, not when it causes other problems.

Here is how you should generate a bug report with Valgrind.

#### Build driver in development mode

```
$ ./build.FluffOS develop
```

#### Install Valgrind and set kernel debug correct parameter

```
$ sudo apt-get install valgrind
$ echo 0 > /proc/sys/kernel/yama/ptrace_scope (on ubuntu)
```

#### Launch driver under Valgrind.

    $ Valgrind --leak-check=full --track-origins=yes --db-attach=yes \
        --malloc-fill=0x75 --free-fill=0x55 ../driver <your config file name>

#### Login to your lib as usual, do something fishy.

(it will be slow, that is okay) You may also have to relax your `maximum eval cost`
setting, if necessary. When Valgrind halts and prints out a backtrace with `Invalid read of size 1`, or `Invalid write of size 1`, save the entire stack trace.

Then press "y" and "enter" to drop into GDB, do "bt", and then do "info locals"
if needed.

#### Known False Positves

Ignore these when you see it happen. Just press `N` or `Enter` to continue.

- With `HAVE_ZLIB`, on `restore_object()` it will complains `Jump base on uninitialized value` with a stacktrace to `zlib`, this is normal.
