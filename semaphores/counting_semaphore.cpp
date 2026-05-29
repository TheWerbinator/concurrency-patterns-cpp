// counting_semaphore.cpp — bounded resource pool.
//
// std::counting_semaphore<N> (C++20) lets up to N threads hold permits
// at the same time. Use when you have a fixed-size pool: connection
// pools, GPU compute slots, "max 3 concurrent uploads."

#include <chrono>
#include <iostream>
#include <mutex>
#include <semaphore>
#include <thread>
#include <vector>

namespace {

constexpr int kPoolSize = 3;
constexpr int kWorkers = 10;

std::counting_semaphore<kPoolSize> slots(kPoolSize);

std::mutex stdout_mutex;
template <typename... Args>
void log_line(Args&&... args) {
    std::scoped_lock lock(stdout_mutex);
    (std::cout << ... << std::forward<Args>(args)) << '\n';
}

}  // namespace

int main() {
    std::vector<std::thread> workers;
    workers.reserve(kWorkers);

    for (int id = 0; id < kWorkers; ++id) {
        workers.emplace_back([id] {
            log_line("worker ", id, " waiting for slot");
            slots.acquire();
            log_line("worker ", id, " got slot, working");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            log_line("worker ", id, " releasing slot");
            slots.release();
        });
    }
    for (auto& w : workers) w.join();
    return 0;
}
