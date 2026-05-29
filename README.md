# concurrency-patterns-cpp

Modern C++ implementations of the canonical concurrency problems, with
ThreadSanitizer + AddressSanitizer in CI. Each subdirectory is a small,
self-contained program that demonstrates one primitive or one
problem-pattern pair (naive → broken → fixed).

The point of the repo is to make the layer beneath every async runtime —
mutexes, semaphores, condition variables, atomics — visible and
inspectable. If you've used `asyncio.Lock` in Python or `RwLock` in Rust
without thinking about what they cost, this is what's underneath.

## About this repo

Original implementations: assignments and quizzes from **CS 3600
(Operating Systems)** at Southern Utah University. Originally written
in mixed C and C++ for pthread-based grading. This repo:

- Ports everything to **modern C++20** (`std::thread`, `std::mutex`,
  `std::shared_mutex`, `std::counting_semaphore`, `std::scoped_lock`)
- Reorganizes by **problem**, not by quiz number
- Adds a **Google Benchmark** throughput harness
- Adds CI with **ThreadSanitizer + AddressSanitizer** including a test
  that asserts TSAN catches the intentional race in `race.cpp`
- Per-problem READMEs structured as problem → naive → broken trace → fix → why

The algorithms are coursework; the modernization, benchmark, and CI are
recent (2026).

## Modules

| Directory | Demonstrates |
|---|---|
| `race-conditions/` | Why `++x` is not atomic. `std::atomic<int>` fix. TSAN catches the unfixed version. |
| `producer-consumer/` | Bounded buffer with `std::mutex` + two `std::condition_variable`s. |
| `dining-philosophers/` | Resource-hierarchy + `std::scoped_lock` to dodge deadlock. |
| `reader-writer/` | `std::shared_mutex` for many-reader, one-writer workloads. |
| `semaphores/` | `std::binary_semaphore`, `std::counting_semaphore`, and the four RAII lock wrappers. |
| `parent-child/` | POSIX `fork()` race between parent and child writing to fd 1. |
| `benchmark/` | Google Benchmark comparing unsynchronized vs mutex vs atomic vs thread-local. |

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Binaries land in `build/<module>/<name>`. Each module has its own
README with run instructions.

## Sanitizers

```bash
# ThreadSanitizer — find data races
cmake -B build-tsan -DSAN=tsan -DCMAKE_BUILD_TYPE=Debug
cmake --build build-tsan
./build-tsan/race-conditions/race    # TSAN will report the race

# AddressSanitizer — find memory bugs
cmake -B build-asan -DSAN=asan -DCMAKE_BUILD_TYPE=Debug
cmake --build build-asan
```

GitHub Actions runs both sanitizers on every push (see
`.github/workflows/ci.yml`). The TSAN job specifically asserts that
`race.cpp` IS flagged — a regression where TSAN no longer caught it
would fail CI.

## Stack

- C++20 (g++-13 or clang-17+)
- CMake 3.20+
- Google Benchmark (fetched via `FetchContent`)
- GitHub Actions: build / TSAN / ASAN matrix

## Why this exists

Most application code never touches a mutex directly anymore — it goes
through `asyncio`, `tokio`, Goroutines, or some other runtime that
hides the primitives. That's fine until the abstraction leaks: a
deadlock in a database driver, a torn read across cores, a livelock
that only appears under load. When that happens, knowing what's
underneath stops being optional.

This repo is small enough to read in one sitting, and structured so
each module's README is the explanation of one primitive. Not a
tutorial; a reference you can come back to.

## License

MIT — see [LICENSE](LICENSE).
