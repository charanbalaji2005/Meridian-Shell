// src/shell/line_editor.cpp
#include "line_editor.hpp"
#include "autosuggest_engine.hpp"
#include "../dev/git_intel.hpp"
#include "../dev/icon_provider.hpp"
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

// ─── Raw Mode ───────────────────────────────────────────────────────────────

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

// ─── CWD Helpers ────────────────────────────────────────────────────────────

std::string format_cwd(const std::string& cwd) {
    std::string dir = cwd.empty() ? "~" : cwd;
    const char* home = std::getenv("HOME");
    if (home && dir.rfind(home, 0) == 0) {
        dir = "~" + dir.substr(std::strlen(home));
    }
    return dir;
}

static std::string detect_project_language(const std::string& dir) {
    std::string d = dir.empty() ? "." : dir;
    if (access((d + "/CMakeLists.txt").c_str(), F_OK) == 0 ||
        access((d + "/Makefile").c_str(), F_OK) == 0) return "󰙲 ";
    if (access((d + "/Cargo.toml").c_str(), F_OK) == 0) return " ";
    if (access((d + "/pyproject.toml").c_str(), F_OK) == 0 ||
        access((d + "/requirements.txt").c_str(), F_OK) == 0 ||
        access((d + "/setup.py").c_str(), F_OK) == 0) return " ";
    if (access((d + "/package.json").c_str(), F_OK) == 0 ||
        access((d + "/tsconfig.json").c_str(), F_OK) == 0) return " ";
    if (access((d + "/go.mod").c_str(), F_OK) == 0) return " ";
    if (access((d + "/pom.xml").c_str(), F_OK) == 0 ||
        access((d + "/build.gradle").c_str(), F_OK) == 0) return " ";
    if (access((d + "/Dockerfile").c_str(), F_OK) == 0 ||
        access((d + "/compose.yaml").c_str(), F_OK) == 0) return "󰡨 ";
    if (access((d + "/flake.nix").c_str(), F_OK) == 0) return " ";
    return "";
}

// ─── Suggestion Rendering ────────────────────────────────────────────────────

// Returns the current terminal width
static int terminal_width() {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) return ws.ws_col;
    return 80;
}

// Renders prompt + user_input + dim ghost suffix in one line.
// cursor_pos is the position within user_input (0-based).
static void render_prompt_with_suggestion(
    std::ostream& out,
    const std::string& prompt,           // already ANSI-colored Powerline prompt
    const std::string& user_input,       // what the user has typed
    const std::string& suggestion_suffix, // the dim ghost part (after user_input)
    int cursor_pos
) {
    // Clear the current line and redraw
    out << "\r\033[K";                          // carriage return + erase line
    out << prompt;                              // powerline prompt (no newline)
    out << "\033[0m";                           // reset colors from prompt
    out << user_input;                          // actual typed text (normal)

    if (!suggestion_suffix.empty()) {
        // Dim ghost color: dark grey, dim attribute
        out << "\033[38;2;85;95;115;2m";        // 256-grayish + dim
        out << suggestion_suffix;
        out << "\033[0m";
    }

    // Reposition cursor to cursor_pos within user_input
    int input_len = static_cast<int>(user_input.size());
    int back = input_len - cursor_pos;
    if (!suggestion_suffix.empty()) back += static_cast<int>(suggestion_suffix.size());
    if (back > 0) {
        out << "\033[" << back << "D";
    }
    out.flush();
}

// ─── Tab Completion Menu ─────────────────────────────────────────────────────

// Returns the common prefix of a set of raw (icon-stripped-free) filenames
static std::string common_prefix(const std::vector<std::string>& names) {
    if (names.empty()) return "";
    if (names.size() == 1) return names[0];
    std::string cp = names[0];
    for (size_t i = 1; i < names.size(); ++i) {
        size_t j = 0;
        while (j < cp.size() && j < names[i].size() && cp[j] == names[i][j]) ++j;
        cp = cp.substr(0, j);
    }
    return cp;
}

// Draw the tab completion popup (below current line) and return number of lines drawn.
// candidates: the formatted (colored, icon) completion entries.
static int draw_completion_menu(
    std::ostream& out,
    const std::vector<std::string>& candidates,
    int max_show = 8
) {
    if (candidates.empty()) return 0;

    int show = std::min(static_cast<int>(candidates.size()), max_show);
    int lines = 0;

    out << "\r\n";
    lines++;

    if (candidates.size() == 1) {
        // Single candidate: display inline on a dim line
        out << "\033[38;2;80;90;110m  " << candidates[0] << "\033[0m";
        out << "\r\n";
        lines++;
    } else {
        // Multiple: show a compact list
        for (int i = 0; i < show; ++i) {
            out << "  " << candidates[i];
            if (i < show - 1) out << "   ";
            // Wrap every 3 items for readability
            if ((i + 1) % 3 == 0 && i < show - 1) {
                out << "\r\n";
                lines++;
            }
        }
        if (candidates.size() > static_cast<size_t>(max_show)) {
            out << "  \033[38;2;100;110;130m(+" << (candidates.size() - max_show)
                << " more, press Tab again)\033[0m";
        }
        out << "\r\n";
        lines++;
    }

    // Move cursor back up
    out << "\033[" << lines << "A\r";
    out.flush();
    return lines;
}

static void clear_completion_menu(std::ostream& out, int lines_drawn) {
    if (lines_drawn <= 0) return;
    for (int i = 0; i < lines_drawn; ++i) {
        out << "\n\033[2K\r";
    }
    out << "\033[" << lines_drawn << "A\r";
    out.flush();
}

} // namespace

// ─── LineEditor Interface ─────────────────────────────────────────────────────

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
    std::string dir_icon = dev::IconProvider::icon_for_directory(dir, git_status.is_git_repo);
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

        ss << "\033[48;2;124;58;237;38;2;255;255;255;1m  " << u << "@" << hname << " "
           << "\033[48;2;24;156;184;38;2;124;58;237m";
    } else {
        ss << "\033[48;2;26;108;218;38;2;255;255;255;1m " << time_buf << " "
           << "\033[48;2;24;156;184;38;2;26;108;218m";
    }

    // Segment 2: Directory with icon
    ss << "\033[48;2;24;156;184;38;2;255;255;255;1m " << dir_icon << dir << " ";

    // Segment 3: Git Segment with enhanced indicators
    if (git_status.is_git_repo && !git_status.branch_name.empty()) {
        std::string branch = git_status.branch_name;
        dev::IconTier tier = dev::IconProvider::detect_tier();

        std::string git_extra;
        if (git_status.ahead_count > 0)
            git_extra += std::string(" ") + dev::IconSet::git_ahead(tier) + std::to_string(git_status.ahead_count);
        if (git_status.behind_count > 0)
            git_extra += std::string(" ") + dev::IconSet::git_behind(tier) + std::to_string(git_status.behind_count);
        if (git_status.unstaged_count > 0)
            git_extra += " ✚" + std::to_string(git_status.unstaged_count);
        if (git_status.staged_count > 0)
            git_extra += " ●" + std::to_string(git_status.staged_count);
        if (git_status.untracked_count > 0)
            git_extra += " ?" + std::to_string(git_status.untracked_count);
        if (git_status.is_clean && git_status.ahead_count == 0 && git_status.behind_count == 0)
            git_extra += " ✔";

        ss << "\033[48;2;16;185;129;38;2;24;156;184m"
           << "\033[48;2;16;185;129;38;2;20;35;28;1m " << dev::IconSet::git_branch(tier)
           << branch << git_extra << " "
           << "\033[0;38;2;16;185;129m\033[0m";
    } else {
        ss << "\033[0;38;2;24;156;184m\033[0m";
    }

    return ss.str();
}

std::string LineEditor::build_powerline_prompt(const std::string& /*cwd*/) {
    std::string user = "user";
    uid_t uid = geteuid();
    struct passwd* pw = getpwuid(uid);
    if (pw && pw->pw_name) user = pw->pw_name;

    std::ostringstream ss;
    // Dim grey hollow circle dot glyph
    ss << "\033[38;2;120;130;150m○ \033[0m";

    if (uid == 0) {
        // Elevated Root Prompt in Crimson Red
        ss << "\033[48;2;220;38;38;38;2;255;255;255;1m ⚡ root "
           << "\033[0;38;2;220;38;38m\033[0m \033[1;31m❯\033[0m ";
    } else {
        // Standard User Prompt: Vibrant Golden Yellow badge
        ss << "\033[48;2;212;180;27;38;2;85;35;110;1m @" << user << " "
           << "\033[0;38;2;212;180;27m\033[0m \033[1;33m❯\033[0m ";
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

// ─── Main Read Line ───────────────────────────────────────────────────────────

std::string LineEditor::read_line(
    std::istream& in,
    std::ostream& out,
    const std::string& prompt,
    const std::vector<std::string>& history
) {
    if (!is_terminal_interactive()) {
        std::string line;
        if (std::getline(in, line)) return line;
        return "";
    }

    // Get current working directory for filesystem completions
    char cwd_buf[4096] = "";
    getcwd(cwd_buf, sizeof(cwd_buf));
    std::string cwd = cwd_buf;

    RawModeScope raw;
    std::string current_line;
    std::string original_line;
    std::string current_suggestion;   // The full suggestion string (superset of current_line)
    int cursor_pos = 0;
    int history_idx = static_cast<int>(history.size());
    int preview_lines_drawn = 0;
    int completion_lines_drawn = 0;

    auto update_suggestion = [&]() {
        if (current_line.empty()) {
            current_suggestion = "";
            return;
        }
        current_suggestion = AutosuggestEngine::get_suggestion(current_line, history, cwd);
        // Ensure it's actually a longer extension of the current line
        if (!current_suggestion.empty() && current_suggestion.size() <= current_line.size()) {
            current_suggestion = "";
        }
    };

    auto suggestion_suffix = [&]() -> std::string {
        if (current_suggestion.empty()) return "";
        return AutosuggestEngine::get_suggestion_suffix(current_line, current_suggestion);
    };

    // Full prompt refresh with ghost suggestion
    auto refresh_prompt = [&]() {
        render_prompt_with_suggestion(
            out, prompt, current_line, suggestion_suffix(), cursor_pos);
    };

    update_suggestion();
    refresh_prompt();

    while (true) {
        char c = 0;
        if (read(STDIN_FILENO, &c, 1) <= 0) break;

        // ── Enter: Commit line ────────────────────────────────────────────
        if (c == '\n' || c == '\r') {
            if (preview_lines_drawn > 0) {
                clear_history_preview(out, preview_lines_drawn);
                preview_lines_drawn = 0;
            }
            if (completion_lines_drawn > 0) {
                clear_completion_menu(out, completion_lines_drawn);
                completion_lines_drawn = 0;
            }
            // Print accepted line without ghost suffix
            out << "\r\033[K" << prompt << "\033[0m" << current_line << "\n";
            out.flush();
            return current_line;
        }

        // ── Ctrl+C: Cancel ────────────────────────────────────────────────
        if (c == 3) {
            if (preview_lines_drawn > 0) {
                clear_history_preview(out, preview_lines_drawn);
                preview_lines_drawn = 0;
            }
            if (completion_lines_drawn > 0) {
                clear_completion_menu(out, completion_lines_drawn);
                completion_lines_drawn = 0;
            }
            current_suggestion = "";
            out << "^C\n";
            out.flush();
            return "";
        }

        // ── Ctrl+D: Pane Split Vertical ───────────────────────────────────
        if (c == 4) {
            if (preview_lines_drawn > 0) { clear_history_preview(out, preview_lines_drawn); preview_lines_drawn = 0; }
            if (completion_lines_drawn > 0) { clear_completion_menu(out, completion_lines_drawn); completion_lines_drawn = 0; }
            auto& tree = workspace::get_session_pane_tree();
            uint32_t new_id = tree.split_pane(tree.active_pane_id(), workspace::SplitDirection::Vertical);
            out << "\n\033[38;2;34;197;94m✔\033[0m Pane split vertically [ID: " << new_id
                << " | Active: " << tree.active_pane_id()
                << " | Total: " << tree.count_panes() << "] (Ctrl+Shift+D)\n";
            out.flush();
            refresh_prompt();
            continue;
        }

        // ── Ctrl+E: Pane Split Horizontal ─────────────────────────────────
        if (c == 5) {
            if (preview_lines_drawn > 0) { clear_history_preview(out, preview_lines_drawn); preview_lines_drawn = 0; }
            if (completion_lines_drawn > 0) { clear_completion_menu(out, completion_lines_drawn); completion_lines_drawn = 0; }
            auto& tree = workspace::get_session_pane_tree();
            uint32_t new_id = tree.split_pane(tree.active_pane_id(), workspace::SplitDirection::Horizontal);
            out << "\n\033[38;2;34;197;94m✔\033[0m Pane split horizontally [ID: " << new_id
                << " | Active: " << tree.active_pane_id()
                << " | Total: " << tree.count_panes() << "] (Ctrl+Shift+E)\n";
            out.flush();
            refresh_prompt();
            continue;
        }

        // ── Ctrl+F: Universal Search ──────────────────────────────────────
        if (c == 6) {
            if (preview_lines_drawn > 0) { clear_history_preview(out, preview_lines_drawn); preview_lines_drawn = 0; }
            if (completion_lines_drawn > 0) { clear_completion_menu(out, completion_lines_drawn); completion_lines_drawn = 0; }
            std::string q = current_line.empty() ? "error" : current_line;
            vt::ScreenBuffer screen;
            dev::RichHistory r_hist;
            auto matches = dev::UniversalSearch::search_all(screen, r_hist, q);
            out << "\n\033[1;36m┌─── Universal Search (Ctrl+Shift+F) ──────────────────────────┐\033[0m\n"
                << "\033[1;37mQuery: \"" << q << "\" (" << matches.size() << " matches)\033[0m\n";
            for (const auto& m : matches) {
                out << "  \033[1;33m[" << m.source_label << "]\033[0m " << m.line_content << "\n";
            }
            out << "\033[1;36m└──────────────────────────────────────────────────────────────┘\033[0m\n";
            out.flush();
            refresh_prompt();
            continue;
        }

        // ── Ctrl+L: Clear viewport ────────────────────────────────────────
        if (c == 12) {
            if (preview_lines_drawn > 0) { clear_history_preview(out, preview_lines_drawn); preview_lines_drawn = 0; }
            if (completion_lines_drawn > 0) { clear_completion_menu(out, completion_lines_drawn); completion_lines_drawn = 0; }
            // Full VT/ANSI screen clear: home, erase display, erase scrollback
            out << "\033[H\033[2J\033[3J";
            out.flush();
            refresh_prompt();
            continue;
        }

        // ── Ctrl+P: Command Palette ───────────────────────────────────────
        if (c == 16) {
            if (preview_lines_drawn > 0) { clear_history_preview(out, preview_lines_drawn); preview_lines_drawn = 0; }
            if (completion_lines_drawn > 0) { clear_completion_menu(out, completion_lines_drawn); completion_lines_drawn = 0; }
            dev::CommandPalette palette;
            auto results = palette.search(current_line);
            out << "\n\033[1;37m┌─── Meridian Command Palette (Ctrl+Shift+P) ──────────────────┐\033[0m\n";
            for (const auto& a : results) {
                out << "\033[1;37m│\033[0m \033[1;36m[" << a.category << "]\033[0m \033[1;37m" << a.title << "\033[0m "
                    << "\033[38;2;140;150;170m(" << a.shortcut << ")\033[0m\n";
            }
            out << "\033[1;37m└──────────────────────────────────────────────────────────────┘\033[0m\n";
            out.flush();
            refresh_prompt();
            continue;
        }

        // ── Ctrl+Z: Toggle zoom ───────────────────────────────────────────
        if (c == 26) {
            if (preview_lines_drawn > 0) { clear_history_preview(out, preview_lines_drawn); preview_lines_drawn = 0; }
            if (completion_lines_drawn > 0) { clear_completion_menu(out, completion_lines_drawn); completion_lines_drawn = 0; }
            auto& tree = workspace::get_session_pane_tree();
            tree.toggle_zoom();
            if (tree.is_zoomed()) {
                out << "\n\033[38;2;245;158;11m⛶\033[0m Pane ZOOMED [ID: " << tree.active_pane_id() << "]\n";
            } else {
                out << "\n\033[38;2;59;130;246m⧉\033[0m Pane Zoom RESTORED [Active: " << tree.active_pane_id()
                    << " | Total: " << tree.count_panes() << "]\n";
            }
            out.flush();
            refresh_prompt();
            continue;
        }

        // ── Backspace ─────────────────────────────────────────────────────
        if (c == 127 || c == 8) {
            if (preview_lines_drawn > 0) {
                clear_history_preview(out, preview_lines_drawn);
                preview_lines_drawn = 0;
                history_idx = static_cast<int>(history.size());
            }
            if (completion_lines_drawn > 0) {
                clear_completion_menu(out, completion_lines_drawn);
                completion_lines_drawn = 0;
            }
            if (cursor_pos > 0) {
                current_line.erase(cursor_pos - 1, 1);
                original_line = current_line;
                cursor_pos--;
                update_suggestion();
                refresh_prompt();
            }
            continue;
        }

        // ── Tab: Completion ───────────────────────────────────────────────
        if (c == '\t') {
            if (preview_lines_drawn > 0) {
                clear_history_preview(out, preview_lines_drawn);
                preview_lines_drawn = 0;
            }
            if (completion_lines_drawn > 0) {
                clear_completion_menu(out, completion_lines_drawn);
                completion_lines_drawn = 0;
            }

            auto candidates = AutosuggestEngine::get_tab_completions_for_line(
                current_line.substr(0, cursor_pos), cwd, 12);

            if (candidates.size() == 1) {
                // Single candidate: extract the completion and insert it
                // Candidates are ANSI-colored; extract the plain text suffix
                // The last space-separated "word" in current_line
                auto last_sp = current_line.find_last_of(" \t", cursor_pos);
                std::string word_prefix = (last_sp == std::string::npos)
                    ? current_line.substr(0, cursor_pos)
                    : current_line.substr(last_sp + 1, cursor_pos - last_sp - 1);

                // Use autosuggest filesystem completion for clean text
                std::string completion = AutosuggestEngine::get_filesystem_completion(word_prefix, cwd);
                if (!completion.empty() && completion != word_prefix) {
                    std::string new_line = (last_sp == std::string::npos)
                        ? completion + current_line.substr(cursor_pos)
                        : current_line.substr(0, last_sp + 1) + completion + current_line.substr(cursor_pos);
                    int new_cursor = (last_sp == std::string::npos)
                        ? static_cast<int>(completion.size())
                        : static_cast<int>(last_sp + 1 + completion.size());
                    current_line = new_line;
                    original_line = current_line;
                    cursor_pos = new_cursor;
                }
                update_suggestion();
                refresh_prompt();
            } else if (candidates.size() > 1) {
                // Multiple candidates: find common prefix and advance, then show menu
                // Strip ANSI for common prefix computation
                // Just show the menu below
                completion_lines_drawn = draw_completion_menu(out, candidates, 9);
                refresh_prompt();
            } else {
                // No candidates: try autosuggest inline
                if (!current_suggestion.empty()) {
                    current_line = current_suggestion;
                    original_line = current_line;
                    cursor_pos = static_cast<int>(current_line.size());
                    current_suggestion = "";
                    update_suggestion();
                    refresh_prompt();
                }
            }
            continue;
        }

        // ── Escape sequences (arrows, home, end, ctrl+arrows) ────────────
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

            // Standalone ESC: dismiss suggestion / history
            if (seq.empty()) {
                if (!current_suggestion.empty()) {
                    current_suggestion = "";
                    refresh_prompt();
                } else if (preview_lines_drawn > 0) {
                    clear_history_preview(out, preview_lines_drawn);
                    preview_lines_drawn = 0;
                    current_line = original_line;
                    cursor_pos = static_cast<int>(current_line.size());
                    history_idx = static_cast<int>(history.size());
                    update_suggestion();
                    refresh_prompt();
                }
                if (completion_lines_drawn > 0) {
                    clear_completion_menu(out, completion_lines_drawn);
                    completion_lines_drawn = 0;
                }
                continue;
            }

            // Alt+Up (pane navigation)
            if (seq == "[1;3A" || seq == "[1;9A" || seq == "[1;4A" || seq == "[1;7A") {
                if (preview_lines_drawn > 0) { clear_history_preview(out, preview_lines_drawn); preview_lines_drawn = 0; }
                if (completion_lines_drawn > 0) { clear_completion_menu(out, completion_lines_drawn); completion_lines_drawn = 0; }
                auto& tree = workspace::get_session_pane_tree();
                auto adj = tree.find_adjacent_pane(tree.active_pane_id(), workspace::NavigationDirection::Up);
                if (adj.has_value()) {
                    tree.set_active_pane(adj.value());
                    out << "\n\033[38;2;0;229;255m▲\033[0m Focused pane UP [ID: " << adj.value() << "]\n";
                } else {
                    out << "\n\033[38;2;140;150;170mℹ No adjacent pane above\033[0m\n";
                }
                out.flush(); refresh_prompt(); continue;
            }

            // Alt+Down (pane navigation)
            if (seq == "[1;3B" || seq == "[1;9B" || seq == "[1;4B" || seq == "[1;7B") {
                if (preview_lines_drawn > 0) { clear_history_preview(out, preview_lines_drawn); preview_lines_drawn = 0; }
                if (completion_lines_drawn > 0) { clear_completion_menu(out, completion_lines_drawn); completion_lines_drawn = 0; }
                auto& tree = workspace::get_session_pane_tree();
                auto adj = tree.find_adjacent_pane(tree.active_pane_id(), workspace::NavigationDirection::Down);
                if (adj.has_value()) {
                    tree.set_active_pane(adj.value());
                    out << "\n\033[38;2;0;229;255m▼\033[0m Focused pane DOWN [ID: " << adj.value() << "]\n";
                } else {
                    out << "\n\033[38;2;140;150;170mℹ No adjacent pane below\033[0m\n";
                }
                out.flush(); refresh_prompt(); continue;
            }

            // Alt+Left (pane navigation)
            if (seq == "[1;3D" || seq == "[1;9D" || seq == "[1;4D" || seq == "[1;7D") {
                if (preview_lines_drawn > 0) { clear_history_preview(out, preview_lines_drawn); preview_lines_drawn = 0; }
                if (completion_lines_drawn > 0) { clear_completion_menu(out, completion_lines_drawn); completion_lines_drawn = 0; }
                auto& tree = workspace::get_session_pane_tree();
                auto adj = tree.find_adjacent_pane(tree.active_pane_id(), workspace::NavigationDirection::Left);
                if (adj.has_value()) {
                    tree.set_active_pane(adj.value());
                    out << "\n\033[38;2;0;229;255m◀\033[0m Focused pane LEFT [ID: " << adj.value() << "]\n";
                } else {
                    out << "\n\033[38;2;140;150;170mℹ No adjacent pane to the left\033[0m\n";
                }
                out.flush(); refresh_prompt(); continue;
            }

            // Alt+Right (pane navigation) — only if NOT at end of input with suggestion
            if (seq == "[1;3C" || seq == "[1;9C" || seq == "[1;4C" || seq == "[1;7C") {
                if (preview_lines_drawn > 0) { clear_history_preview(out, preview_lines_drawn); preview_lines_drawn = 0; }
                if (completion_lines_drawn > 0) { clear_completion_menu(out, completion_lines_drawn); completion_lines_drawn = 0; }
                auto& tree = workspace::get_session_pane_tree();
                auto adj = tree.find_adjacent_pane(tree.active_pane_id(), workspace::NavigationDirection::Right);
                if (adj.has_value()) {
                    tree.set_active_pane(adj.value());
                    out << "\n\033[38;2;0;229;255m▶\033[0m Focused pane RIGHT [ID: " << adj.value() << "]\n";
                } else {
                    out << "\n\033[38;2;140;150;170mℹ No adjacent pane to the right\033[0m\n";
                }
                out.flush(); refresh_prompt(); continue;
            }

            // Ctrl+Shift+D: split vertical
            if (seq == "[1;6D" || seq == "[1;5D" || seq == "[1;2D" || seq == "[68;6u" ||
                seq == "[100;6u" || seq == "[4;6~" || seq == "[4;5~") {
                if (preview_lines_drawn > 0) { clear_history_preview(out, preview_lines_drawn); preview_lines_drawn = 0; }
                if (completion_lines_drawn > 0) { clear_completion_menu(out, completion_lines_drawn); completion_lines_drawn = 0; }
                auto& tree = workspace::get_session_pane_tree();
                uint32_t new_id = tree.split_pane(tree.active_pane_id(), workspace::SplitDirection::Vertical);
                out << "\n\033[38;2;34;197;94m✔\033[0m Pane split vertically [ID: " << new_id << "]\n";
                out.flush(); refresh_prompt(); continue;
            }

            // Ctrl+Shift+E: split horizontal
            if (seq == "[1;6E" || seq == "[1;5E" || seq == "[1;2E" || seq == "[69;6u" ||
                seq == "[101;6u" || seq == "[5;6~" || seq == "[5;5~") {
                if (preview_lines_drawn > 0) { clear_history_preview(out, preview_lines_drawn); preview_lines_drawn = 0; }
                if (completion_lines_drawn > 0) { clear_completion_menu(out, completion_lines_drawn); completion_lines_drawn = 0; }
                auto& tree = workspace::get_session_pane_tree();
                uint32_t new_id = tree.split_pane(tree.active_pane_id(), workspace::SplitDirection::Horizontal);
                out << "\n\033[38;2;34;197;94m✔\033[0m Pane split horizontally [ID: " << new_id << "]\n";
                out.flush(); refresh_prompt(); continue;
            }

            // Ctrl+Shift+Z: toggle zoom
            if (seq == "[1;6Z" || seq == "[1;5Z" || seq == "[26;6~" || seq == "[26;5~") {
                if (preview_lines_drawn > 0) { clear_history_preview(out, preview_lines_drawn); preview_lines_drawn = 0; }
                if (completion_lines_drawn > 0) { clear_completion_menu(out, completion_lines_drawn); completion_lines_drawn = 0; }
                auto& tree = workspace::get_session_pane_tree();
                tree.toggle_zoom();
                out.flush(); refresh_prompt(); continue;
            }

            // ─── Ctrl+Right Arrow: Accept next word of suggestion ────────
            if (seq == "[1;5C" || seq == "[5C" || seq == "OC") {
                if (completion_lines_drawn > 0) { clear_completion_menu(out, completion_lines_drawn); completion_lines_drawn = 0; }
                if (!current_suggestion.empty() &&
                    cursor_pos == static_cast<int>(current_line.size())) {
                    // Accept next word
                    std::string new_line = AutosuggestEngine::accept_next_word(current_line, current_suggestion);
                    if (new_line != current_line) {
                        current_line = new_line;
                        original_line = current_line;
                        cursor_pos = static_cast<int>(current_line.size());
                        update_suggestion();
                        refresh_prompt();
                        continue;
                    }
                }
                // Otherwise move cursor right word-by-word
                if (cursor_pos < static_cast<int>(current_line.size())) {
                    // Skip non-spaces then spaces
                    while (cursor_pos < static_cast<int>(current_line.size()) && current_line[cursor_pos] != ' ')
                        cursor_pos++;
                    while (cursor_pos < static_cast<int>(current_line.size()) && current_line[cursor_pos] == ' ')
                        cursor_pos++;
                    refresh_prompt();
                }
                continue;
            }

            // Ctrl+Left Arrow: move word left
            if (seq == "[1;5D" || seq == "[5D" || seq == "OD") {
                if (completion_lines_drawn > 0) { clear_completion_menu(out, completion_lines_drawn); completion_lines_drawn = 0; }
                if (cursor_pos > 0) {
                    while (cursor_pos > 0 && current_line[cursor_pos - 1] == ' ') cursor_pos--;
                    while (cursor_pos > 0 && current_line[cursor_pos - 1] != ' ') cursor_pos--;
                    refresh_prompt();
                }
                continue;
            }

            // ─── Up Arrow: History backward ──────────────────────────────
            if (seq == "[A" || seq == "OA") {
                if (completion_lines_drawn > 0) { clear_completion_menu(out, completion_lines_drawn); completion_lines_drawn = 0; }
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
                    current_suggestion = "";
                    refresh_prompt();
                }
                continue;
            }

            // ─── Down Arrow: History forward ─────────────────────────────
            if (seq == "[B" || seq == "OB") {
                if (completion_lines_drawn > 0) { clear_completion_menu(out, completion_lines_drawn); completion_lines_drawn = 0; }
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
                    current_suggestion = "";
                    refresh_prompt();
                } else if (history_idx + 1 == static_cast<int>(history.size())) {
                    history_idx++;
                    if (preview_lines_drawn > 0) {
                        clear_history_preview(out, preview_lines_drawn);
                        preview_lines_drawn = 0;
                    }
                    current_line = original_line;
                    cursor_pos = static_cast<int>(current_line.size());
                    update_suggestion();
                    refresh_prompt();
                }
                continue;
            }

            // ─── Right Arrow ─────────────────────────────────────────────
            if (seq == "[C") {
                if (completion_lines_drawn > 0) { clear_completion_menu(out, completion_lines_drawn); completion_lines_drawn = 0; }
                if (!current_suggestion.empty() &&
                    cursor_pos == static_cast<int>(current_line.size())) {
                    // Accept the full suggestion
                    current_line = current_suggestion;
                    original_line = current_line;
                    cursor_pos = static_cast<int>(current_line.size());
                    current_suggestion = "";
                    update_suggestion();
                    refresh_prompt();
                } else if (cursor_pos < static_cast<int>(current_line.size())) {
                    cursor_pos++;
                    refresh_prompt();
                }
                continue;
            }

            // ─── Left Arrow ──────────────────────────────────────────────
            if (seq == "[D") {
                if (completion_lines_drawn > 0) { clear_completion_menu(out, completion_lines_drawn); completion_lines_drawn = 0; }
                if (cursor_pos > 0) {
                    cursor_pos--;
                    refresh_prompt();
                }
                continue;
            }

            // ─── Home ────────────────────────────────────────────────────
            if (seq == "[H" || seq == "OH" || seq == "[1~" || seq == "[7~") {
                if (completion_lines_drawn > 0) { clear_completion_menu(out, completion_lines_drawn); completion_lines_drawn = 0; }
                cursor_pos = 0;
                refresh_prompt();
                continue;
            }

            // ─── End: Accept suggestion or jump to end ───────────────────
            if (seq == "[F" || seq == "OF" || seq == "[4~" || seq == "[8~") {
                if (completion_lines_drawn > 0) { clear_completion_menu(out, completion_lines_drawn); completion_lines_drawn = 0; }
                if (!current_suggestion.empty() &&
                    cursor_pos == static_cast<int>(current_line.size())) {
                    // Accept the full suggestion
                    current_line = current_suggestion;
                    original_line = current_line;
                    cursor_pos = static_cast<int>(current_line.size());
                    current_suggestion = "";
                    update_suggestion();
                    refresh_prompt();
                } else {
                    cursor_pos = static_cast<int>(current_line.size());
                    refresh_prompt();
                }
                continue;
            }

            // ─── Delete key ──────────────────────────────────────────────
            if (seq == "[3~") {
                if (completion_lines_drawn > 0) { clear_completion_menu(out, completion_lines_drawn); completion_lines_drawn = 0; }
                if (cursor_pos < static_cast<int>(current_line.size())) {
                    current_line.erase(cursor_pos, 1);
                    original_line = current_line;
                    update_suggestion();
                    refresh_prompt();
                }
                continue;
            }

            continue;
        }

        // ── Normal printable characters ───────────────────────────────────
        if ((unsigned char)c >= 32) {
            if (preview_lines_drawn > 0) {
                clear_history_preview(out, preview_lines_drawn);
                preview_lines_drawn = 0;
                history_idx = static_cast<int>(history.size());
            }
            if (completion_lines_drawn > 0) {
                clear_completion_menu(out, completion_lines_drawn);
                completion_lines_drawn = 0;
            }

            // Handle multi-byte UTF-8 sequences
            std::string chars;
            chars += c;
            if ((unsigned char)c >= 0xC0) {
                int extra = 0;
                if ((unsigned char)c >= 0xF0) extra = 3;
                else if ((unsigned char)c >= 0xE0) extra = 2;
                else if ((unsigned char)c >= 0xC0) extra = 1;
                for (int i = 0; i < extra; ++i) {
                    if (has_pending_input(STDIN_FILENO, 10)) {
                        char nb = 0;
                        if (read(STDIN_FILENO, &nb, 1) > 0) chars += nb;
                    }
                }
            }

            current_line.insert(cursor_pos, chars);
            original_line = current_line;
            cursor_pos += static_cast<int>(chars.size());
            update_suggestion();
            refresh_prompt();
        }
    }

    return current_line;
}

} // namespace meridian::shell
