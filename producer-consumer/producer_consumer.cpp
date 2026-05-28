// producer_consumer.cpp — bounded buffer with mutex + condition variables.
//
// One producer thread pushes integers into a fixed-capacity queue. One
// consumer thread pops them off. The buffer is bounded, so the producer
// must block when full and the consumer must block when empty. Two
// condition variables — `not_full` and `not_empty` — coordinate the wait/
// notify cycle; a single mutex protects the queue itself.
//
// Pattern generalizes to log pipelines, work queues, channel implementations.

#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <thread>

namespace {

template <typename T>
class BoundedBuffer {
public:
    explicit BoundedBuffer(std::size_t capacity) : capacity_(capacity) {}

    void push(T value) {
        std::unique_lock lock(mutex_);
        not_full_.wait(lock, [&] { return queue_.size() < capacity_ || closed_; });
        if (closed_) return;
        queue_.push(std::move(value));
        not_empty_.notify_one();
    }

    std::optional<T> pop() {
        std::unique_lock lock(mutex_);
        not_empty_.wait(lock, [&] { return !queue_.empty() || closed_; });
        if (queue_.empty()) return std::nullopt;  // closed, drained
        T value = std::move(queue_.front());
        queue_.pop();
        not_full_.notify_one();
        return value;
    }

    void close() {
        {
            std::scoped_lock lock(mutex_);
            closed_ = true;
        }
        not_full_.notify_all();
        not_empty_.notify_all();
    }

private:
    const std::size_t capacity_;
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable not_full_;
    std::condition_variable not_empty_;
    bool closed_ = false;
};

}  // namespace

int main() {
    BoundedBuffer<int> buffer(4);
    constexpr int kItems = 20;

    std::thread producer([&] {
        for (int i = 1; i <= kItems; ++i) {
            std::cout << "produce " << i << '\n' << std::flush;
            buffer.push(i);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        buffer.close();
    });

    std::thread consumer([&] {
        while (auto item = buffer.pop()) {
            std::cout << "         consume " << *item << '\n' << std::flush;
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
        std::cout << "consumer drained\n";
    });

    producer.join();
    consumer.join();
    return 0;
}
