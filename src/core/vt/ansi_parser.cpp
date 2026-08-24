// meridian-terminal-core / vt / ansi_parser.cpp
#include "ansi_parser.hpp"

namespace meridian::vt {

AnsiParser::AnsiParser(ScreenBuffer& screen) : screen_(screen) {}

void AnsiParser::feed(const std::string& data) { feed(data.data(), data.size()); }

void AnsiParser::feed(const char* data, std::size_t len) {
    for (std::size_t i = 0; i < len; ++i) {
        feed_byte(static_cast<uint8_t>(data[i]));
    }
}

void AnsiParser::feed_byte(uint8_t b) {
    switch (state_) {
        case State::Ground:            handle_ground(b); break;
        case State::Escape:            handle_escape(b); break;
        case State::CsiParam:          handle_csi(b); break;
        case State::OscString:         handle_osc(b); break;
        case State::OscDiscard:        handle_osc_discard(b); break;
        case State::Utf8Continuation:  handle_utf8_continuation(b); break;
    }
}

void AnsiParser::handle_ground(uint8_t b) {
    if (b == 0x1B) { state_ = State::Escape; return; }
    if (b == '\n') { screen_.newline(); return; }
    if (b == '\r') { screen_.carriage_return(); return; }
    if (b == '\b') { screen_.backspace(); return; }
    if (b == '\t') { screen_.tab(); return; }
    if (b < 0x20) { return; } // other C0 controls (incl. BEL outside OSC): ignore, don't crash

    if (b < 0x80) {
        screen_.put_codepoint(static_cast<char32_t>(b), current_attrs_);
        return;
    }
    if ((b & 0xE0) == 0xC0) { utf8_codepoint_ = b & 0x1F; utf8_remaining_ = 1; state_ = State::Utf8Continuation; return; }
    if ((b & 0xF0) == 0xE0) { utf8_codepoint_ = b & 0x0F; utf8_remaining_ = 2; state_ = State::Utf8Continuation; return; }
    if ((b & 0xF8) == 0xF0) { utf8_codepoint_ = b & 0x07; utf8_remaining_ = 3; state_ = State::Utf8Continuation; return; }
    // Invalid UTF-8 lead byte (stray continuation byte, 0xFF, etc.):
    // drop it silently instead of corrupting the grid or crashing.
}

void AnsiParser::handle_utf8_continuation(uint8_t b) {
    if ((b & 0xC0) == 0x80) {
        utf8_codepoint_ = (utf8_codepoint_ << 6) | (b & 0x3F);
        if (--utf8_remaining_ == 0) {
            screen_.put_codepoint(utf8_codepoint_, current_attrs_);
            state_ = State::Ground;
        }
        return;
    }
    // Malformed/truncated UTF-8 sequence: abandon it and re-process this
    // byte as if we were in Ground. This recurses at most once, so a
    // malformed stream can never wedge or crash the parser.
    utf8_remaining_ = 0;
    state_ = State::Ground;
    handle_ground(b);
}

void AnsiParser::handle_escape(uint8_t b) {
    if (b == '[') { reset_csi(); state_ = State::CsiParam; return; }
    if (b == ']') { reset_osc(); state_ = State::OscString; return; }
    if (b == '7') { screen_.save_cursor(); state_ = State::Ground; return; }
    if (b == '8') { screen_.restore_cursor(); state_ = State::Ground; return; }
    if (b == 'c') {
        screen_.erase_in_display(2);
        screen_.set_cursor(0, 0);
        current_attrs_ = Attributes{};
        state_ = State::Ground;
        return;
    }
    if (b == 'D') { screen_.newline(); state_ = State::Ground; return; }
    if (b == 'M') { screen_.move_cursor_rel(-1, 0); state_ = State::Ground; return; }
    if (b == 'E') { screen_.carriage_return(); screen_.newline(); state_ = State::Ground; return; }
    // Any other single-char escape (charset selection, '#'-sequences, ...)
    // is accepted and ignored. We always return to Ground so an
    // unsupported escape can never wedge the state machine.
    state_ = State::Ground;
}

void AnsiParser::handle_csi(uint8_t b) {
    if (b == 0x1B) { reset_csi(); state_ = State::Escape; return; }
    if (b == '?' && csi_params_.empty()) { csi_private_ = true; return; }
    if ((b >= '0' && b <= '9') || b == ';' || b == ':') {
        if (csi_params_.size() < kMaxCsiLen) csi_params_.push_back(static_cast<char>(b));
        return;
    }
    if (b >= 0x40 && b <= 0x7E) {
        dispatch_csi(b);
        state_ = State::Ground;
        return;
    }
    if (csi_params_.size() >= kMaxCsiLen) {
        // Runaway/malformed sequence that never reaches a valid final byte:
        // give up on it cleanly instead of growing forever.
        reset_csi();
        state_ = State::Ground;
        return;
    }
    // Intermediate bytes (0x20-0x2F): accepted, ignored, keep waiting.
}

void AnsiParser::handle_osc(uint8_t b) {
    auto commit_title = [&]() {
        auto semi = osc_buffer_.find(';');
        if (semi != std::string::npos) {
            std::string code = osc_buffer_.substr(0, semi);
            if (code == "0" || code == "2") window_title_ = osc_buffer_.substr(semi + 1);
        }
    };
    if (b == 0x07) { commit_title(); state_ = State::Ground; return; }
    if (b == 0x1B) {
        // Either the start of an ST (ESC \) terminator, or a fresh escape
        // aborting this OSC outright. Either way, close out the OSC and
        // hand the next byte to Escape-state handling.
        commit_title();
        state_ = State::Escape;
        return;
    }
    if (osc_buffer_.size() < kMaxOscLen) {
        osc_buffer_.push_back(static_cast<char>(b));
    } else {
        // Runaway OSC string: stop accumulating (bounded memory) but keep
        // discarding bytes — silently, not as screen text — until a real
        // terminator shows up. Dumping straight back to Ground here would
        // let the tail of a huge malformed OSC get rendered as literal
        // characters, which is worse than just waiting it out.
        reset_osc();
        state_ = State::OscDiscard;
    }
}

void AnsiParser::handle_osc_discard(uint8_t b) {
    if (b == 0x07) { state_ = State::Ground; return; }
    if (b == 0x1B) { state_ = State::Escape; return; }
    // Anything else: keep discarding.
}

std::vector<long> AnsiParser::parse_params() const {
    std::vector<long> out;
    if (csi_params_.empty()) return out;
    std::size_t start = 0;
    while (start <= csi_params_.size()) {
        std::size_t semi = csi_params_.find(';', start);
        std::string tok = csi_params_.substr(start, semi == std::string::npos ? std::string::npos : semi - start);
        if (tok.empty()) {
            out.push_back(-1); // empty param means "use default"
        } else {
            try {
                out.push_back(std::stol(tok));
            } catch (...) {
                out.push_back(-1);
            }
        }
        if (semi == std::string::npos) break;
        start = semi + 1;
    }
    return out;
}

void AnsiParser::dispatch_csi(uint8_t final_byte) {
    auto params = parse_params();
    auto get = [&](std::size_t idx, long def) -> long {
        if (idx < params.size() && params[idx] >= 0) return params[idx];
        return def;
    };

    if (csi_private_ && (final_byte == 'h' || final_byte == 'l')) {
        dispatch_decset(params, final_byte == 'h');
        return;
    }

    switch (final_byte) {
        case 'A': screen_.move_cursor_rel(-static_cast<int>(get(0, 1)), 0); break;
        case 'B': screen_.move_cursor_rel(static_cast<int>(get(0, 1)), 0); break;
        case 'C': screen_.move_cursor_rel(0, static_cast<int>(get(0, 1))); break;
        case 'D': screen_.move_cursor_rel(0, -static_cast<int>(get(0, 1))); break;
        case 'G': screen_.set_cursor(screen_.cursor_row(), static_cast<int>(get(0, 1)) - 1); break;
        case 'd': screen_.set_cursor(static_cast<int>(get(0, 1)) - 1, screen_.cursor_col()); break;
        case 'H':
        case 'f':
            screen_.set_cursor(static_cast<int>(get(0, 1)) - 1, static_cast<int>(get(1, 1)) - 1);
            break;
        case 'J': screen_.erase_in_display(static_cast<int>(get(0, 0))); break;
        case 'K': screen_.erase_in_line(static_cast<int>(get(0, 0))); break;
        case 'L': screen_.insert_lines(static_cast<int>(get(0, 1))); break;
        case 'M': screen_.delete_lines(static_cast<int>(get(0, 1))); break;
        case 'P': screen_.delete_chars(static_cast<int>(get(0, 1))); break;
        case '@': screen_.insert_chars(static_cast<int>(get(0, 1))); break;
        case 'S': screen_.scroll_up(static_cast<int>(get(0, 1))); break;
        case 'T': screen_.scroll_down(static_cast<int>(get(0, 1))); break;
        case 'r': screen_.set_scroll_region(static_cast<int>(get(0, 0)), static_cast<int>(get(1, 0))); break;
        case 'm': dispatch_sgr(params); break;
        case 's': screen_.save_cursor(); break;
        case 'u': screen_.restore_cursor(); break;
        default:
            // Unrecognized final byte: accepted, ignored, never fatal.
            break;
    }
}

void AnsiParser::dispatch_sgr(const std::vector<long>& params) {
    if (params.empty()) { current_attrs_ = Attributes{}; return; }
    auto get = [&](std::size_t idx) -> long { return (idx < params.size() && params[idx] >= 0) ? params[idx] : 0; };

    std::size_t i = 0;
    while (i < params.size()) {
        long code = params[i] < 0 ? 0 : params[i];
        if (code >= 30 && code <= 37) { current_attrs_.fg = Color::indexed(static_cast<uint8_t>(code - 30)); }
        else if (code >= 40 && code <= 47) { current_attrs_.bg = Color::indexed(static_cast<uint8_t>(code - 40)); }
        else if (code >= 90 && code <= 97) { current_attrs_.fg = Color::indexed(static_cast<uint8_t>(code - 90 + 8)); }
        else if (code >= 100 && code <= 107) { current_attrs_.bg = Color::indexed(static_cast<uint8_t>(code - 100 + 8)); }
        else if (code == 38 || code == 48) {
            if (i + 1 < params.size()) {
                long mode = get(i + 1);
                if (mode == 5 && i + 2 < params.size()) {
                    uint8_t idx = static_cast<uint8_t>(get(i + 2));
                    if (code == 38) current_attrs_.fg = Color::indexed(idx); else current_attrs_.bg = Color::indexed(idx);
                    i += 2;
                } else if (mode == 2 && i + 4 < params.size()) {
                    uint8_t r = static_cast<uint8_t>(get(i + 2));
                    uint8_t g = static_cast<uint8_t>(get(i + 3));
                    uint8_t b = static_cast<uint8_t>(get(i + 4));
                    if (code == 38) current_attrs_.fg = Color::rgb(r, g, b); else current_attrs_.bg = Color::rgb(r, g, b);
                    i += 4;
                } else {
                    i += 1;
                }
            }
        } else {
            switch (code) {
                case 0: current_attrs_ = Attributes{}; break;
                case 1: current_attrs_.bold = true; break;
                case 2: current_attrs_.dim = true; break;
                case 3: current_attrs_.italic = true; break;
                case 4: current_attrs_.underline = true; break;
                case 5: case 6: current_attrs_.blink = true; break;
                case 7: current_attrs_.reverse = true; break;
                case 8: current_attrs_.hidden = true; break;
                case 9: current_attrs_.strikethrough = true; break;
                case 21: current_attrs_.bold = false; break;
                case 22: current_attrs_.bold = false; current_attrs_.dim = false; break;
                case 23: current_attrs_.italic = false; break;
                case 24: current_attrs_.underline = false; break;
                case 25: current_attrs_.blink = false; break;
                case 27: current_attrs_.reverse = false; break;
                case 28: current_attrs_.hidden = false; break;
                case 29: current_attrs_.strikethrough = false; break;
                case 39: current_attrs_.fg = Color::default_color(); break;
                case 49: current_attrs_.bg = Color::default_color(); break;
                default: break; // unknown SGR code: ignore, never fatal
            }
        }
        ++i;
    }
}

void AnsiParser::dispatch_decset(const std::vector<long>& params, bool set) {
    for (long p : params) {
        if (p < 0) continue;
        switch (p) {
            case 1049:
                if (set) { screen_.save_cursor(); screen_.enter_alt_screen(); }
                else { screen_.exit_alt_screen(); screen_.restore_cursor(); }
                break;
            case 47:
            case 1047:
                if (set) screen_.enter_alt_screen(); else screen_.exit_alt_screen();
                break;
            default:
                // Cursor visibility (25), mouse reporting (1000/1002/1003/1006),
                // bracketed paste (2004), etc: accepted and ignored — they
                // don't affect the cell grid this milestone models.
                break;
        }
    }
}

void AnsiParser::reset_csi() { csi_params_.clear(); csi_private_ = false; }
void AnsiParser::reset_osc() { osc_buffer_.clear(); }

} // namespace meridian::vt
