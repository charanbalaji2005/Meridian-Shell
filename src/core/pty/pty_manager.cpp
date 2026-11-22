// zephyr-terminal-core / pty / pty_manager.cpp
#include "pty_manager.hpp"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#if defined(__APPLE__)
#include <util.h>
#elif defined(_WIN32)
// Windows PTY handled in platform/WindowsConPTY.cpp
#else
#include <pty.h>
#endif
#include <csignal>
#include <cstdlib>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

namespace zephyr::pty {

PtyManager::~PtyManager() {
    terminate(100);
}

std::string PtyManager::resolve_executable(const std::string& program, const std::string& path_list) {
    if (program.empty()) return "";
    auto is_exec = [](const std::string& p) {
        struct stat st{};
        return ::stat(p.c_str(), &st) == 0 && S_ISREG(st.st_mode) && ::access(p.c_str(), X_OK) == 0;
    };
    if (program.find('/') != std::string::npos) return is_exec(program) ? program : "";
    std::size_t start = 0;
    while (start <= path_list.size()) {
        std::size_t end = path_list.find(':', start);
        if (end == std::string::npos) end = path_list.size();
        std::string dir = path_list.substr(start, end - start);
        if (dir.empty()) dir = ".";
        std::string candidate = dir + "/" + program;
        if (is_exec(candidate)) return candidate;
        start = end + 1;
    }
    return "";
}

bool PtyManager::spawn(const SpawnOptions& opts) {
    if (child_pid_ > 0) {
        last_error_ = "spawn called while a child is still running";
        return false;
    }

    // Resolve bare names (e.g. "ssh", "zsh") against PATH up front.
    std::string exe = opts.program;
    if (exe.find('/') == std::string::npos) {
        std::string path_list;
        for (const auto& kv : opts.env) {
            if (kv.rfind("PATH=", 0) == 0) path_list = kv.substr(5);
        }
        if (path_list.empty()) {
            const char* p = std::getenv("PATH");
            path_list = p ? p : "/usr/local/bin:/usr/bin:/bin";
        }
        exe = resolve_executable(exe, path_list);
        if (exe.empty()) {
            last_error_ = "program not found in PATH: " + opts.program;
            return false;
        }
    }

    int master = -1, slave = -1;
    struct winsize ws{};
    ws.ws_row = static_cast<unsigned short>(opts.rows);
    ws.ws_col = static_cast<unsigned short>(opts.cols);

    if (openpty(&master, &slave, nullptr, nullptr, &ws) != 0) {
        last_error_ = std::string("openpty failed: ") + std::strerror(errno);
        return false;
    }
    // The master must not leak into children spawned later (other tabs):
    // a leaked master keeps the PTY open and closing a tab would never
    // deliver a hang-up to its shell.
    fcntl(master, F_SETFD, FD_CLOEXEC);

    // Build argv/envp in the parent, before forking, so the child only has
    // to call execve — no allocation races between fork and exec.
    std::vector<std::string> argv_storage;
    argv_storage.push_back(opts.program);
    for (auto& a : opts.args) argv_storage.push_back(a);
    std::vector<char*> argv;
    argv.reserve(argv_storage.size() + 1);
    for (auto& s : argv_storage) argv.push_back(s.data());
    argv.push_back(nullptr);

    bool use_custom_env = !opts.env.empty();
    std::vector<std::string> env_storage = opts.env;
    std::vector<char*> envp;
    if (use_custom_env) {
        envp.reserve(env_storage.size() + 1);
        for (auto& s : env_storage) envp.push_back(s.data());
        envp.push_back(nullptr);
    }

    pid_t pid = fork();
    if (pid < 0) {
        last_error_ = std::string("fork failed: ") + std::strerror(errno);
        ::close(master);
        ::close(slave);
        return false;
    }

    if (pid == 0) {
        // ---- child ----
        ::close(master);
        // GUI toolkits may ignore/block signals; give the shell a clean slate.
        sigset_t all;
        sigemptyset(&all);
        sigprocmask(SIG_SETMASK, &all, nullptr);
        for (int sig : {SIGINT, SIGQUIT, SIGTERM, SIGHUP, SIGPIPE, SIGCHLD, SIGTSTP, SIGTTIN, SIGTTOU}) {
            signal(sig, SIG_DFL);
        }
        setsid();
        if (ioctl(slave, TIOCSCTTY, 0) != 0) _exit(126);
        dup2(slave, STDIN_FILENO);
        dup2(slave, STDOUT_FILENO);
        dup2(slave, STDERR_FILENO);
        if (slave > STDERR_FILENO) ::close(slave);

        if (!opts.cwd.empty() && chdir(opts.cwd.c_str()) != 0) _exit(127);

        if (use_custom_env) {
            execve(exe.c_str(), argv.data(), envp.data());
        } else {
            execv(exe.c_str(), argv.data());
        }
        // execve/execv only returns on failure.
        _exit(127);
    }

    // ---- parent ----
    ::close(slave);
    master_fd_ = master;
    child_pid_ = pid;
    return true;
}

ssize_t PtyManager::read(char* buf, std::size_t len) {
    if (master_fd_ < 0) return -1;
    return ::read(master_fd_, buf, len);
}

ssize_t PtyManager::write(const std::string& data) {
    if (master_fd_ < 0) return -1;
    return ::write(master_fd_, data.data(), data.size());
}

void PtyManager::resize(int rows, int cols) {
    if (master_fd_ < 0) return;
    struct winsize ws{};
    ws.ws_row = static_cast<unsigned short>(rows);
    ws.ws_col = static_cast<unsigned short>(cols);
    ioctl(master_fd_, TIOCSWINSZ, &ws);
}

int PtyManager::wait_for_exit(int* out_signal) {
    if (child_pid_ <= 0) return -1;
    int status = 0;
    pid_t r = waitpid(child_pid_, &status, 0);
    if (r < 0) return -1;
    child_pid_ = -1;
    if (WIFEXITED(status)) {
        if (out_signal) *out_signal = 0;
        return WEXITSTATUS(status);
    }
    if (WIFSIGNALED(status)) {
        if (out_signal) *out_signal = WTERMSIG(status);
        return -1;
    }
    return -1;
}

bool PtyManager::try_reap(int* exit_code) {
    if (child_pid_ <= 0) return true;
    int status = 0;
    pid_t r = waitpid(child_pid_, &status, WNOHANG);
    if (r == 0) return false;          // still running
    child_pid_ = -1;
    if (exit_code) {
        *exit_code = (r > 0 && WIFEXITED(status)) ? WEXITSTATUS(status) : -1;
    }
    return true;
}

void PtyManager::terminate(int grace_ms) {
    if (master_fd_ >= 0) {
        ::close(master_fd_);
        master_fd_ = -1;
    }
    if (child_pid_ <= 0) return;
    // The child called setsid(), so its pid is also its process-group id:
    // signal the whole group (shell + foreground job) like a real hang-up.
    ::kill(-child_pid_, SIGHUP);
    ::kill(child_pid_, SIGCONT); // a stopped shell must run to act on SIGHUP
    for (int waited = 0; waited < grace_ms; waited += 5) {
        if (try_reap()) return;
        usleep(5000);
    }
    ::kill(-child_pid_, SIGKILL);
    ::kill(child_pid_, SIGKILL);
    waitpid(child_pid_, nullptr, 0);
    child_pid_ = -1;
}

} // namespace zephyr::pty
