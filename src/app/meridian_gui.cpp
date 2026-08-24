#include "meridian_gui.hpp"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

namespace meridian::gui {

MeridianGui::MeridianGui() {
    tabs_ = {"1: main", "2: server", "3: agent"};
    active_tab_ = 0;
}

std::string MeridianGui::render_frame(int width, int height) {
    if (width < 60) width = 80;
    if (height < 20) height = 24;

    std::ostringstream ss;
    auto metrics = sys_monitor_.sample();
    auto git = dev::GitIntel::inspect_directory(".");

    // 1. Top Tab & Title Bar
    ss << "\033[1;37;44m";
    ss << " 🚀 MERIDIAN 2.0 ";
    for (size_t i = 0; i < tabs_.size(); ++i) {
        if (static_cast<int>(i) == active_tab_) {
            ss << "\033[1;30;47m [" << tabs_[i] << "] \033[1;37;44m";
        } else {
            ss << "  " << tabs_[i] << "  ";
        }
    }
    int used_header = 17 + 12 * static_cast<int>(tabs_.size());
    std::string sys_badge = " CPU: " + std::to_string(static_cast<int>(metrics.cpu_percent)) + "% | RAM: " + std::to_string(metrics.mem_used_bytes / (1024 * 1024)) + "MB ";
    int pad = width - used_header - static_cast<int>(sys_badge.size());
    if (pad > 0) {
        for (int p = 0; p < pad; ++p) ss << " ";
    }
    ss << sys_badge << "\033[0m\n";

    // 2. Main Body Split Panes
    int content_height = height - 4;
    int left_width = (width * 3) / 5;
    int right_width = width - left_width - 3;

    for (int r = 0; r < content_height; ++r) {
        ss << "\033[1;34m│\033[0m ";
        // Left main pane content
        if (r == 0) {
            ss << "\033[1;32m┌── Active Terminal Pane ──────────────────────────┐\033[0m";
            int pad_l = left_width - 50;
            for (int p = 0; p < pad_l; ++p) ss << " ";
        } else if (r == 1) {
            ss << "\033[0;36mmeridian:~/workspace$\033[0m npm run dev";
            int pad_l = left_width - 34;
            for (int p = 0; p < pad_l; ++p) ss << " ";
        } else if (r == 2) {
            ss << "\033[0;32m[ready]\033[0m Server listening on http://localhost:3000";
            int pad_l = left_width - 49;
            for (int p = 0; p < pad_l; ++p) ss << " ";
        } else if (r == 3) {
            ss << "\033[0;33m[ai-agent]\033[0m Intent engine active • GPU 144Hz pipeline";
            int pad_l = left_width - 51;
            for (int p = 0; p < pad_l; ++p) ss << " ";
        } else if (r == 5) {
            ss << "\033[1;36m┌── Command History Preview (↑/↓) ────────────────┐\033[0m";
            int pad_l = left_width - 50;
            for (int p = 0; p < pad_l; ++p) ss << " ";
        } else if (r == 6) {
            ss << "\033[1;32m│ ▶ \033[1;37;44m #24: git commit -m \"Meridian 2.0 release\" \033[0m\033[1;36m  │\033[0m";
            int pad_l = left_width - 49;
            for (int p = 0; p < pad_l; ++p) ss << " ";
        } else if (r == 7) {
            ss << "\033[1;36m│   #23: docker compose up -d                      │\033[0m";
            int pad_l = left_width - 50;
            for (int p = 0; p < pad_l; ++p) ss << " ";
        } else if (r == 8) {
            ss << "\033[1;36m└──────────────────────────────────────────────────┘\033[0m";
            int pad_l = left_width - 50;
            for (int p = 0; p < pad_l; ++p) ss << " ";
        } else {
            for (int p = 0; p < left_width; ++p) ss << " ";
        }

        // Splitter
        ss << " \033[1;34m│\033[0m ";

        // Right side panel (Telemetry & Git)
        if (r == 0) {
            ss << "\033[1;33m📊 System & Git Telemetry\033[0m";
        } else if (r == 1) {
            ss << "CPU Load:  " << static_cast<int>(metrics.cpu_percent) << "% [";
            int bars = static_cast<int>(metrics.cpu_percent) / 10;
            for (int b = 0; b < 10; ++b) ss << (b < bars ? "■" : " ");
            ss << "]";
        } else if (r == 2) {
            ss << "Memory:    " << (metrics.mem_used_bytes / (1024 * 1024)) << " / " << (metrics.mem_total_bytes / (1024 * 1024)) << " MB";
        } else if (r == 3) {
            ss << "Disk:      " << static_cast<int>(metrics.disk_percent) << "% used";
        } else if (r == 5) {
            ss << "\033[1;35m📁 File Explorer\033[0m";
        } else if (r == 6) {
            ss << " 📁 src/ (core, ai, dev, app)";
        } else if (r == 7) {
            ss << " 📁 packaging/ (rpm, deb, aur)";
        } else if (r == 8) {
            ss << " 📄 CMakeLists.txt & Makefile";
        } else if (r == 10 && git.is_git_repo) {
            ss << " " << git.branch_name << (git.is_clean ? " ✓ clean" : " ! changes");
        } else {
            for (int p = 0; p < right_width; ++p) ss << " ";
        }

        ss << "\033[1;34m│\033[0m\n";
    }

    // 3. Bottom Status Bar
    ss << "\033[1;30;47m";
    ss << " " << (git.is_git_repo ? (" " + git.branch_name) : "⚡ Local") << " │ ";
    ss << status_message_;
    int used_footer = 12 + static_cast<int>(status_message_.size());
    int pad_f = width - used_footer - 18;
    if (pad_f > 0) {
        for (int p = 0; p < pad_f; ++p) ss << " ";
    }
    ss << " GPL-3.0-or-later \033[0m\n";

    return ss.str();
}

int MeridianGui::run() {
    struct winsize ws;
    int w = 80, h = 24;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) {
        w = ws.ws_col;
        h = ws.ws_row;
    }

    std::cout << "\033[?1049h\033[2J\033[H"; // Enter alternate screen & clear
    std::cout << render_frame(w, h);
    std::cout.flush();

    // If running in automated non-interactive / test environment
    if (!isatty(STDIN_FILENO)) {
        return 0;
    }

    std::cout << "\n\033[1;33mPress [q] or [Enter] to return to shell...\033[0m\n";
    std::string line;
    std::getline(std::cin, line);
    std::cout << "\033[?1049l"; // Exit alternate screen
    return 0;
}

} // namespace meridian::gui
