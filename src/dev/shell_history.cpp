// zephyr-dev / shell_history.cpp
#include "shell_history.hpp"

#include <sstream>
#include <unordered_set>

namespace zephyr::dev {

namespace {

std::vector<std::string> split_lines(const std::string& content) {
    std::vector<std::string> lines;
    std::string line;
    std::istringstream in(content);
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        lines.push_back(line);
    }
    return lines;
}

// zsh stores bytes 0x83..0x9F (and NUL) as 0x83 followed by (byte ^ 32).
std::string zsh_unmetafy(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (std::size_t i = 0; i < s.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        if (c == 0x83 && i + 1 < s.size()) {
            out.push_back(static_cast<char>(static_cast<unsigned char>(s[++i]) ^ 32));
        } else {
            out.push_back(static_cast<char>(c));
        }
    }
    return out;
}

bool is_blank(const std::string& s) { return s.find_first_not_of(" \t") == std::string::npos; }

} // namespace

std::vector<std::string> parse_bash_history(const std::string& content) {
    std::vector<std::string> out;
    for (const auto& line : split_lines(content)) {
        // "#1700000000" lines are HISTTIMEFORMAT timestamps.
        if (line.size() > 1 && line[0] == '#' && line.find_first_not_of("0123456789", 1) == std::string::npos) continue;
        if (!is_blank(line)) out.push_back(line);
    }
    return out;
}

std::vector<std::string> parse_zsh_history(const std::string& raw) {
    std::vector<std::string> out;
    std::string pending;
    bool continuing = false;
    for (auto line : split_lines(zsh_unmetafy(raw))) {
        if (!continuing && line.rfind(": ", 0) == 0) {
            // EXTENDED_HISTORY: ": <start>:<elapsed>;<command>"
            auto semi = line.find(';');
            if (semi != std::string::npos) line = line.substr(semi + 1);
        }
        if (!line.empty() && line.back() == '\\') {
            line.pop_back();
            pending += line + "\n";
            continuing = true;
            continue;
        }
        pending += line;
        continuing = false;
        if (!is_blank(pending)) out.push_back(pending);
        pending.clear();
    }
    if (!is_blank(pending)) out.push_back(pending);
    return out;
}

std::vector<std::string> parse_fish_history(const std::string& content) {
    std::vector<std::string> out;
    const std::string prefix = "- cmd: ";
    for (const auto& line : split_lines(content)) {
        if (line.rfind(prefix, 0) != 0) continue;
        std::string cmd;
        const std::string body = line.substr(prefix.size());
        for (std::size_t i = 0; i < body.size(); ++i) {
            if (body[i] == '\\' && i + 1 < body.size()) {
                char n = body[++i];
                cmd.push_back(n == 'n' ? '\n' : n);
            } else {
                cmd.push_back(body[i]);
            }
        }
        if (!is_blank(cmd)) out.push_back(cmd);
    }
    return out;
}

std::vector<std::string> dedupe_recent_first(const std::vector<std::string>& oldest_first, std::size_t limit) {
    std::vector<std::string> out;
    std::unordered_set<std::string> seen;
    for (auto it = oldest_first.rbegin(); it != oldest_first.rend() && out.size() < limit; ++it) {
        if (seen.insert(*it).second) out.push_back(*it);
    }
    return out;
}

std::string shell_quote(const std::string& s) {
    if (!s.empty() && s.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_-./=:@%+,") ==
                          std::string::npos) {
        return s;
    }
    std::string out = "'";
    for (char c : s) {
        if (c == '\'') out += "'\\''";
        else out.push_back(c);
    }
    return out + "'";
}

} // namespace zephyr::dev
