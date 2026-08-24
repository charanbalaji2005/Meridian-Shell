#include "git_intel.hpp"

#include <array>
#include <cstdio>
#include <fstream>
#include <memory>
#include <sstream>

namespace meridian::dev {

namespace {

std::string trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

std::string parent_directory(const std::string& path) {
    if (path.empty() || path == "/") return "";
    auto slash = path.find_last_of('/');
    if (slash == std::string::npos) return "";
    if (slash == 0) return "/";
    return path.substr(0, slash);
}

} // namespace

std::string GitIntel::run_git(const std::string& dir, const std::string& args) {
    std::string cmd = "git -C \"" + dir + "\" " + args + " 2>/dev/null";
    std::array<char, 256> buf;
    std::string out;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";
    while (fgets(buf.data(), static_cast<int>(buf.size()), pipe) != nullptr) {
        out += buf.data();
    }
    pclose(pipe);
    return out;
}

GitRepoStatus GitIntel::inspect_directory(const std::string& start_dir) {
    GitRepoStatus status;

    std::string toplevel = trim(run_git(start_dir, "rev-parse --show-toplevel"));
    if (toplevel.empty()) {
        // Fallback: walk filesystem up to find .git
        std::string dir = start_dir;
        for (int depth = 0; depth < 64 && !dir.empty(); ++depth) {
            std::string git_path = dir + "/.git";
            std::ifstream head_file(git_path + "/HEAD");
            if (head_file.is_open()) {
                toplevel = dir;
                status.is_git_repo = true;
                status.root_dir = dir;
                std::string line;
                if (std::getline(head_file, line)) {
                    line = trim(line);
                    std::string prefix = "ref: refs/heads/";
                    if (line.rfind(prefix, 0) == 0) status.branch_name = line.substr(prefix.size());
                    else if (!line.empty()) status.branch_name = line;
                }
                break;
            }
            dir = parent_directory(dir);
        }
        if (!status.is_git_repo) {
            status.is_git_repo = false;
            return status;
        }
    } else {
        status.is_git_repo = true;
        status.root_dir = toplevel;
    }

    // Branch
    std::string branch = trim(run_git(toplevel, "branch --show-current"));
    if (!branch.empty()) {
        status.branch_name = branch;
    } else if (status.branch_name.empty()) {
        std::string head_hash = trim(run_git(toplevel, "rev-parse --short HEAD"));
        if (!head_hash.empty()) {
            status.branch_name = "HEAD detached at " + head_hash;
        } else {
            status.branch_name = "main";
        }
    }
    std::string rev_count = trim(run_git(toplevel, "rev-list --left-right --count @{u}...HEAD"));
    if (!rev_count.empty()) {
        std::istringstream ss(rev_count);
        ss >> status.behind_count >> status.ahead_count;
    }

    // Status porcelain
    std::string status_out = run_git(toplevel, "status --porcelain");
    std::istringstream ss(status_out);
    std::string line;
    while (std::getline(ss, line)) {
        if (line.size() < 4) continue;
        GitFileChange ch;
        ch.index_status = line[0];
        ch.work_status = line[1];
        ch.path = line.substr(3);

        if (ch.index_status == '?' && ch.work_status == '?') {
            status.untracked_count++;
        } else {
            if (ch.index_status != ' ' && ch.index_status != '?') status.staged_count++;
            if (ch.work_status != ' ' && ch.work_status != '?') status.unstaged_count++;
        }
        status.changed_files.push_back(ch);
    }

    status.is_clean = (status.staged_count == 0 && status.unstaged_count == 0 && status.untracked_count == 0);
    return status;
}

std::string GitRepoStatus::format_summary_bar() const {
    if (!is_git_repo) return "";
    std::ostringstream ss;
    ss << "  " << branch_name;
    if (ahead_count > 0) ss << " ↑" << ahead_count;
    if (behind_count > 0) ss << " ↓" << behind_count;
    if (staged_count > 0) ss << " +" << staged_count;
    if (unstaged_count > 0) ss << " !" << unstaged_count;
    if (untracked_count > 0) ss << " ?" << untracked_count;
    if (is_clean) ss << " ✓";
    return ss.str();
}

std::string GitRepoStatus::format_panel() const {
    if (!is_git_repo) return "Not a Git repository.\n";
    std::ostringstream ss;
    ss << "┌─── Meridian Git ────────────────────────────────────────\n";
    ss << "│ Branch: " << branch_name << " (" << head_commit_hash << ")\n";
    if (ahead_count > 0 || behind_count > 0) {
        ss << "│ Upstream: ↑ " << ahead_count << " ahead, ↓ " << behind_count << " behind\n";
    }
    ss << "│ ────────────────────────────────────────────────────────\n";
    if (is_clean) {
        ss << "│ Working tree is clean ✓\n";
    } else {
        ss << "│ Changes (" << (staged_count + unstaged_count + untracked_count) << " files):\n";
        for (std::size_t i = 0; i < std::min<std::size_t>(changed_files.size(), 12); ++i) {
            const auto& f = changed_files[i];
            ss << "│  " << f.index_status << f.work_status << "  " << f.path << "\n";
        }
        if (changed_files.size() > 12) {
            ss << "│  ... and " << (changed_files.size() - 12) << " more files\n";
        }
    }
    ss << "└─────────────────────────────────────────────────────────\n";
    return ss.str();
}

} // namespace meridian::dev

