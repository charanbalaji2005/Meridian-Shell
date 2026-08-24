#pragma once
// meridian-shell / line_editor.hpp
//
// Interactive line editor featuring graphical command preview navigation.
// When Up Arrow is pressed, displays an interactive graphical box with
// previous commands, durations, and metadata. Down Arrow steps forward.

#include <iostream>
#include <string>
#include <vector>

namespace meridian::shell {

class LineEditor {
public:
    static bool is_terminal_interactive();

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

