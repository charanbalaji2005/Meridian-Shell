#pragma once
// zephyr-dev / shell_history.hpp
//
// Read-only parsers for the history files of common shells, used by the
// GUI history panel. Entries are only ever *displayed* and, on explicit
// user action, typed at the prompt — never executed by Zephyr.

#include <cstddef>
#include <string>
#include <vector>

namespace zephyr::dev {

// Each parser returns commands oldest-first.
std::vector<std::string> parse_bash_history(const std::string& content);
std::vector<std::string> parse_zsh_history(const std::string& content);  // plain + EXTENDED_HISTORY, metafied bytes
std::vector<std::string> parse_fish_history(const std::string& content); // "- cmd: ..." entries

// Most-recent-first list without duplicates (keeps the newest occurrence).
std::vector<std::string> dedupe_recent_first(const std::vector<std::string>& oldest_first, std::size_t limit);

// POSIX single-quote quoting: safe to paste into any sh-compatible shell.
std::string shell_quote(const std::string& s);

} // namespace zephyr::dev
