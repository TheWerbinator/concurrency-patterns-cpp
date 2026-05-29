// sync_throughput.cpp — throughput of four shared-counter strategies.
//
// Same workload (N threads each performing M increments on a shared
// integer), four implementations:
//
//   1. unsynchronized plain int        — fast but wrong (race)
//   2. std::mutex around the increment — correct, slow
//   3. std::atomic<int>::fetch_add     — correct, much faster than mutex
//   4. thread-local + final reduction  — correct, almost as fast as #1
//
// Lesson: atomic looks "cheap" but loses to thread-local aggregation
// by an order of magnitude under contention. Always benchmark before
// assuming a single primitive is "the" answer.

#include <atomic>
#include <benchmark/benchmark.h>
#include <mutex>
#include <numeric>
#include <thread>
#include <vector>

namespace {

constexpr int kIncrementsPerThread = 100'000;

void run_threads(int n_threads, auto&& work) {
    std::vector<std::thread> threads;
    threads.reserve(n_threads);
    for (int t = 0; t < n_threads; ++t) threads.emplace_back(work);
    for (auto& th : threads) th.join();
}

// ---- 1. unsynchronized (data race; result is wrong) ----
void BM_Unsynchronized(benchmark::State& state) {
    const int n_threads = static_cast<int>(state.range(0));
    for (auto _ : state) {
        int counter = 0;
        run_threads(n_threads, [&] {
            for (int i = 0; i < kIncrementsPerThread; ++i) {
                ++counter;
            }
        });
        benchmark::DoNotOptimize(counter);
    }
}
BENCHMARK(BM_Unsynchronized)->RangeMultiplier(2)->Range(1, 8)->UseRealTime();

// ---- 2. std::mutex ----
void BM_Mutex(benchmark::State& state) {
    const int n_threads = static_cast<int>(state.range(0));
    for (auto _ : state) {
        int counter = 0;
        std::mutex m;
        run_threads(n_threads, [&] {
            for (int i = 0; i < kIncrementsPerThread; ++i) {
                std::scoped_lock lock(m);
                ++counter;
            }
        });
        benchmark::DoNotOptimize(counter);
    }
}
BENCHMARK(BM_Mutex)->RangeMultiplier(2)->Range(1, 8)->UseRealTime();

// ---- 3. std::atomic<int>::fetch_add ----
void BM_Atomic(benchmark::State& state) {
    const int n_threads = static_cast<int>(state.range(0));
    for (auto _ : state) {
        std::atomic<int> counter{0};
        run_threads(n_threads, [&] {
            for (int i = 0; i < kIncrementsPerThread; ++i) {
                counter.fetch_add(1, std::memory_order_relaxed);
            }
        });
        benchmark::DoNotOptimize(counter.load());
    }
}
BENCHMARK(BM_Atomic)->RangeMultiplier(2)->Range(1, 8)->UseRealTime();

// ---- 4. thread-local + final reduction ----
void BM_ThreadLocal(benchmark::State& state) {
    const int n_threads = static_cast<int>(state.range(0));
    for (auto _ : state) {
        std::vector<int> per_thread(static_cast<std::size_t>(n_threads), 0);
        std::vector<std::thread> threads;
        threads.reserve(n_threads);
        for (int t = 0; t < n_threads; ++t) {
            threads.emplace_back([&per_thread, t] {
                int local = 0;
                for (int i = 0; i < kIncrementsPerThread; ++i) {
                    ++local;
                }
                per_thread[static_cast<std::size_t>(t)] = local;
            });
        }
        for (auto& th : threads) th.join();
        const int total = std::accumulate(per_thread.begin(), per_thread.end(), 0);
        benchmark::DoNotOptimize(total);
    }
}
BENCHMARK(BM_ThreadLocal)->RangeMultiplier(2)->Range(1, 8)->UseRealTime();

}  // namespace

BENCHMARK_MAIN();
