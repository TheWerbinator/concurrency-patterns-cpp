// race.cpp — broken on purpose.
//
// Two threads each increment a shared int 1,000,000 times. The expected
// final value is 2,000,000. Read-modify-write on a plain int is not atomic,
// so increments are lost when threads interleave their load/add/store.
//
// Run repeatedly: the result varies and is almost always less than expected.
// Under ThreadSanitizer (build with -DSAN=tsan) this program is flagged as
// a data race.

#include <iostream>
#include <thread>

namespace {

constexpr int kIterations = 1'000'000;
int shared_counter = 0;

void increment_unsafely() {
    for (int i = 0; i < kIterations; ++i) {
        // Compiles to load -> add -> store. Another thread can read between
        // our load and store, write a new value, and then have its write
        // clobbered by our store.
        ++shared_counter;
    }
}

}  // namespace

int main() {
    std::thread t1(increment_unsafely);
    std::thread t2(increment_unsafely);
    t1.join();
    t2.join();

    const int expected = 2 * kIterations;
    std::cout << "expected: " << expected << '\n'
              << "actual:   " << shared_counter << '\n'
              << "lost:     " << (expected - shared_counter) << '\n';
    return shared_counter == expected ? 0 : 1;
}
