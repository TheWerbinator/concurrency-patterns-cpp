// mutex_raii.cpp — RAII lock wrappers, side-by-side.
//
// Modern C++ code should never call lock() / unlock() directly. An
// exception thrown between them leaks the lock, deadlocking the next
// thread that tries to acquire it. The four RAII wrappers each cover
// a different scenario:
//
//   std::lock_guard   — simplest, scope-bound, no manual control
//   std::unique_lock  — scope-bound but movable + can defer/release
//   std::scoped_lock  — acquires multiple mutexes deadlock-free (C++17)
//   std::shared_lock  — for std::shared_mutex's reader side (C++14)

#include <iostream>
#include <mutex>
#include <shared_mutex>

namespace {

std::mutex m1, m2;
std::shared_mutex shared_m;

void demo_lock_guard() {
    std::lock_guard guard(m1);
    std::cout << "lock_guard: locked m1, will release at scope exit\n";
}

void demo_unique_lock_with_defer() {
    std::unique_lock guard(m1, std::defer_lock);
    std::cout << "unique_lock: constructed without locking\n";
    guard.lock();
    std::cout << "unique_lock: explicitly locked\n";
    guard.unlock();
    std::cout << "unique_lock: explicitly unlocked early\n";
    // guard's destructor runs without re-locking
}

void demo_scoped_lock_two_mutexes() {
    // Acquires m1 and m2 atomically via std::lock — no deadlock even if
    // another thread does scoped_lock(m2, m1) at the same time.
    std::scoped_lock guard(m1, m2);
    std::cout << "scoped_lock: both m1 and m2 locked atomically\n";
}

void demo_shared_lock() {
    std::shared_lock reader(shared_m);
    std::cout << "shared_lock: reading concurrently with other readers\n";
}

}  // namespace

int main() {
    demo_lock_guard();
    demo_unique_lock_with_defer();
    demo_scoped_lock_two_mutexes();
    demo_shared_lock();
    return 0;
}
