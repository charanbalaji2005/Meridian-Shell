#include "line_editor.hpp"
#include "../dev/git_intel.hpp"
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

    std::ostringstream ss;
    // Badge 1: Vibrant Cobalt Blue Date/Time Badge
    ss << "  \033[48;2;26;108;218;38;2;255;255;255;1m " << time_buf << " "
    // Arrow from Cobalt Blue to Ocean Cyan
       << "\033[48;2;24;156;184;38;2;26;108;218m"
    // Badge 2: Vibrant Ocean Cyan Directory Badge
       << "\033[48;2;24;156;184;38;2;255;255;255;1m " << dir << " ";

    // Live Git Repo Status
    auto git_status = dev::GitIntel::inspect_directory(cwd.empty() ? "." : cwd);
    if (git_status.is_git_repo && !git_status.branch_name.empty()) {
        std::string branch = git_status.branch_name;
        int unstaged = git_status.unstaged_count + git_status.untracked_count;
        int staged = git_status.staged_count;
        int ahead = git_status.ahead_count;
        int behind = git_status.behind_count;

        std::string git_extra;
        if (ahead > 0) git_extra += " ↑" + std::to_string(ahead);
        if (behind > 0) git_extra += " ↓" + std::to_string(behind);
        if (unstaged > 0) git_extra += " " + std::to_string(unstaged) + "✸";
        if (staged > 0) git_extra += " " + std::to_string(staged) + "●";
        if (git_status.is_clean) git_extra += " ✔";

        // Arrow from Ocean Cyan to Emerald Green
        ss << "\033[48;2;16;185;129;38;2;24;156;184m"
        // Badge 3: Vibrant Emerald Green Git Badge (matching user reference)
           << "\033[48;2;16;185;129;38;2;20;35;28;1m 󰘬 origin ☊ " << branch << git_extra << " "
        // Ending Arrow from Emerald Green to Terminal Default
           << "\033[0;38;2;16;185;129m\033[0m";
    } else {
        // Ending Arrow from Ocean Cyan to Terminal Default
        ss << "\033[0;38;2;24;156;184m\033[0m";
    }

    return ss.str();
}

std::string LineEditor::build_powerline_prompt(const std::string& /*cwd*/) {
    // Detect username
    std::string user = "charanbalaji";
    struct passwd* pw = getpwuid(geteuid());
    if (pw && pw->pw_name) user = pw->pw_name;

    std::ostringstream ss;
    // Single-line prompt: Vibrant Golden Yellow badge with purple/dark text (@username )
    ss << "  \033[48;2;212;180;27;38;2;85;35;110;1m @" << user << " "
       << "\033[0;38;2;212;180;27m\033[0m ";
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

        // Ctrl+D (EOF on empty line)
        if (c == 4) {
            if (current_line.empty()) {
                if (preview_lines_drawn > 0) {
                    clear_history_preview(out, preview_lines_drawn);
                }
                return "exit";
            }
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

        // Ctrl+P / Ctrl+Shift+P: Terminal Anime / Artwork Selector
        if (c == 16) {
            if (preview_lines_drawn > 0) {
                clear_history_preview(out, preview_lines_drawn);
                preview_lines_drawn = 0;
            }
            auto themes = core::ArtGallery::list_themes();
            std::string cur = core::ArtGallery::get_configured_choice();

            out << "\n\033[1;36m┌─── MERIDIAN ANIME & ARTWORK SELECTOR (Ctrl+P) ──────────────────────────────────┐\033[0m\n";
            for (size_t i = 0; i < themes.size(); ++i) {
                bool is_active = (cur == themes[i].first || cur == std::to_string(i));
                out << "\033[1;36m│\033[0m " << (is_active ? "\033[1;32m●\033[0m" : " ")
                    << " \033[1;33m[" << i << "]\033[0m \033[1;37m" << themes[i].second;
                int pad = 58 - static_cast<int>(themes[i].second.size());
                for (int p = 0; p < pad; ++p) out << " ";
                out << (is_active ? "\033[1;32m[ACTIVE]\033[0m" : "        ") << " \033[1;36m│\033[0m\n";
            }
            bool is_random = (cur == "random" || cur.empty());
            out << "\033[1;36m│\033[0m " << (is_random ? "\033[1;32m●\033[0m" : " ")
                << " \033[1;33m[r]\033[0m \033[1;37mRandom / Rotating on every startup                         \033[0m"
                << (is_random ? "\033[1;32m[ACTIVE]\033[0m" : "        ") << " \033[1;36m│\033[0m\n";
            out << "\033[1;36m└─────────────────────────────────────────────────────────────────────────────────┘\033[0m\n"
                << "\033[0;37mTip: Type '\033[1;32mpic set <0-9|name|path>\033[0m' or '\033[1;32mpic set random\033[0m' to switch anytime!\033[0m\n";
            out.flush();
            current_line = "pic set ";
            cursor_pos = static_cast<int>(current_line.size());
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

        // Escape key or Escape sequences (Arrows, Home, End)
        if (c == 27) {
            // Check if this is a standalone ESC keypress or the beginning of a sequence
            if (!has_pending_input(STDIN_FILENO, 50)) {
                // Standalone ESC pressed: Dismiss history preview and restore original text
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

            char seq[3] = {0, 0, 0};
            if (read(STDIN_FILENO, &seq[0], 1) <= 0) continue;
            if (read(STDIN_FILENO, &seq[1], 1) <= 0) continue;

            if (seq[0] == '[') {
                // Up Arrow: History backward & show preview
                if (seq[1] == 'A') {
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
                }
                // Down Arrow: History forward
                else if (seq[1] == 'B') {
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
                }
                // Right Arrow
                else if (seq[1] == 'C') {
                    if (cursor_pos < static_cast<int>(current_line.size())) {
                        cursor_pos++;
                        refresh_prompt();
                    }
                }
                // Left Arrow
                else if (seq[1] == 'D') {
                    if (cursor_pos > 0) {
                        cursor_pos--;
                        refresh_prompt();
                    }
                }
                // Home
                else if (seq[1] == 'H') {
                    cursor_pos = 0;
                    refresh_prompt();
                }
                // End
                else if (seq[1] == 'F') {
                    cursor_pos = static_cast<int>(current_line.size());
                    refresh_prompt();
                }
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
