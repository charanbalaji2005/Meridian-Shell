#include "shell.hpp"
#include "line_editor.hpp"
#include "../core/terminal_image.hpp"
#include "../core/art_gallery.hpp"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <pwd.h>
#include <sstream>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <unistd.h>

namespace meridian::shell {

namespace {

void render_reference_layout_header(std::ostream& out) {
    // 1. Detect dynamic Linux system metadata
    char hostname[256] = "linux";
    gethostname(hostname, sizeof(hostname));
    std::string user = "user";
    struct passwd* pw = getpwuid(geteuid());
    if (pw && pw->pw_name) {
        user = pw->pw_name;
    }

    struct utsname uts{};
    std::string kernel = "Linux";
    if (uname(&uts) == 0) {
        kernel = std::string(uts.sysname) + " " + uts.release;
    }

    const char* xdg_desktop = std::getenv("XDG_CURRENT_DESKTOP");
    const char* session_type = std::getenv("XDG_SESSION_TYPE");
    std::string wm = (std::getenv("HYPRLAND_INSTANCE_SIGNATURE")) ? "Hyprland 0.56.1 (Wayland)" :
                     (xdg_desktop ? (std::string(xdg_desktop) + " (" + (session_type ? session_type : "Wayland") + ")") : "Hyprland 0.56.1 (Wayland)");

    const char* shell_env = std::getenv("SHELL");
    std::string shell_name = shell_env ? shell_env : "zsh";
    auto slash = shell_name.find_last_of('/');
    if (slash != std::string::npos) shell_name = shell_name.substr(slash + 1);
    std::string shell_str = shell_name + " 5.9";

    // Dynamic RAM calculation from /proc/meminfo
    std::string ram_str = "7.66 GiB / 15.25 GiB";
    std::ifstream meminfo("/proc/meminfo");
    if (meminfo.is_open()) {
        std::string line;
        uint64_t total_kb = 0, avail_kb = 0;
        while (std::getline(meminfo, line)) {
            if (line.rfind("MemTotal:", 0) == 0) {
                std::istringstream mss(line.substr(9));
                mss >> total_kb;
            } else if (line.rfind("MemAvailable:", 0) == 0) {
                std::istringstream mss(line.substr(13));
                mss >> avail_kb;
            }
        }
        if (total_kb > 0) {
            uint64_t used_kb = (total_kb > avail_kb) ? (total_kb - avail_kb) : 0;
            double used_gib = static_cast<double>(used_kb) / (1024.0 * 1024.0);
            double total_gib = static_cast<double>(total_kb) / (1024.0 * 1024.0);
            std::ostringstream ross;
            ross << std::fixed << std::setprecision(2) << used_gib << " GiB / " << total_gib << " GiB";
            ram_str = ross.str();
        }
    }

    // Dynamic Uptime from /proc/uptime
    std::string uptime_str = "1 hour, 11 mins";
    std::ifstream uptime_file("/proc/uptime");
    if (uptime_file.is_open()) {
        double seconds = 0;
        uptime_file >> seconds;
        uint64_t total_sec = static_cast<uint64_t>(seconds);
        uint64_t hours = (total_sec % 86400) / 3600;
        uint64_t mins = (total_sec % 3600) / 60;
        uptime_str = std::to_string(hours) + (hours == 1 ? " hour, " : " hours, ") + std::to_string(mins) + " mins";
    }

    // 2. Picture graphic: emit direct chunked Kitty GPU raster sequence
    auto theme = core::ArtGallery::get_active_artwork(56, 22);
    out << "\n" << theme.image.to_kitty_graphics_escape(30, 30, 28, 10);
    std::vector<std::string> art_lines = core::ArtGallery::render_artwork_lines(theme.image, 28, 10);

    // 3. System info lines (sleek Nerd Font glyphs matching reference theme)
    std::vector<std::string> sys_lines = {
        "\033[1;37m" + user + "@" + hostname + "\033[0m",
        "\033[38;2;120;130;150m───────\033[38;2;200;210;230m⭘\033[38;2;120;130;150m───────\033[0m",
        "\033[38;2;140;163;136m󰌽 \033[38;2;170;180;200m→ \033[38;2;220;230;245m" + kernel + "\033[0m",
        "\033[38;2;110;165;185m󰨇 \033[38;2;170;180;200m→ \033[38;2;220;230;245m" + wm + "\033[0m",
        "\033[38;2;155;135;175m󰞷 \033[38;2;170;180;200m→ \033[38;2;220;230;245m" + shell_str + "\033[0m",
        "\033[38;2;90;175;170m \033[38;2;170;180;200m→ \033[38;2;220;230;245mmeridian 2.0\033[0m",
        "\033[38;2;215;185;135m󰘚 \033[38;2;170;180;200m→ \033[38;2;220;230;245m" + ram_str + "\033[0m",
        "\033[38;2;205;135;145m󱑂 \033[38;2;170;180;200m→ \033[38;2;220;230;245m" + uptime_str + "\033[0m",
        "\033[38;2;120;130;150m───────\033[38;2;200;210;230m⭘\033[38;2;120;130;150m───────\033[0m",
        "\033[38;2;45;106;116m● \033[38;2;78;135;144m● \033[38;2;125;111;141m● \033[38;2;168;91;107m● \033[38;2;201;95;78m● \033[38;2;217;129;87m● \033[38;2;235;196;122m● \033[38;2;243;224;181m● \033[38;2;140;163;136m●\033[0m"
    };

    // 4. Print Side-by-Side Header with 2 spaces left margin, 4 spaces clean gap
    size_t max_rows = std::max(art_lines.size(), sys_lines.size());
    for (size_t r = 0; r < max_rows; ++r) {
        out << "  "; // 2 spaces left margin
        if (r < art_lines.size()) {
            out << art_lines[r];
        } else {
            out << "                            ";
        }
        out << "    "; // 4 spaces clean gap
        if (r < sys_lines.size()) {
            out << sys_lines[r];
        }
        out << "\n";
    }
    out << "\n"; // Clean vertical space before Powerline badge
    out.flush();
}

} // namespace

Shell::Shell(bool interactive) : interactive_(interactive) {}

std::string Shell::prompt() const {
    char cwd[4096];
    if (getcwd(cwd, sizeof(cwd))) return "meridian:" + std::string(cwd) + "$ ";
    return "meridian$ ";
}

int Shell::run_command(const std::string& command, std::ostream& err) {
    std::string error;
    int status = executor_.run_line(command, &error);
    if (!error.empty()) { err << "meridian-shell: " << error << "\n"; return 1; }
    return status < 0 ? 1 : status;
}

int Shell::run_interactive(std::istream& in, std::ostream& out, std::ostream& err) {
    bool is_real_interactive = (&in == &std::cin && LineEditor::is_terminal_interactive());

    if (is_real_interactive) {
        out << "\033[2J\033[H"; // Clear screen & home cursor so no previous shell prompt is visible
        render_reference_layout_header(out);
    }

    while (true) {
        std::string line;
        char cwd[4096] = "";
        getcwd(cwd, sizeof(cwd));

        if (is_real_interactive) {
            std::string date_badge = LineEditor::build_date_badge(cwd);
            out << date_badge << "\n";
            out.flush();

            std::string powerline_p = LineEditor::build_powerline_prompt(cwd);
            line = LineEditor::read_line(in, out, powerline_p, executor_.history());
            if (line == "exit" && executor_.history().empty()) {
                return 0;
            }
        } else {
            if (interactive_) {
                out << prompt();
                out.flush();
            }
            if (!std::getline(in, line)) break;
        }

        // Handle clear builtin (wipe screen, image and scrollback completely)
        std::string trimmed = line;
        while (!trimmed.empty() && (trimmed.front() == ' ' || trimmed.front() == '\t' || trimmed.front() == '\r')) trimmed.erase(0, 1);
        while (!trimmed.empty() && (trimmed.back() == ' ' || trimmed.back() == '\t' || trimmed.back() == '\r')) trimmed.pop_back();

        if (trimmed == "clear" && is_real_interactive) {
            out << "\033[3J\033[2J\033[H\033_Ga=d,d=a\033\\";
            out.flush();
            executor_.push_history(line);
            continue;
        }

        std::string error;
        int status = executor_.run_line(line, &error);
        if (!error.empty()) err << "meridian-shell: " << error << "\n";
        else if (status >= 0 && !trimmed.empty()) executor_.push_history(line);

        if (executor_.exit_requested()) return executor_.exit_code();

        if (is_real_interactive && !trimmed.empty()) {
            out << "\n";
            out.flush();
        }
    }
    return 0;
}

} // namespace meridian::shell
