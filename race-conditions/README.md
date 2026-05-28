# race-conditions

The simplest possible demonstration that `++x` is not atomic.

## The problem

Two threads each increment a shared `int` one million times. Expected final
value: 2,000,000. Actual:

```
$ ./race
expected: 2000000
actual:   1543278
lost:     456722
```

`++x` compiles to three operations: load, add, store. If thread B reads
between thread A's load and store, A's increment vanishes when A writes
the stale value back.

## The fix

`std::atomic<int>` promotes the increment to a single hardware-atomic
operation (`LOCK XADD` on x86, `LDADDAL` on ARMv8.1+). No load/store
window for another thread to slip into:

```
$ ./fixed
expected: 2000000
actual:   2000000
```

## Reproducing under ThreadSanitizer

```bash
cmake -B build-tsan -DSAN=tsan
cmake --build build-tsan --target race
./build-tsan/race-conditions/race
```

TSAN reports the race in `race.cpp` line for line. `fixed.cpp` runs clean
under TSAN — atomics are part of the language's race-free contract.

## Why this matters

Every other module in this repo exists because plain memory access between
threads is not safe. Mutexes, semaphores, condition variables — all of
them are tools for making interleavings safe. The race demonstrated here
is the bug they exist to prevent.
