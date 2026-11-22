#pragma once
// zephyr-terminal-core / pty / pty_manager.hpp
//
// Owns a real Linux pseudo-terminal and the child process attached to it.
// Uses openpty()/fork()/execve() directly — no shelling out through
// system("...") or /bin/sh -c, per the project's execution-safety rule.

#include <string>
#include <sys/types.h>
#include <vector>

namespace zephyr::pty {

struct SpawnOptions {
    std::string program = "/bin/bash";        // argv[0] / executable path
    std::vector<std::string> args;             // argv[1..]
    std::vector<std::string> env;              // "KEY=VALUE"; empty = inherit current environ
    std::string cwd;                           // empty = inherit
    int rows = 24;
    int cols = 80;
};

class PtyManager {
public:
    PtyManager() = default;
    ~PtyManager();

    PtyManager(const PtyManager&) = delete;
    PtyManager& operator=(const PtyManager&) = delete;

    // Opens a PTY pair, forks, and execs `opts.program` in the child with
    // its controlling terminal set to the PTY slave. Returns false and
    // sets last_error() on failure (parent process only — child failures
    // surface as a non-zero exit status from wait_for_exit()).
    bool spawn(const SpawnOptions& opts);

    ssize_t read(char* buf, std::size_t len);
    ssize_t write(const std::string& data);

    void resize(int rows, int cols);

    // Blocks until the child exits. Returns the exit code, or -1 if it
    // was killed by a signal (in which case *out_signal is set).
    int wait_for_exit(int* out_signal = nullptr);

    // Non-blocking reap. Returns true (and fills *exit_code, -1 for a
    // signal death) if the child has exited; false if it is still running.
    bool try_reap(int* exit_code = nullptr);

    // Orderly shutdown used when a terminal tab/pane is closed: closes the
    // master side, sends SIGHUP to the child's process group (as a real
    // terminal hang-up would), waits up to `grace_ms`, then SIGKILLs and
    // reaps so no zombie or orphaned shell is left behind. Idempotent.
    void terminate(int grace_ms = 200);

    // Resolves a bare program name (no '/') against a PATH-style list.
    // Returns "" if nothing executable is found. Done in the parent before
    // fork() so the child never has to search or allocate.
    static std::string resolve_executable(const std::string& program, const std::string& path_list);

    bool is_running() const { return child_pid_ > 0; }
    pid_t child_pid() const { return child_pid_; }
    int master_fd() const { return master_fd_; }

    const std::string& last_error() const { return last_error_; }

private:
    int master_fd_ = -1;
    pid_t child_pid_ = -1;
    std::string last_error_;
};

} // namespace zephyr::pty
