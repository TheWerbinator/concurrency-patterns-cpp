// fixed.cpp — atomic fix for race.cpp.
//
// Same workload, same thread count. Swapping `int` for `std::atomic<int>`
// promotes the increment to a single hardware-atomic read-modify-write
// (LOCK XADD on x86, LDADDAL on ARMv8.1+). No interleaving possible; the
// final value is always exactly 2 * kIterations.

#include <atomic>
#include <iostream>
#include <thread>

namespace {

constexpr int kIterations = 1'000'000;
std::atomic<int> shared_counter{0};

void increment_atomically() {
    for (int i = 0; i < kIterations; ++i) {
        shared_counter.fetch_add(1, std::memory_order_relaxed);
    }
}

}  // namespace

int main() {
    std::thread t1(increment_atomically);
    std::thread t2(increment_atomically);
    t1.join();
    t2.join();

    const int expected = 2 * kIterations;
    const int actual = shared_counter.load(std::memory_order_relaxed);
    std::cout << "expected: " << expected << '\n'
              << "actual:   " << actual << '\n';
    return actual == expected ? 0 : 1;
}
