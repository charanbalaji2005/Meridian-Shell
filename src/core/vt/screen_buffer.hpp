#pragma once
// zephyr-terminal-core / vt / screen_buffer.hpp
//
// Owns the actual grid of cells, the cursor, the scrollback, and the
// primary/alternate screen split. The ANSI parser drives this through a
// small, deliberate API — it never reaches into the grid directly.

#include "types.hpp"
#include "graphics.hpp"
#include <deque>
#include <string>
#include <vector>

namespace zephyr::vt {

// Cursor shape requested by the application via DECSCUSR (CSI Ps SP q).
// `Default` means "use the user's configured style".
enum class AppCursorShape : uint8_t { Default, Block, Underline, Bar };

// Private terminal modes toggled by DECSET/DECRST that a frontend needs to
// honour when encoding input or drawing the cursor.
struct TerminalModes {
    bool cursor_visible = true;          // DECTCEM  (?25)
    bool application_cursor_keys = false; // DECCKM   (?1)
    bool bracketed_paste = false;         // ?2004
    bool cursor_blink = true;             // ?12 / DECSCUSR odd values
    AppCursorShape cursor_shape = AppCursorShape::Default;
};

// One search hit, addressed in absolute line coordinates
// (0 = oldest scrollback line, scrollback().size() = first visible row).
struct SearchMatch {
    int line = 0;
    int col = 0;
    int length = 0;
    bool operator==(const SearchMatch& o) const {
        return line == o.line && col == o.col && length == o.length;
    }
};

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
    void reverse_index();    // ESC M: cursor up, scrolling down at the top margin

    // Erasing. mode: 0 = cursor->end, 1 = start->cursor, 2 = all
    void erase_in_line(int mode);
    void erase_in_display(int mode);
    void insert_lines(int n);
    void delete_lines(int n);
    void delete_chars(int n);
    void insert_chars(int n);
    void erase_chars(int n); // ECH: blank n cells from the cursor without shifting

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

    // ---- Scrollback management -------------------------------------------
    void clear_scrollback();
    void set_scrollback_limit(std::size_t limit);
    std::size_t scrollback_limit() const { return scrollback_limit_; }

    // ---- Viewport (what a frontend draws) ---------------------------------
    // view_offset() is how many lines the user has scrolled back into
    // history: 0 = live screen. While scrolled back, the viewport stays
    // anchored to the same content as new output pushes lines into history.
    int view_offset() const { return view_offset_; }
    void scroll_view(int delta_lines); // positive = further back in history
    void reset_view() { view_offset_ = 0; }
    // Cell at a viewport position, honouring view_offset(). Columns beyond a
    // (narrower, pre-resize) scrollback row return a blank cell.
    const Cell& visible_cell(int row, int col) const;
    // Absolute line index of viewport row `row`.
    int viewport_to_line(int row) const;

    // ---- Absolute line access (scrollback + live grid) --------------------
    int total_lines() const { return static_cast<int>(scrollback_.size()) + rows_; }
    const Cell& line_cell(int line, int col) const;
    int line_length(int line) const;
    std::string line_text(int line) const;            // UTF-8, trailing spaces trimmed
    std::string text_range(int line0, int col0, int line1, int col1) const;

    // Case-insensitive matching folds ASCII and Latin-1 letters only.
    std::vector<SearchMatch> find_all(const std::string& utf8_needle, bool case_sensitive) const;

    TerminalModes& modes() { return modes_; }
    const TerminalModes& modes() const { return modes_; }

    GraphicsEngine& graphics() { return graphics_; }
    const GraphicsEngine& graphics() const { return graphics_; }

    // OSC 8 Hyperlinks
    uint32_t register_hyperlink(const std::string& uri, const std::string& id = "");
    std::string get_hyperlink(uint32_t id) const;

    // OSC 7 Working Directory tracking
    void set_working_directory(const std::string& dir) { working_directory_ = dir; }
    const std::string& working_directory() const { return working_directory_; }

    // OSC 52 Clipboard
    void set_clipboard(const std::string& content) { clipboard_ = content; }
    const std::string& clipboard() const { return clipboard_; }

    // OSC 133 Shell Integration / Semantic Prompts
    enum class SemanticPromptState { None, Prompt, CommandInput, CommandOutput, CommandFinished };
    void set_semantic_state(SemanticPromptState state, int exit_code = 0) {
        semantic_state_ = state;
        if (state == SemanticPromptState::CommandFinished) {
            last_command_exit_code_ = exit_code;
        }
    }
    SemanticPromptState semantic_state() const { return semantic_state_; }
    int last_command_exit_code() const { return last_command_exit_code_; }

    // Plain-text dump of the visible screen, one line per row, trailing
    // spaces trimmed. Used by tests and the headless demo — not part of
    // the "real" rendering path (a GPU renderer would read cells directly).
    std::string dump_text() const;
    std::string dump_row_text(int row) const;

private:
    std::vector<std::vector<Cell>>& grid();
    const std::vector<std::vector<Cell>>& grid() const;
    std::vector<Cell>& row_at(int row);
    const std::vector<Cell>& line_row(int line) const;
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
    int alt_saved_cursor_row_ = 0;
    int alt_saved_cursor_col_ = 0;
    int view_offset_ = 0;
    TerminalModes modes_;

    GraphicsEngine graphics_;

    std::vector<std::string> hyperlink_table_{""}; // 0 = no hyperlink
    std::string working_directory_;
    std::string clipboard_;
    SemanticPromptState semantic_state_ = SemanticPromptState::None;
    int last_command_exit_code_ = 0;
};

} // namespace zephyr::vt
