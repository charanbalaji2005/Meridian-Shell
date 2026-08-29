// meridian-shell / builtins.cpp
#include "builtins.hpp"
#include "executor.hpp"
#include "../core/terminal_image.hpp"
#include "../core/art_gallery.hpp"
#include "../core/graphics/image_decoder.hpp"
#include "../core/graphics/pixel_art_renderer.hpp"
#include "../core/graphics/graphics_manager.hpp"
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
#include "../dev/platform_manager.hpp"
#include "../dev/github_integration.hpp"
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
#include <sys/stat.h>
#include <unistd.h>

extern char** environ;

namespace meridian::shell {

namespace {
const char* kBuiltinNames[] = {
    "cd", "pwd", "echo", "exit", "export", "unset", "env",
    "history", "jobs", "fg", "bg", "help", "type", "which", "clear", "alias",
    "pic", "ai", "palette", "search", "split", "zoom", "pane", "monitor", "gitintel", "files", "ssh-mgr", "plugins", "perf", "auth",
    "vscode", "update", "stats", "telemetry", "gh", "github"
};
} // namespace

bool is_builtin(const std::string& name) {
    for (auto* n : kBuiltinNames)
        if (name == n) return true;
    return false;
}

std::vector<std::string> get_builtin_names() {
    std::vector<std::string> list;
    for (auto* n : kBuiltinNames) {
        list.push_back(n);
    }
    return list;
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
    auto& gm = graphics::GraphicsManager::instance();

    // 1. pic --debug [filepath]
    if (argv.size() >= 2 && argv[1] == "--debug") {
        std::string target = (argv.size() >= 3) ? argv[2] : "resources/images/gallery/gojo_awakening.png";
        graphics::ImageDebugReport report;
        if (!gm.generate_debug_report(target, report)) {
            // Check alias candidates
            auto decoded = graphics::ImageDecoder::decode_file(target);
            if (!decoded.is_valid()) {
                std::cerr << "meridian: failed to decode image: " << target << "\n";
                return 1;
            }
            gm.generate_debug_report(target, report);
        }

        std::cout << "Image:\n"
                  << "  Format: " << report.format << "\n"
                  << "  Original: " << report.original_width << "x" << report.original_height << "\n"
                  << "  Decoded: " << report.decoded_format << "\n"
                  << "  Texture: " << report.texture_width << "x" << report.texture_height << "\n"
                  << "  Position: " << static_cast<int>(report.position_x) << "," << static_cast<int>(report.position_y) << "\n"
                  << "  Display: " << static_cast<int>(report.display_width) << "x" << static_cast<int>(report.display_height) << "\n"
                  << "  GPU: " << (report.gpu_enabled ? "enabled" : "disabled") << "\n";
        return 0;
    }

    // 2. pic --clear [id] / pic clear
    if (argv.size() > 1 && (argv[1] == "--clear" || argv[1] == "clear")) {
        if (argv.size() >= 3) {
            try {
                uint64_t id = std::stoull(argv[2]);
                gm.remove_image(id);
            } catch (...) {}
        } else {
            gm.clear_all_images();
        }
        std::cout << "\033_Ga=d,d=a\033\\\033]1337;File=inline=0:\007\n";
        return 0;
    }

    // 3. pic --list / pic list: Show active GPU image objects and gallery themes
    if (argv.size() > 1 && (argv[1] == "--list" || argv[1] == "list")) {
        auto active_images = gm.list_images();
        if (!active_images.empty()) {
            std::cout << "\033[1;36mActive GPU Image Layers (" << active_images.size() << "):\033[0m\n";
            for (const auto& img : active_images) {
                std::cout << "  \033[1;33m[ID " << img.id << "]\033[0m "
                          << "\033[1;37m" << img.source_path << "\033[0m "
                          << "(" << img.original_width << "x" << img.original_height << " " << img.format << ") "
                          << "pos: (" << img.x << "," << img.y << ") "
                          << "size: " << img.display_width << "x" << img.display_height << "\n";
            }
            return 0;
        }

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
        std::cout << "\033[1;36m└─────────────────────────────────────────────────────────────────────────────┘\033[0m\n";
        return 0;
    }

    // 4. pic add <file> [alias]
    if (argv.size() >= 3 && (argv[1] == "add" || argv[1] == "--add")) {
        std::string src = argv[2];
        std::string name = (argv.size() >= 4) ? argv[3] : "";
        if (name.empty()) {
            size_t slash = src.find_last_of("/\\");
            name = (slash != std::string::npos) ? src.substr(slash + 1) : src;
            size_t dot = name.find_last_of('.');
            if (dot != std::string::npos) name = name.substr(0, dot);
        }
        const char* home = std::getenv("HOME");
        std::string dest_dir = std::string(home ? home : ".") + "/.config/meridian/gallery";
        system(("mkdir -p " + dest_dir).c_str());
        std::string dest_file = dest_dir + "/" + name + ".png";

        auto decoded = graphics::ImageDecoder::decode_file(src);
        if (!decoded.is_valid()) {
            std::cerr << "\033[31mmeridian: could not load source image:\033[0m " << src << "\n";
            return 1;
        }
        std::string cmd = "cp -f \"" + src + "\" \"" + dest_file + "\"";
        system(cmd.c_str());
        std::cout << "\033[38;2;34;197;94m✔\033[0m Image successfully added to Meridian Gallery as '\033[1;33m" << name << "\033[0m'!\n"
                  << "  \033[0;37m• View directly:\033[0m      pic " << name << "\n"
                  << "  \033[0;37m• Set on startup:\033[0m     pic set " << name << "\n";
        return 0;
    }

    // 5. pic set <name|index|file>
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
        std::string hw_esc = core::TerminalImage::render_hardware_image_escape(choice, 60);
        if (!hw_esc.empty()) std::cout << hw_esc;
        return 0;
    }

    // 6. pic (with zero arguments) -> Show active theme raster
    if (argv.size() <= 1) {
        auto theme = core::ArtGallery::get_active_artwork(56, 22);
        std::cout << "\033[1;37mActive Theme:\033[0m \033[1;33m" << theme.title << "\033[0m\n";
        std::string hw_esc = core::TerminalImage::render_hardware_image_escape(theme.id, 60);
        if (!hw_esc.empty()) {
            std::cout << hw_esc;
            return 0;
        }
        return 0;
    }

    // 7. pic <filepath|name> [options] — NATIVE GPU HARDWARE RASTER IMAGE RENDERING
    std::string filepath;
    graphics::ImageObject img_cfg;
    img_cfg.placement = graphics::ImagePlacementType::CursorRelative;
    img_cfg.fit_mode = graphics::ImageFitMode::Contain;
    img_cfg.filter = graphics::ImageScaleFilter::Smooth;

    for (size_t i = 1; i < argv.size(); ++i) {
        if (argv[i] == "--fit" && i + 1 < argv.size()) {
            std::string fit = argv[++i];
            if (fit == "cover") img_cfg.fit_mode = graphics::ImageFitMode::Cover;
            else if (fit == "stretch") img_cfg.fit_mode = graphics::ImageFitMode::Stretch;
            else img_cfg.fit_mode = graphics::ImageFitMode::Contain;
        } else if (argv[i] == "--width" && i + 1 < argv.size()) {
            std::string w_str = argv[++i];
            if (!w_str.empty() && w_str.back() == '%') {
                w_str.pop_back();
                try { img_cfg.width_percentage = std::stof(w_str); } catch (...) {}
            } else {
                try { img_cfg.display_width = std::stof(w_str); } catch (...) {}
            }
        } else if (argv[i] == "--height" && i + 1 < argv.size()) {
            std::string h_str = argv[++i];
            if (!h_str.empty() && h_str.back() == '%') {
                h_str.pop_back();
                try { img_cfg.height_percentage = std::stof(h_str); } catch (...) {}
            } else {
                try { img_cfg.display_height = std::stof(h_str); } catch (...) {}
            }
        } else if (argv[i] == "--x" && i + 1 < argv.size()) {
            try {
                img_cfg.x = std::stof(argv[++i]);
                img_cfg.placement = graphics::ImagePlacementType::AbsolutePixels;
            } catch (...) {}
        } else if (argv[i] == "--y" && i + 1 < argv.size()) {
            try {
                img_cfg.y = std::stof(argv[++i]);
                img_cfg.placement = graphics::ImagePlacementType::AbsolutePixels;
            } catch (...) {}
        } else if (argv[i] == "--opacity" && i + 1 < argv.size()) {
            try { img_cfg.opacity = std::stof(argv[++i]); } catch (...) {}
        } else if (argv[i] == "--z-index" && i + 1 < argv.size()) {
            try { img_cfg.z_index = std::stoi(argv[++i]); } catch (...) {}
        } else if (argv[i][0] != '-') {
            if (argv[i][0] == '#' || argv[i].rfind("//", 0) == 0) {
                break; // Ignore inline shell comments
            }
            if (filepath.empty()) {
                filepath = argv[i];
            }
        }
    }

    if (filepath.empty()) {
        std::cerr << "meridian: please specify an image file to display (e.g. pic anime.png)\n";
        return 1;
    }

    // Hardware GPU Graphics check
    if (!gm.is_gpu_available()) {
        std::cerr << "Meridian: native graphics rendering is unavailable.\n";
        return 1;
    }

    // Register with native GraphicsManager
    std::string err;
    uint64_t image_id = gm.add_image_file(filepath, img_cfg, &err);
    if (image_id == 0 && !err.empty()) {
        std::cerr << err << "\n";
        return 1;
    }

    // Emit Photorealistic Native Hardware Raster Stream (iTerm2 OSC 1337 + Kitty Protocol)
    int target_cols = static_cast<int>(img_cfg.display_width > 0 ? (img_cfg.display_width / 9.0f) : 0);
    std::string hw_esc = core::TerminalImage::render_hardware_image_escape(filepath, target_cols);
    if (!hw_esc.empty()) {
        std::cout << hw_esc;
        return 0;
    }

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
    (void)ctx;
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
    auto& tree = workspace::get_session_pane_tree();
    if (dir == "v" || dir == "vertical" || dir == "-v") {
        uint32_t new_id = tree.split_pane(tree.active_pane_id(), workspace::SplitDirection::Vertical);
        std::cout << "\033[38;2;34;197;94m✔\033[0m Pane split vertically [ID: " << new_id
                  << " | Active: " << tree.active_pane_id()
                  << " | Total: " << tree.count_panes() << "] [Ctrl+Shift+D]\n";
    } else {
        uint32_t new_id = tree.split_pane(tree.active_pane_id(), workspace::SplitDirection::Horizontal);
        std::cout << "\033[38;2;34;197;94m✔\033[0m Pane split horizontally [ID: " << new_id
                  << " | Active: " << tree.active_pane_id()
                  << " | Total: " << tree.count_panes() << "] [Ctrl+Shift+E]\n";
    }
    return 0;
}

static int builtin_zoom() {
    auto& tree = workspace::get_session_pane_tree();
    tree.toggle_zoom();
    if (tree.is_zoomed()) {
        std::cout << "\033[38;2;245;158;11m⛶\033[0m Pane ZOOMED / Maximized [ID: " << tree.active_pane_id() << "] [Ctrl+Shift+Z]\n";
    } else {
        std::cout << "\033[38;2;59;130;246m⧉\033[0m Pane Zoom RESTORED / Unmaximized [Active ID: " << tree.active_pane_id()
                  << " | Total: " << tree.count_panes() << "] [Ctrl+Shift+Z]\n";
    }
    return 0;
}

static int builtin_pane(const std::vector<std::string>& argv) {
    auto& tree = workspace::get_session_pane_tree();
    if (argv.size() < 2 || argv[1] == "list" || argv[1] == "status") {
        std::cout << "┌─── Meridian Panes (Active ID: " << tree.active_pane_id()
                  << " | Total: " << tree.count_panes()
                  << " | Zoomed: " << (tree.is_zoomed() ? "Yes" : "No") << ") ───\n";
        auto items = tree.compute_layout(80, 24);
        for (const auto& item : items) {
            std::cout << "│ Pane " << item.pane_id << ": " << item.title
                      << " [" << item.bounds.width << "x" << item.bounds.height
                      << " at " << item.bounds.x << "," << item.bounds.y << "]"
                      << (item.is_focused ? " \033[1;32m(FOCUSED)\033[0m" : "") << "\n";
        }
        std::cout << "└─────────────────────────────────────────────────────────────\n";
        return 0;
    }

    std::string sub = argv[1];
    if (sub == "zoom" || sub == "z") {
        return builtin_zoom();
    }
    if (sub == "up" || sub == "k") {
        auto adj = tree.find_adjacent_pane(tree.active_pane_id(), workspace::NavigationDirection::Up);
        if (adj.has_value()) {
            tree.set_active_pane(adj.value());
            std::cout << "\033[38;2;0;229;255m▲\033[0m Focused pane UP [ID: " << adj.value() << "] [Alt+Up]\n";
        } else {
            std::cout << "\033[38;2;140;150;170mℹ No adjacent pane above current active pane (ID: " << tree.active_pane_id() << ")\033[0m\n";
        }
        return 0;
    }
    if (sub == "down" || sub == "j") {
        auto adj = tree.find_adjacent_pane(tree.active_pane_id(), workspace::NavigationDirection::Down);
        if (adj.has_value()) {
            tree.set_active_pane(adj.value());
            std::cout << "\033[38;2;0;229;255m▼\033[0m Focused pane DOWN [ID: " << adj.value() << "] [Alt+Down]\n";
        } else {
            std::cout << "\033[38;2;140;150;170mℹ No adjacent pane below current active pane (ID: " << tree.active_pane_id() << ")\033[0m\n";
        }
        return 0;
    }
    if (sub == "left" || sub == "h") {
        auto adj = tree.find_adjacent_pane(tree.active_pane_id(), workspace::NavigationDirection::Left);
        if (adj.has_value()) {
            tree.set_active_pane(adj.value());
            std::cout << "\033[38;2;0;229;255m◀\033[0m Focused pane LEFT [ID: " << adj.value() << "] [Alt+Left]\n";
        } else {
            std::cout << "\033[38;2;140;150;170mℹ No adjacent pane to the left of active pane (ID: " << tree.active_pane_id() << ")\033[0m\n";
        }
        return 0;
    }
    if (sub == "right" || sub == "l") {
        auto adj = tree.find_adjacent_pane(tree.active_pane_id(), workspace::NavigationDirection::Right);
        if (adj.has_value()) {
            tree.set_active_pane(adj.value());
            std::cout << "\033[38;2;0;229;255m▶\033[0m Focused pane RIGHT [ID: " << adj.value() << "] [Alt+Right]\n";
        } else {
            std::cout << "\033[38;2;140;150;170mℹ No adjacent pane to the right of active pane (ID: " << tree.active_pane_id() << ")\033[0m\n";
        }
        return 0;
    }
    if (sub == "close" || sub == "x") {
        if (tree.close_pane(tree.active_pane_id())) {
            std::cout << "\033[38;2;34;197;94m✔\033[0m Closed pane. Active pane is now " << tree.active_pane_id() << "\n";
        } else {
            std::cout << "Cannot close last remaining pane.\n";
        }
        return 0;
    }

    std::cout << "Usage: pane [list|zoom|up|down|left|right|close]\n";
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

static int builtin_auth(const std::vector<std::string>& argv) {
    std::string service = (argv.size() >= 2) ? argv[1] : "github";
    if (service != "github" && service != "gh") {
        std::cout << "Usage: auth github [--status | --token <token> | --user <name> <email>]\n";
        return 0;
    }

    std::string flag = (argv.size() >= 3) ? argv[2] : "";

    const char* home = std::getenv("HOME");
    std::string home_str = home ? home : "/tmp";
    std::string ssh_dir = home_str + "/.ssh";
    std::string key_file = ssh_dir + "/id_ed25519";
    std::string pub_key_file = key_file + ".pub";
    std::string config_file = ssh_dir + "/config";

    // 1. Direct Token Provisioning (e.g. auth github --token ghp_xxxx)
    if (flag == "--token" && argv.size() >= 4) {
        std::string token = argv[3];
        std::string user = (argv.size() >= 5) ? argv[4] : "git";

        int s = system("git config --global credential.helper store");
        (void)s;
        std::string cred_line = "https://" + user + ":" + token + "@github.com\n";
        std::ofstream cred_file(home_str + "/.git-credentials", std::ios::app);
        if (cred_file.is_open()) {
            cred_file << cred_line;
            chmod((home_str + "/.git-credentials").c_str(), 0600);
        }
        std::cout << "\033[1;32m✔ GitHub Personal Access Token saved to secure credential store!\033[0m\n";
        return 0;
    }

    // 2. Direct User Profile Provisioning (e.g. auth github --user username email@example.com)
    if (flag == "--user" && argv.size() >= 5) {
        std::string u = argv[3];
        std::string e = argv[4];
        int s1 = system(("git config --global user.name \"" + u + "\"").c_str());
        int s2 = system(("git config --global user.email \"" + e + "\"").c_str());
        (void)s1; (void)s2;
        std::cout << "\033[1;32m✔ Git identity configured: " << u << " <" << e << ">\033[0m\n";
        return 0;
    }

    std::cout << "\n\033[1;36m┌─── Meridian GitHub Authentication Manager ───────────────────────────────────┐\033[0m\n";

    // 1. Check & Ensure SSH Key exists
    bool has_key = (access(key_file.c_str(), F_OK) == 0 && access(pub_key_file.c_str(), F_OK) == 0);
    if (!has_key) {
        std::cout << "\033[1;33mℹ Generating new ED25519 SSH key for GitHub...\033[0m\n";
        int s1 = system(("mkdir -p " + ssh_dir + " && chmod 700 " + ssh_dir).c_str());
        int s2 = system(("ssh-keygen -t ed25519 -C \"meridian-user\" -f " + key_file + " -N \"\"").c_str());
        (void)s1; (void)s2;
    }

    // 2. Ensure Port 443 Config exists in ~/.ssh/config
    std::ifstream cfg_in(config_file);
    std::string cfg_contents;
    if (cfg_in.is_open()) {
        std::stringstream ss; ss << cfg_in.rdbuf();
        cfg_contents = ss.str();
    }
    if (cfg_contents.find("ssh.github.com") == std::string::npos) {
        std::ofstream cfg_out(config_file, std::ios::app);
        if (cfg_out.is_open()) {
            cfg_out << "\nHost github.com\n"
                    << "    Hostname ssh.github.com\n"
                    << "    Port 443\n"
                    << "    User git\n"
                    << "    IdentityFile " << key_file << "\n";
            chmod(config_file.c_str(), 0600);
        }
    }

    // 3. Read Public Key
    std::string pub_key;
    std::ifstream pub_in(pub_key_file);
    if (pub_in.is_open()) {
        std::getline(pub_in, pub_key);
    }

    // 4. Test Live GitHub Authentication
    std::cout << "\033[1;37m│\033[0m Checking connection to GitHub (Port 443)...\n";
    FILE* pipe = popen("ssh -o StrictHostKeyChecking=accept-new -T git@github.com 2>&1", "r");
    std::string auth_output;
    if (pipe) {
        char buf[256];
        while (fgets(buf, sizeof(buf), pipe) != nullptr) {
            auth_output += buf;
        }
        pclose(pipe);
    }

    bool is_authenticated = (auth_output.find("successfully authenticated") != std::string::npos);

    if (is_authenticated) {
        std::string gh_user = "user";
        auto hi_pos = auth_output.find("Hi ");
        if (hi_pos != std::string::npos) {
            auto ex_pos = auth_output.find('!', hi_pos + 3);
            if (ex_pos != std::string::npos) {
                gh_user = auth_output.substr(hi_pos + 3, ex_pos - (hi_pos + 3));
            }
        }

        std::cout << "\033[1;37m│\033[0m \033[1;32m● STATUS: AUTHENTICATED\033[0m as \033[1;33m@" << gh_user << "\033[0m\n"
                  << "\033[1;37m│\033[0m \033[38;2;34;197;94m✔ Git push / pull write access is fully operational!\033[0m\n";
    } else {
        std::cout << "\033[1;37m│\033[0m \033[1;31m○ STATUS: NOT YET LINKED\033[0m\n"
                  << "\033[1;37m│\033[0m Follow these 2 quick steps to link your GitHub account:\n"
                  << "\033[1;37m│\033[0m\n"
                  << "\033[1;37m│\033[0m \033[1;33mStep 1:\033[0m Copy your Meridian SSH Public Key below:\n"
                  << "\033[1;37m│\033[0m \033[1;36m" << pub_key << "\033[0m\n"
                  << "\033[1;37m│\033[0m\n"
                  << "\033[1;37m│\033[0m \033[1;33mStep 2:\033[0m Add it in your browser at:\n"
                  << "\033[1;37m│\033[0m 👉 \033[1;34mhttps://github.com/settings/ssh/new\033[0m\n";
    }

    std::cout << "\033[1;36m└──────────────────────────────────────────────────────────────────────────────┘\033[0m\n\n"
              << "\033[1;37mCommands:\033[0m\n"
              << "  \033[1;32mauth github\033[0m                          Check status & display SSH setup key\n"
              << "  \033[1;32mauth github --token <token>\033[0m          Authenticate via Personal Access Token\n"
              << "  \033[1;32mauth github --user <name> <email>\033[0m    Set global Git commit author\n";

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
        "  gitintel         inspect Git branch divergence and changes\n"
        "  files [dir]      view tree file explorer with git badges\n"
        "  ssh-mgr [alias]  manage & connect to SSH remote workspaces\n"
        "  auth [service]   manage GitHub / remote authentication & SSH keys\n"
        "  vscode [act]     manage VS Code terminal profiles & auto-detection\n"
        "  update           check and install latest Meridian releases\n"
        "  stats            view anonymous usage statistics and growth\n"
        "  telemetry        manage opt-in anonymous metrics and privacy\n"
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
    if (name == "zoom") return builtin_zoom();
    if (name == "pane") return builtin_pane(argv);
    if (name == "monitor") return builtin_monitor();
    if (name == "gitintel") return builtin_git();
    if (name == "files") return builtin_files(argv);
    if (name == "ssh-mgr") return builtin_ssh(argv);
    if (name == "auth") return builtin_auth(argv);
    if (name == "gh" || name == "github") return dev::GitHubIntegration::handle_gh_command(argv);
    if (name == "vscode") return dev::PlatformManager::handle_vscode(argv);
    if (name == "update" || name == "upgrade") return dev::PlatformManager::handle_update(argv);
    if (name == "stats" || name == "stat") return dev::PlatformManager::handle_stats(argv);
    if (name == "telemetry") return dev::PlatformManager::handle_telemetry(argv);
    if (name == "plugins") return builtin_plugins();
    if (name == "perf" || name == "performance") return builtin_perf();
    if (name == "help") return builtin_help();
    if (name == "clear") {
        std::cout << "\033[H\033[2J\033[3J" << std::flush;
        return 0;
    }
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
