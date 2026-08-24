#include "shell.hpp"
#include "line_editor.hpp"
#include <unistd.h>

namespace meridian::shell {

Shell::Shell(bool interactive) : interactive_(interactive) {}

std::string Shell::prompt() const {
    char cwd[4096];
    if (getcwd(cwd, sizeof(cwd))) return "meridian:" + std::string(cwd) + "$ ";
    return "meridian$ ";
}

int Shell::run_command(const std::string& command, std::ostream& err) {
    std::string error;
    int status = executor_.run_line(command, &error);
    if (!error.empty()) { err << "meridian-shell: " << error << "\n"; return 1; }
    return status < 0 ? 1 : status;
}

int Shell::run_interactive(std::istream& in, std::ostream& out, std::ostream& err) {
    while (true) {
        std::string line;
        if (&in == &std::cin && LineEditor::is_terminal_interactive()) {
            line = LineEditor::read_line(in, out, prompt(), executor_.history());
            if (line == "exit" && executor_.history().empty()) {
                return 0;
            }
        } else {
            if (interactive_) {
                out << prompt();
                out.flush();
            }
            if (!std::getline(in, line)) break;
        }

        std::string error;
        int status = executor_.run_line(line, &error);
        if (!error.empty()) err << "meridian-shell: " << error << "\n";
        else if (status >= 0) executor_.push_history(line);

        if (executor_.exit_requested()) return executor_.exit_code();
    }
    return 0;
}

} // namespace meridian::shell
