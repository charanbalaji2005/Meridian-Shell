// zephyr-config / session_state.cpp
#include "session_state.hpp"
#include "../core/config.hpp"

#include <cstdio>
#include <cstdlib>

namespace zephyr::config {

namespace {

constexpr int kMaxDepth = 16;
constexpr int kMaxChildren = 16;

std::string encode(const std::string& s) {
    std::string out;
    for (unsigned char c : s) {
        bool safe = (c > 0x20 && c < 0x7F) && c != '%' && c != '(' && c != ')' && c != ',' &&
                    c != '|' && c != '[' && c != ']' && c != '"';
        if (safe) {
            out.push_back(static_cast<char>(c));
        } else {
            char buf[4];
            std::snprintf(buf, sizeof(buf), "%%%02X", c);
            out += buf;
        }
    }
    return out;
}

int hexv(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

bool decode(const std::string& s, std::string& out) {
    out.clear();
    for (std::size_t i = 0; i < s.size(); ++i) {
        if (s[i] != '%') { out.push_back(s[i]); continue; }
        if (i + 2 >= s.size()) return false; // truncated %XX escape
        int hi = hexv(s[i + 1]), lo = hexv(s[i + 2]);
        if (hi < 0 || lo < 0) return false;
        out.push_back(static_cast<char>(hi * 16 + lo));
        i += 2;
    }
    return true;
}

struct Parser {
    const std::string& s;
    std::size_t pos = 0;

    bool eat(char c) {
        if (pos < s.size() && s[pos] == c) { ++pos; return true; }
        return false;
    }
    std::string until_any(const char* stops) {
        std::size_t start = pos;
        while (pos < s.size() && std::string(stops).find(s[pos]) == std::string::npos) ++pos;
        return s.substr(start, pos - start);
    }
    bool node(PaneLayout& out, int depth) {
        if (depth > kMaxDepth) return false;
        if (eat('P')) {
            if (!eat('(')) return false;
            std::string prof = until_any("|)");
            if (!eat('|')) return false;
            std::string cwd = until_any(")");
            if (!eat(')')) return false;
            out = PaneLayout{};
            return decode(prof, out.profile) && decode(cwd, out.cwd);
        }
        if (eat('S')) {
            out = PaneLayout{};
            out.is_split = true;
            if (eat('1')) out.side_by_side = true;
            else if (!eat('0')) return false;
            if (eat('[')) {
                while (!eat(']')) {
                    std::string num = until_any(",]");
                    if (num.empty() || num.size() > 6) return false;
                    for (char c : num) if (c < '0' || c > '9') return false;
                    out.sizes.push_back(std::atoi(num.c_str()));
                    eat(',');
                    if (out.sizes.size() > kMaxChildren) return false;
                }
            }
            if (!eat('(')) return false;
            do {
                if (out.children.size() >= kMaxChildren) return false;
                PaneLayout child;
                if (!node(child, depth + 1)) return false;
                out.children.push_back(std::move(child));
            } while (eat(','));
            if (!eat(')')) return false;
            if (out.children.size() < 2) return false;
            if (!out.sizes.empty() && out.sizes.size() != out.children.size()) out.sizes.clear();
            return true;
        }
        return false;
    }
};

} // namespace

int PaneLayout::leaf_count() const {
    if (!is_split) return 1;
    int n = 0;
    for (const auto& c : children) n += c.leaf_count();
    return n;
}

std::string serialize_layout(const PaneLayout& layout) {
    if (!layout.is_split) {
        return "P(" + encode(layout.profile) + "|" + encode(layout.cwd) + ")";
    }
    std::string out = layout.side_by_side ? "S1" : "S0";
    if (!layout.sizes.empty() && layout.sizes.size() == layout.children.size()) {
        out += "[";
        for (std::size_t i = 0; i < layout.sizes.size(); ++i) {
            if (i) out += ",";
            out += std::to_string(layout.sizes[i] < 0 ? 0 : layout.sizes[i]);
        }
        out += "]";
    }
    out += "(";
    for (std::size_t i = 0; i < layout.children.size(); ++i) {
        if (i) out += ",";
        out += serialize_layout(layout.children[i]);
    }
    return out + ")";
}

bool parse_layout(const std::string& text, PaneLayout& out) {
    Parser p{text};
    if (!p.node(out, 0)) return false;
    return p.pos == text.size();
}

bool SessionStore::load(SessionState& out) const {
    Config cfg(path_);
    if (!cfg.load()) return false;
    out = SessionState{};
    int count = std::atoi(cfg.get("session.tab_count", "0").c_str());
    for (int i = 0; i < count && i < 64; ++i) {
        std::string k = "session.tab." + std::to_string(i) + ".";
        TabLayout tab;
        if (!decode(cfg.get(k + "title"), tab.title)) tab.title.clear();
        if (!parse_layout(cfg.get(k + "layout"), tab.root)) continue; // skip corrupt tabs
        out.tabs.push_back(std::move(tab));
    }
    out.active_tab = std::atoi(cfg.get("session.active_tab", "0").c_str());
    if (out.active_tab < 0 || out.active_tab >= static_cast<int>(out.tabs.size())) out.active_tab = 0;
    return !out.tabs.empty();
}

bool SessionStore::save(const SessionState& state) const {
    Config cfg(path_);
    cfg.set("session.tab_count", std::to_string(state.tabs.size()));
    cfg.set("session.active_tab", std::to_string(state.active_tab));
    for (std::size_t i = 0; i < state.tabs.size(); ++i) {
        std::string k = "session.tab." + std::to_string(i) + ".";
        cfg.set(k + "title", encode(state.tabs[i].title));
        cfg.set(k + "layout", serialize_layout(state.tabs[i].root));
    }
    return cfg.save();
}

bool SessionStore::clear() const {
    return std::remove(path_.c_str()) == 0;
}

} // namespace zephyr::config
