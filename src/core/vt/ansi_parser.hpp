#pragma once
// meridian-terminal-core / vt / ansi_parser.hpp
//
// A real state-machine parser for the ANSI/VT escape sequences a shell and
// its child programs actually emit (SGR colors/attributes, cursor motion,
// erase, alternate screen, scroll regions, OSC window title). It decodes
// UTF-8 as it goes and feeds decoded codepoints + SGR state into a
// ScreenBuffer. Malformed or truncated sequences are recovered from, never
// crash-worthy — see tests/test_ansi_parser.cpp for adversarial input cases.

#include "screen_buffer.hpp"
#include "types.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace meridian::vt {

class AnsiParser {
public:
    explicit AnsiParser(ScreenBuffer& screen);

    // Feed a raw byte chunk as read from the PTY. Safe to call with any
    // byte sequence, including one that splits a UTF-8 sequence or an
    // escape sequence across calls.
    void feed(const std::string& data);
    void feed(const char* data, std::size_t len);

    // Most recent OSC 0/2 (window title) request, if any.
    const std::string& window_title() const { return window_title_; }

    const Attributes& current_attrs() const { return current_attrs_; }

private:
    enum class State { Ground, Escape, CsiParam, OscString, OscDiscard, Utf8Continuation };

    void feed_byte(uint8_t b);
    void handle_ground(uint8_t b);
    void handle_escape(uint8_t b);
    void handle_csi(uint8_t b);
    void handle_osc(uint8_t b);
    void handle_osc_discard(uint8_t b);
    void handle_utf8_continuation(uint8_t b);

    void dispatch_csi(uint8_t final_byte);
    void dispatch_sgr(const std::vector<long>& params);
    void dispatch_decset(const std::vector<long>& params, bool set);

    std::vector<long> parse_params() const;
    void reset_csi();
    void reset_osc();

    ScreenBuffer& screen_;
    State state_ = State::Ground;

    std::string csi_params_;        // raw digits/semicolons/colons
    bool csi_private_ = false;      // '?' prefix seen (DECSET/DECRST family)

    std::string osc_buffer_;

    Attributes current_attrs_{};
    std::string window_title_;

    // UTF-8 decode state (persists across feed() calls)
    char32_t utf8_codepoint_ = 0;
    int utf8_remaining_ = 0;

    static constexpr std::size_t kMaxCsiLen = 128;
    static constexpr std::size_t kMaxOscLen = 8192;
};

} // namespace meridian::vt
