// zephyr-terminal-core / vt / screen_buffer.cpp
#include "screen_buffer.hpp"
#include <algorithm>

namespace zephyr::vt {

ScreenBuffer::ScreenBuffer(int rows, int cols, std::size_t scrollback_limit)
    : rows_(rows), cols_(cols), scrollback_limit_(scrollback_limit), scroll_bottom_(rows - 1) {
    primary_.assign(static_cast<std::size_t>(rows_), std::vector<Cell>(static_cast<std::size_t>(cols_)));
    alternate_.assign(static_cast<std::size_t>(rows_), std::vector<Cell>(static_cast<std::size_t>(cols_)));
}

std::vector<std::vector<Cell>>& ScreenBuffer::grid() {
    return alt_active_ ? alternate_ : primary_;
}
const std::vector<std::vector<Cell>>& ScreenBuffer::grid() const {
    return alt_active_ ? alternate_ : primary_;
}

std::vector<Cell>& ScreenBuffer::row_at(int row) {
    return grid()[static_cast<std::size_t>(row)];
}

void ScreenBuffer::clear_row(std::vector<Cell>& row, const Attributes& attrs) {
    for (auto& c : row) {
        c.codepoint = U' ';
        c.width = 1;
        c.attrs = attrs;
    }
}

void ScreenBuffer::resize(int rows, int cols) {
    if (rows <= 0 || cols <= 0) return;
    auto resize_one = [&](std::vector<std::vector<Cell>>& g) {
        g.resize(static_cast<std::size_t>(rows));
        for (auto& row : g) row.resize(static_cast<std::size_t>(cols));
    };
    resize_one(primary_);
    resize_one(alternate_);
    rows_ = rows;
    cols_ = cols;
    scroll_top_ = 0;
    scroll_bottom_ = rows_ - 1;
    cursor_row_ = std::min(cursor_row_, rows_ - 1);
    cursor_col_ = std::min(cursor_col_, cols_ - 1);
    pending_wrap_ = false;
}

void ScreenBuffer::set_cursor(int row, int col) {
    cursor_row_ = std::clamp(row, 0, rows_ - 1);
    cursor_col_ = std::clamp(col, 0, cols_ - 1);
    pending_wrap_ = false;
}

void ScreenBuffer::move_cursor_rel(int drow, int dcol) {
    set_cursor(cursor_row_ + drow, cursor_col_ + dcol);
}

int ScreenBuffer::effective_scroll_bottom() const {
    return scroll_bottom_;
}

void ScreenBuffer::scroll_up(int n) {
    if (n <= 0) return;
    auto& g = grid();
    for (int i = 0; i < n; ++i) {
        // Only rows that scroll off the very top of a full-screen region
        // (scroll_top_ == 0) feed the scrollback, and only on the primary
        // screen — alternate-screen apps (vim, htop...) manage their own
        // redraw and shouldn't pollute scrollback history.
        if (scroll_top_ == 0 && !alt_active_) {
            if (scrollback_limit_ > 0) {
                scrollback_.push_back(g[static_cast<std::size_t>(scroll_top_)]);
                if (scrollback_.size() > scrollback_limit_) {
                    scrollback_.pop_front();
                } else if (view_offset_ > 0) {
                    // Keep a scrolled-back viewport looking at the same content.
                    view_offset_ = std::min(view_offset_ + 1, static_cast<int>(scrollback_.size()));
                }
            }
        }
        for (int r = scroll_top_; r < scroll_bottom_; ++r) {
            g[static_cast<std::size_t>(r)] = g[static_cast<std::size_t>(r + 1)];
        }
        clear_row(g[static_cast<std::size_t>(scroll_bottom_)], Attributes{});
    }
}

void ScreenBuffer::scroll_down(int n) {
    if (n <= 0) return;
    auto& g = grid();
    for (int i = 0; i < n; ++i) {
        for (int r = scroll_bottom_; r > scroll_top_; --r) {
            g[static_cast<std::size_t>(r)] = g[static_cast<std::size_t>(r - 1)];
        }
        clear_row(g[static_cast<std::size_t>(scroll_top_)], Attributes{});
    }
}

void ScreenBuffer::newline() {
    pending_wrap_ = false;
    if (cursor_row_ == effective_scroll_bottom()) {
        scroll_up(1);
    } else if (cursor_row_ < rows_ - 1) {
        cursor_row_++;
    }
}

void ScreenBuffer::carriage_return() {
    cursor_col_ = 0;
    pending_wrap_ = false;
}

void ScreenBuffer::backspace() {
    if (cursor_col_ > 0) cursor_col_--;
    pending_wrap_ = false;
}

void ScreenBuffer::tab() {
    int next_stop = ((cursor_col_ / 8) + 1) * 8;
    cursor_col_ = std::min(next_stop, cols_ - 1);
    pending_wrap_ = false;
}

void ScreenBuffer::reverse_index() {
    pending_wrap_ = false;
    if (cursor_row_ == scroll_top_) {
        scroll_down(1);
    } else if (cursor_row_ > 0) {
        cursor_row_--;
    }
}

void ScreenBuffer::put_codepoint(char32_t cp, const Attributes& attrs) {
    int width = codepoint_width(cp);

    if (width == 0) {
        // Combining mark: merge visually onto the previous cell instead of
        // consuming a fresh column. If there's nowhere to attach it, drop it
        // rather than corrupting the grid.
        int col = cursor_col_ - 1;
        if (pending_wrap_) { col = cursor_col_; }
        if (col >= 0 && col < cols_) {
            // We don't maintain per-cell combining-mark chains in this
            // simplified model; keep the base character as-is. This is a
            // documented simplification (see docs/terminal-emulation.md).
        }
        return;
    }

    if (pending_wrap_) {
        carriage_return();
        newline();
    }

    if (width == 2 && cursor_col_ == cols_ - 1) {
        // Wide glyph doesn't fit in the last column: wrap first.
        carriage_return();
        newline();
    }

    auto& row = row_at(cursor_row_);
    row[static_cast<std::size_t>(cursor_col_)].codepoint = cp;
    row[static_cast<std::size_t>(cursor_col_)].width = static_cast<uint8_t>(width);
    row[static_cast<std::size_t>(cursor_col_)].attrs = attrs;

    if (width == 2 && cursor_col_ + 1 < cols_) {
        row[static_cast<std::size_t>(cursor_col_ + 1)].codepoint = 0;
        row[static_cast<std::size_t>(cursor_col_ + 1)].width = 0;
        row[static_cast<std::size_t>(cursor_col_ + 1)].attrs = attrs;
    }

    int advance = width;
    if (cursor_col_ + advance >= cols_) {
        cursor_col_ = cols_ - 1;
        pending_wrap_ = true;
    } else {
        cursor_col_ += advance;
    }
}

void ScreenBuffer::erase_in_line(int mode) {
    auto& row = row_at(cursor_row_);
    int start = 0, end = cols_ - 1;
    if (mode == 0) { start = cursor_col_; end = cols_ - 1; }
    else if (mode == 1) { start = 0; end = cursor_col_; }
    else { start = 0; end = cols_ - 1; }
    for (int c = start; c <= end; ++c) {
        row[static_cast<std::size_t>(c)] = Cell{};
    }
}

void ScreenBuffer::erase_in_display(int mode) {
    auto& g = grid();
    if (mode == 0) {
        erase_in_line(0);
        for (int r = cursor_row_ + 1; r < rows_; ++r) clear_row(g[static_cast<std::size_t>(r)], Attributes{});
    } else if (mode == 1) {
        erase_in_line(1);
        for (int r = 0; r < cursor_row_; ++r) clear_row(g[static_cast<std::size_t>(r)], Attributes{});
    } else {
        for (int r = 0; r < rows_; ++r) clear_row(g[static_cast<std::size_t>(r)], Attributes{});
    }
}

void ScreenBuffer::insert_lines(int n) {
    if (n <= 0) return;
    auto& g = grid();
    if (cursor_row_ < scroll_top_ || cursor_row_ > scroll_bottom_) return;
    for (int i = 0; i < n; ++i) {
        for (int r = scroll_bottom_; r > cursor_row_; --r) {
            g[static_cast<std::size_t>(r)] = g[static_cast<std::size_t>(r - 1)];
        }
        clear_row(g[static_cast<std::size_t>(cursor_row_)], Attributes{});
    }
}

void ScreenBuffer::delete_lines(int n) {
    if (n <= 0) return;
    auto& g = grid();
    if (cursor_row_ < scroll_top_ || cursor_row_ > scroll_bottom_) return;
    for (int i = 0; i < n; ++i) {
        for (int r = cursor_row_; r < scroll_bottom_; ++r) {
            g[static_cast<std::size_t>(r)] = g[static_cast<std::size_t>(r + 1)];
        }
        clear_row(g[static_cast<std::size_t>(scroll_bottom_)], Attributes{});
    }
}

void ScreenBuffer::delete_chars(int n) {
    if (n <= 0) return;
    auto& row = row_at(cursor_row_);
    for (int i = 0; i < n; ++i) {
        for (int c = cursor_col_; c < cols_ - 1; ++c) {
            row[static_cast<std::size_t>(c)] = row[static_cast<std::size_t>(c + 1)];
        }
        row[static_cast<std::size_t>(cols_ - 1)] = Cell{};
    }
}

void ScreenBuffer::insert_chars(int n) {
    if (n <= 0) return;
    auto& row = row_at(cursor_row_);
    for (int i = 0; i < n; ++i) {
        for (int c = cols_ - 1; c > cursor_col_; --c) {
            row[static_cast<std::size_t>(c)] = row[static_cast<std::size_t>(c - 1)];
        }
        row[static_cast<std::size_t>(cursor_col_)] = Cell{};
    }
}

void ScreenBuffer::erase_chars(int n) {
    if (n <= 0) return;
    auto& row = row_at(cursor_row_);
    int end = std::min(cols_, cursor_col_ + n);
    for (int c = cursor_col_; c < end; ++c) row[static_cast<std::size_t>(c)] = Cell{};
    pending_wrap_ = false;
}

void ScreenBuffer::enter_alt_screen() {
    if (alt_active_) return;
    alt_saved_cursor_row_ = cursor_row_;
    alt_saved_cursor_col_ = cursor_col_;
    alt_active_ = true;
    view_offset_ = 0;
    erase_in_display(2);
    cursor_row_ = 0;
    cursor_col_ = 0;
    pending_wrap_ = false;
}

void ScreenBuffer::exit_alt_screen() {
    if (!alt_active_) return;
    alt_active_ = false;
    set_cursor(alt_saved_cursor_row_, alt_saved_cursor_col_);
    pending_wrap_ = false;
}

void ScreenBuffer::set_scroll_region(int top_1based, int bottom_1based) {
    if (top_1based <= 0 || bottom_1based <= 0) {
        scroll_top_ = 0;
        scroll_bottom_ = rows_ - 1;
        return;
    }
    int top = std::clamp(top_1based - 1, 0, rows_ - 1);
    int bottom = std::clamp(bottom_1based - 1, 0, rows_ - 1);
    if (top >= bottom) { scroll_top_ = 0; scroll_bottom_ = rows_ - 1; return; }
    scroll_top_ = top;
    scroll_bottom_ = bottom;
}

void ScreenBuffer::save_cursor() {
    saved_cursor_row_ = cursor_row_;
    saved_cursor_col_ = cursor_col_;
}

void ScreenBuffer::restore_cursor() {
    set_cursor(saved_cursor_row_, saved_cursor_col_);
}

const Cell& ScreenBuffer::cell_at(int row, int col) const {
    return grid()[static_cast<std::size_t>(row)][static_cast<std::size_t>(col)];
}

uint32_t ScreenBuffer::register_hyperlink(const std::string& uri, const std::string& /*id*/) {
    if (uri.empty()) return 0;
    for (std::size_t i = 1; i < hyperlink_table_.size(); ++i) {
        if (hyperlink_table_[i] == uri) return static_cast<uint32_t>(i);
    }
    hyperlink_table_.push_back(uri);
    return static_cast<uint32_t>(hyperlink_table_.size() - 1);
}

std::string ScreenBuffer::get_hyperlink(uint32_t id) const {
    if (id == 0 || id >= hyperlink_table_.size()) return "";
    return hyperlink_table_[id];
}

std::string ScreenBuffer::dump_row_text(int row) const {
    std::string out;
    const auto& g = grid();
    const auto& r = g[static_cast<std::size_t>(row)];
    for (const auto& cell : r) {
        if (cell.width == 0) continue; // continuation cell of a wide glyph
        char32_t cp = cell.codepoint;
        if (cp == 0) cp = U' ';
        if (cp < 0x80) {
            out.push_back(static_cast<char>(cp));
        } else {
            // Minimal UTF-8 encode for dump/debug purposes.
            if (cp < 0x800) {
                out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
                out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
            } else if (cp < 0x10000) {
                out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
                out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
                out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
            } else {
                out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
                out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
                out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
                out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
            }
        }
    }
    while (!out.empty() && out.back() == ' ') out.pop_back();
    return out;
}

std::string ScreenBuffer::dump_text() const {
    std::string out;
    for (int r = 0; r < rows_; ++r) {
        out += dump_row_text(r);
        if (r + 1 < rows_) out.push_back('\n');
    }
    return out;
}


// ---- Scrollback / viewport / search ----------------------------------------

namespace {

const Cell& blank_cell() {
    static const Cell blank{};
    return blank;
}

void append_utf8(std::string& out, char32_t cp) {
    if (cp < 0x80) {
        out.push_back(static_cast<char>(cp));
    } else if (cp < 0x800) {
        out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp < 0x10000) {
        out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
}

std::u32string decode_utf8(const std::string& s) {
    std::u32string out;
    std::size_t i = 0;
    while (i < s.size()) {
        auto b = static_cast<unsigned char>(s[i]);
        char32_t cp = 0;
        int extra = 0;
        if (b < 0x80) { cp = b; }
        else if ((b & 0xE0) == 0xC0) { cp = b & 0x1F; extra = 1; }
        else if ((b & 0xF0) == 0xE0) { cp = b & 0x0F; extra = 2; }
        else if ((b & 0xF8) == 0xF0) { cp = b & 0x07; extra = 3; }
        else { ++i; continue; } // invalid lead byte: skip it
        ++i;
        for (int k = 0; k < extra && i < s.size(); ++k, ++i) {
            cp = (cp << 6) | (static_cast<unsigned char>(s[i]) & 0x3F);
        }
        out.push_back(cp);
    }
    return out;
}

char32_t fold_case(char32_t cp) {
    if (cp >= U'A' && cp <= U'Z') return cp + 32;
    if (cp >= 0xC0 && cp <= 0xDE && cp != 0xD7) return cp + 32; // Latin-1 uppercase
    return cp;
}

} // namespace

void ScreenBuffer::clear_scrollback() {
    scrollback_.clear();
    view_offset_ = 0;
}

void ScreenBuffer::set_scrollback_limit(std::size_t limit) {
    scrollback_limit_ = limit;
    while (scrollback_.size() > scrollback_limit_) scrollback_.pop_front();
    view_offset_ = std::min(view_offset_, static_cast<int>(scrollback_.size()));
}

void ScreenBuffer::scroll_view(int delta_lines) {
    if (alt_active_) { view_offset_ = 0; return; } // full-screen apps own the viewport
    int max_offset = static_cast<int>(scrollback_.size());
    view_offset_ = std::clamp(view_offset_ + delta_lines, 0, max_offset);
}

int ScreenBuffer::viewport_to_line(int row) const {
    return static_cast<int>(scrollback_.size()) - view_offset_ + row;
}

const std::vector<Cell>& ScreenBuffer::line_row(int line) const {
    int sb = static_cast<int>(scrollback_.size());
    if (line < sb) return scrollback_[static_cast<std::size_t>(line)];
    return grid()[static_cast<std::size_t>(line - sb)];
}

const Cell& ScreenBuffer::line_cell(int line, int col) const {
    if (line < 0 || line >= total_lines() || col < 0) return blank_cell();
    const auto& row = line_row(line);
    if (col >= static_cast<int>(row.size())) return blank_cell();
    return row[static_cast<std::size_t>(col)];
}

int ScreenBuffer::line_length(int line) const {
    if (line < 0 || line >= total_lines()) return 0;
    return static_cast<int>(line_row(line).size());
}

const Cell& ScreenBuffer::visible_cell(int row, int col) const {
    if (view_offset_ == 0 || alt_active_) {
        if (row < 0 || row >= rows_ || col < 0 || col >= cols_) return blank_cell();
        return cell_at(row, col);
    }
    return line_cell(viewport_to_line(row), col);
}

std::string ScreenBuffer::line_text(int line) const {
    return text_range(line, 0, line, line_length(line) - 1);
}

std::string ScreenBuffer::text_range(int line0, int col0, int line1, int col1) const {
    if (line0 > line1 || (line0 == line1 && col0 > col1)) {
        std::swap(line0, line1);
        std::swap(col0, col1);
    }
    line0 = std::max(0, line0);
    line1 = std::min(total_lines() - 1, line1);
    std::string out;
    for (int line = line0; line <= line1; ++line) {
        int len = line_length(line);
        int c0 = (line == line0) ? std::max(0, col0) : 0;
        int c1 = (line == line1) ? std::min(len - 1, col1) : len - 1;
        std::string row_text;
        for (int c = c0; c <= c1; ++c) {
            const Cell& cell = line_cell(line, c);
            if (cell.width == 0) continue; // continuation half of a wide glyph
            append_utf8(row_text, cell.codepoint == 0 ? U' ' : cell.codepoint);
        }
        while (!row_text.empty() && row_text.back() == ' ') row_text.pop_back();
        out += row_text;
        if (line < line1) out.push_back('\n');
    }
    return out;
}

std::vector<SearchMatch> ScreenBuffer::find_all(const std::string& utf8_needle, bool case_sensitive) const {
    std::vector<SearchMatch> matches;
    std::u32string needle = decode_utf8(utf8_needle);
    if (needle.empty()) return matches;
    if (!case_sensitive) for (auto& cp : needle) cp = fold_case(cp);

    std::u32string hay;
    std::vector<int> col_of;
    for (int line = 0; line < total_lines(); ++line) {
        hay.clear();
        col_of.clear();
        int len = line_length(line);
        for (int c = 0; c < len; ++c) {
            const Cell& cell = line_cell(line, c);
            if (cell.width == 0) continue;
            char32_t cp = cell.codepoint == 0 ? U' ' : cell.codepoint;
            hay.push_back(case_sensitive ? cp : fold_case(cp));
            col_of.push_back(c);
        }
        std::size_t pos = hay.find(needle);
        while (pos != std::u32string::npos) {
            int start_col = col_of[pos];
            std::size_t last = pos + needle.size() - 1;
            const Cell& last_cell = line_cell(line, col_of[last]);
            int end_col = col_of[last] + (last_cell.width == 2 ? 1 : 0);
            matches.push_back(SearchMatch{line, start_col, end_col - start_col + 1});
            pos = hay.find(needle, pos + needle.size());
        }
    }
    return matches;
}

} // namespace zephyr::vt
