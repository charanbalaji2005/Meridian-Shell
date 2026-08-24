#pragma once
// meridian-terminal-core / vt / screen_buffer.hpp
//
// Owns the actual grid of cells, the cursor, the scrollback, and the
// primary/alternate screen split. The ANSI parser drives this through a
// small, deliberate API — it never reaches into the grid directly.

#include "types.hpp"
#include <deque>
#include <string>
#include <vector>

namespace meridian::vt {

class ScreenBuffer {
public:
    explicit ScreenBuffer(int rows = 24, int cols = 80, std::size_t scrollback_limit = 10000);

    void resize(int rows, int cols);
    int rows() const { return rows_; }
    int cols() const { return cols_; }

    int cursor_row() const { return cursor_row_; }
    int cursor_col() const { return cursor_col_; }
    void set_cursor(int row, int col);
    void move_cursor_rel(int drow, int dcol);

    // Writing (advances cursor; wraps and scrolls as needed)
    void put_codepoint(char32_t cp, const Attributes& attrs);
    void newline();          // move to next line, scrolling within region if needed
    void carriage_return();  // col -> 0
    void backspace();
    void tab();

    // Erasing. mode: 0 = cursor->end, 1 = start->cursor, 2 = all
    void erase_in_line(int mode);
    void erase_in_display(int mode);
    void insert_lines(int n);
    void delete_lines(int n);
    void delete_chars(int n);
    void insert_chars(int n);

    // Alternate screen buffer (used by full-screen apps: vim, htop, less...)
    void enter_alt_screen();
    void exit_alt_screen();
    bool in_alt_screen() const { return alt_active_; }

    // Scroll region, 1-based inclusive as in DECSTBM; (0,0) = full screen
    void set_scroll_region(int top_1based, int bottom_1based);
    void scroll_up(int n);
    void scroll_down(int n);

    // Cursor save/restore (DECSC/DECRSC, ESC 7 / ESC 8)
    void save_cursor();
    void restore_cursor();

    const Cell& cell_at(int row, int col) const;
    const std::deque<std::vector<Cell>>& scrollback() const { return scrollback_; }

    // Plain-text dump of the visible screen, one line per row, trailing
    // spaces trimmed. Used by tests and the headless demo — not part of
    // the "real" rendering path (a GPU renderer would read cells directly).
    std::string dump_text() const;
    std::string dump_row_text(int row) const;

private:
    std::vector<std::vector<Cell>>& grid();
    const std::vector<std::vector<Cell>>& grid() const;
    std::vector<Cell>& row_at(int row);
    void clear_row(std::vector<Cell>& row, const Attributes& attrs);
    int effective_scroll_bottom() const;

    int rows_;
    int cols_;
    int cursor_row_ = 0;
    int cursor_col_ = 0;
    bool pending_wrap_ = false; // deferred autowrap flag
    std::size_t scrollback_limit_;
    int scroll_top_ = 0;    // 0-based
    int scroll_bottom_;     // 0-based inclusive

    std::vector<std::vector<Cell>> primary_;
    std::vector<std::vector<Cell>> alternate_;
    bool alt_active_ = false;
    std::deque<std::vector<Cell>> scrollback_;

    int saved_cursor_row_ = 0;
    int saved_cursor_col_ = 0;
};

} // namespace meridian::vt
