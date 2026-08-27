#include "line_editor.hpp"
#include "../dev/git_intel.hpp"
#include "../dev/command_palette.hpp"
#include "../dev/universal_search.hpp"
#include "../dev/rich_history.hpp"
#include "../core/vt/screen_buffer.hpp"
#include "../workspace/pane_tree.hpp"
#include "../core/art_gallery.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <dirent.h>
#include <poll.h>
#include <pwd.h>
#include <sstream>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

namespace meridian::shell {

namespace {

struct RawModeScope {
    struct termios original_termios;
    bool active = false;

    RawModeScope() {
        if (isatty(STDIN_FILENO)) {
            if (tcgetattr(STDIN_FILENO, &original_termios) == 0) {
                struct termios raw = original_termios;
                raw.c_lflag &= ~(ICANON | ECHO);
                raw.c_cc[VMIN] = 1;
                raw.c_cc[VTIME] = 0;
                if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == 0) {
                    active = true;
                }
            }
        }
    }

    ~RawModeScope() {
        if (active) {
            tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
        }
    }
};

bool has_pending_input(int fd, int timeout_ms) {
    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;
    pfd.revents = 0;
    return poll(&pfd, 1, timeout_ms) > 0 && (pfd.revents & POLLIN);
}

std::string format_cwd(const std::string& cwd) {
    std::string dir = cwd.empty() ? "~" : cwd;
    const char* home = std::getenv("HOME");
    if (home && dir.rfind(home, 0) == 0) {
        dir = "~" + dir.substr(std::strlen(home));
    }
    return dir;
}

static std::string get_directory_icon(const std::string& formatted_dir, bool is_git_repo) {
    if (formatted_dir == "~") return " ";
    std::string lower = formatted_dir;
    for (auto& c : lower) c = std::tolower(c);

    if (lower.find("download") != std::string::npos) return " ";
    if (lower.find("document") != std::string::npos) return "󰈙 ";
    if (lower.find("desktop") != std::string::npos) return " ";
    if (lower.find("picture") != std::string::npos || lower.find("photo") != std::string::npos) return " ";
    if (lower.find("video") != std::string::npos || lower.find("movie") != std::string::npos) return " ";
    if (lower.find("music") != std::string::npos) return " ";
    if (lower.find("project") != std::string::npos || lower.find("workspace") != std::string::npos || lower.find("src") != std::string::npos || lower.find("repo") != std::string::npos) return " ";
    if (lower.find("build") != std::string::npos || lower.find("target") != std::string::npos || lower.find("out") != std::string::npos) return " ";
    if (lower.find(".config") != std::string::npos || lower.find("/etc") != std::string::npos) return " ";
    if (is_git_repo) return " ";
    return " ";
}

static std::string detect_project_language(const std::string& dir) {
    std::string d = dir.empty() ? "." : dir;
    if (access((d + "/CMakeLists.txt").c_str(), F_OK) == 0 || access((d + "/Makefile").c_str(), F_OK) == 0) return "󰙲";
    if (access((d + "/Cargo.toml").c_str(), F_OK) == 0) return "";
    if (access((d + "/pyproject.toml").c_str(), F_OK) == 0 || access((d + "/requirements.txt").c_str(), F_OK) == 0 || access((d + "/setup.py").c_str(), F_OK) == 0) return "";
    if (access((d + "/package.json").c_str(), F_OK) == 0 || access((d + "/tsconfig.json").c_str(), F_OK) == 0) return "";
    if (access((d + "/go.mod").c_str(), F_OK) == 0) return "";
    if (access((d + "/pom.xml").c_str(), F_OK) == 0 || access((d + "/build.gradle").c_str(), F_OK) == 0) return "";
    if (access((d + "/Dockerfile").c_str(), F_OK) == 0 || access((d + "/compose.yaml").c_str(), F_OK) == 0) return "󰡨";
    if (access((d + "/flake.nix").c_str(), F_OK) == 0) return "";
    return "";
}

} // namespace

bool LineEditor::is_terminal_interactive() {
    return isatty(STDIN_FILENO) && isatty(STDOUT_FILENO);
}

std::string LineEditor::build_date_badge(const std::string& cwd) {
    std::string dir = format_cwd(cwd);

    // Live Date/Time
    std::time_t now = std::time(nullptr);
    std::tm* local_tm = std::localtime(&now);
    char time_buf[64];
    std::strftime(time_buf, sizeof(time_buf), "%a %d %b  -  %H:%M", local_tm);

    // Inspect Git Status & Context
    auto git_status = dev::GitIntel::inspect_directory(cwd.empty() ? "." : cwd);
    std::string dir_icon = get_directory_icon(dir, git_status.is_git_repo);
    std::string lang_icon = detect_project_language(cwd);

    std::ostringstream ss;

    // Segment 1: Date/Time or Remote SSH Host
    const char* ssh_conn = std::getenv("SSH_CONNECTION");
    const char* ssh_client = std::getenv("SSH_CLIENT");
    bool is_ssh = (ssh_conn != nullptr || ssh_client != nullptr);

    if (is_ssh) {
        char hname[256] = "remote";
        gethostname(hname, sizeof(hname));
        std::string u = "user";
        struct passwd* pw = getpwuid(geteuid());
        if (pw && pw->pw_name) u = pw->pw_name;

        ss << "\033[48;2;124;58;237;38;2;255;255;255;1m  " << u << "@" << hname << " "
           << "\033[48;2;24;156;184;38;2;124;58;237m";
    } else {
        ss << "\033[48;2;26;108;218;38;2;255;255;255;1m " << time_buf << " "
           << "\033[48;2;24;156;184;38;2;26;108;218m";
    }

    // Segment 2: Context-Aware Directory + Project Language
    ss << "\033[48;2;24;156;184;38;2;255;255;255;1m " << dir << " ";
    // Segment 3: Intelligent Git Segment
    if (git_status.is_git_repo && !git_status.branch_name.empty()) {
        std::string branch = git_status.branch_name;
        int unstaged = git_status.unstaged_count;
        int untracked = git_status.untracked_count;
        int staged = git_status.staged_count;
        int ahead = git_status.ahead_count;
        int behind = git_status.behind_count;

        std::string git_extra;
        if (ahead > 0) git_extra += " ↑" + std::to_string(ahead);
        if (behind > 0) git_extra += " ↓" + std::to_string(behind);
        if (unstaged > 0) git_extra += " " + std::to_string(unstaged) + "✗";
        if (untracked > 0) git_extra += " " + std::to_string(untracked) + "?";
        if (staged > 0) git_extra += " " + std::to_string(staged) + "●";
        if (git_status.is_clean && ahead == 0 && behind == 0) git_extra += " ✔";

        ss << "\033[48;2;16;185;129;38;2;24;156;184m"
           << "\033[48;2;16;185;129;38;2;20;35;28;1m  " << branch << git_extra << " "
           << "\033[0;38;2;16;185;129m\033[0m";
    } else {
        ss << "\033[0;38;2;24;156;184m\033[0m";
    }

    return ss.str();
}

std::string LineEditor::build_powerline_prompt(const std::string& /*cwd*/) {
    // Detect username & privilege level
    std::string user = "user";
    uid_t uid = geteuid();
    struct passwd* pw = getpwuid(uid);
    if (pw && pw->pw_name) user = pw->pw_name;

    std::ostringstream ss;
    // Dim grey hollow circle dot glyph matching user reference
    ss << "\033[38;2;120;130;150m○ \033[0m";

    if (uid == 0) {
        // Elevated Root Prompt in Crimson Red
        ss << "\033[48;2;220;38;38;38;2;255;255;255;1m ⚡ root "
           << "\033[0;38;2;220;38;38m\033[0m \033[1;31m❯\033[0m ";
    } else {
        // Standard User Prompt: Vibrant Golden Yellow badge with purple/dark text (@username  ❯)
        ss << "\033[48;2;212;180;27;38;2;85;35;110;1m @" << user << " "
           << "\033[0;38;2;212;180;27m\033[0m \033[1;33m❯\033[0m ";
    }
    return ss.str();
}

void LineEditor::clear_history_preview(std::ostream& out, int lines_drawn) {
    if (lines_drawn <= 0) return;
    for (int i = 0; i < lines_drawn; ++i) {
        out << "\n\033[2K\r";
    }
    out << "\033[" << lines_drawn << "A\r";
    out.flush();
}

void LineEditor::draw_history_preview(
    std::ostream& out,
    const std::vector<std::string>& history,
    int selected_idx,
    int visible_count
) {
    if (history.empty()) return;

    out << "\n\033[2K\r\033[1;36m┌─── Command History Preview (↑/↓ to navigate, Enter to run, Esc to close) ───┐\033[0m";
    int total = static_cast<int>(history.size());
    int start = std::max(0, total - visible_count);
    int count = 0;

    for (int i = total - 1; i >= start; --i) {
        count++;
        bool is_selected = (i == selected_idx);
        std::string cmd = history[i];
        if (cmd.size() > 56) cmd = cmd.substr(0, 53) + "...";

        out << "\n\033[2K\r";
        if (is_selected) {
            out << "\033[1;32m│ ▶ \033[1;37;44m " << cmd;
            int pad = 62 - static_cast<int>(cmd.size());
            for (int p = 0; p < pad; ++p) out << " ";
            out << "\033[0m\033[1;32m │\033[0m";
        } else {
            out << "\033[1;36m│   \033[0;37m " << cmd;
            int pad = 62 - static_cast<int>(cmd.size());
            for (int p = 0; p < pad; ++p) out << " ";
            out << "\033[1;36m │\033[0m";
        }
    }
    out << "\n\033[2K\r\033[1;36m└───────────────────────────────────────────────────────────────────────────────┘\033[0m";

    int lines_drawn = count + 2;
    out << "\033[" << lines_drawn << "A\r";
    out.flush();
}

std::string LineEditor::read_line(
    std::istream& in,
    std::ostream& out,
    const std::string& prompt,
    const std::vector<std::string>& history
) {
    if (!is_terminal_interactive()) {
        std::string line;
        if (std::getline(in, line)) {
            return line;
        }
        return "";
    }

    RawModeScope raw;
    std::string current_line;
    std::string original_line;
    int cursor_pos = 0;
    int history_idx = static_cast<int>(history.size());
    int preview_lines_drawn = 0;

    auto refresh_prompt = [&]() {
        out << "\r\033[K" << prompt << current_line;
        if (cursor_pos < static_cast<int>(current_line.size())) {
            int back = static_cast<int>(current_line.size()) - cursor_pos;
            out << "\033[" << back << "D";
        }
        out.flush();
    };

    refresh_prompt();

    while (true) {
        char c = 0;
        if (read(STDIN_FILENO, &c, 1) <= 0) break;

        // Enter: Commit line and clear preview box completely
        if (c == '\n' || c == '\r') {
            if (preview_lines_drawn > 0) {
                clear_history_preview(out, preview_lines_drawn);
                preview_lines_drawn = 0;
            }
            refresh_prompt();
            out << "\n";
            out.flush();
            return current_line;
        }

        // Ctrl+C: Cancel line and dismiss preview
        if (c == 3) {
            if (preview_lines_drawn > 0) {
                clear_history_preview(out, preview_lines_drawn);
                preview_lines_drawn = 0;
            }
            out << "^C\n";
            out.flush();
            return "";
        }

        // Ctrl+D / Ctrl+Shift+D: Split active pane vertically
        if (c == 4) {
            if (preview_lines_drawn > 0) {
                clear_history_preview(out, preview_lines_drawn);
                preview_lines_drawn = 0;
            }
            auto& tree = workspace::get_session_pane_tree();
            uint32_t new_id = tree.split_pane(tree.active_pane_id(), workspace::SplitDirection::Vertical);
            out << "\n\033[38;2;34;197;94m✔\033[0m Pane split vertically [ID: " << new_id
                << " | Active: " << tree.active_pane_id()
                << " | Total: " << tree.count_panes() << "] (Ctrl+Shift+D)\n";
            out.flush();
            refresh_prompt();
            continue;
        }

        // Ctrl+E / Ctrl+Shift+E: Split active pane horizontally
        if (c == 5) {
            if (preview_lines_drawn > 0) {
                clear_history_preview(out, preview_lines_drawn);
                preview_lines_drawn = 0;
            }
            auto& tree = workspace::get_session_pane_tree();
            uint32_t new_id = tree.split_pane(tree.active_pane_id(), workspace::SplitDirection::Horizontal);
            out << "\n\033[38;2;34;197;94m✔\033[0m Pane split horizontally [ID: " << new_id
                << " | Active: " << tree.active_pane_id()
                << " | Total: " << tree.count_panes() << "] (Ctrl+Shift+E)\n";
            out.flush();
            refresh_prompt();
            continue;
        }

        // Ctrl+F / Ctrl+Shift+F: Universal Search across scrollback buffer & history
        if (c == 6) {
            if (preview_lines_drawn > 0) {
                clear_history_preview(out, preview_lines_drawn);
                preview_lines_drawn = 0;
            }
            std::string q = current_line;
            if (q.empty()) q = "error";
            vt::ScreenBuffer screen;
            dev::RichHistory r_hist;
            auto matches = dev::UniversalSearch::search_all(screen, r_hist, q);
            out << "\n\033[1;36m┌─── Universal Search (Ctrl+Shift+F) ──────────────────────────┐\033[0m\n"
                << "\033[1;37mQuery: \"" << q << "\" (" << matches.size() << " matches across scrollback & history)\033[0m\n";
            for (const auto& m : matches) {
                out << "  \033[1;33m[" << m.source_label << "]\033[0m " << m.line_content << "\n";
            }
            out << "\033[1;36m└──────────────────────────────────────────────────────────────┘\033[0m\n";
            out.flush();
            refresh_prompt();
            continue;
        }

        // Ctrl+L: Clear screen completely (removes header, images and scrollback)
        if (c == 12) {
            if (preview_lines_drawn > 0) {
                clear_history_preview(out, preview_lines_drawn);
                preview_lines_drawn = 0;
            }
            out << "\033[3J\033[2J\033[H\033_Ga=d,d=a\033\\";
            out.flush();
            refresh_prompt();
            continue;
        }

        // Ctrl+P / Ctrl+Shift+P: Command Palette Fuzzy Search & Action Launcher
        if (c == 16) {
            if (preview_lines_drawn > 0) {
                clear_history_preview(out, preview_lines_drawn);
                preview_lines_drawn = 0;
            }
            dev::CommandPalette palette;
            auto results = palette.search(current_line);

            out << "\n\033[1;37m┌─── Meridian Command Palette (Ctrl+Shift+P) ──────────────────┐\033[0m\n";
            for (const auto& a : results) {
                out << "\033[1;37m│\033[0m \033[1;36m[" << a.category << "]\033[0m \033[1;37m" << a.title << "\033[0m "
                    << "\033[38;2;140;150;170m(" << a.shortcut << ")\033[0m\n";
            }
            out << "\033[1;37m└──────────────────────────────────────────────────────────────┘\033[0m\n"
                << "\033[0;37mTip: Type '\033[1;32mpalette <query>\033[0m' or '\033[1;32mpic set <theme>\033[0m' anytime!\033[0m\n";
            out.flush();
            refresh_prompt();
            continue;
        }

        // Ctrl+Z / Ctrl+Shift+Z: Toggle zoom on active pane
        if (c == 26) {
            if (preview_lines_drawn > 0) {
                clear_history_preview(out, preview_lines_drawn);
                preview_lines_drawn = 0;
            }
            auto& tree = workspace::get_session_pane_tree();
            tree.toggle_zoom();
            if (tree.is_zoomed()) {
                out << "\n\033[38;2;245;158;11m⛶\033[0m Pane ZOOMED / Maximized [ID: " << tree.active_pane_id() << "] (Ctrl+Shift+Z)\n";
            } else {
                out << "\n\033[38;2;59;130;246m⧉\033[0m Pane Zoom RESTORED / Unmaximized [Active ID: " << tree.active_pane_id()
                    << " | Total: " << tree.count_panes() << "] (Ctrl+Shift+Z)\n";
            }
            out.flush();
            refresh_prompt();
            continue;
        }

        // Backspace: in-place character deletion
        if (c == 127 || c == 8) {
            if (preview_lines_drawn > 0) {
                clear_history_preview(out, preview_lines_drawn);
                preview_lines_drawn = 0;
                history_idx = static_cast<int>(history.size());
            }
            if (cursor_pos > 0) {
                current_line.erase(cursor_pos - 1, 1);
                original_line = current_line;
                cursor_pos--;
                refresh_prompt();
            }
            continue;
        }

        // Tab completion (files in current directory)
        if (c == '\t') {
            if (preview_lines_drawn > 0) {
                clear_history_preview(out, preview_lines_drawn);
                preview_lines_drawn = 0;
            }
            auto last_space = current_line.find_last_of(" \t");
            std::string prefix = (last_space == std::string::npos) ? current_line : current_line.substr(last_space + 1);
            if (!prefix.empty()) {
                DIR* dir = opendir(".");
                if (dir) {
                    struct dirent* entry;
                    while ((entry = readdir(dir)) != nullptr) {
                        std::string name(entry->d_name);
                        if (name.rfind(prefix, 0) == 0 && name != "." && name != "..") {
                            std::string append_part = name.substr(prefix.size());
                            current_line.insert(cursor_pos, append_part);
                            original_line = current_line;
                            cursor_pos += static_cast<int>(append_part.size());
                            refresh_prompt();
                            break;
                        }
                    }
                    closedir(dir);
                }
            }
            continue;
        }

        // Escape key or Escape sequences (Arrows, Alt+Arrows, Ctrl+Shift+X, Home, End)
        if (c == 27) {
            std::string seq;
            while (has_pending_input(STDIN_FILENO, 30)) {
                char ch = 0;
                if (read(STDIN_FILENO, &ch, 1) <= 0) break;
                seq += ch;
                if (seq.size() > 1 && ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch == '~' || ch == '@')) {
                    break;
                }
            }

            // Standalone ESC pressed: Dismiss history preview and restore original text
            if (seq.empty()) {
                if (preview_lines_drawn > 0) {
                    clear_history_preview(out, preview_lines_drawn);
                    preview_lines_drawn = 0;
                    current_line = original_line;
                    cursor_pos = static_cast<int>(current_line.size());
                    history_idx = static_cast<int>(history.size());
                    refresh_prompt();
                }
                continue;
            }

            // --- 1. Alt + Up Arrow: Navigate focus UP ---
            if (seq == "[1;3A" || seq == "[1;9A" || seq == "[1;4A" || seq == "[1;7A" ||
                seq == "\033[A" || seq == "[a" || seq == "[57352;3u") {
                if (preview_lines_drawn > 0) {
                    clear_history_preview(out, preview_lines_drawn);
                    preview_lines_drawn = 0;
                }
                auto& tree = workspace::get_session_pane_tree();
                auto adj = tree.find_adjacent_pane(tree.active_pane_id(), workspace::NavigationDirection::Up);
                if (adj.has_value()) {
                    tree.set_active_pane(adj.value());
                    out << "\n\033[38;2;0;229;255m▲\033[0m Focused pane UP [ID: " << adj.value() << "] (Alt+Up)\n";
                } else {
                    out << "\n\033[38;2;140;150;170mℹ No adjacent pane above current active pane (ID: " << tree.active_pane_id() << ")\033[0m\n";
                }
                out.flush();
                refresh_prompt();
                continue;
            }

            // --- 2. Alt + Down Arrow: Navigate focus DOWN ---
            if (seq == "[1;3B" || seq == "[1;9B" || seq == "[1;4B" || seq == "[1;7B" ||
                seq == "\033[B" || seq == "[b" || seq == "[57353;3u") {
                if (preview_lines_drawn > 0) {
                    clear_history_preview(out, preview_lines_drawn);
                    preview_lines_drawn = 0;
                }
                auto& tree = workspace::get_session_pane_tree();
                auto adj = tree.find_adjacent_pane(tree.active_pane_id(), workspace::NavigationDirection::Down);
                if (adj.has_value()) {
                    tree.set_active_pane(adj.value());
                    out << "\n\033[38;2;0;229;255m▼\033[0m Focused pane DOWN [ID: " << adj.value() << "] (Alt+Down)\n";
                } else {
                    out << "\n\033[38;2;140;150;170mℹ No adjacent pane below current active pane (ID: " << tree.active_pane_id() << ")\033[0m\n";
                }
                out.flush();
                refresh_prompt();
                continue;
            }

            // --- 3. Alt + Left Arrow: Navigate focus LEFT ---
            if (seq == "[1;3D" || seq == "[1;9D" || seq == "[1;4D" || seq == "[1;7D" ||
                seq == "\033[D" || seq == "[d" || seq == "[57350;3u") {
                if (preview_lines_drawn > 0) {
                    clear_history_preview(out, preview_lines_drawn);
                    preview_lines_drawn = 0;
                }
                auto& tree = workspace::get_session_pane_tree();
                auto adj = tree.find_adjacent_pane(tree.active_pane_id(), workspace::NavigationDirection::Left);
                if (adj.has_value()) {
                    tree.set_active_pane(adj.value());
                    out << "\n\033[38;2;0;229;255m◀\033[0m Focused pane LEFT [ID: " << adj.value() << "] (Alt+Left)\n";
                } else {
                    out << "\n\033[38;2;140;150;170mℹ No adjacent pane to the left of active pane (ID: " << tree.active_pane_id() << ")\033[0m\n";
                }
                out.flush();
                refresh_prompt();
                continue;
            }

            // --- 4. Alt + Right Arrow: Navigate focus RIGHT ---
            if (seq == "[1;3C" || seq == "[1;9C" || seq == "[1;4C" || seq == "[1;7C" ||
                seq == "\033[C" || seq == "[c" || seq == "[57351;3u") {
                if (preview_lines_drawn > 0) {
                    clear_history_preview(out, preview_lines_drawn);
                    preview_lines_drawn = 0;
                }
                auto& tree = workspace::get_session_pane_tree();
                auto adj = tree.find_adjacent_pane(tree.active_pane_id(), workspace::NavigationDirection::Right);
                if (adj.has_value()) {
                    tree.set_active_pane(adj.value());
                    out << "\n\033[38;2;0;229;255m▶\033[0m Focused pane RIGHT [ID: " << adj.value() << "] (Alt+Right)\n";
                } else {
                    out << "\n\033[38;2;140;150;170mℹ No adjacent pane to the right of active pane (ID: " << tree.active_pane_id() << ")\033[0m\n";
                }
                out.flush();
                refresh_prompt();
                continue;
            }

            // --- 5. Ctrl+Shift+D: Split active pane vertically ---
            if (seq == "[1;6D" || seq == "[1;5D" || seq == "[1;2D" || seq == "[68;6u" ||
                seq == "[100;6u" || seq == "[68;5u" || seq == "[100;5u" || seq == "[4;6~" || seq == "[4;5~") {
                if (preview_lines_drawn > 0) {
                    clear_history_preview(out, preview_lines_drawn);
                    preview_lines_drawn = 0;
                }
                auto& tree = workspace::get_session_pane_tree();
                uint32_t new_id = tree.split_pane(tree.active_pane_id(), workspace::SplitDirection::Vertical);
                out << "\n\033[38;2;34;197;94m✔\033[0m Pane split vertically [ID: " << new_id
                    << " | Active: " << tree.active_pane_id()
                    << " | Total: " << tree.count_panes() << "] (Ctrl+Shift+D)\n";
                out.flush();
                refresh_prompt();
                continue;
            }

            // --- 6. Ctrl+Shift+E: Split active pane horizontally ---
            if (seq == "[1;6E" || seq == "[1;5E" || seq == "[1;2E" || seq == "[69;6u" ||
                seq == "[101;6u" || seq == "[69;5u" || seq == "[101;5u" || seq == "[5;6~" || seq == "[5;5~") {
                if (preview_lines_drawn > 0) {
                    clear_history_preview(out, preview_lines_drawn);
                    preview_lines_drawn = 0;
                }
                auto& tree = workspace::get_session_pane_tree();
                uint32_t new_id = tree.split_pane(tree.active_pane_id(), workspace::SplitDirection::Horizontal);
                out << "\n\033[38;2;34;197;94m✔\033[0m Pane split horizontally [ID: " << new_id
                    << " | Active: " << tree.active_pane_id()
                    << " | Total: " << tree.count_panes() << "] (Ctrl+Shift+E)\n";
                out.flush();
                refresh_prompt();
                continue;
            }

            // --- 7. Ctrl+Shift+Z: Toggle zoom on active pane ---
            if (seq == "[1;6Z" || seq == "[1;5Z" || seq == "[1;2Z" || seq == "[90;6u" ||
                seq == "[122;6u" || seq == "[90;5u" || seq == "[122;5u" || seq == "[26;6~" || seq == "[26;5~") {
                if (preview_lines_drawn > 0) {
                    clear_history_preview(out, preview_lines_drawn);
                    preview_lines_drawn = 0;
                }
                auto& tree = workspace::get_session_pane_tree();
                tree.toggle_zoom();
                if (tree.is_zoomed()) {
                    out << "\n\033[38;2;245;158;11m⛶\033[0m Pane ZOOMED / Maximized [ID: " << tree.active_pane_id() << "] (Ctrl+Shift+Z)\n";
                } else {
                    out << "\n\033[38;2;59;130;246m⧉\033[0m Pane Zoom RESTORED / Unmaximized [Active ID: " << tree.active_pane_id()
                        << " | Total: " << tree.count_panes() << "] (Ctrl+Shift+Z)\n";
                }
                out.flush();
                refresh_prompt();
                continue;
            }

            // --- 8. Regular Navigation: Up, Down, Left, Right, Home, End ---
            if (seq == "[A" || seq == "OA") {
                // Up Arrow: History backward
                if (!history.empty() && history_idx > 0) {
                    if (history_idx == static_cast<int>(history.size())) {
                        original_line = current_line;
                    }
                    history_idx--;
                    current_line = history[history_idx];
                    cursor_pos = static_cast<int>(current_line.size());

                    int items_to_show = std::min(5, static_cast<int>(history.size()));
                    int needed_lines = items_to_show + 2;
                    if (preview_lines_drawn > 0) {
                        clear_history_preview(out, preview_lines_drawn);
                    }
                    draw_history_preview(out, history, history_idx, 5);
                    preview_lines_drawn = needed_lines;
                    refresh_prompt();
                }
                continue;
            }
            if (seq == "[B" || seq == "OB") {
                // Down Arrow: History forward
                if (history_idx + 1 < static_cast<int>(history.size())) {
                    history_idx++;
                    current_line = history[history_idx];
                    cursor_pos = static_cast<int>(current_line.size());

                    int items_to_show = std::min(5, static_cast<int>(history.size()));
                    int needed_lines = items_to_show + 2;
                    if (preview_lines_drawn > 0) {
                        clear_history_preview(out, preview_lines_drawn);
                    }
                    draw_history_preview(out, history, history_idx, 5);
                    preview_lines_drawn = needed_lines;
                    refresh_prompt();
                } else if (history_idx + 1 == static_cast<int>(history.size())) {
                    history_idx++;
                    if (preview_lines_drawn > 0) {
                        clear_history_preview(out, preview_lines_drawn);
                        preview_lines_drawn = 0;
                    }
                    current_line = original_line;
                    cursor_pos = static_cast<int>(current_line.size());
                    refresh_prompt();
                }
                continue;
            }
            if (seq == "[C" || seq == "OC") {
                // Right Arrow
                if (cursor_pos < static_cast<int>(current_line.size())) {
                    cursor_pos++;
                    refresh_prompt();
                }
                continue;
            }
            if (seq == "[D" || seq == "OD") {
                // Left Arrow
                if (cursor_pos > 0) {
                    cursor_pos--;
                    refresh_prompt();
                }
                continue;
            }
            if (seq == "[H" || seq == "OH" || seq == "[1~" || seq == "[7~") {
                // Home
                cursor_pos = 0;
                refresh_prompt();
                continue;
            }
            if (seq == "[F" || seq == "OF" || seq == "[4~" || seq == "[8~") {
                // End
                cursor_pos = static_cast<int>(current_line.size());
                refresh_prompt();
                continue;
            }

            continue;
        }

        // Normal printable characters: typing dismisses preview and updates buffer
        if (c >= 32 && c <= 126) {
            if (preview_lines_drawn > 0) {
                clear_history_preview(out, preview_lines_drawn);
                preview_lines_drawn = 0;
                history_idx = static_cast<int>(history.size());
            }
            current_line.insert(cursor_pos, 1, c);
            original_line = current_line;
            cursor_pos++;
            refresh_prompt();
        }
    }

    return current_line;
}

} // namespace meridian::shell
