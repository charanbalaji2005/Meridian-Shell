#pragma once
// meridian-shell / line_editor.hpp
//
// Interactive line editor featuring graphical command preview navigation,
// live powerline prompt formatting, and terminal-native pic upload (Ctrl+Shift+P).

#include <iostream>
#include <string>
#include <vector>

namespace meridian::shell {

class LineEditor {
public:
    static bool is_terminal_interactive();

    // Builds the Date/Time status badge line (printed once above prompt)
    static std::string build_date_badge(const std::string& cwd);

    // Builds the single-line powerline prompt (without newlines for smooth in-place line editing)
    static std::string build_powerline_prompt(const std::string& cwd);

    // Reads an interactive line from stdin with graphical command preview popup
    // on Up/Down arrow keys. Falls back to std::getline if not a TTY.
    static std::string read_line(
        std::istream& in,
        std::ostream& out,
        const std::string& prompt,
        const std::vector<std::string>& history
    );

private:
    static void draw_history_preview(
        std::ostream& out,
        const std::vector<std::string>& history,
        int selected_idx,
        int visible_count
    );

    static void clear_history_preview(std::ostream& out, int lines_drawn);
};

} // namespace meridian::shell
