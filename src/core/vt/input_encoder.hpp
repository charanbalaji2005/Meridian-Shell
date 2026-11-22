#pragma once
// zephyr-terminal-core / vt / input_encoder.hpp
//
// Translates keyboard input and clipboard pastes into the byte sequences a
// program running in the PTY expects (xterm conventions). Toolkit-free so
// the exact bytes are unit-tested; the Qt view only maps Qt key codes onto
// the Key enum below.

#include <string>

namespace zephyr::vt {

enum class Key {
    Up, Down, Right, Left, Home, End,
    Insert, Delete, PageUp, PageDown,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    Backspace, Enter, Tab, Escape,
};

struct KeyMods {
    bool shift = false;
    bool alt = false;
    bool ctrl = false;
    bool any() const { return shift || alt || ctrl; }
};

// Bytes for a non-text key. `application_cursor` is DECCKM (?1).
std::string encode_key(Key key, KeyMods mods, bool application_cursor);

// Bytes for typed text (UTF-8). Ctrl turns a single ASCII character into
// its C0 control code (Ctrl+A -> 0x01, Ctrl+Space -> NUL, Ctrl+/ -> 0x1F…);
// Alt prefixes ESC (the "meta sends escape" convention).
std::string encode_text(const std::string& utf8, KeyMods mods);

// Prepares clipboard text for the PTY:
//  * removes C0 control characters other than TAB/LF/CR, and DEL — this
//    includes ESC, so pasted text can never inject escape sequences or
//    terminate bracketed-paste mode early ("paste-jacking");
//  * normalises line endings to CR, as a real Enter key would send;
//  * wraps in ESC[200~ … ESC[201~ when the application enabled bracketed
//    paste, so shells insert it literally instead of executing lines.
std::string prepare_paste(const std::string& utf8, bool bracketed);

// True if pasting this text would submit more than one line (i.e. it
// contains a line break that is not just a single trailing one).
bool is_multiline_paste(const std::string& utf8);

} // namespace zephyr::vt
