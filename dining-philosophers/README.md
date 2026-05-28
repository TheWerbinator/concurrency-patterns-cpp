# dining-philosophers

Dijkstra's 1965 problem. Five philosophers, five forks (one between each
pair). To eat, a philosopher needs both adjacent forks. How do they all
make progress without deadlocking?

## The deadlock

Naive code:

```cpp
for (;;) {
    forks[left].lock();
    forks[right].lock();   // ← if every philosopher locks left first,
                            //    the right fork is always held by someone
                            //    else: total deadlock
    eat();
    forks[right].unlock();
    forks[left].unlock();
}
```

Five threads all blocked, none making progress, no error message — the
worst kind of bug to debug from logs alone.

## Two fixes

### 1. Resource hierarchy (used here)

Always lock the lower-numbered fork first. Philosopher 4's "left" is fork
4 and "right" is fork 0; under the hierarchy rule they lock fork 0 first.
The cycle in the wait graph is broken.

### 2. `std::scoped_lock` (also used here)

C++17's `std::scoped_lock` accepts multiple mutexes and acquires them
atomically using `std::lock`, which internally uses a back-off algorithm
(try-and-release) to avoid deadlock. Even without the hierarchy ordering,
`std::scoped_lock(forks[left], forks[right])` is deadlock-free.

We use both in this implementation: hierarchy for clarity, scoped_lock
for guaranteed safety. They're complementary, not redundant.

## Why a thread per philosopher is the cleanest model

Each philosopher is an independent agent with its own state (which bite
they're on). Modeling that as a coroutine or callback-state-machine is
strictly more code for no benefit at this scale. One thread per
philosopher exactly matches the problem domain.

## Run it

```bash
cmake -B build && cmake --build build --target dining_philosophers
./build/dining-philosophers/dining_philosophers
```

Output: interleaved "thinking" and "eating" lines for each philosopher.
Always completes; no deadlock; total ordering of forks is whatever the
scheduler picks.

## What ThreadSanitizer says

Clean. The forks are mutexes; eating accesses no shared mutable state
beyond the forks themselves. `log_line` serializes stdout via a separate
mutex so output lines don't interleave mid-line.
