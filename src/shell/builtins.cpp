// meridian-shell / builtins.cpp
#include "builtins.hpp"
#include "executor.hpp"
#include "../core/terminal_image.hpp"
#include "../core/graphics/image_decoder.hpp"
#include "../ai/intent_engine.hpp"
#include "../ai/error_diagnostics.hpp"

#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <unistd.h>

extern char** environ;

namespace meridian::shell {

namespace {
const char* kBuiltinNames[] = {
    "cd", "pwd", "echo", "exit", "export", "unset", "env",
    "history", "jobs", "fg", "bg", "help", "type", "which", "clear", "alias",
    "pic", "ai"
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
    if (argv.size() >= 3 && argv[1] == "set") {
        std::string src = argv[2];
        const char* home = std::getenv("HOME");
        if (home) {
            if (src.rfind("~/", 0) == 0) {
                src = std::string(home) + src.substr(1);
            }
            std::ifstream test_f(src);
            if (!test_f.is_open()) {
                std::cerr << "\033[38;2;239;68;68m✘\033[0m File not found: " << src << "\n";
                std::cerr << "  Please specify an existing image file on your system.\n";
                return 1;
            }
            test_f.close();

            std::string cfg_dir = std::string(home) + "/.config/meridian";
            std::string cmd = "mkdir -p \"" + cfg_dir + "\" && cp -f \"" + src + "\" \"" + cfg_dir + "/artwork.jpg\" && cp -f \"" + src + "\" \"" + cfg_dir + "/artwork.png\"";
            int res = system(cmd.c_str());
            if (res == 0) {
                std::cout << "\033[38;2;34;197;94m✔\033[0m Default terminal artwork updated to: " << src << "\n";
                return 0;
            }
        }
        std::cerr << "Failed to set default artwork from: " << src << "\n";
        return 1;
    }

    if (argv.size() > 1 && (argv[1] == "--clear" || argv[1] == "clear")) {
        // Delete and clear current image from terminal canvas
        std::cout << "\033_Ga=d,d=a\033\\\n";
        return 0;
    }

    std::string filepath;
    int target_width = 200;
    int target_height = 0; // 0 = auto calculate from aspect ratio

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
        const char* env_art = std::getenv("MERIDIAN_ARTWORK");
        if (env_art && access(env_art, R_OK) == 0) filepath = env_art;
        else {
            const char* home = std::getenv("HOME");
            if (home) {
                std::string p1 = std::string(home) + "/.config/meridian/artwork.jpg";
                std::string p2 = std::string(home) + "/.config/meridian/artwork.png";
                if (access(p1.c_str(), R_OK) == 0) filepath = p1;
                else if (access(p2.c_str(), R_OK) == 0) filepath = p2;
            }
        }
        if (filepath.empty()) {
            if (access("resources/images/artwork.jpg", R_OK) == 0) filepath = "resources/images/artwork.jpg";
            else filepath = "resources/images/artwork.png";
        }
    }

    // Decode original image into 32-bit RGBA8888 pixels preserving original colors & sharp details
    auto decoded = graphics::ImageDecoder::decode_file(filepath);
    if (!decoded.is_valid()) {
        std::cerr << "meridian: image file not found or unsupported format: " << filepath << "\n";
        return 1;
    }

    const auto& frame = decoded.frame(0);
    int src_w = frame.width;
    int src_h = frame.height;

    // Calculate display dimensions preserving original aspect ratio
    int disp_w = target_width > 0 ? target_width : 200;
    int disp_h = target_height > 0 ? target_height : static_cast<int>(std::round(static_cast<float>(src_h * disp_w) / static_cast<float>(src_w)));
    if (disp_h <= 0) disp_h = 1;

    // Direct transmission of original image to GPU / Terminal Canvas via Kitty Graphics Protocol
    std::ifstream f(filepath, std::ios::binary);
    if (f.is_open()) {
        std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        if (!buffer.empty()) {
            static const char b64_tbl[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
            std::string b64;
            b64.reserve(((buffer.size() + 2) / 3) * 4);
            for (size_t k = 0; k < buffer.size(); k += 3) {
                uint32_t b = (buffer[k] << 16) | ((k + 1 < buffer.size() ? buffer[k + 1] : 0) << 8) | (k + 2 < buffer.size() ? buffer[k + 2] : 0);
                b64.push_back(b64_tbl[(b >> 18) & 0x3F]);
                b64.push_back(b64_tbl[(b >> 12) & 0x3F]);
                b64.push_back((k + 1 < buffer.size()) ? b64_tbl[(b >> 6) & 0x3F] : '=');
                b64.push_back((k + 2 < buffer.size()) ? b64_tbl[b & 0x3F] : '=');
            }
            // Clear prior image then display new full-color raster at top-left
            std::cout << "\033_Ga=d,d=a\033\\\033_Ga=T,f=100,t=d,x=30,y=30,c=" << (disp_w / 8) << ",r=" << (disp_h / 16) << ";" << b64 << "\033\\\n";
            return 0;
        }
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
        "  pic <image-file> render original full-color raster image directly on canvas\n"
        "                   Options: --clear, --width <W>, --height <H>, set <path>\n"
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
    if (name == "help") return builtin_help();
    if (name == "clear") { std::cout << "\x1b[2J\x1b[H"; return 0; }
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
