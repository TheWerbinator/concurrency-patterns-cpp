# producer-consumer

Bounded buffer between one producer and one consumer thread, coordinated
by `std::mutex` + two `std::condition_variable`s.

## The problem

A producer generates items faster than a consumer can process them. We
need a buffer between them — but a buffer can't be unbounded (memory) and
can't busy-wait (CPU). The producer must **block when the buffer is full**;
the consumer must **block when the buffer is empty**.

## The pattern

Two condition variables:

- `not_full` — producer waits on this when buffer is full; consumer signals it after pop
- `not_empty` — consumer waits on this when buffer is empty; producer signals it after push

A single mutex protects the underlying `std::queue`. Both waits use the
predicate form of `wait()` so spurious wakeups don't cause incorrect behavior.

Closing the channel is handled by a `closed_` flag + broadcasting both
condition variables. A consumer woken on a closed-and-empty buffer returns
`std::nullopt` instead of blocking forever.

## Why a condition variable instead of polling

A busy loop checking the size of the buffer burns one CPU core per waiting
thread for no reason. `wait()` parks the thread until another thread calls
`notify_one()`. The kernel's scheduler manages the wake — no CPU cost
while waiting.

## Why two condition variables instead of one

A single CV would force a `notify_all` on every push and pop because we
can't tell which side was waiting. Two CVs let each side signal only the
other, avoiding thundering-herd wakes when there are many waiters.

## Run it

```bash
cmake -B build && cmake --build build --target producer_consumer
./build/producer-consumer/producer_consumer
```

Output interleaves "produce N" and "consume N" lines — buffer stays
within capacity, no deadlock, drains cleanly at close.
