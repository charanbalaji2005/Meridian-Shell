// src/app/shell_main.cpp
//
// Standalone entry point for Meridian Shell. Runs as an ordinary
// foreground process reading stdin / writing stdout — exactly what
// PtyManager would spawn as `opts.program` if a user picked Meridian
// Shell as their shell (see src/app/demo_main.cpp for that spawn path).
// The actual REPL/prompt/job-control-enable logic lives in
// src/shell/shell.{hpp,cpp} — this file is just argv handling.

#include "../shell/shell.hpp"

#include <iostream>
#include <string>
#include <unistd.h>

int main(int argc, char** argv) {
    // `meridian-shell -c "cmd"` runs one command and exits — the same
    // convention /bin/bash follows, and what lets other programs (and
    // this project's own $(...) command substitution) invoke it.
    if (argc >= 3 && std::string(argv[1]) == "-c") {
        meridian::shell::Shell shell(/*interactive=*/false);
        return shell.run_command(argv[2], std::cerr);
    }

    bool interactive = isatty(STDIN_FILENO);
    meridian::shell::Shell shell(interactive);
    if (interactive) shell.enable_job_control(); // real, once, in this real process only
    return shell.run_interactive(std::cin, std::cout, std::cerr);
}
