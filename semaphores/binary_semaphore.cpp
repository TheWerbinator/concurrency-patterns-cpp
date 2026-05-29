// binary_semaphore.cpp — one-shot signaling between threads.
//
// std::binary_semaphore (C++20) is a single-permit semaphore. Common use:
// a "wait until I'm ready" handshake where one thread initializes a
// resource and others wait until it's safe to proceed. Unlike a mutex,
// the thread that releases the permit doesn't have to be the one that
// acquired it.

#include <chrono>
#include <iostream>
#include <semaphore>
#include <thread>

namespace {

std::binary_semaphore ready(0);  // start with zero permits — workers must wait
int shared_resource = 0;

}  // namespace

int main() {
    std::thread initializer([] {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        shared_resource = 42;
        std::cout << "initializer: resource ready, releasing permit\n";
        ready.release();
    });

    std::thread worker([] {
        std::cout << "worker: waiting for resource\n";
        ready.acquire();   // blocks until initializer releases
        std::cout << "worker: got resource = " << shared_resource << '\n';
    });

    initializer.join();
    worker.join();
    return 0;
}
