# reader-writer

`std::shared_mutex` (C++17): many readers can hold the lock simultaneously;
a writer gets exclusive access. The canonical "snapshot config / route
table / embedding cache" lock.

## Why not just a plain mutex

If reads are 100× more common than writes (typical for config snapshots),
a plain `std::mutex` serializes all of them — even though reads don't
conflict with each other. Throughput collapses.

`std::shared_mutex` lets N readers run in parallel and only blocks when
a writer wants to publish a new version. Reads pay only the cache-line
synchronization cost; the lock acquisition itself is cheap.

## When NOT to use shared_mutex

- **Balanced or write-heavy workloads.** shared_mutex acquisitions are
  more expensive than `std::mutex` — they have to track reader count and
  wake-up policy. Below ~5× read:write ratio, plain mutex usually wins.
- **Tiny critical sections.** If the critical section is a single load
  or atomic update, atomics + memory-order are cheaper than any lock.
- **When you can copy.** `std::atomic<std::shared_ptr<T>>` lets readers
  grab a snapshot pointer cheaply and never block writers — at the cost
  of one allocation per write.

## Read/write API mapping

| Action | C++ type |
|---|---|
| Reader acquires (shared) | `std::shared_lock<std::shared_mutex>` |
| Writer acquires (exclusive) | `std::unique_lock<std::shared_mutex>` |
| Either, scope-bound | `std::lock_guard` is **not** shared-aware; don't use it |

## What the demo shows

6 readers + 2 writers race against a shared `ConfigSnapshot`. Multiple
reader log lines appear inside the same time window — they really are
concurrent. Writer lines never interleave with reader lines (the
exclusive lock blocks all readers for the duration of the write).

## Reader/writer starvation

This implementation uses the default `std::shared_mutex` policy, which
is unspecified by the standard. On most implementations (glibc, MSVC),
writers will eventually starve if readers arrive faster than they can be
drained. For starvation-bounded behavior, layer a "writer waiting" flag
or use a custom lock — outside this demo's scope.
