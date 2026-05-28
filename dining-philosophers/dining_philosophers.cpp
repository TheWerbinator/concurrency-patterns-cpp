// dining_philosophers.cpp — five philosophers, five forks, no deadlock.
//
// Each philosopher needs the two adjacent forks to eat. The naive approach
// (every philosopher: grab left fork, grab right fork) deadlocks: if all
// five grab the left simultaneously, every right fork is held by someone
// else and nobody can proceed.
//
// Resource-hierarchy fix: always lock the lower-numbered fork first. The
// fifth philosopher then takes fork 0 before fork 4, breaking the cycle.
// `std::scoped_lock` performs the multi-lock acquisition atomically using
// std::lock under the hood, which uses a deadlock-avoidance algorithm —
// belt-and-suspenders given we already break the cycle by ordering.

#include <array>
#include <chrono>
#include <iostream>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

namespace {

constexpr int kPhilosophers = 5;
constexpr int kBites = 3;

std::mutex stdout_mutex;
template <typename... Args>
void log_line(Args&&... args) {
    std::scoped_lock lock(stdout_mutex);
    (std::cout << ... << std::forward<Args>(args)) << '\n';
}

}  // namespace

int main() {
    std::array<std::mutex, kPhilosophers> forks;
    std::vector<std::thread> philosophers;
    philosophers.reserve(kPhilosophers);

    for (int id = 0; id < kPhilosophers; ++id) {
        philosophers.emplace_back([id, &forks] {
            std::mt19937 rng(static_cast<unsigned>(id) * 1099511628211u);
            std::uniform_int_distribution<int> wait_ms(10, 80);

            const int left = id;
            const int right = (id + 1) % kPhilosophers;
            // Resource hierarchy: always lock the lower index first.
            const int first = std::min(left, right);
            const int second = std::max(left, right);

            for (int bite = 1; bite <= kBites; ++bite) {
                std::this_thread::sleep_for(std::chrono::milliseconds(wait_ms(rng)));
                log_line("phil ", id, " thinking");
                // scoped_lock acquires both atomically; std::lock under the
                // hood uses a back-off algorithm to avoid deadlock even if
                // we'd ignored the hierarchy.
                std::scoped_lock lock(forks[first], forks[second]);
                log_line("phil ", id, " eating (bite ", bite, "/", kBites, ")");
                std::this_thread::sleep_for(std::chrono::milliseconds(wait_ms(rng)));
            }
            log_line("phil ", id, " done");
        });
    }

    for (auto& p : philosophers) p.join();
    log_line("all philosophers fed");
    return 0;
}
