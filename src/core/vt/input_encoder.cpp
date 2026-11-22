// zephyr-terminal-core / vt / input_encoder.cpp
#include "input_encoder.hpp"

namespace zephyr::vt {

namespace {

// xterm modifier parameter: 1 + (shift ? 1) + (alt ? 2) + (ctrl ? 4)
int modifier_param(KeyMods m) {
    return 1 + (m.shift ? 1 : 0) + (m.alt ? 2 : 0) + (m.ctrl ? 4 : 0);
}

// Keys of the form CSI <final> / SS3 <final> (cursor keys, Home/End, F1-F4).
std::string ss3_or_csi(char final_byte, KeyMods m, bool use_ss3) {
    if (m.any()) return "\x1b[1;" + std::to_string(modifier_param(m)) + final_byte;
    return use_ss3 ? std::string("\x1bO") + final_byte : std::string("\x1b[") + final_byte;
}

// Keys of the form CSI <n> ~ (Insert, Delete, PgUp/PgDn, F5-F12).
std::string tilde(int n, KeyMods m) {
    if (m.any()) return "\x1b[" + std::to_string(n) + ";" + std::to_string(modifier_param(m)) + "~";
    return "\x1b[" + std::to_string(n) + "~";
}

} // namespace

std::string encode_key(Key key, KeyMods m, bool app_cursor) {
    switch (key) {
        case Key::Up: return ss3_or_csi('A', m, app_cursor);
        case Key::Down: return ss3_or_csi('B', m, app_cursor);
        case Key::Right: return ss3_or_csi('C', m, app_cursor);
        case Key::Left: return ss3_or_csi('D', m, app_cursor);
        case Key::Home: return ss3_or_csi('H', m, app_cursor);
        case Key::End: return ss3_or_csi('F', m, app_cursor);
        case Key::Insert: return tilde(2, m);
        case Key::Delete: return tilde(3, m);
        case Key::PageUp: return tilde(5, m);
        case Key::PageDown: return tilde(6, m);
        case Key::F1: return ss3_or_csi('P', m, true);
        case Key::F2: return ss3_or_csi('Q', m, true);
        case Key::F3: return ss3_or_csi('R', m, true);
        case Key::F4: return ss3_or_csi('S', m, true);
        case Key::F5: return tilde(15, m);
        case Key::F6: return tilde(17, m);
        case Key::F7: return tilde(18, m);
        case Key::F8: return tilde(19, m);
        case Key::F9: return tilde(20, m);
        case Key::F10: return tilde(21, m);
        case Key::F11: return tilde(23, m);
        case Key::F12: return tilde(24, m);
        case Key::Backspace:
            if (m.ctrl) return "\x08";
            return m.alt ? "\x1b\x7f" : "\x7f";
        case Key::Enter: return m.alt ? "\x1b\r" : "\r";
        case Key::Tab: return m.shift ? "\x1b[Z" : (m.alt ? "\x1b\t" : "\t");
        case Key::Escape: return "\x1b";
    }
    return "";
}

std::string encode_text(const std::string& utf8, KeyMods m) {
    if (utf8.empty()) return "";
    std::string out = utf8;
    if (m.ctrl && utf8.size() == 1) {
        unsigned char c = static_cast<unsigned char>(utf8[0]);
        if (c >= 'a' && c <= 'z') out = std::string(1, static_cast<char>(c - 'a' + 1));
        else if (c >= '@' && c <= '_') out = std::string(1, static_cast<char>(c - '@')); // @ A-Z [ \ ] ^ _
        else if (c == ' ' || c == '2') out = std::string(1, '\0');
        else if (c == '3') out = "\x1b";
        else if (c == '4') out = "\x1c";
        else if (c == '5') out = "\x1d";
        else if (c == '6') out = "\x1e";
        else if (c == '7' || c == '/' || c == '-') out = "\x1f";
        else if (c == '8' || c == '?') out = "\x7f";
    }
    if (m.alt) out = "\x1b" + out;
    return out;
}

std::string prepare_paste(const std::string& utf8, bool bracketed) {
    std::string clean;
    clean.reserve(utf8.size());
    for (std::size_t i = 0; i < utf8.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(utf8[i]);
        if (c == '\r') {
            clean.push_back('\r');
            if (i + 1 < utf8.size() && utf8[i + 1] == '\n') ++i; // CRLF -> CR
            continue;
        }
        if (c == '\n') { clean.push_back('\r'); continue; }
        if (c == '\t') { clean.push_back('\t'); continue; }
        if (c < 0x20 || c == 0x7F) continue; // drop ESC and other controls
        clean.push_back(static_cast<char>(c));
    }
    if (bracketed) return "\x1b[200~" + clean + "\x1b[201~";
    return clean;
}

bool is_multiline_paste(const std::string& utf8) {
    std::size_t end = utf8.size();
    while (end > 0 && (utf8[end - 1] == '\n' || utf8[end - 1] == '\r')) --end;
    return utf8.find_first_of("\r\n") < end;
}

} // namespace zephyr::vt
