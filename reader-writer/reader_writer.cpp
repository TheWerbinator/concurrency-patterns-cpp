// reader_writer.cpp — many readers, one writer, no torn reads.
//
// std::shared_mutex (C++17) lets any number of readers hold the lock
// simultaneously, but writers get exclusive access. Use when reads vastly
// outnumber writes — e.g. config snapshots, route tables, embedding
// caches. With balanced read/write ratio a plain std::mutex usually wins
// because shared_mutex has higher per-lock overhead.

#include <chrono>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <vector>

namespace {

constexpr int kReaders = 6;
constexpr int kWriters = 2;
constexpr int kRoundsPerThread = 5;

struct ConfigSnapshot {
    int version = 0;
    int value = 100;
};

ConfigSnapshot config;
std::shared_mutex config_mutex;

std::mutex stdout_mutex;
template <typename... Args>
void log_line(Args&&... args) {
    std::scoped_lock lock(stdout_mutex);
    (std::cout << ... << std::forward<Args>(args)) << '\n';
}

void reader(int id) {
    for (int i = 0; i < kRoundsPerThread; ++i) {
        std::shared_lock lock(config_mutex);  // many readers concurrent
        log_line("reader ", id, " saw v=", config.version, " value=", config.value);
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
    }
}

void writer(int id) {
    for (int i = 0; i < kRoundsPerThread; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
        std::unique_lock lock(config_mutex);  // exclusive
        ++config.version;
        config.value += 10;
        log_line("writer ", id, " bumped to v=", config.version,
                 " value=", config.value);
    }
}

}  // namespace

int main() {
    std::vector<std::thread> threads;
    threads.reserve(kReaders + kWriters);
    for (int i = 0; i < kReaders; ++i) threads.emplace_back(reader, i);
    for (int i = 0; i < kWriters; ++i) threads.emplace_back(writer, i);
    for (auto& t : threads) t.join();
    log_line("done, final v=", config.version);
    return 0;
}
