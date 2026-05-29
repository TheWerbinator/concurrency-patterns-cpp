# benchmark

Throughput comparison of four ways to maintain a shared counter under
contention, using Google Benchmark.

## What it measures

Same workload: N threads, each performing 100,000 increments on a shared
integer. Four implementations:

| # | Strategy | Correct? | Expected speed |
|---|---|---|---|
| 1 | unsynchronized plain `int` | No (data race) | fastest, but result is garbage |
| 2 | `std::mutex` around the increment | Yes | slowest |
| 3 | `std::atomic<int>::fetch_add` | Yes | ~10× faster than mutex |
| 4 | thread-local int + final reduction | Yes | ~same as #1, with correct result |

`N` ranges from 1 to 8 threads (`->RangeMultiplier(2)->Range(1, 8)`).

## The lesson

The first instinct when reaching for "thread-safe counter" is usually
`std::atomic`. It's correct and noticeably faster than a mutex. But
under heavy contention it loses to **per-thread accumulation + a single
final reduction** by another order of magnitude — every `fetch_add` is
still a synchronized memory operation, and at 8 threads they all
serialize through the cache-line MESI protocol.

The thread-local strategy wins because each thread updates a private
variable in its own L1 cache for the duration of the loop, with one
synchronized write per thread at the end. Real-world examples: per-CPU
counters in the Linux kernel, per-shard statistics in databases,
sharded LRU caches.

When to pick which:

- **mutex** — when the critical section is non-trivial (multiple
  variables, or hostile to atomic compose)
- **atomic** — when the critical section is a single read-modify-write,
  contention is low
- **thread-local + reduce** — when contention is high and you can
  tolerate a join-point at the end

## Run

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target sync_throughput
./build/benchmark/sync_throughput
```

Typical output on an 8-core machine (numbers will vary):

```
BM_Unsynchronized/8/real_time      0.4 ms
BM_Mutex/8/real_time              42.1 ms
BM_Atomic/8/real_time              5.6 ms
BM_ThreadLocal/8/real_time         0.5 ms
```

## Caveat

Build in **Release**. Debug builds disable optimizations that change
the relative cost of these operations dramatically; the comparison is
only meaningful at `-O2` or higher.
