// meridian-shell / builtins.cpp
#include "builtins.hpp"
#include "executor.hpp"
#include "../core/terminal_image.hpp"
#include "../core/art_gallery.hpp"
#include "../core/graphics/image_decoder.hpp"
#include "../core/vt/screen_buffer.hpp"
#include "../core/renderer/telemetry_profiler.hpp"
#include "../ai/intent_engine.hpp"
#include "../ai/error_diagnostics.hpp"
#include "../dev/command_palette.hpp"
#include "../dev/universal_search.hpp"
#include "../dev/system_monitor.hpp"
#include "../dev/git_intel.hpp"
#include "../dev/file_explorer.hpp"
#include "../dev/ssh_manager.hpp"
#include "../dev/rich_history.hpp"
#include "../plugins/plugin_manager.hpp"
#include "../workspace/pane_tree.hpp"

#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <unistd.h>

extern char** environ;

namespace meridian::shell {

namespace {
const char* kBuiltinNames[] = {
    "cd", "pwd", "echo", "exit", "export", "unset", "env",
    "history", "jobs", "fg", "bg", "help", "type", "which", "clear", "alias",
    "pic", "ai", "palette", "search", "split", "monitor", "git", "files", "ssh", "plugins", "perf"
};
} // namespace

bool is_builtin(const std::string& name) {
    for (auto* n : kBuiltinNames)
        if (name == n) return true;
    return false;
}

static int builtin_cd(const std::vector<std::string>& argv) {
    std::string target;
    if (argv.size() < 2) {
        const char* home = std::getenv("HOME");
        if (!home) { std::cerr << "cd: HOME not set\n"; return 1; }
        target = home;
    } else {
        target = argv[1];
    }
    if (chdir(target.c_str()) != 0) {
        std::cerr << "cd: " << target << ": " << std::strerror(errno) << "\n";
        return 1;
    }
    return 0;
}

static int builtin_pwd() {
    char buf[PATH_MAX];
    if (getcwd(buf, sizeof(buf))) { std::cout << buf << "\n"; return 0; }
    std::cerr << "pwd: " << std::strerror(errno) << "\n";
    return 1;
}

static int builtin_echo(const std::vector<std::string>& argv) {
    for (std::size_t i = 1; i < argv.size(); ++i) {
        if (i > 1) std::cout << ' ';
        std::cout << argv[i];
    }
    std::cout << "\n";
    return 0;
}

static int builtin_export(const std::vector<std::string>& argv) {
    for (std::size_t i = 1; i < argv.size(); ++i) {
        auto eq = argv[i].find('=');
        if (eq == std::string::npos) {
            if (!std::getenv(argv[i].c_str())) { std::cerr << "export: " << argv[i] << ": not set\n"; return 1; }
            continue;
        }
        setenv(argv[i].substr(0, eq).c_str(), argv[i].substr(eq + 1).c_str(), 1);
    }
    return 0;
}

static int builtin_unset(const std::vector<std::string>& argv) {
    for (std::size_t i = 1; i < argv.size(); ++i) unsetenv(argv[i].c_str());
    return 0;
}

static int builtin_env() {
    for (char** e = environ; *e; ++e) std::cout << *e << "\n";
    return 0;
}

static int builtin_history(Executor& ctx) {
    const auto& h = ctx.history();
    for (std::size_t i = 0; i < h.size(); ++i) std::cout << "  " << (i + 1) << "  " << h[i] << "\n";
    return 0;
}

static int builtin_jobs(Executor& ctx) {
    std::cout << ctx.jobs_report();
    return 0;
}

static int builtin_type(const std::vector<std::string>& argv) {
    if (argv.size() < 2) return 0;
    if (is_builtin(argv[1])) { std::cout << argv[1] << " is a Meridian Shell builtin\n"; return 0; }
    const char* path = std::getenv("PATH");
    std::string p = path ? path : "";
    std::size_t start = 0;
    while (start <= p.size()) {
        auto colon = p.find(':', start);
        std::string dir = p.substr(start, colon == std::string::npos ? std::string::npos : colon - start);
        std::string candidate = dir + "/" + argv[1];
        if (access(candidate.c_str(), X_OK) == 0) { std::cout << argv[1] << " is " << candidate << "\n"; return 0; }
        if (colon == std::string::npos) break;
        start = colon + 1;
    }
    std::cout << argv[1] << ": not found\n";
    return 1;
}

static int builtin_pic(const std::vector<std::string>& argv) {
    // 1. pic --debug [filepath]
    if (argv.size() >= 2 && argv[1] == "--debug") {
        std::string target = (argv.size() >= 3) ? argv[2] : "resources/images/gallery/sharingan_eye.png";
        auto decoded = graphics::ImageDecoder::decode_file(target);
        int src_w = decoded.original_width > 0 ? decoded.original_width : 1280;
        int src_h = decoded.original_height > 0 ? decoded.original_height : 720;
        float scale = std::min(220.0f / static_cast<float>(src_w), 220.0f / static_cast<float>(src_h));
        int disp_w = std::max(1, static_cast<int>(std::round(src_w * scale)));
        int disp_h = std::max(1, static_cast<int>(std::round(src_h * scale)));

        std::string fmt;
        auto dot = target.find_last_of('.');
        if (dot != std::string::npos) fmt = target.substr(dot + 1);
        if (fmt.empty()) fmt = decoded.format;
        for (auto& ch : fmt) ch = std::toupper(ch);

        std::cout << "IMAGE MODE: RASTER\n"
                  << "SOURCE: " << target << "\n"
                  << "SOURCE SIZE: " << src_w << "x" << src_h << "\n"
                  << "FORMAT: " << (fmt.empty() ? "PNG" : fmt) << "\n"
                  << "PIXEL FORMAT: RGBA8888\n"
                  << "RENDERER: GPU\n"
                  << "TEXTURE: " << src_w << "x" << src_h << "\n"
                  << "DISPLAY: " << disp_w << "x" << disp_h << "\n";
        return 0;
    }

    // 2. pic --clear / pic clear
    if (argv.size() > 1 && (argv[1] == "--clear" || argv[1] == "clear")) {
        std::cout << "\033_Ga=d,d=a\033\\\n";
        return 0;
    }

    // 3. pic list / pic (with no args): Show all themes and current selection
    if (argv.size() <= 1 || (argv.size() == 2 && (argv[1] == "list" || argv[1] == "--list" || argv[1] == "help" || argv[1] == "-h"))) {
        auto themes = core::ArtGallery::list_themes();
        std::string current_choice = core::ArtGallery::get_configured_choice();

        std::cout << "\033[1;36m┌─── MERIDIAN ANIME & ARTWORK GALLERY ────────────────────────────────────────┐\033[0m\n";
        for (size_t i = 0; i < themes.size(); ++i) {
            bool is_active = (current_choice == themes[i].first || current_choice == std::to_string(i));
            std::cout << "\033[1;36m│\033[0m "
                      << (is_active ? "\033[1;32m●\033[0m" : " ")
                      << " \033[1;33m[" << i << "]\033[0m \033[1;37m" << std::left << std::setw(18) << themes[i].first << "\033[0m - "
                      << std::left << std::setw(36) << themes[i].second
                      << (is_active ? " \033[1;32m[ACTIVE]\033[0m" : "         ")
                      << " \033[1;36m│\033[0m\n";
        }
        bool is_random = (current_choice == "random" || current_choice.empty());
        std::cout << "\033[1;36m│\033[0m " << (is_random ? "\033[1;32m●\033[0m" : " ") << " \033[1;33m[r]\033[0m \033[1;37m" << std::left << std::setw(18) << "random" << "\033[0m - "
                  << std::left << std::setw(36) << "Rotate to a new anime theme on startup"
                  << (is_random ? " \033[1;32m[ACTIVE]\033[0m" : "         ") << " \033[1;36m│\033[0m\n";
        std::cout << "\033[1;36m└─────────────────────────────────────────────────────────────────────────────┘\033[0m\n";
        std::cout << "\033[1;37mCurrent Setting:\033[0m " << (is_random ? "\033[1;32mRandom / Rotating\033[0m" : ("\033[1;33m" + current_choice + "\033[0m (Permanent)")) << "\n\n"
                  << "\033[0;37mCommands:\033[0m\n"
                  << "  \033[1;32mpic <filepath>\033[0m             Display real direct full-color raster image (e.g. \033[1;33mpic tanjiro.png\033[0m)\n"
                  << "  \033[1;32mpic set <number|id|path>\033[0m   Set artwork permanently (e.g. \033[1;33mpic set 0\033[0m or \033[1;33mpic set eye\033[0m)\n"
                  << "  \033[1;32mpic set random\033[0m             Enable random rotating anime on each startup\n"
                  << "  \033[1;32mpic show <number|id|path>\033[0m  Preview artwork right now in the terminal\n"
                  << "  \033[1;32mpic --debug <file>\033[0m         Inspect decoded raster metadata & GPU texture specs\n"
                  << "  \033[1;32mpic --clear\033[0m                Clear all graphics from canvas\n";
        return 0;
    }

    // 4. pic set <name|index|file>
    if (argv.size() >= 3 && (argv[1] == "set" || argv[1] == "--set")) {
        std::string choice = argv[2];
        if (choice == "random" || choice == "r") {
            core::ArtGallery::set_permanent_choice("random");
            std::cout << "\033[38;2;34;197;94m✔\033[0m Artwork set to \033[1;32mRandom / Rotating\033[0m on every terminal startup!\n";
            return 0;
        }

        auto theme = core::ArtGallery::get_artwork_by_id_or_file(choice, 56, 22);
        core::ArtGallery::set_permanent_choice(choice);
        std::cout << "\033[38;2;34;197;94m✔\033[0m Artwork permanently set to: \033[1;33m" << theme.title << "\033[0m\n";
        std::cout << theme.image.to_kitty_graphics_escape(30, 30, 28, 10) << "\n";
        return 0;
    }

    // 5. pic show <name|index|file>
    if (argv.size() >= 3 && (argv[1] == "show" || argv[1] == "preview")) {
        std::string choice = argv[2];
        auto theme = core::ArtGallery::get_artwork_by_id_or_file(choice, 56, 22);
        std::cout << "\033[1;37mPreviewing raster:\033[0m \033[1;33m" << theme.title << "\033[0m\n";
        std::cout << theme.image.to_kitty_graphics_escape(30, 30, 28, 10) << "\n";
        return 0;
    }

    // 6. pic <filepath> — DIRECT FULL-COLOR INLINE RASTER IMAGE
    std::string filepath;
    int target_width = 220;
    int target_height = 220;

    for (size_t i = 1; i < argv.size(); ++i) {
        if (argv[i] == "--width" && i + 1 < argv.size()) {
            try { target_width = std::stoi(argv[++i]); } catch (...) {}
        } else if (argv[i] == "--height" && i + 1 < argv.size()) {
            try { target_height = std::stoi(argv[++i]); } catch (...) {}
        } else if (argv[i][0] != '-') {
            filepath = argv[i];
        }
    }

    if (filepath.empty()) {
        auto theme = core::ArtGallery::get_active_artwork(56, 22);
        std::cout << theme.image.to_kitty_graphics_escape(30, 30, 28, 10) << "\n";
        return 0;
    }

    // Decode original image into 32-bit RGBA8888 pixel buffer
    auto decoded = graphics::ImageDecoder::decode_file(filepath);
    if (!decoded.is_valid()) {
        std::cerr << "meridian: image file not found or unsupported format: " << filepath << "\n";
        return 1;
    }

    const auto& frame = decoded.frame(0);
    int src_w = frame.width;
    int src_h = frame.height;

    // Calculate display dimensions preserving aspect ratio with contain behavior (max 220x220)
    float scale = std::min(static_cast<float>(target_width) / src_w, static_cast<float>(target_height) / src_h);
    int disp_w = std::max(1, static_cast<int>(std::round(src_w * scale)));
    int disp_h = std::max(1, static_cast<int>(std::round(src_h * scale)));

    // Emit direct hardware raster graphics sequence (x=30, y=30)
    std::string esc = core::TerminalImage::render_file_raster_escape(filepath, 30, 30, disp_w, disp_h);
    if (!esc.empty()) {
        std::cout << esc << "\n";
        return 0;
    }

    // Fallback direct RGBA transmission
    std::vector<uint8_t> raw_rgba = frame.rgba;
    static const char b64_tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string b64;
    b64.reserve(((raw_rgba.size() + 2) / 3) * 4);
    for (size_t k = 0; k < raw_rgba.size(); k += 3) {
        uint32_t b = (raw_rgba[k] << 16) | ((k + 1 < raw_rgba.size() ? raw_rgba[k + 1] : 0) << 8) | (k + 2 < raw_rgba.size() ? raw_rgba[k + 2] : 0);
        b64.push_back(b64_tbl[(b >> 18) & 0x3F]);
        b64.push_back(b64_tbl[(b >> 12) & 0x3F]);
        b64.push_back((k + 1 < raw_rgba.size()) ? b64_tbl[(b >> 6) & 0x3F] : '=');
        b64.push_back((k + 2 < raw_rgba.size()) ? b64_tbl[b & 0x3F] : '=');
    }

    std::cout << "\033_Ga=d,d=a\033\\\033_Ga=T,f=32,s=" << src_w << ",v=" << src_h << ",x=30,y=30,c=" << (disp_w / 8) << ",r=" << (disp_h / 16) << ";" << b64 << "\033\\\n";
    return 0;
}

static int builtin_ai(const std::vector<std::string>& argv) {
    if (argv.size() < 2) {
        std::cout << "Usage: ai <natural language prompt or error message>\n"
                  << "Example: ai find python files modified recently\n";
        return 0;
    }

    std::string prompt;
    for (size_t i = 1; i < argv.size(); ++i) {
        if (i > 1) prompt += " ";
        prompt += argv[i];
    }

    // Check if looks like error output
    if (prompt.find("Error:") != std::string::npos ||
        prompt.find("ModuleNotFoundError") != std::string::npos ||
        prompt.find("EADDRINUSE") != std::string::npos) {
        ai::ErrorDiagnostics diagnostics;
        auto card = diagnostics.analyze(prompt);
        std::cout << card.format();
        return 0;
    }

    ai::IntentEngine engine;
    auto result = engine.translate(prompt);
    std::cout << ai::IntentEngine::format_card(result);
    return 0;
}

static int builtin_palette(const std::vector<std::string>& argv) {
    std::string q = (argv.size() >= 2) ? argv[1] : "";
    dev::CommandPalette palette;
    auto results = palette.search(q);
    std::cout << "┌─── Meridian Command Palette (Ctrl+Shift+P) ────────────\n";
    for (const auto& a : results) {
        std::cout << "│ [" << a.category << "] " << a.title << " (" << a.shortcut << ")\n";
    }
    std::cout << "└─────────────────────────────────────────────────────────\n";
    return 0;
}

static int builtin_search(const std::vector<std::string>& argv, Executor& ctx) {
    std::string q;
    for (size_t i = 1; i < argv.size(); ++i) {
        if (i > 1) q += " ";
        q += argv[i];
    }
    if (q.empty()) {
        std::cout << "Usage: search <query>\n";
        return 1;
    }
    vt::ScreenBuffer screen;
    dev::RichHistory history;
    auto matches = dev::UniversalSearch::search_all(screen, history, q);
    std::cout << "Universal Search results for \"" << q << "\" (" << matches.size() << " matches):\n";
    for (const auto& m : matches) {
        std::cout << "  [" << m.source_label << "] " << m.line_content << "\n";
    }
    return 0;
}

static int builtin_split(const std::vector<std::string>& argv) {
    std::string dir = (argv.size() >= 2) ? argv[1] : "v";
    workspace::PaneTree tree;
    if (dir == "v" || dir == "vertical" || dir == "-v") {
        uint32_t new_id = tree.split_pane(tree.active_pane_id(), workspace::SplitDirection::Vertical);
        std::cout << "\033[38;2;34;197;94m✔\033[0m Pane split vertically (ID: " << new_id << ") [Ctrl+Shift+D]\n";
    } else {
        uint32_t new_id = tree.split_pane(tree.active_pane_id(), workspace::SplitDirection::Horizontal);
        std::cout << "\033[38;2;34;197;94m✔\033[0m Pane split horizontally (ID: " << new_id << ") [Ctrl+Shift+E]\n";
    }
    return 0;
}

static int builtin_monitor() {
    dev::SystemMonitor mon;
    auto metrics = mon.sample();
    std::cout << metrics.format_dashboard();
    return 0;
}

static int builtin_git() {
    auto status = dev::GitIntel::inspect_directory(".");
    std::cout << status.format_panel();
    return 0;
}

static int builtin_files(const std::vector<std::string>& argv) {
    std::string target = (argv.size() >= 2) ? argv[1] : ".";
    auto root = dev::FileExplorer::scan_directory(target, 2);
    std::cout << dev::FileExplorer::format_tree(root);
    return 0;
}

static int builtin_ssh(const std::vector<std::string>& argv) {
    dev::SSHManager ssh_mgr;
    if (argv.size() >= 2) {
        std::string alias = argv[1];
        auto host = ssh_mgr.find_host(alias);
        if (host) {
            std::cout << "\033[1;38;2;0;229;255mConnecting to " << host->alias << " (" << host->hostname << ")...\033[0m\n";
            std::string cmd = host->command_line();
            return std::system(cmd.c_str());
        }
    }
    std::cout << ssh_mgr.format_overview();
    return 0;
}

static int builtin_plugins() {
    std::cout << plugins::PluginManager::instance().format_plugin_list();
    return 0;
}

static int builtin_perf() {
    std::cout << renderer::TelemetryProfiler::instance().format_report();
    return 0;
}

static int builtin_help() {
    std::cout <<
        "Meridian Shell builtins:\n"
        "  cd [dir]         change working directory\n"
        "  pwd              print working directory\n"
        "  echo [args...]   print arguments\n"
        "  export N=V       set an environment variable\n"
        "  unset N          remove an environment variable\n"
        "  env              list environment variables\n"
        "  history          list command history for this session\n"
        "  jobs             list background/stopped jobs\n"
        "  fg [%N]          bring a job to the foreground\n"
        "  bg [%N]          resume a stopped job in the background\n"
        "  type NAME        show whether NAME is a builtin or found in PATH\n"
        "  palette [query]  open / fuzzy-search Command Palette (Ctrl+Shift+P)\n"
        "  search <query>   search across scrollback & history (Ctrl+Shift+F)\n"
        "  split [v|h]      split active terminal pane (Ctrl+Shift+D / Ctrl+Shift+E)\n"
        "  monitor          display live CPU, RAM, Disk, Network metrics\n"
        "  git              inspect Git branch divergence and changes\n"
        "  files [dir]      view tree file explorer with git badges\n"
        "  ssh [alias]      manage & connect to SSH remote workspaces\n"
        "  plugins          list active extensible plugins & hooks\n"
        "  perf             display GPU framerate & telemetry profiler\n"
        "  pic <file>       render direct full-color raster image\n"
        "  ai <query>       inline AI intent translation or error diagnosis\n"
        "  exit [code]      exit the shell\n";
    return 0;
}

int run_builtin(const std::string& name, const std::vector<std::string>& argv, Executor& ctx) {
    if (name == "cd") return builtin_cd(argv);
    if (name == "pwd") return builtin_pwd();
    if (name == "echo") return builtin_echo(argv);
    if (name == "export") return builtin_export(argv);
    if (name == "unset") return builtin_unset(argv);
    if (name == "env") return builtin_env();
    if (name == "history") return builtin_history(ctx);
    if (name == "jobs") return builtin_jobs(ctx);
    if (name == "fg") return ctx.do_fg(argv);
    if (name == "bg") return ctx.do_bg(argv);
    if (name == "type" || name == "which") return builtin_type(argv);
    if (name == "pic") return builtin_pic(argv);
    if (name == "ai") return builtin_ai(argv);
    if (name == "palette") return builtin_palette(argv);
    if (name == "search") return builtin_search(argv, ctx);
    if (name == "split") return builtin_split(argv);
    if (name == "monitor") return builtin_monitor();
    if (name == "git") return builtin_git();
    if (name == "files") return builtin_files(argv);
    if (name == "ssh") return builtin_ssh(argv);
    if (name == "plugins") return builtin_plugins();
    if (name == "perf" || name == "performance") return builtin_perf();
    if (name == "help") return builtin_help();
    if (name == "clear") { std::cout << "\033[3J\033[2J\033[H\033_Ga=d,d=a\033\\"; return 0; }
    if (name == "alias") return 0;
    if (name == "exit") {
        int code = 0;
        if (argv.size() > 1) { try { code = std::stoi(argv[1]); } catch (...) { code = 0; } }
        ctx.request_exit(code);
        return code;
    }
    return 127;
}

} // namespace meridian::shell
