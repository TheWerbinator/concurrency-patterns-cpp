# semaphores

Three small programs covering C++20 semaphores and the four RAII lock
wrappers every modern C++ programmer should know.

## Binary semaphore (`binary_semaphore.cpp`)

`std::binary_semaphore` (C++20) is a one-permit semaphore. Use for:

- **One-shot signaling** — "wait until I'm ready" between unrelated threads
- **Latch-style coordination** — release once, multiple acquirers wait
- **Cross-thread handoff** where the releaser ≠ the acquirer

Differs from `std::mutex`: a mutex must be released by the same thread
that acquired it. A semaphore can be released by anyone. That makes
mutexes wrong for cross-thread signaling.

## Counting semaphore (`counting_semaphore.cpp`)

`std::counting_semaphore<N>` (C++20) bounds concurrent access to a fixed
resource pool. The example caps concurrent workers at 3 even though 10
are submitted — useful for connection pools, GPU compute slots, "max
concurrent uploads," anywhere the natural primitive is "N permits
available."

## RAII lock wrappers (`mutex_raii.cpp`)

Direct `lock()` / `unlock()` calls are a bug waiting to happen — any
exception between them leaks the lock. The four wrappers, ordered by
when to pick each:

| Wrapper | When |
|---|---|
| `std::lock_guard` | Default. Scope-bound, zero overhead. Single mutex. |
| `std::scoped_lock` | Multiple mutexes acquired atomically (deadlock-free). C++17. |
| `std::unique_lock` | Need to unlock early, or pass ownership, or combine with `std::condition_variable::wait`. |
| `std::shared_lock` | Reader side of `std::shared_mutex`. |

Rule of thumb: start with `lock_guard`; upgrade to `scoped_lock` if you
need multiple mutexes; reach for `unique_lock` only when you actually
need its flexibility.

## C++20 requirement

These examples require C++20 for `<semaphore>`. The top-level CMake sets
`CMAKE_CXX_STANDARD 20`. On Ubuntu, g++-12 or later; macOS, Xcode 14+;
Windows, MSVC 19.30+.
