// race_fork.cpp — POSIX fork() race between parent and child.
//
// fork() creates a separate process whose memory is copied-on-write from
// the parent's. They no longer share heap, but they DO share file
// descriptors. Both write to stdout (fd 1) concurrently — the kernel
// guarantees write(2) is atomic for buffers up to PIPE_BUF, but
// printf/iostream buffer locally then flush, so output can interleave
// mid-line if both processes flush at the same time.
//
// Mitigation: use unbuffered write(2) directly, or std::cout.put with
// std::endl flush after each character group, or coordinate with a
// pipe / shared memory lock.

#include <cstdio>
#include <cstring>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>

namespace {

constexpr int kLines = 20;

void busy_writer(const char* who) {
    char buf[64];
    for (int i = 0; i < kLines; ++i) {
        const int n = std::snprintf(buf, sizeof(buf),
                                    "%s line %d\n", who, i);
        // write(2) is atomic up to PIPE_BUF (usually 4096) bytes —
        // so each call delivers its full buffer without interleaving.
        // Switch to printf and rebuild to see the interleaving bug.
        ::write(STDOUT_FILENO, buf, static_cast<std::size_t>(n));
    }
}

}  // namespace

int main() {
    const pid_t pid = ::fork();
    if (pid < 0) {
        std::perror("fork");
        return 1;
    }

    if (pid == 0) {
        // Child process. Its copy of `pid` is 0.
        busy_writer("child ");
        return 0;
    }

    // Parent process. pid is the child's PID.
    busy_writer("parent");
    int status = 0;
    ::waitpid(pid, &status, 0);
    return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
}
