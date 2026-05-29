# parent-child

POSIX `fork()` example. Parent and child are separate processes — no
shared heap — but they share open file descriptors. This module
demonstrates the resulting race on stdout (fd 1).

## Why this matters even in a thread-centric repo

`fork()` is one of the few primitives where "process" and "thread"
semantics blur. The child gets a copy of the parent's memory but
**shares** open files, sockets, and signal handlers. That sharing across
process boundaries has the same write-conflict character as a shared
mutable variable across threads — and the same set of fixes (atomic
operations or explicit synchronization).

## What this program does

`fork()` produces a parent and a child. Each runs `busy_writer`, which
writes 20 lines to stdout. Both processes write to the same fd 1.

The example uses `write(2)` directly, which the POSIX standard
guarantees is atomic for buffers up to `PIPE_BUF` (typically 4096
bytes). Each line therefore lands intact — no interleaving mid-line.

## The bug-flavored variant

Swap `write(2)` for `printf(3)` or `std::cout <<` and rebuild. C library
stdio buffers locally per process; both processes flush independently
to fd 1; output can interleave mid-line:

```
parchild line 0
ent line 0
parent linchile 1
d line 1
```

The fix is to either (a) ensure each logical record fits in a single
atomic `write(2)`, or (b) coordinate with explicit synchronization
across the process boundary (a pipe, a file lock, shared memory + a
process-shared mutex).

## Platform note

`<unistd.h>` + `fork()` are POSIX. The top-level CMakeLists.txt only
adds this subdirectory when `UNIX` is true — Windows builds skip it.
The Win32 equivalent is `CreateProcess`, which does not share memory or
fds with the caller by default; the race here doesn't exist on Windows
the same way.

## Run

```bash
cmake -B build && cmake --build build --target race_fork
./build/parent-child/race_fork
```

Output: 40 interleaved-but-intact lines, parent and child contributing
20 each. Exit code 0.
